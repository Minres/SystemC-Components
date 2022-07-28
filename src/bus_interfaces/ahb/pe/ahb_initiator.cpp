/*******************************************************************************
 * Copyright 2020-2022 MINRES Technologies GmbH
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

#include "ahb_initiator.h"
#include <atp/timing_params.h>
#include <scc/report.h>

using namespace sc_core;
using namespace ahb;
using namespace ahb::pe;

namespace {
uint8_t log2n(uint8_t siz) { return ((siz > 1) ? 1 + log2n(siz >> 1) : 0); }

} // anonymous namespace

ahb_initiator_b::ahb_initiator_b(sc_core::sc_module_name nm,
                                 sc_core::sc_port_b<tlm::tlm_fw_transport_if<tlm::tlm_base_protocol_types>>& port,
                                 size_t transfer_width, bool coherent)
: sc_module(nm)
, socket_fw(port)
, transfer_width_in_bytes(transfer_width / 8)
, coherent(coherent) {
    add_attribute(artv);
    add_attribute(awtv);
    add_attribute(wbv);
    add_attribute(rbr);
    add_attribute(br);
}

ahb_initiator_b::~ahb_initiator_b() {
    for(auto& e : tx_state_by_id)
        delete e.second;
}

tlm::tlm_sync_enum ahb_initiator_b::nb_transport_bw(payload_type& trans, phase_type& phase, sc_core::sc_time& t) {

    auto it = tx_state_by_id.find(&trans);
    sc_assert(it != tx_state_by_id.end());
    it->second->peq.notify(std::make_tuple(&trans, phase), t);
    return tlm::TLM_ACCEPTED;
}

void ahb_initiator_b::invalidate_direct_mem_ptr(sc_dt::uint64 start_range, sc_dt::uint64 end_range) {}

tlm::tlm_phase ahb_initiator_b::send(payload_type& trans, ahb_initiator_b::tx_state* txs, tlm::tlm_phase phase) {
    sc_core::sc_time delay;
    SCCTRACE(SCMOD) << "Send REQ";
    tlm::tlm_sync_enum ret = socket_fw->nb_transport_fw(trans, phase, delay);
    if(ret == tlm::TLM_UPDATED) {
        wait(delay);
        return phase;
    } else {
        auto entry = txs->peq.get();
        sc_assert(std::get<0>(entry) == &trans);
        return std::get<1>(entry);
    }
}

void ahb_initiator_b::transport(payload_type& trans, bool blocking) {
    SCCTRACE(SCMOD) << "got transport req for id=" << &trans;
    if(blocking) {
        sc_time t;
        socket_fw->b_transport(trans, t);
    } else {
        auto it = tx_state_by_id.find(&trans);
        if(it == tx_state_by_id.end()) {
            bool success;
            std::tie(it, success) = tx_state_by_id.insert(std::make_pair(&trans, new tx_state()));
        }
        auto& txs = it->second;
        auto timing_e = trans.set_extension<atp::timing_params>(nullptr);

        txs->active_tx = &trans;
        SCCTRACE(SCMOD) << "start transport req for id=" << &trans;

        auto* ext = trans.get_extension<ahb::ahb_extension>();
        /// Timing
        auto delay_in_cycles =
            trans.is_read() ? (timing_e ? timing_e->artv : artv.value) : (timing_e ? timing_e->awtv : awtv.value);
        if(delay_in_cycles)
            delay_in_cycles--; // one cycle implicitly executed
        for(unsigned i = 0; i < delay_in_cycles; ++i)
            wait(clk_i.posedge_event());
        auto burst_length = 0U;
        switch(ext->get_burst()) {
        case ahb::burst_e::SINGLE:
            burst_length = 1;
            break;
        case ahb::burst_e::INCR:
            burst_length = 1;
            break;
        case ahb::burst_e::WRAP4:
        case ahb::burst_e::INCR4:
            burst_length = 4;
            break;
        case ahb::burst_e::WRAP8:
        case ahb::burst_e::INCR8:
            burst_length = 8;
            break;
        case ahb::burst_e::WRAP16:
        case ahb::burst_e::INCR16:
            burst_length = 16;
            break;
        }
        tlm::tlm_phase next_phase{tlm::UNINITIALIZED_PHASE};
        addr_chnl.wait();
        SCCTRACE(SCMOD) << "starting read address phase of tx with id=" << &trans;
        auto res = send(trans, txs, tlm::BEGIN_REQ);
        if(res == ahb::BEGIN_PARTIAL_RESP || res == tlm::BEGIN_RESP)
            next_phase = res;
        else if(res != tlm::END_REQ)
            SCCERR(SCMOD) << "target did not repsond with END_REQ to a BEGIN_REQ";
        wait(clk_i.posedge_event());
        auto finished = false;
        const auto exp_burst_length = burst_length;
        data_chnl.wait();
        addr_chnl.post();
        do {
            // waiting for response
            auto entry = next_phase == tlm::UNINITIALIZED_PHASE ? txs->peq.get() : std::make_tuple(&trans, next_phase);
            next_phase = tlm::UNINITIALIZED_PHASE;
            // Handle optional CRESP response
            if(std::get<0>(entry) == &trans && std::get<1>(entry) == tlm::BEGIN_RESP) {
                SCCTRACE(SCMOD) << "received last beat of tx with id=" << &trans;
                auto delay_in_cycles = timing_e ? (trans.is_read() ? timing_e->rbr : timing_e->br) : br.value;
                for(unsigned i = 0; i < delay_in_cycles; ++i)
                    wait(clk_i.posedge_event());
                trans.set_response_status(tlm::TLM_OK_RESPONSE);
                burst_length--;
                tlm::tlm_phase phase = tlm::END_RESP;
                sc_time delay = clk_if ? clk_if->period() - 1_ps : SC_ZERO_TIME;
                socket_fw->nb_transport_fw(trans, phase, delay);
                if(burst_length)
                    SCCWARN(SCMOD) << "got wrong number of burst beats, expected " << exp_burst_length << ", got "
                                   << exp_burst_length - burst_length;
                wait(clk_i.posedge_event());
                finished = true;
            } else if(std::get<0>(entry) == &trans &&
                      std::get<1>(entry) == ahb::BEGIN_PARTIAL_RESP) { // RDAT without CRESP case
                SCCTRACE(SCMOD) << "received beat of tx with id=" << &trans;
                auto delay_in_cycles = timing_e ? timing_e->rbr : rbr.value;
                for(unsigned i = 0; i < delay_in_cycles; ++i)
                    wait(clk_i.posedge_event());
                burst_length--;
                tlm::tlm_phase phase = ahb::END_PARTIAL_RESP;
                sc_time delay = clk_if ? clk_if->period() - 1_ps : SC_ZERO_TIME;
                auto res = socket_fw->nb_transport_fw(trans, phase, delay);
                if(res == tlm::TLM_UPDATED) {
                    next_phase = phase;
                    wait(delay);
                }
            }
        } while(!finished);
        data_chnl.post();
        SCCTRACE(SCMOD) << "finished non-blocking protocol";
        txs->active_tx = nullptr;
        any_tx_finished.notify(SC_ZERO_TIME);
    }
    SCCTRACE(SCMOD) << "finished transport req for id=" << &trans;
}
