/*******************************************************************************
 * Copyright 2026 MINRES Technologies GmbH
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *******************************************************************************/

#ifndef _SYSC_NB_ROUTER_NB_DECODER_H_
#define _SYSC_NB_ROUTER_NB_DECODER_H_

#include "types.h"
#include <scc/peq.h>
#include <scc/report.h>
#include <scc/router_types.h>
#include <sysc/kernel/sc_simcontext.h>
#include <sysc/kernel/sc_wait.h>
#include <tlm/scc/tlm_gp_shared.h>
#include <tlm>
#include <tlm_core/tlm_2/tlm_2_interfaces/tlm_fw_bw_ifs.h>
#include <tlm_core/tlm_2/tlm_generic_payload/tlm_phase.h>
#include <util/range_lut.h>

namespace scc {
namespace at_router {
template <typename TYPES = tlm::tlm_base_protocol_types>
struct nb_decoder : public tlm::tlm_fw_nonblocking_transport_if<typename TYPES::tlm_payload_type, typename TYPES::tlm_phase_type>,
                    public tlm::tlm_bw_nonblocking_transport_if<typename TYPES::tlm_payload_type, typename TYPES::tlm_phase_type> {
    using this_class = nb_decoder<TYPES>;
    using tlm_generic_payload = typename TYPES::tlm_payload_type;
    using tlm_phase = typename TYPES::tlm_phase_type;

    sc_core::sc_signal_in_if<sc_core::sc_time>* clk_if;

    t_port<TYPES> tport;

    sc_core::sc_vector<i_port<TYPES>> iport;

    nb_decoder(char const* name, unsigned bus_width, unsigned egress_cnt, util::range_lut<unsigned> const& decoder,
               std::vector<range_entry> const& tranges)
    : tport((std::string(name) + "_tport").c_str())
    , iport((std::string(name) + "_iport").c_str(), egress_cnt)
    , buswidth(bus_width)
    , decoder(decoder)
    , tranges(tranges)
    , name(name) {
        sc_core::sc_spawn_options opts;
        opts.spawn_method();
        sc_core::sc_spawn([this]() { bw(); }, nullptr, &opts);
        tport.fw.bind(*this);
        tport.set_width = [this](unsigned width) { this->buswidth = width; };
        for(auto& p : iport)
            p.bw.bind(*this);
    }

    sc_core::sc_time const& get_clk_period() {
        return tport.clk.get_interface() ? tport.clk->read() : (clk_if ? clk_if->read() : sc_core::SC_ZERO_TIME);
    }

    bool adjust_to_clock_edge(sc_core::sc_time& t) {
        auto clk_period = get_clk_period();
        if(clk_period.value()) {
            auto offset = t % clk_period;
            if(offset.value()) {
                t += clk_period - offset; // align to next clock edge
                return true;
            }
        }
        return false;
    }

    tlm::tlm_sync_enum nb_transport_fw(tlm_generic_payload& trans, tlm_phase& phase, sc_core::sc_time& t) override {
        if(phase == tlm::BEGIN_REQ) {
            auto addr = trans.get_address();
            auto idx = decoder.getEntry(addr);
            if(idx == decoder.null_entry) {
                if(default_idx == std::numeric_limits<size_t>::max()) {
                    trans.set_response_status(tlm::TLM_ADDRESS_ERROR_RESPONSE);
                    if(phase == tlm::BEGIN_REQ)
                        phase = tlm::END_RESP;
                    return tlm::TLM_COMPLETED;
                }
                idx = default_idx;
            } else if(tranges[idx].remap) {
                trans.set_address(trans.get_address() - (tranges[idx].remap ? tranges[idx].base : 0));
            }
            adjust_to_clock_edge(t);
            auto res = iport[idx].fw->nb_transport_fw(trans, phase, t);
            if(res == tlm::TLM_UPDATED && phase == tlm::BEGIN_RESP) {
                adjust_to_clock_edge(t);
                que.notify(tlm::scc::tlm_gp_shared_ptr(&trans), t);
            }
            return res;
        }
        if(phase == tlm::END_RESP) {
            adjust_to_clock_edge(t);
            retrigger.notify(t);
            return tlm::TLM_COMPLETED;
        }
        return tlm::TLM_ACCEPTED;
    }

    tlm::tlm_sync_enum nb_transport_bw(tlm_generic_payload& trans, tlm_phase& phase, sc_core::sc_time& t) override {
        if(phase == tlm::BEGIN_RESP) {
            SCCTRACE(name) << "Received BW trans 0x" << std::hex << &trans;
            que.notify(tlm::scc::tlm_gp_shared_ptr(&trans), t);
            phase = tlm::END_RESP;
            // support SCC::LT template paramter for buswidth (otherwise divide by zero)
            const unsigned width = buswidth ? buswidth : 64u;
            auto cycles = trans.is_read() ? (trans.get_data_length() * 8 + width - 1) / width : 1;
            auto clk_period = get_clk_period();
            t += (cycles * get_clk_period()) - 1_ps;
            return tlm::TLM_UPDATED;
        } else if(phase == tlm::END_REQ) {
            return tlm::TLM_ACCEPTED;
        } else {
            SCCFATAL("nb_arbitter") << __FUNCTION__ << ": illegal phase received: " << phase.get_name();
        }
        return tlm::TLM_COMPLETED;
    }

    void bw() {
        if(que.has_next()) {
            auto trans = que.get();
            SCCTRACE(name) << "Executing BW trans 0x" << std::hex << trans.get();
            tlm_phase phase = tlm::BEGIN_RESP;
            sc_core::sc_time t;
            auto res = tport.bw->nb_transport_bw(*trans, phase, t);
            if(res == tlm::TLM_UPDATED && phase == tlm::END_RESP || res == tlm::TLM_COMPLETED) {
                adjust_to_clock_edge(t);
                sc_core::next_trigger(t);
                SCCTRACE(name) << "next_trigger @+" << t;
                return;
            }
            SCCTRACE(name) << "next_trigger with retrigger";
            sc_core::next_trigger(retrigger);
            return;
        }
        SCCTRACE(name) << "next_trigger with que trigger";
        sc_core::next_trigger(que.event());
    }

    void set_default_target(size_t idx) { default_idx = idx; }

private:
    unsigned buswidth;
    i_port<TYPES>& add_isckt() {
        auto* sckt = new i_port<TYPES>("iport" + iport.size());
        iport.push_back(sckt);
        sckt->bw.bind(*this);
        return *sckt;
    }

    util::range_lut<unsigned> const& decoder;
    std::vector<range_entry> const& tranges;

    size_t default_idx = std::numeric_limits<size_t>::max();
    using que_entry = tlm::scc::tlm_gp_shared_ptr;
    scc::peq<que_entry> que;
    sc_core::sc_event retrigger;
    const std::string name;
};
} // namespace at_router
} // namespace scc
#endif
