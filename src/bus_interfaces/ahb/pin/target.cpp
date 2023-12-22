/*******************************************************************************
 * Copyright 2019-2023 MINRES Technologies GmbH
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
#include "target.h"
#include <ahb/ahb_tlm.h>
#include <scc/report.h>
#include <scc/utilities.h>
#include <tlm/scc/tlm_mm.h>

using namespace ahb::pin;
using namespace sc_core;

template <unsigned DWIDTH, unsigned AWIDTH>
target<DWIDTH, AWIDTH>::target(const sc_module_name& nm)
: sc_module(nm) {
    SC_HAS_PROCESS(target);
    SC_THREAD(bus_addr_task);
    SC_THREAD(bus_data_task);
    isckt.register_nb_transport_bw([this](tlm::tlm_generic_payload& gp, tlm::tlm_phase& phase, sc_core::sc_time& delay) {
        if(phase == tlm::END_REQ) {
            end_req_evt.notify(delay);
            waiting4end_req = false;
        } else if(phase == tlm::BEGIN_RESP) {
            if(waiting4end_req) {
                end_req_evt.notify(delay);
                waiting4end_req = false;
            }
            resp_que.notify(gp, delay);
        }
        return tlm::TLM_ACCEPTED;
    });
}

template <unsigned DWIDTH, unsigned AWIDTH> target<DWIDTH, AWIDTH>::~target() = default;

template <unsigned DWIDTH, unsigned AWIDTH> void target<DWIDTH, AWIDTH>::bus_addr_task() {
    auto const width_exp = scc::ilog2(DWIDTH / 8);
    wait(SC_ZERO_TIME);
    auto& htrans = HTRANS_i.read();
    auto& hsel = HSEL_i.read();
    auto& hready = HREADY_o.read();
    auto& size = HSIZE_i.read();
    while(true) {
        if(!HRESETn_i.read()) {
            wait(HRESETn_i.posedge_event());
        } else {
            wait(HCLK_i.posedge_event());
            if(hsel && hready && htrans > 1) { // HTRANS/BUSY or IDLE check
                unsigned sz = size;
                if(sz > width_exp)
                    SCCERR(SCMOD) << "Access size (" << sz << ") is larger than bus wDWIDTH(" << width_exp << ")!";
                unsigned length = (1 << sz);
                auto gp = tlm::scc::tlm_mm<>::get().allocate<ahb::ahb_extension>(length);
                gp->acquire();
                gp->set_address(HADDR_i.read());
                auto* ext = gp->get_extension<ahb_extension>();
                ext->set_locked(HMASTLOCK_i.read());
                ext->set_protection(HPROT_i.read());
                ext->set_seq(htrans == 3);
                ext->set_burst(static_cast<ahb::burst_e>(HBURST_i.read().to_uint()));
                if(HWRITE_i.read()) {
                    gp->set_write();
                } else {
                    gp->set_read();
                }
                tx_in_flight.notify(*gp);
            }
        }
    }
}

template <unsigned DWIDTH, unsigned AWIDTH> void target<DWIDTH, AWIDTH>::bus_data_task() {
    auto const width = DWIDTH / 8;
    auto& wdata = HWDATA_i.read();
    wait(SC_ZERO_TIME);
    while(true) {
        if(!HRESETn_i.read()) {
            HREADY_o.write(false);
            wait(HRESETn_i.posedge_event());
        } else {
            HREADY_o.write(true);
            auto gp = wait4tx(tx_in_flight);
            auto ext = gp->template get_extension<ahb::ahb_extension>();
            auto start_offs = gp->get_address() & (width - 1);
            sc_assert((start_offs + gp->get_data_length()) <= width);
            auto len = gp->get_data_length();
            HREADY_o.write(false);
            if(gp->is_write()) {
                wait(HCLK_i.negedge_event());
                for(size_t i = start_offs * 8, j = 0; i < DWIDTH; i += 8, ++j)
                    *(uint8_t*)(gp->get_data_ptr() + j) = wdata.range(i + 7, i).to_uint();
            }
            SCCDEBUG(SCMOD) << "Send beg req for " << (gp->is_write() ? "write to" : "read from") << " addr 0x" << std::hex
                            << gp->get_address();
            sc_time delay;
            tlm::tlm_phase phase{tlm::BEGIN_REQ};
            auto res = isckt->nb_transport_fw(*gp, phase, delay);
            if(res == tlm::TLM_ACCEPTED) {
                waiting4end_req = true;
                wait(end_req_evt);
                phase = tlm::END_REQ;
            }
            SCCDEBUG(SCMOD) << "Recv end req for " << (gp->is_write() ? "write to" : "read from") << " addr 0x" << std::hex
                            << gp->get_address();
            if(phase != tlm::BEGIN_RESP) {
                auto resp = wait4tx(resp_que);
                sc_assert(gp == resp);
            }
            SCCDEBUG(SCMOD) << "Recv beg resp for " << (gp->is_write() ? "write to" : "read from") << " addr 0x" << std::hex
                            << gp->get_address();
            if(gp->is_read()) {
                data_t data{0};
                for(size_t i = start_offs * 8, j = 0; j < len; i += 8, ++j)
                    data.range(i + 7, i) = *(uint8_t*)(gp->get_data_ptr() + j);
                HRDATA_o.write(data);
            }
            delay = sc_core::SC_ZERO_TIME;
            phase = tlm::END_RESP;
            SCCDEBUG(SCMOD) << "Send end resp for " << (gp->is_write() ? "write to" : "read from") << " addr 0x" << std::hex
                            << gp->get_address();
            res = isckt->nb_transport_fw(*gp, phase, delay);
            gp->release();
            HREADY_o.write(true);
            wait(HCLK_i.posedge_event());
        }
    }
}
template class ahb::pin::target<32, 32>;
template class ahb::pin::target<64, 32>;
template class ahb::pin::target<128, 32>;
template class ahb::pin::target<256, 32>;
template class ahb::pin::target<512, 32>;
template class ahb::pin::target<1024, 32>;
template class ahb::pin::target<32, 64>;
template class ahb::pin::target<64, 64>;
template class ahb::pin::target<128, 64>;
template class ahb::pin::target<256, 64>;
template class ahb::pin::target<512, 64>;
template class ahb::pin::target<1024, 64>;
