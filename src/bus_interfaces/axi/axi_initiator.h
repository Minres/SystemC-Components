/*******************************************************************************
 * Copyright 2021 MINRES Technologies GmbH
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

#pragma once

#define SC_INCLUDE_DYNAMIC_PROCESSES

#include <axi/axi_tlm.h>
#include <axi/pe/simple_initiator.h>

#include <systemc>
#include <tlm>
#include <tlm_utils/simple_target_socket.h>

namespace axi {
/**
 * @brief axi_initiator class provides an input_socket for incoming TLM transactions.
 * It attaches AXI extension to the tlm_generic_payload and forwards it to the AXI Protocol Engine.
 */
class axi_initiator_base : public sc_core::sc_module {
public:
    sc_core::sc_in<bool> clk_i{"clk_i"};
    tlm_utils::simple_target_socket<axi_initiator_base> b_tsck{"b_tsck"};

    /**
     * Create and attach AXI extension.
     * AXI protocol engine expects the incoming transactions to be controlled by a memory manager.
     * If transaction does not have an associated memory manager, copy the payload into a new one with mm.
     */
    tlm::tlm_generic_payload* create_axi_trans(tlm::tlm_generic_payload& p);

    void b_transport(tlm::tlm_generic_payload& trans, sc_core::sc_time& delay);

    axi_initiator_base(const sc_core::sc_module_name& nm, axi::pe::simple_initiator_b& pe, uint32_t width);

    virtual ~axi_initiator_base() {}

    void setTxSetupCb(const std::function<void(tlm::tlm_generic_payload& p)>& setupCb) { setup_cb = setupCb; }

private:
    axi::pe::simple_initiator_b& pe;
    uint32_t buswidth{0};
    unsigned id{0};
    std::function<void(tlm::tlm_generic_payload& p)> setup_cb;
};

template <unsigned int BUSWIDTH = 32> class axi_initiator : public axi_initiator_base {
public:
    axi::axi_initiator_socket<BUSWIDTH> isck{"isck"};

    axi_initiator(sc_core::sc_module_name nm)
    : axi_initiator_base(nm, pe, BUSWIDTH) {
        pe.clk_i(clk_i);
    };

    virtual ~axi_initiator(){};

    axi::pe::simple_axi_initiator<BUSWIDTH> pe{"pe", isck};
};

} // namespace axi
