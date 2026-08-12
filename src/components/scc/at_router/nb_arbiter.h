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

#ifndef _SYSC_NB_ROUTER_NB_ARBITTER_H_
#define _SYSC_NB_ROUTER_NB_ARBITTER_H_
#include <limits>
#include <memory>
#include <systemc>
#ifndef SC_INCLUDE_DYNAMIC_PROCESSES
#define SC_INCLUDE_DYNAMIC_PROCESSES
#endif

#include "types.h"
#include <scc/peq.h>
#include <scc/report.h>
#include <stdexcept>
#include <systemc>
#include <tlm/scc/tlm_gp_shared.h>
#include <tlm>
#include <util/range_lut.h>

namespace scc {
namespace at_router {
template <typename TYPES = tlm::tlm_base_protocol_types>
struct nb_arbiter : public sc_core::sc_module,

                    public tlm::tlm_bw_nonblocking_transport_if<typename TYPES::tlm_payload_type, typename TYPES::tlm_phase_type> {
    struct source_id : public tlm::tlm_extension<source_id> {
        tlm::tlm_extension_base* clone() const override { throw std::runtime_error("clone() is not supported"); }
        void copy_from(tlm::tlm_extension_base const& ext) override { throw std::runtime_error("clone() is not supported"); }
        size_t src_idx = 0;
        source_id* last_source_id = nullptr;
        source_id(size_t idx, source_id* last)
        : src_idx(idx)
        , last_source_id(last) {}
        source_id() = default;
    };

    using this_class = nb_arbiter<TYPES>;
    using tlm_generic_payload = typename TYPES::tlm_payload_type;
    using tlm_phase = typename TYPES::tlm_phase_type;

    sc_core::sc_in<sc_core::sc_time> clk_i{"clk_i"};

    sc_core::sc_vector<t_port<TYPES>> tport{"tport"};
    i_port<TYPES> iport{"iport"};

    nb_arbiter(sc_core::sc_module_name nm, unsigned buswidth)
    : sc_module(nm)
    , buswidth(buswidth) {
        sc_core::sc_spawn([this]() { arbitrate(); }, nullptr, nullptr);
        iport.bw.bind(*this);
    }

    tlm::tlm_sync_enum nb_transport_bw(tlm_generic_payload& trans, tlm_phase& phase, sc_core::sc_time& t) override {
        auto* ext = trans.template get_extension<source_id>();
        if(!ext) {
            SCCFATAL("nb_arbitter") << __FUNCTION__ << ": missing source_id extension in backward path";
            trans.set_response_status(tlm::TLM_GENERIC_ERROR_RESPONSE);
            return tlm::TLM_COMPLETED;
        }
        trans.set_extension(ext->last_source_id);
        if(phase == tlm::BEGIN_RESP) {
            auto res = tport[ext->src_idx].bw->nb_transport_bw(trans, phase, t);
            if(res != tlm::TLM_ACCEPTED)
                return res;
            phase = tlm::END_RESP;
            auto cycles = trans.is_read() ? (trans.get_data_length() * 8 + buswidth - 1) / buswidth : 1;
            t = t + (cycles * clk_i.read()) - 1_ps;
            return tlm::TLM_COMPLETED;
        }
        return tlm::TLM_ACCEPTED;
    }

    void before_end_of_elaboration() override {
        for(auto idx = 0u; idx < tport.size(); ++idx) {
            actors.emplace_back(std::make_unique<fw_actor>(this, idx));
            tport[idx].fw.bind(*actors.back());
        }
    }

    struct fw_actor : public tlm::tlm_fw_nonblocking_transport_if<typename TYPES::tlm_payload_type, typename TYPES::tlm_phase_type> {
        tlm::tlm_sync_enum nb_transport_fw(tlm_generic_payload& trans, tlm_phase& phase, sc_core::sc_time& t) override {
            if(phase == tlm::BEGIN_REQ) {
                trans.set_extension(new source_id(idx, trans.template get_extension<source_id>()));
                que.notify(tlm::scc::tlm_gp_shared_ptr(&trans), t);
                phase = tlm::END_REQ;
                auto cycles = trans.is_write() ? (trans.get_data_length() * 8 + owner->buswidth - 1) / owner->buswidth : 1;
                t = t + (cycles * owner->clk_i.read()) - 1_ps;
                return owner->tport[idx].bw->nb_transport_bw(trans, phase, t);
            } else if(phase == tlm::END_RESP) {
                return tlm::TLM_COMPLETED;
            } else {
                SCCFATAL("nb_arbitter") << __FUNCTION__ << ": illegal phase received: " << phase.get_name();
            }
            return tlm::TLM_COMPLETED;
        }
        fw_actor(nb_arbiter<TYPES>* owner, unsigned idx)
        : owner(owner)
        , idx(idx) {}

        using que_entry = tlm::scc::tlm_gp_shared_ptr;
        scc::peq<que_entry> que;
        nb_arbiter<TYPES>* owner;
        unsigned idx;
    };

private:
    const unsigned buswidth;
    t_port<TYPES>& add_isckt() {
        auto* sckt = new t_port<TYPES>("tport" + tport.size());
        auto idx = tport.size();
        tport.push_back(sckt);
        sckt->fw.bind(*this);
        return *sckt;
    }

    void arbitrate() {
        wait(sc_core::SC_ZERO_TIME);
        sc_core::sc_event_or_list evt;
        for(auto& a : actors)
            evt |= a->que.event();
        while(true) {
            if(clk_i.read() == sc_core::SC_ZERO_TIME) {
                do
                    wait(clk_i->value_changed_event());
                while(clk_i.read() == sc_core::SC_ZERO_TIME);
            } else {
                wait(evt);
                auto cycles = sc_core::sc_time_stamp() / clk_i.read();
                for(size_t i = 0; i < actors.size(); ++i) {
                    last_selected = (last_selected + 1) % actors.size();
                    auto& a = actors[last_selected];
                    if(a->que.has_next()) {
                        auto res = a->que.get();
                        tlm::tlm_phase phase = tlm::BEGIN_REQ;
                        sc_core::sc_time t;
                        tport[i].fw->nb_transport_fw(*res, phase, t);
                        break;
                    }
                }
            }
        }
    }
    size_t last_selected = std::numeric_limits<size_t>::max();
    std::vector<std::unique_ptr<fw_actor>> actors;
};

} // namespace at_router
} // namespace scc
#endif