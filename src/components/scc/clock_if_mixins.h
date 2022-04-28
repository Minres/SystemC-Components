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
#ifdef CWR_SYSTEMC
#include "scml_clock/scml_clock_if.h"
#endif

namespace scc {

template <typename BASE>
class ticking_clock : public BASE
#ifdef CWR_SYSTEMC
,
                      public scml_clock_observer
#endif
{
public:
    sc_core::sc_in<bool> clk_i{"clk_i"};

    ticking_clock(sc_core::sc_module_name const& nm)
    : BASE(nm) {}

protected:
    void end_of_elaboration() override {
#ifdef CWR_SYSTEMC
        if(auto scml_clk_if = scml2::get_scml_clock(clk_i)) {
            scml_clk_if->register_observer(this);
            handle_clock_parameters_updated(scml_clk_if);
            BASE::end_of_elaboration();
            return;
        }
#endif
        auto clk_if = dynamic_cast<sc_core::sc_clock*>(clk_i.get_interface());
        sc_assert(clk_if != nullptr);
        this->set_clock_period(clk_if->period());
        BASE::end_of_elaboration();
    }
#ifdef CWR_SYSTEMC
    void handle_clock_parameters_updated(scml_clock_if* clk_if) override {
        this->set_clock_period(clk_if->get_period());
    }
    void handle_clock_deleted(scml_clock_if*) override{};
#endif
};

template <typename BASE> class tickless_clock : public BASE {
public:
    sc_core::sc_in<sc_core::sc_time> clk_i;

    tickless_clock(sc_core::sc_module_name const& nm)
    : BASE(nm) {
        SC_HAS_PROCESS(tickless_clock<BASE>);
        SC_METHOD(clock_cb);
        this->sensitive << clk_i;
    }

private:
    void clock_cb() { this->set_clock_period(clk_i.read()); }
};
} // namespace scc
#endif // _SYSC_CLOCK_IF_MIXINS_H_
