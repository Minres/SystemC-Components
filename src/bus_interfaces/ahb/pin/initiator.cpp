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

#ifndef SC_INCLUDE_DYNAMIC_PROCESSES
#define SC_INCLUDE_DYNAMIC_PROCESSES
#endif
#include "initiator.h"
#include <ahb/ahb_tlm.h>
#include <scc/utilities.h>
#include <scc/report.h>

using namespace ahb::pin;
using namespace sc_core;

template <unsigned WIDTH>
initiator<WIDTH>::initiator(const sc_module_name& nm)
: sc_module(nm) {
    SC_HAS_PROCESS(initiator);
    SC_THREAD(bus_addr_task);
    SC_THREAD(bus_data_task);

    tsckt.register_nb_transport_fw(
        [this](tlm::tlm_generic_payload& payload, tlm::tlm_phase& phase, sc_core::sc_time& delay) -> tlm::tlm_sync_enum {
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

template <unsigned WIDTH> inline void initiator<WIDTH>::bus_addr_task() {
    auto& hready = HREADY_i.read();
    while(true) {
        wait(inqueue.get_event());
        while(auto trans = inqueue.get_next_transaction()) {
            sc_assert(trans->get_data_length()*8<= WIDTH && "Transaction length larger than bus width, this is not supported");
            SCCDEBUG(SCMOD)<<"Recv beg req for read to addr 0x"<<std::hex<<trans->get_address();
            auto bytes_exp = scc::ilog2(trans->get_data_length());
            auto width_exp = scc::ilog2(WIDTH / 8);
            size_t size = 0;
            for(; size < bytes_exp; ++size)
                if(trans->get_address() & (1 << size))
                    break; // i contains the first bit not being 0
            auto* ext = trans->template get_extension<ahb_extension>();
            HADDR_o.write(trans->get_address());
            HWRITE_o.write(trans->is_write());
            HMASTLOCK_o.write(ext->is_locked());
            HPROT_o.write(ext->get_protection());
            HBURST_o.write(static_cast<unsigned>(ext->get_burst()));
            HSIZE_o.write(size);
            HTRANS_o.write(static_cast<unsigned>(ext->is_seq()?trans_e::SEQ:trans_e::NONSEQ));
            do {
                wait(HCLK_i.posedge_event());
            } while(!hready);
            SCCDEBUG(SCMOD)<<"Send end req for read to addr 0x"<<std::hex<<trans->get_address();
            tlm::tlm_phase phase{tlm::END_REQ};
            sc_time delay;
            auto res = tsckt->nb_transport_bw(*trans, phase, delay);
            tx_in_flight.notify(*trans);
            HTRANS_o.write(static_cast<unsigned>(trans_e::IDLE));
        }
    }
}

template <unsigned WIDTH> inline void initiator<WIDTH>::bus_data_task() {
    auto const width = WIDTH / 8;
    auto& hready = HREADY_i.read();
    auto& rdata = HRDATA_i.read();
    while(true) {
        wait(tx_in_flight.get_event());
        while(auto trans = tx_in_flight.get_next_transaction()) {
            auto bytes_exp = scc::ilog2(trans->get_data_length());
            auto width_exp = scc::ilog2(WIDTH / 8);
            size_t size = 0;
            for(; size < width_exp; ++size)
                if(trans->get_address() & (1 << size))
                    break; // i contains the first bit not being 0
            auto beats = bytes_exp < size? 1 : 1 << (bytes_exp - size);
            auto start_offs = trans->get_address() & (width - 1);
            auto len = trans->get_data_length();
           if(trans->is_write()) {
                data_t data{0};
                for(size_t i = start_offs * 8, j = 0; i < WIDTH; i += 8, ++j)
                    data.range(i + 7, i) = *(uint8_t*)(trans->get_data_ptr() + j);
                HWDATA_o.write(data);
                trans->set_response_status(tlm::TLM_OK_RESPONSE);
                tlm::tlm_phase phase{tlm::BEGIN_RESP};
                sc_time delay;
                tsckt->nb_transport_bw(*trans, phase, delay);
            }
            do {
                wait(HCLK_i.posedge_event());
            } while(!hready);
            if(trans->is_read()) {
                for(size_t i = start_offs * 8, j = 0; i < WIDTH; i += 8, ++j)
                    *(uint8_t*)(trans->get_data_ptr() + j) = rdata.range(i + 7, i).to_uint();
                trans->set_response_status(tlm::TLM_OK_RESPONSE);
                tlm::tlm_phase phase{tlm::BEGIN_RESP};
                sc_time delay;
                tsckt->nb_transport_bw(*trans, phase, delay);
            }
            if(trans->has_mm())
                trans->release();
        }
    }
}

template class ahb::pin::initiator<32>;
template class ahb::pin::initiator<64>;
template class ahb::pin::initiator<128>;
template class ahb::pin::initiator<256>;
template class ahb::pin::initiator<512>;
template class ahb::pin::initiator<1024>;
