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

#ifndef _BUS_APB_PIN_INITIATOR_H_
#define _BUS_APB_PIN_INITIATOR_H_

#ifndef SC_INCLUDE_DYNAMIC_PROCESSES
#define SC_INCLUDE_DYNAMIC_PROCESSES
#endif

#include <apb/apb_tlm.h>
#include <scc/report.h>
#include <scc/signal_opt_ports.h>
#include <scc/utilities.h>
#include <tlm/scc/target_mixin.h>
#include <tlm>
#include <tlm_utils/peq_with_get.h>
#include <type_traits>

namespace apb {
namespace pin {

template <unsigned DATA_WIDTH, unsigned ADDR_WIDTH = 32> class initiator : sc_core::sc_module {
    static constexpr bool is_larger(unsigned x) { return x > 64U; }

public:
    using addr_t = sc_dt::sc_uint<ADDR_WIDTH>;
    using data_t = typename std::conditional<is_larger(DATA_WIDTH), sc_dt::sc_biguint<DATA_WIDTH>, sc_dt::sc_uint<DATA_WIDTH>>::type;
    using strb_t = sc_dt::sc_uint<DATA_WIDTH / 8>;

    sc_core::sc_in<bool> PCLK_i{"PCLK_i"};
    sc_core::sc_in<bool> PRESETn_i{"PRESETn_i"};
    sc_core::sc_out<addr_t> PADDR_o{"PADDR_o"};
    scc::sc_out_opt<sc_dt::sc_uint<3>> PPROT_o{"PPROT_o"};
    scc::sc_out_opt<bool> PNSE_o{"PNSE_o"};
    sc_core::sc_out<bool> PSELx_o{"PSELx_o"};
    sc_core::sc_out<bool> PENABLE_o{"PENABLE_o"};
    sc_core::sc_out<bool> PWRITE_o{"PWRITE_o"};
    sc_core::sc_out<data_t> PWDATA_o{"PWDATA_o"};
    scc::sc_out_opt<strb_t> PSTRB_o{"PSTRB_o"};
    sc_core::sc_in<bool> PREADY_i{"PREADY_i"};
    sc_core::sc_in<data_t> PRDATA_i{"PRDATA_i"};
    sc_core::sc_in<bool> PSLVERR_i{"PSLVERR_i"};
    scc::sc_out_opt<bool> PWAKEUP_o{"PWAKEUP_o"};

    tlm::scc::target_mixin<tlm::tlm_target_socket<DATA_WIDTH>> tsckt{"tsckt"};

    initiator(const sc_core::sc_module_name& nm);
    virtual ~initiator() = default;

private:
    void bus_task();

    tlm_utils::peq_with_get<tlm::tlm_generic_payload> inqueue{"inqueue"};
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// implementations of functions
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template <unsigned DATA_WIDTH, unsigned ADDR_WIDTH>
initiator<DATA_WIDTH, ADDR_WIDTH>::initiator(const sc_core::sc_module_name& nm)
: sc_module(nm) {
    SC_HAS_PROCESS(initiator);
    SC_THREAD(bus_task);

    tsckt.register_nb_transport_fw(
        [this](tlm::tlm_generic_payload& payload, tlm::tlm_phase& phase, sc_core::sc_time& delay) -> tlm::tlm_sync_enum {
            if(phase == tlm::BEGIN_REQ) {
                if(payload.has_mm())
                    payload.acquire();
                this->inqueue.notify(payload);
            }
            return tlm::TLM_ACCEPTED;
        });
}

template <unsigned DATA_WIDTH, unsigned ADDR_WIDTH> inline void initiator<DATA_WIDTH, ADDR_WIDTH>::bus_task() {
    while(true) {
        wait(inqueue.get_event());
        while(auto trans = inqueue.get_next_transaction()) {
            auto addr_offset = trans->get_address() & (DATA_WIDTH / 8 - 1);
            auto upper = addr_offset + trans->get_data_length();
            if(!PSTRB_o.get_interface() && (addr_offset || upper != (DATA_WIDTH / 8))) {
                SCCERR(SCMOD) << "Narrow accesses are not supported before APB4 as there is no PSTRB signal! Skipping " << *trans;
                tlm::tlm_phase phase{tlm::END_RESP};
                sc_core::sc_time delay;
                trans->set_response_status(tlm::TLM_GENERIC_ERROR_RESPONSE);
                tsckt->nb_transport_bw(*trans, phase, delay);
            } else if(upper > DATA_WIDTH / 8) {
                SCCERR(SCMOD) << "Illegal length of payload since it would cause a wrap-around! Skipping " << *trans;
                tlm::tlm_phase phase{tlm::END_RESP};
                sc_core::sc_time delay;
                trans->set_response_status(tlm::TLM_GENERIC_ERROR_RESPONSE);
                tsckt->nb_transport_bw(*trans, phase, delay);
            } else {
                SCCDEBUG(SCMOD) << "Recv beg req for read to addr 0x" << std::hex << trans->get_address() << ", starting APB setup phase, ";
                auto* ext = trans->template get_extension<apb_extension>();
                if(trans->is_write()) {
                    data_t data{0};
                    strb_t strb{0};
                    // Handle TLM byte enables if present
                    if(trans->get_byte_enable_ptr() && trans->get_byte_enable_length() > 0) {
                        for(size_t i = 0; i < DATA_WIDTH / 8; ++i) {
                            if(i >= addr_offset && i < upper) {
                                auto be_idx = (i - addr_offset) % trans->get_byte_enable_length();
                                if(trans->get_byte_enable_ptr()[be_idx] != 0) {
                                    data.range(i * 8 + 7, i * 8) = *(trans->get_data_ptr() + i - addr_offset);
                                    strb[i] = 1;
                                }
                            }
                        }
                    } else {
                        // No byte enables, write contiguous data
                        for(size_t i = 0; i < upper; ++i) {
                            if(i >= addr_offset) {
                                data.range(i * 8 + 7, i * 8) = *(trans->get_data_ptr() + i - addr_offset);
                                strb[i] = 1;
                            }
                        }
                    }
                    PWDATA_o.write(data);
                    if(PSTRB_o.get_interface())
                        PSTRB_o.write(strb);
                } else if(PSTRB_o.get_interface()) {
                    // From spec : For read transfers the bus master must drive all bits of PSTRB LOW
                    PSTRB_o.write(0);
                }
                PWRITE_o.write(trans->is_write());
                PADDR_o.write(trans->get_address() - addr_offset); // adjust address to be aligned
                PSELx_o.write(true);
                if(PPROT_o.get_interface())
                    PPROT_o.write(ext ? ext->get_protection() : false);
                if(PNSE_o.get_interface())
                    PNSE_o.write(ext ? ext->is_nse() : false);
                wait(PCLK_i.posedge_event());
                SCCDEBUG(SCMOD) << "APB setup phase finished, sending end req for access to addr 0x" << std::hex << trans->get_address();
                tlm::tlm_phase phase{tlm::END_REQ};
                sc_core::sc_time delay;
                tsckt->nb_transport_bw(*trans, phase, delay);
                SCCDEBUG(SCMOD) << "Starting APB access phase";
                PENABLE_o.write(true);
                while(!PREADY_i.read())
                    wait(PREADY_i.value_changed_event());
                wait(PCLK_i.posedge_event());
                if(trans->is_read()) {
                    auto data = PRDATA_i.read();
                    for(size_t j = addr_offset, i = 0; i < trans->get_data_length(); ++j, ++i)
                        *(trans->get_data_ptr() + i) = data(8 * j + 7, 8 * j).to_uint();
                }
                trans->set_response_status(PSLVERR_i.read() ? tlm::TLM_GENERIC_ERROR_RESPONSE : tlm::TLM_OK_RESPONSE);
                phase = tlm::BEGIN_RESP;
                delay = sc_core::SC_ZERO_TIME;
                SCCDEBUG(SCMOD) << "Sending beg resp for access to addr 0x" << std::hex << trans->get_address();
                tsckt->nb_transport_bw(*trans, phase, delay);
                SCCDEBUG(SCMOD) << "APB access phase finished";
                PENABLE_o.write(false);
                PSELx_o.write(false);
                if(trans->has_mm())
                    trans->release();
            }
        }
    }
}
} // namespace pin
} // namespace apb

#endif /* _BUS_APB_PIN_INITIATOR_H_ */
