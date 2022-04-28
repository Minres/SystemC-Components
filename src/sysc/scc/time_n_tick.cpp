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
#define SC_INCLUDE_DYNAMIC_PROCESSES
#include "tick2time.h"
#include "time2tick.h"

#include <systemc>
#ifdef CWR_SYSTEMC
#include "scml_clock/scml_clock_if.h"
#endif

namespace scc {

tick2time::tick2time(sc_core::sc_module_name nm)
: sc_core::sc_module(nm) {}

void tick2time::end_of_elaboration() {
#ifdef CWR_SYSTEMC
    if(auto scml_clk_if = scml2::get_scml_clock(clk_i)) {
        scml_clk_if->register_observer(this);
        handle_clock_parameters_updated(scml_clk_if);
        return;
    }
#endif
    if(auto clk_if = dynamic_cast<sc_core::sc_clock*>(clk_i.get_interface())) {
        this->clk_period = clk_if->period();
        clk_o.write(clk_period);
    } else {
        sc_core::sc_spawn_options opts;
        opts.set_stack_size(0x1000);
        sc_core::sc_spawn(
            [this]() {
                while(true) {
                    wait(clk_i.posedge_event());
                    clk_period = sc_core::sc_time_stamp() - last_tick;
                    clk_o.write(clk_period);
                    last_tick = sc_core::sc_time_stamp();
                }
            },
            nullptr, &opts);
    }
}
#ifdef CWR_SYSTEMC
void tick2time::handle_clock_parameters_updated(scml_clock_if* clk_if) {
    this->clk_period = clk_if->get_period();
    clk_o.write(clk_period);
}
void tick2time::handle_clock_deleted(scml_clock_if*){};
#endif

inline void time2tick::clocker() {
    while(true) {
        auto t = clk_i.read();
        if(t == sc_core::SC_ZERO_TIME) {
            wait(clk_i.value_changed_event());
            t = clk_i.read();
        }
        clk_o = true;
        wait(t / 2);
        clk_o = false;
        wait(t - t / 2);
    }
}
} // namespace scc
