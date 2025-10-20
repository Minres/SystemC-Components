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

#ifndef _BUS_APB_PIN_TARGET_H_
#define _BUS_APB_PIN_TARGET_H_

#include <apb/apb_tlm.h>
#include <scc/peq.h>
#include <scc/report.h>
#include <scc/signal_opt_ports.h>
#include <scc/utilities.h>
#include <tlm/scc/initiator_mixin.h>
#include <tlm/scc/tlm_mm.h>
#include <tlm>
#include <tlm_utils/peq_with_get.h>

//! TLM2.0 components modeling APB
namespace apb {
//! pin level adapters
namespace pin {

template <unsigned DATA_WIDTH, unsigned ADDR_WIDTH> class target : sc_core::sc_module {
    static constexpr bool is_larger(unsigned x) { return x > 64U; }

public:
    using addr_t = typename std::conditional<is_larger(ADDR_WIDTH), sc_dt::sc_biguint<ADDR_WIDTH>, sc_dt::sc_uint<ADDR_WIDTH>>::type;
    using data_t = typename std::conditional<is_larger(DATA_WIDTH), sc_dt::sc_biguint<DATA_WIDTH>, sc_dt::sc_uint<DATA_WIDTH>>::type;
    using strb_t = sc_dt::sc_uint<DATA_WIDTH / 8>;

    sc_core::sc_in<bool> PCLK_i{"PCLK_i"};
    sc_core::sc_in<bool> PRESETn_i{"PRESETn_i"};
    sc_core::sc_in<addr_t> PADDR_i{"PADDR_i"};
    scc::sc_in_opt<sc_dt::sc_uint<3>> PPROT_i{"PPROT_i"};
    scc::sc_in_opt<bool> PNSE_i{"PNSE_i"};
    sc_core::sc_in<bool> PSELx_i{"PSELx_i"};
    sc_core::sc_in<bool> PENABLE_i{"PENABLE_i"};
    sc_core::sc_in<bool> PWRITE_i{"PWRITE_i"};
    sc_core::sc_in<data_t> PWDATA_i{"PWDATA_i"};
    scc::sc_in_opt<strb_t> PSTRB_i{"PSTRB_i"};
    sc_core::sc_out<bool> PREADY_o{"PREADY_o"};
    sc_core::sc_out<data_t> PRDATA_o{"PRDATA_o"};
    sc_core::sc_out<bool> PSLVERR_o{"PSLVERR_o"};
    scc::sc_in_opt<bool> PWAKEUP_i{"PWAKEUP_i"};

    tlm::scc::initiator_mixin<tlm::tlm_initiator_socket<0>> isckt{"isckt"};

    target(const sc_core::sc_module_name& nm);
    virtual ~target();

private:
    void bus_task();
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
    bool waiting4end_req{false};
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// implementations of functions
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template <unsigned DATA_WIDTH, unsigned ADDR_WIDTH>
target<DATA_WIDTH, ADDR_WIDTH>::target(const sc_core::sc_module_name& nm)
: sc_module(nm) {
    SC_HAS_PROCESS(target);
    SC_THREAD(bus_task);
    isckt.register_nb_transport_bw([this](tlm::tlm_generic_payload& trans, tlm::tlm_phase& phase, sc_core::sc_time& delay) {
        if(phase == tlm::END_REQ) {
            end_req_evt.notify(delay);
            waiting4end_req = false;
        } else if(phase == tlm::BEGIN_RESP) {
            if(waiting4end_req) {
                end_req_evt.notify(delay);
                waiting4end_req = false;
            }
            resp_que.notify(trans, delay);
        }
        return tlm::TLM_ACCEPTED;
    });
}

template <unsigned DATA_WIDTH, unsigned ADDR_WIDTH> target<DATA_WIDTH, ADDR_WIDTH>::~target() = default;

template <unsigned DATA_WIDTH, unsigned ADDR_WIDTH> void target<DATA_WIDTH, ADDR_WIDTH>::bus_task() {
    wait(sc_core::SC_ZERO_TIME);
    wait(PCLK_i.posedge_event());
    while(true) {
        if(!PRESETn_i.read()) {
            wait(PRESETn_i.posedge_event());
            wait(PCLK_i.posedge_event());
        } else {
            PREADY_o.write(false);
            wait(sc_core::SC_ZERO_TIME);
            while(!PSELx_i.read())
                wait(PSELx_i.value_changed_event());
            SCCDEBUG(SCMOD) << "Starting APB setup phase";
            unsigned length = DATA_WIDTH / 8;
            auto trans = tlm::scc::tlm_mm<>::get().allocate<apb::apb_extension>(length);
            tlm::scc::tlm_gp_mm::add_data_ptr(length, trans, true);
            trans->acquire();
            trans->set_streaming_width(length);
            trans->set_address(PADDR_i.read());
            auto* ext = trans->get_extension<apb_extension>();
            if(PPROT_i.get_interface())
                ext->set_protection(PPROT_i.read().to_uint());
            if(PNSE_i.get_interface())
                ext->set_nse(PNSE_i.read());
            if(PWRITE_i.read()) {
                trans->set_write();
                auto data = PWDATA_i.read();
                if(PSTRB_i.get_interface()) {
                    auto strb = PSTRB_i.read();
                    // Copy all data bytes and use byte enables for sparse strobes
                    for(size_t j = 0; j < DATA_WIDTH / 8; ++j) {
                        *(trans->get_data_ptr() + j) = data(8 * j + 7, 8 * j).to_uint();
                        *(trans->get_byte_enable_ptr() + j) = strb[j] ? 0xFF : 0x00;
                    }
                    trans->set_byte_enable_length(DATA_WIDTH / 8);
                } else {
                    for(size_t j = 0; j < DATA_WIDTH / 8; ++j)
                        *(trans->get_data_ptr() + j) = data(8 * j + 7, 8 * j).to_uint();
                    trans->set_byte_enable_length(0);
                }
            } else {
                trans->set_read();
            }
            sc_core::sc_time delay;
            tlm::tlm_phase phase{tlm::BEGIN_REQ};
            SCCDEBUG(SCMOD) << "Recv beg req for read to addr 0x" << std::hex << trans->get_address();
            auto res = isckt->nb_transport_fw(*trans, phase, delay);
            if(res == tlm::TLM_ACCEPTED) {
                waiting4end_req = true;
                wait(end_req_evt);
                phase = tlm::END_REQ;
            }
            SCCDEBUG(SCMOD) << "Recv end req for " << (trans->is_write() ? "write to" : "read from") << " addr 0x" << std::hex
                            << trans->get_address();
            SCCDEBUG(SCMOD) << "APB setup phase, finished";
            wait(PENABLE_i.posedge_event());
            if(phase != tlm::BEGIN_RESP) {
                auto resp = wait4tx(resp_que);
                sc_assert(trans == resp);
            }
            SCCDEBUG(SCMOD) << "Recv beg resp for " << (trans->is_write() ? "write to" : "read from") << " addr 0x" << std::hex
                            << trans->get_address() << ", starting access phase";
            delay = sc_core::SC_ZERO_TIME;
            phase = tlm::END_RESP;
            isckt->nb_transport_fw(*trans, phase, delay);
            if(trans->is_read()) {
                data_t data{0};
                for(size_t j = 0; j < DATA_WIDTH / 8; ++j)
                    data.range(j * 8 + 7, j * 8) = *(trans->get_data_ptr() + j);
                PRDATA_o.write(data);
            }
            PREADY_o.write(true);
            PSLVERR_o.write(trans->get_response_status() != tlm::TLM_OK_RESPONSE);
            wait(PCLK_i.posedge_event());
            SCCDEBUG(SCMOD) << "APB access phase finished";
        }
    }
}
} // namespace pin
} /* namespace apb */

#endif /* _BUS_APB_PIN_TARGET_H_ */
