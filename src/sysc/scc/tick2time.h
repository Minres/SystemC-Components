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

#ifndef _SCC_TICK2TIME_H_
#define _SCC_TICK2TIME_H_

//clang-format off
#include "utilities.h"
#include <systemc>
#ifdef CWR_SYSTEMC
#include "scml_clock/scml_clock_if.h"
#endif
//clang-format on

/** \ingroup scc-sysc
 *  @{
 */
/**@{*/
//! @brief SCC SystemC utilities
namespace scc {
/**
 * @struct time2tick
 * @brief translate a tick-less clock (sc_time based) to boolean clock
 *
 */
struct tick2time : public sc_core::sc_module
#ifdef CWR_SYSTEMC
,
                   public scml_clock_observer
#endif
{
    //! the clock output
    sc_core::sc_out<sc_core::sc_time> clk_o{"clk_o"};
    //! the clock input
    sc_core::sc_in<bool> clk_i{"clk_i"};
    /**
     * the constructor
     *
     * @param nm the name
     */
    explicit tick2time(sc_core::sc_module_name nm);

protected:
    void end_of_elaboration() override;

private:
    sc_core::sc_time clk_period;
    sc_core::sc_time last_tick;
#ifdef CWR_SYSTEMC
    void handle_clock_parameters_updated(scml_clock_if* clk_if);
    void handle_clock_deleted(scml_clock_if*) override;
#endif
};
} // namespace scc
/** @} */ // end of scc-sysc
#endif /* _SCC_TICK2TIME_H_ */
