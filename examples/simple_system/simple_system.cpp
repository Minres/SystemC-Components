////////////////////////////////////////////////////////////////////////////////
// Copyright 2017 eyck@minres.com
//
// Licensed under the Apache License, Version 2.0 (the "License"); you may not
// use this file except in compliance with the License.  You may obtain a copy
// of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
// WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
// License for the specific language governing permissions and limitations under
// the License.
////////////////////////////////////////////////////////////////////////////////
/*
 * simplesystem.cpp
 *
 *  Created on: 17.09.2017
 *      Author: eyck@minres.com
 */

#include "simple_system.h"

namespace sysc {

simple_system::simple_system(sc_core::sc_module_name nm)
: sc_core::sc_module(nm)
, NAMED(i_master)
, NAMED(i_router, 4, 1)
, NAMED(i_uart)
, NAMED(i_spi)
, NAMED(i_gpio)
, NAMED(i_plic)
, NAMED(s_clk)
, NAMED(s_rst)
, NAMED(s_global_interrupts, 256)
, NAMED(s_core_interrupt)
, NAMED(s_gpio, 32)
{
    // todo: discuss naming conventions (s_<signal> vs. <port>_i/_o) --> covnert into _s

    // bus connections
    i_master.intor(i_router.target[0]);
    i_router.bind_target(i_plic.socket, 0, "plic");
    i_router.bind_target(i_uart.socket, 1, "uart");
    i_router.bind_target(i_spi.socket,  2, "spi");
    i_router.bind_target(i_gpio.socket, 3, "gpio");

    // target address ranges
    for (const auto &e : e300_plat_map)
        i_router.add_target_range(e.name, e.start, e.size);

    // clock/reset connections
    i_uart.clk_i(s_clk);
    i_spi.clk_i(s_clk);
    i_gpio.clk_i(s_clk);
    i_plic.clk_i(s_clk);
    s_clk.write(10_ns);

    i_uart.rst_i(s_rst);
    i_spi.rst_i(s_rst);
    i_gpio.rst_i(s_rst);
    i_plic.rst_i(s_rst);
    i_master.rst_i(s_rst);

    // interrupt connections
    i_plic.core_interrupt_o(s_core_interrupt);
    i_plic.global_interrupts_i.bind(s_global_interrupts);
    i_master.global_interrupts_o(s_global_interrupts);
    i_master.core_interrupt_i(s_core_interrupt);

    for(auto i=0U; i<s_gpio.size(); ++i){
      s_gpio[i].in(i_gpio.pins_o[i]);
      i_gpio.pins_i[i](s_gpio[i].out);
    }

    SC_THREAD(gen_reset);
}

void simple_system::gen_reset() {
    s_rst = true;
    wait(10_ns);
    s_rst = false;
}

} /* namespace sysc */
