/*******************************************************************************
 * Copyright 2018 MINRES Technologies GmbH
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

#ifndef _SCC_TIME2TICK_H_
#define _SCC_TIME2TICK_H_

#include "utilities.h"

namespace scc {
struct time2tick : public sc_core::sc_module {
    SC_HAS_PROCESS(time2tick);// NOLINT
    sc_core::sc_in<sc_core::sc_time> clk_i;
    sc_core::sc_out<bool> clk_o;

    time2tick(sc_core::sc_module_name nm)
    : sc_core::sc_module(nm) {
        SC_THREAD(clocker);
    }

private:
    sc_core::sc_time clk_period;
    void clocker() {
        while (true) {
            auto t = clk_i.read();
            if (t == sc_core::SC_ZERO_TIME) {
                wait(clk_i.value_changed_event());
                t = clk_i.read();
            }
            clk_o = true;
            wait(t / 2);
            clk_o = false;
            wait(t - t / 2);
        }
    }
};
}
#endif /* _SCC_TIME2TICK_H_ */
