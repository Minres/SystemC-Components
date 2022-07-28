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

#ifndef _BUS_AHB_PIN_TARGET_H_
#define _BUS_AHB_PIN_TARGET_H_

#include <tlm/scc/initiator_mixin.h>
#include <tlm>

//! TLM2.0 components modeling AHB
namespace ahb {
//! pin level adapters
namespace pin {

template <unsigned WIDTH> class target : sc_core::sc_module {
    static constexpr bool is_larger(unsigned x) { return x > 64U; }
    using data_t = typename std::conditional<is_larger(WIDTH), sc_dt::sc_biguint<WIDTH>, sc_dt::sc_uint<WIDTH>>::type;

public:
    sc_core::sc_in<bool> HCLK_i{"HCLK_i"};
    sc_core::sc_in<bool> HRESETn_i{"HRESETn_i"};
    sc_core::sc_in<sc_dt::sc_uint<32>> HADDR_i{"HADDR_i"};
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

    tlm::scc::initiator_mixin<tlm::tlm_initiator_socket<WIDTH>> isckt{"isckt"};

    target(const sc_core::sc_module_name& nm);
    virtual ~target();

private:
    void bfm_thread();
    void handle_data_phase(unsigned& beat_cnt);
    tlm::tlm_generic_payload* addr_payload{nullptr};
    tlm::tlm_generic_payload* data_payload{nullptr};
    sc_core::sc_fifo<tlm::tlm_generic_payload*> active{"active_tx", 1};
};

} // namespace pin
} /* namespace ahb */

#endif /* _BUS_AHB_PIN_TARGET_H_ */
