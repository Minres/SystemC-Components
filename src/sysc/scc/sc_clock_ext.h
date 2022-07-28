/*******************************************************************************
 * Copyright 2022 MINRES Technologies GmbH
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

#ifndef _SCC_SC_CLOCK_EXT_H_
#define _SCC_SC_CLOCK_EXT_H_

#include <sysc/communication/sc_clock.h>

/** \ingroup scc-sysc
 *  @{
 */
/**@{*/
//! @brief SCC SystemC utilities
namespace scc {
/**
 * \brief A clock source with construction time configurable start delay
 */
struct sc_clock_ext : public sc_core::sc_clock {

    sc_core::sc_attribute<sc_core::sc_time> period;
    sc_core::sc_attribute<double> duty_cycle;
    sc_core::sc_attribute<sc_core::sc_time> initial_delay;

    sc_clock_ext( const char* name_, const sc_core::sc_time& period_, double duty_cycle_ = 0.5, const sc_core::sc_time& start_time_ = sc_core::SC_ZERO_TIME, bool posedge_first_ = true )
    : sc_core::sc_clock(name_, period_, duty_cycle_, start_time_, posedge_first_)
    , period("period", period_)
    , duty_cycle("duty_cycle", duty_cycle_)
    , initial_delay("start_time", start_time_)
    {
        add_attribute(initial_delay);
    }

    virtual ~sc_clock_ext() = default;

protected:
    void end_of_elaboration() override {
        init(period.value, duty_cycle.value, initial_delay.value, m_posedge_first);
        if(initial_delay.value!=m_start_time) {
            if( m_posedge_first ) {
                m_next_posedge_event.cancel();
                m_next_posedge_event.notify( initial_delay.value );
            } else {
                m_next_negedge_event.cancel();
                m_next_negedge_event.notify( initial_delay.value );
            }
        }
    }
};
}
/** @} */ // end of scc-sysc
#endif //
