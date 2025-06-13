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

#ifndef _BUS_AHB_PIN_INITIATOR_H_
#define _BUS_AHB_PIN_INITIATOR_H_

#ifndef SC_INCLUDE_DYNAMIC_PROCESSES
#define SC_INCLUDE_DYNAMIC_PROCESSES
#endif

#include <ahb/ahb_tlm.h>
#include <scc/report.h>
#include <scc/utilities.h>
#include <tlm/scc/target_mixin.h>
#include <tlm>
#include <tlm_utils/peq_with_get.h>
#include <type_traits>

namespace ahb {
namespace pin {

template <unsigned DATA_WIDTH, unsigned ADDR_WIDTH = 32> class initiator : sc_core::sc_module {
    static constexpr bool is_larger(unsigned x) { return x > 64U; }
    using addr_t = sc_dt::sc_uint<ADDR_WIDTH>;
    using data_t = typename std::conditional<is_larger(DATA_WIDTH), sc_dt::sc_biguint<DATA_WIDTH>, sc_dt::sc_uint<DATA_WIDTH>>::type;

public:
    sc_core::sc_in<bool> HCLK_i{"HCLK_i"};
    sc_core::sc_in<bool> HRESETn_i{"HRESETn_i"};
    sc_core::sc_out<addr_t> HADDR_o{"HADDR_o"};
    sc_core::sc_out<sc_dt::sc_uint<3>> HBURST_o{"HBURST_o"};
    sc_core::sc_out<bool> HMASTLOCK_o{"HMASTLOCK_o"};
    sc_core::sc_out<sc_dt::sc_uint<4>> HPROT_o{"HPROT_o"};
    sc_core::sc_out<sc_dt::sc_uint<3>> HSIZE_o{"HSIZE_o"};
    sc_core::sc_out<sc_dt::sc_uint<2>> HTRANS_o{"HTRANS_o"};
    sc_core::sc_out<data_t> HWDATA_o{"HWDATA_o"};
    sc_core::sc_out<bool> HWRITE_o{"HWRITE_o"};
    sc_core::sc_in<data_t> HRDATA_i{"HRDATA_i"};
    sc_core::sc_in<bool> HREADY_i{"HREADY_i"};
    sc_core::sc_in<bool> HRESP_i{"HRESP_i"};

    tlm::scc::target_mixin<tlm::tlm_target_socket<DATA_WIDTH>> tsckt{"tsckt"};

    initiator(const sc_core::sc_module_name& nm);
    virtual ~initiator() = default;

private:
    struct data_phase_desc {
        tlm::tlm_generic_payload* gp;
        unsigned size;
        unsigned length;
    };
    void bus_addr_task();
    void bus_data_task();

    tlm_utils::peq_with_get<tlm::tlm_generic_payload> inqueue{"inqueue"};
    tlm_utils::peq_with_get<tlm::tlm_generic_payload> tx_in_flight{"tx_in_flight"};
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// implementations of functions
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template <unsigned DATA_WIDTH, unsigned ADDR_WIDTH>
initiator<DATA_WIDTH, ADDR_WIDTH>::initiator(const sc_core::sc_module_name& nm)
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

template <unsigned DATA_WIDTH, unsigned ADDR_WIDTH> inline void initiator<DATA_WIDTH, ADDR_WIDTH>::bus_addr_task() {
    auto& hready = HREADY_i.read();
    while(true) {
        wait(inqueue.get_event());
        while(auto trans = inqueue.get_next_transaction()) {
            sc_assert(trans->get_data_length() * 8 <= DATA_WIDTH && "Transaction length larger than bus width, this is not supported");
            SCCDEBUG(SCMOD) << "Recv beg req for read to addr 0x" << std::hex << trans->get_address();
            auto bytes_exp = scc::ilog2(trans->get_data_length());
            auto width_exp = scc::ilog2(DATA_WIDTH / 8);
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
            HTRANS_o.write(static_cast<unsigned>(ext->is_seq() ? trans_e::SEQ : trans_e::NONSEQ));
            do {
                wait(HCLK_i.posedge_event());
            } while(!hready);
            SCCDEBUG(SCMOD) << "Send end req for read to addr 0x" << std::hex << trans->get_address();
            tlm::tlm_phase phase{tlm::END_REQ};
            sc_core::sc_time delay;
            auto res = tsckt->nb_transport_bw(*trans, phase, delay);
            tx_in_flight.notify(*trans);
            HTRANS_o.write(static_cast<unsigned>(trans_e::IDLE));
        }
    }
}

template <unsigned DATA_WIDTH, unsigned ADDR_WIDTH> inline void initiator<DATA_WIDTH, ADDR_WIDTH>::bus_data_task() {
    auto const width = DATA_WIDTH / 8;
    auto& hready = HREADY_i.read();
    auto& rdata = HRDATA_i.read();
    while(true) {
        wait(tx_in_flight.get_event());
        while(auto trans = tx_in_flight.get_next_transaction()) {
            auto bytes_exp = scc::ilog2(trans->get_data_length());
            auto width_exp = scc::ilog2(DATA_WIDTH / 8);
            size_t size = 0;
            for(; size < width_exp; ++size)
                if(trans->get_address() & (1 << size))
                    break; // i contains the first bit not being 0
            auto beats = bytes_exp < size ? 1 : 1 << (bytes_exp - size);
            auto start_offs = trans->get_address() & (width - 1);
            auto len = trans->get_data_length();
            if(trans->is_write()) {
                data_t data{0};
                for(size_t i = start_offs * 8, j = 0; i < DATA_WIDTH; i += 8, ++j)
                    data.range(i + 7, i) = *(uint8_t*)(trans->get_data_ptr() + j);
                HWDATA_o.write(data);
                trans->set_response_status(tlm::TLM_OK_RESPONSE);
                tlm::tlm_phase phase{tlm::BEGIN_RESP};
                sc_core::sc_time delay;
                tsckt->nb_transport_bw(*trans, phase, delay);
            }
            do {
                wait(HCLK_i.posedge_event());
            } while(!hready);
            if(trans->is_read()) {
                for(size_t i = start_offs * 8, j = 0; i < DATA_WIDTH; i += 8, ++j)
                    *(uint8_t*)(trans->get_data_ptr() + j) = rdata.range(i + 7, i).to_uint();
                trans->set_response_status(tlm::TLM_OK_RESPONSE);
                tlm::tlm_phase phase{tlm::BEGIN_RESP};
                sc_core::sc_time delay;
                tsckt->nb_transport_bw(*trans, phase, delay);
            }
            if(trans->has_mm())
                trans->release();
        }
    }
}
} // namespace pin
} /* namespace ahb */

#endif /* _BUS_AHB_PIN_INITIATOR_H_ */
