/*******************************************************************************
 * Copyright 2017 eyck@minres.com
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
 * License for the specific language governing permissions and limitations under
 * the License.
 ******************************************************************************/
/*
 * simplesystem.h
 *
 *  Created on: 17.09.2017
 *      Author: eyck@minres.com
 */

#ifndef SIMPLESYSTEM_H_
#define SIMPLESYSTEM_H_

#include "gpio.h"
#include "plic.h"
#include "spi.h"
#include "uart.h"

#include "test_initiator.h"
#include <array>
#include <scc/router.h>
#include <sysc/kernel/sc_module.h>
#include <scc/socket_width_adapter.h>

namespace sysc {

class simple_system : public sc_core::sc_module {
public:
    SC_HAS_PROCESS(simple_system);

    test_initiator i_master;
    scc::socket_width_adapter<32, scc::LT> adapt{"adapt"};
    scc::router<> i_router;
    uart i_uart;
    spi i_spi;
    gpio i_gpio;
    plic i_plic;
    sc_core::sc_signal<sc_core::sc_time> s_clk;
    sc_core::sc_signal<bool> s_rst;
    sc_core::sc_vector<sc_core::sc_signal<bool>> s_global_interrupts;
    sc_core::sc_signal<bool, sc_core::SC_MANY_WRITERS> s_core_interrupt;
    sc_core::sc_vector<tlm::scc::tlm_signal<>> s_gpio;

    simple_system(sc_core::sc_module_name nm);

protected:
    void gen_reset();

#include "gen/e300_plat_t.h"
};

} /* namespace sysc */

#endif /* SIMPLESYSTEM_H_ */
