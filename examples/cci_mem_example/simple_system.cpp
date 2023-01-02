/*******************************************************************************
 * Copyright 2023 MINRES Technologies GmbH
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
#include "simple_system.h"

namespace sysc {

simple_system::simple_system(sc_core::sc_module_name nm)
: sc_core::sc_module(nm)
{
    // bus connections
    master.intor(router.target[0]);
    router.bind_target(mem.target, 	0,     0, 4_kB);
    router.bind_target(uart.socket, 1, 64_kB, 4_kB);

    // clock/reset connections
    uart.clk_i(clk);
    clk.write(10_ns);

    uart.rst_i(rst);
    master.rst_i(rst);

    SC_THREAD(gen_reset);
}

void simple_system::gen_reset() {
    rst = true;
    wait(10_ns);
    rst = false;
}

} /* namespace sysc */
