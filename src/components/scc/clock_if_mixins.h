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

#ifndef _SYSC_CLOCK_IF_MIXINS_H_
#define _SYSC_CLOCK_IF_MIXINS_H_

#include <systemc>

namespace scc {

template<typename BASE>
class ticking_clock : BASE {
public:
    sc_core::sc_in<bool> clk_i{"clk_i"};

    ticking_clock(sc_core::sc_module_name& nm) : BASE(nm) {}

protected:
    void end_of_elaboration() override{
        auto clk_if=dynamic_cast<sc_core::sc_clock*>(clk_i.get_interface());
        sc_assert(clk_if!=nullptr);
        this->set_clock_period(clk_if->period());
    }
};

template<typename BASE>
class tickless_clock : BASE {
public:
    sc_core::sc_in<sc_core::sc_time> clk_i;

    tickless_clock(sc_core::sc_module_name& nm) : BASE(nm) {
        SC_HAS_PROCESS(tickless_clock<BASE>);
        SC_METHOD(clock_cb);
        sensitive << clk_i;
    }

private:
    void clock_cb() { this->set_clock_period(clk_i.read()); }
};
}
#endif // _SYSC_CLOCK_IF_MIXINS_H_
