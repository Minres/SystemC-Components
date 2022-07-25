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
#include <axi/pe/simple_target.h>
#include <scc/peq.h>

#include <systemc>
#include <tlm>
#include <tlm_utils/simple_initiator_socket.h>

namespace axi {
/**
 * @brief axi_target class instantiates the AXI Protocol Engine.
 * It accesses the Protocol Engine with access() callback function and
 * forwards the transactions via the output_socket.
 */
class axi_target_base : public sc_core::sc_module {
public:
    sc_core::sc_in<bool> clk_i{"clk_i"};
    tlm_utils::simple_initiator_socket<axi_target_base> isck{"isck"};

    axi_target_base(const sc_core::sc_module_name& nm, axi::pe::axi_target_pe& pe);
    virtual ~axi_target_base(){};

protected:
    unsigned access(tlm::tlm_generic_payload& trans);

private:
    axi::pe::axi_target_pe& pe;
    scc::peq<tlm::tlm_generic_payload*> peq;

    void trans_queue();
};

template <unsigned int BUSWIDTH = 32> class axi_target : public axi_target_base {
public:
    axi::axi_target_socket<BUSWIDTH> tsck{"tsck"};

    axi_target(sc_core::sc_module_name nm)
    : axi_target_base(nm, pe) {
        pe.clk_i(clk_i);
        pe.set_operation_cb([this](tlm::tlm_generic_payload& trans) -> unsigned { return access(trans); });
    };

    virtual ~axi_target(){};

    axi::pe::simple_target<BUSWIDTH> pe{"pe", tsck};
};

} // namespace axi
