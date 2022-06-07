/*******************************************************************************
 * Copyright 2019-2022 MINRES Technologies GmbH
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

#define SC_INCLUDE_DYNAMIC_PROCESSES
#include "initiator.h"
#include <ahb/ahb_tlm.h>
#include <scc/utilities.h>

using namespace ahb::pin;
using namespace sc_core;

template <unsigned WIDTH>
initiator<WIDTH>::initiator(const sc_module_name& nm)
: sc_module(nm) {
    SC_HAS_PROCESS(initiator);
    sc_core::sc_get_curr_simcontext()->create_thread_process(
        "bus_task_0", false, static_cast<sc_core::SC_ENTRY_FUNC>(&initiator::bus_task), this, nullptr);
    sc_core::sc_get_curr_simcontext()->create_thread_process(
        "bus_task_1", false, static_cast<sc_core::SC_ENTRY_FUNC>(&initiator::bus_task), this, nullptr);
    sc_core::sc_get_curr_simcontext()->create_thread_process(
        "bus_task_2", false, static_cast<sc_core::SC_ENTRY_FUNC>(&initiator::bus_task), this, nullptr);
    tsckt.register_nb_transport_fw([this](tlm::tlm_generic_payload& payload, tlm::tlm_phase& phase,
                                          sc_core::sc_time& delay) -> tlm::tlm_sync_enum {
        if(phase == tlm::BEGIN_REQ) {
            if(payload.has_mm())
                payload.acquire();
            auto* ext = payload.get_extension<ahb_extension>();
            if(!ext) {
                ext = new ahb_extension();
                payload.set_extension(ext);
            }
            this->inqueue.notify(payload);
        }
        return tlm::TLM_ACCEPTED;
    });
}

template <unsigned WIDTH> inline void initiator<WIDTH>::bus_task() {
    data_t data{0};
    while(true) {
        wait(inqueue.get_event());
        while(auto* trans = inqueue.get_next_transaction()) {
            addr_phase.lock();
            auto* ext = trans->template get_extension<ahb_extension>();
            HADDR_o.write(trans->get_address());
            HWRITE_o.write(trans->is_write());
            HMASTLOCK_o.write(ext->is_locked());
            HPROT_o.write(ext->get_protection());
            HBURST_o.write(static_cast<unsigned>(ext->get_burst()));
            auto log_bytes = scc::ilog2(trans->get_data_length());
            auto log_width = scc::ilog2(WIDTH / 8);
            size_t size = 0;
            for(; size < log_width; ++size)
                if(trans->get_address() & (1 << size))
                    break; // i contains the first bit not being 0
            HSIZE_o.write(size);
            HTRANS_o.write(static_cast<unsigned>(trans_e::NONSEQ));
            wait(HCLK_i.posedge_event());
            data_phase.lock();
            tlm::tlm_phase phase{tlm::END_REQ};
            sc_time delay;
            tsckt->nb_transport_bw(*trans, phase, delay);
            addr_phase.unlock();
            auto beats = 1 << (log_bytes - size);
            auto offset = 0U;
            for(size_t i = 0; i < beats; ++i) {
                if(beats == 1)
                    HTRANS_o.write(static_cast<unsigned>(trans_e::IDLE));
                else if(i > 0)
                    HTRANS_o.write(static_cast<unsigned>(trans_e::SEQ));
                if(trans->is_write()) {
                    for(size_t j = 0, k = 0; k < WIDTH / 8; j += 8, ++k, ++offset)
                        data.range(j + 7, j) = *(uint8_t*)(trans->get_data_ptr() + offset);
                    HWDATA_o.write(data);
                }
                wait(HCLK_i.posedge_event());
                while(!HREADY_i.read())
                    wait(HCLK_i.posedge());
                if(trans->is_read()) {
                    data = HRDATA_i.read();
                    for(size_t j = 0, k = 0; k < WIDTH / 8; j += 8, ++k, ++offset)
                        *(uint8_t*)(trans->get_data_ptr() + offset) = data.range(j + 7, j).to_uint();
                }
            }
            phase = tlm::BEGIN_RESP;
            delay = SC_ZERO_TIME;
            tsckt->nb_transport_bw(*trans, phase, delay);
            if(trans->has_mm())
                trans->release();
            HTRANS_o.write(static_cast<unsigned>(trans_e::IDLE));
            data_phase.unlock();
        }
    }
}

template class ahb::pin::initiator<32>;
template class ahb::pin::initiator<64>;
template class ahb::pin::initiator<128>;
template class ahb::pin::initiator<256>;
template class ahb::pin::initiator<512>;
template class ahb::pin::initiator<1024>;
