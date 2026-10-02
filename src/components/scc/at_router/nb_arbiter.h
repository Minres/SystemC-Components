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
#include <algorithm>
#include <cstdint>
#include <limits>
#include <memory>
#include <sysc/kernel/sc_simcontext.h>
#include <sysc/kernel/sc_time.h>
#include <systemc>
#include <unordered_map>
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
struct nb_arbiter : public tlm::tlm_bw_nonblocking_transport_if<typename TYPES::tlm_payload_type, typename TYPES::tlm_phase_type> {
    using this_class = nb_arbiter<TYPES>;
    using tlm_generic_payload = typename TYPES::tlm_payload_type;
    using tlm_phase = typename TYPES::tlm_phase_type;

    sc_core::sc_signal_in_if<sc_core::sc_time>* clk_if;

    sc_core::sc_vector<t_port<TYPES>> tport;
    i_port<TYPES> iport;

    nb_arbiter(char const* name, unsigned buswidth, unsigned igress_cnt)
    : tport((std::string(name) + "_tport").c_str(), igress_cnt)
    , iport((std::string(name) + "_iport").c_str())
    , buswidth(buswidth)
    , name(name) {
        sc_core::sc_spawn([this]() { arbitrate(); }, nullptr, nullptr);
        iport.bw.bind(*this);
        for(auto idx = 0u; idx < tport.size(); ++idx) {
            actors.emplace_back(std::make_unique<fw_actor>(this, idx));
            tport[idx].fw.bind(*actors.back());
        }
    }

    sc_core::sc_time const& get_clk_period() {
        return iport.clk.get_interface() ? iport.clk->read() : (clk_if ? clk_if->read() : sc_core::SC_ZERO_TIME);
    }

    tlm::tlm_sync_enum nb_transport_bw(tlm_generic_payload& trans, tlm_phase& phase, sc_core::sc_time& t) override {
        if(!source_by_tx.count(&trans)) {
            SCCFATAL("nb_arbitter") << __FUNCTION__ << ": missing source_id extension in backward path";
            trans.set_response_status(tlm::TLM_GENERIC_ERROR_RESPONSE);
            return tlm::TLM_COMPLETED;
        }
        if(phase == tlm::BEGIN_RESP) {
            t += sc_core::sc_time_stamp() % get_clk_period();
            auto res = tport[source_by_tx[&trans]].bw->nb_transport_bw(trans, phase, t);
            if(res != tlm::TLM_ACCEPTED)
                return res;
            phase = tlm::END_RESP;
            // support SCC::LT template parameter for buswidth (otherwise divide by zero)
            const unsigned width = buswidth ? buswidth : 64u;
            auto cycles = trans.is_read() ? (trans.get_data_length() * 8 + width - 1) / width : 1;
            t = t + (cycles * get_clk_period()); // - 1_ps;
            source_by_tx.erase(&trans);
            return tlm::TLM_COMPLETED;
        }
        return tlm::TLM_ACCEPTED;
    }

    struct fw_actor : public tlm::tlm_fw_nonblocking_transport_if<typename TYPES::tlm_payload_type, typename TYPES::tlm_phase_type> {
        tlm::tlm_sync_enum nb_transport_fw(tlm_generic_payload& trans, tlm_phase& phase, sc_core::sc_time& t) override {
            if(phase == tlm::BEGIN_REQ) {
                que.notify(tlm::scc::tlm_gp_shared_ptr(&trans), t);
                phase = tlm::END_REQ;
                // support SCC::LT template paramter for buswidth (otherwise divide by zero)
                const unsigned width = owner->buswidth ? owner->buswidth : 64u;
                auto cycles = trans.is_write() ? (trans.get_data_length() * 8 + width - 1) / width : 1;
                t = t + (cycles * owner->get_clk_period()); // - 1_ps;
                return tlm::TLM_UPDATED;
            } else if(phase == tlm::END_RESP) {
                owner->source_by_tx.erase(&trans);
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
    const std::string name;
    sc_core::sc_event retrigger;

    void arbitrate() {
        sc_core::wait(sc_core::SC_ZERO_TIME);
        sc_core::sc_event_or_list evt;
        for(auto& a : actors)
            evt |= a->que.event();
        evt |= retrigger;
        while(true) {
            if(get_clk_period() == sc_core::SC_ZERO_TIME) {
                do
                    sc_core::wait(iport.clk.get_interface() ? iport.clk.value_changed_event() : clk_if->value_changed_event());
                while(get_clk_period() == sc_core::SC_ZERO_TIME);
            } else {
                sc_core::wait(evt);
                SCCTRACEALL(name) << "[" << __FUNCTION__ << "]:"
                                  << "got que_event, last_selected=" << last_selected;
                auto clk_period = get_clk_period();
                sc_core::sc_time t;
                for(size_t i = 0; i < actors.size(); ++i) {
                    last_selected = (last_selected + 1) % actors.size();
                    auto& a = actors[last_selected];
                    if(a->que.has_next()) {
                        SCCTRACEALL(name) << "[" << __FUNCTION__ << "]:"
                                          << "serving request from que " << last_selected;
                        auto trans = a->que.get();
                        source_by_tx[trans.get()] = last_selected;
                        tlm::tlm_phase phase = tlm::BEGIN_REQ;
                        auto status = iport.fw->nb_transport_fw(*trans, phase, t);
                        if(clk_period.value() && t.value() % clk_period.value()) {
                            auto cycles = static_cast<unsigned>(t / clk_period);
                            sc_core::wait((cycles + 1) * clk_period);
                        } else
                            sc_core::wait(t);
                        if(status == tlm::TLM_COMPLETED ||
                           (status == tlm::TLM_UPDATED && (phase == tlm::BEGIN_RESP || phase == tlm::END_RESP))) {
                            auto t_resp = clk_period.value() ? sc_core::sc_time_stamp() % clk_period : sc_core::SC_ZERO_TIME;
                            if(status == tlm::TLM_COMPLETED) {
                                phase = tlm::BEGIN_RESP;
                            }
                            tport[last_selected].bw->nb_transport_bw(*trans, phase, t_resp);
                        }
                        break;
                    }
                }
                // we need to retrigger in case there is another transaction waiting to be served
                auto pending_transaction =
                    std::any_of(actors.begin(), actors.end(), [](std::unique_ptr<fw_actor> const& a) { return a->que.has_next(); });
                if(pending_transaction) {
                    SCCTRACEALL(name) << "[" << __FUNCTION__ << "]:"
                                      << " retriggering arbitration";
                    retrigger.notify(sc_core::SC_ZERO_TIME);
                }
            }
        }
    }
    size_t last_selected = std::numeric_limits<size_t>::max();
    std::vector<std::unique_ptr<fw_actor>> actors;
    std::unordered_map<void*, unsigned> source_by_tx;
};

} // namespace at_router
} // namespace scc
#endif
