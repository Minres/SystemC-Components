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
    auto& hready = PREADY_i.read();
    while(true) {
        wait(inqueue.get_event());
        while(auto trans = inqueue.get_next_transaction()) {
            auto addr_offset = trans->get_address() & (DATA_WIDTH / 8 - 1);
            auto upper = addr_offset + trans->get_data_length();
            if(!PSTRB_o.get_interface() && addr_offset && upper != (DATA_WIDTH / 8)) {
                SCCERR(SCMOD) << "Narrow accesses are not supported as there is no PSTRB signal! Skipping " << *trans;
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
                auto bytes_exp = scc::ilog2(trans->get_data_length());
                auto width_exp = scc::ilog2(DATA_WIDTH / 8);
                size_t size = 0;
                for(; size < bytes_exp; ++size)
                    if(trans->get_address() & (1 << size))
                        break; // i contains the first bit not being 0
                auto* ext = trans->template get_extension<apb_extension>();
                if(trans->is_write()) {
                    if(upper <= DATA_WIDTH / 8) {
                        data_t data{0};
                        strb_t strb{0};
                        for(size_t i = 0; i < upper; ++i) {
                            if(i >= addr_offset) {
                                data.range(i * 8 + 7, i * 8) = *(trans->get_data_ptr() + i - addr_offset);
                                strb[i] = 1;
                            }
                        }
                        PWDATA_o.write(data);
                        if(PSTRB_o.get_interface())
                            PSTRB_o.write(strb);
                    }
                }
                PWRITE_o.write(trans->is_write());
                PADDR_o.write(trans->get_address() - addr_offset); // adjust address to be aligned
                PSELx_o.write(true);
                if(PPROT_o.get_interface() && ext)
                    PPROT_o.write(ext ? ext->get_protection() : 0);
                if(PNSE_o.get_interface())
                    PNSE_o.write(ext ? ext->is_nse() : false);
                wait(PCLK_i.posedge_event());
                SCCDEBUG(SCMOD) << "APB setup phase finished, sending end req for access to addr 0x" << std::hex << trans->get_address();
                tlm::tlm_phase phase{tlm::END_REQ};
                sc_core::sc_time delay;
                auto res = tsckt->nb_transport_bw(*trans, phase, delay);
                SCCDEBUG(SCMOD) << "Starting APB access phase";
                PENABLE_o.write(true);
                wait(1_ps);
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
                res = tsckt->nb_transport_bw(*trans, phase, delay);
                SCCDEBUG(SCMOD) << "APB access phase finished";
                if(trans->has_mm())
                    trans->release();
                PENABLE_o.write(false);
                PSELx_o.write(false);
            }
        }
    }
}
} // namespace pin
} // namespace apb

#endif /* _BUS_APB_PIN_INITIATOR_H_ */
