/*******************************************************************************
 * Copyright 2019-2024 MINRES Technologies GmbH
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

#ifndef _BUS_AHB_PIN_TARGET_H_
#define _BUS_AHB_PIN_TARGET_H_

#include <ahb/ahb_tlm.h>
#include <scc/peq.h>
#include <scc/report.h>
#include <scc/utilities.h>
#include <tlm/scc/initiator_mixin.h>
#include <tlm/scc/tlm_mm.h>
#include <tlm>
#include <tlm_utils/peq_with_get.h>

//! TLM2.0 components modeling AHB
namespace ahb {
//! pin level adapters
namespace pin {

template <unsigned DATA_WIDTH, unsigned ADDR_WIDTH> class target : sc_core::sc_module {
    static constexpr bool is_larger(unsigned x) { return x > 64U; }
    using addr_t = typename std::conditional<is_larger(ADDR_WIDTH), sc_dt::sc_biguint<ADDR_WIDTH>, sc_dt::sc_uint<ADDR_WIDTH>>::type;
    using data_t = typename std::conditional<is_larger(DATA_WIDTH), sc_dt::sc_biguint<DATA_WIDTH>, sc_dt::sc_uint<DATA_WIDTH>>::type;

public:
    sc_core::sc_in<bool> HCLK_i{"HCLK_i"};
    sc_core::sc_in<bool> HRESETn_i{"HRESETn_i"};
    sc_core::sc_in<addr_t> HADDR_i{"HADDR_i"};
    sc_core::sc_in<sc_dt::sc_uint<3>> HBURST_i{"HBURST_i"};
    sc_core::sc_in<bool> HMASTLOCK_i{"HMASTLOCK_i"};
    sc_core::sc_in<sc_dt::sc_uint<4>> HPROT_i{"HPROT_i"};
    sc_core::sc_in<sc_dt::sc_uint<3>> HSIZE_i{"HSIZE_i"};
    sc_core::sc_in<sc_dt::sc_uint<2>> HTRANS_i{"HTRANS_i"};
    sc_core::sc_in<data_t> HWDATA_i{"HWDATA_i"};
    sc_core::sc_in<bool> HWRITE_i{"HWRITE_i"};
    sc_core::sc_in<bool> HSEL_i{"HSEL_i"};
    sc_core::sc_out<data_t> HRDATA_o{"HRDATA_o"};
    sc_core::sc_out<bool> HREADY_o{"HREADY_o"};
    sc_core::sc_out<bool> HRESP_o{"HRESP_o"};

    tlm::scc::initiator_mixin<tlm::tlm_initiator_socket<0>> isckt{"isckt"};

    target(const sc_core::sc_module_name& nm);
    virtual ~target();

private:
    void bus_addr_task();
    void bus_data_task();
    static tlm::tlm_generic_payload* wait4tx(tlm_utils::peq_with_get<tlm::tlm_generic_payload>& que) {
        tlm::tlm_generic_payload* ret = que.get_next_transaction();
        while(!ret) {
            ::sc_core::wait(que.get_event());
            ret = que.get_next_transaction();
        }
        return ret;
    }
    sc_core::sc_event end_req_evt;
    tlm_utils::peq_with_get<tlm::tlm_generic_payload> resp_que{"resp_que"};
    tlm_utils::peq_with_get<tlm::tlm_generic_payload> tx_in_flight{"tx_in_flight"};
    bool waiting4end_req{false};
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// implementations of functions
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template <unsigned DATA_WIDTH, unsigned ADDR_WIDTH>
target<DATA_WIDTH, ADDR_WIDTH>::target(const sc_core::sc_module_name& nm)
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

template <unsigned DATA_WIDTH, unsigned ADDR_WIDTH> target<DATA_WIDTH, ADDR_WIDTH>::~target() = default;

template <unsigned DATA_WIDTH, unsigned ADDR_WIDTH> void target<DATA_WIDTH, ADDR_WIDTH>::bus_addr_task() {
    auto const width_exp = scc::ilog2(DATA_WIDTH / 8);
    wait(sc_core::SC_ZERO_TIME);
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
                gp->set_streaming_width(length);
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

template <unsigned DATA_WIDTH, unsigned ADDR_WIDTH> void target<DATA_WIDTH, ADDR_WIDTH>::bus_data_task() {
    auto const width = DATA_WIDTH / 8;
    auto& wdata = HWDATA_i.read();
    wait(sc_core::SC_ZERO_TIME);
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
                for(size_t i = start_offs * 8, j = 0; i < DATA_WIDTH; i += 8, ++j)
                    *(uint8_t*)(gp->get_data_ptr() + j) = wdata.range(i + 7, i).to_uint();
            }
            SCCDEBUG(SCMOD) << "Send beg req for " << (gp->is_write() ? "write to" : "read from") << " addr 0x" << std::hex
                            << gp->get_address();
            sc_core::sc_time delay;
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
            if(gp->is_response_error()) {
                HREADY_o.write(false);
                HRESP_o.write(true);
            } else {
                HREADY_o.write(true);
                HRESP_o.write(false);
            }
            gp->release();
            HREADY_o.write(true);
            wait(HCLK_i.posedge_event());
        }
    }
}

} // namespace pin
} /* namespace ahb */

#endif /* _BUS_AHB_PIN_TARGET_H_ */
