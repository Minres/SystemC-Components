/*******************************************************************************
 * Copyright 2017, 2018 MINRES Technologies GmbH
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

#include "utilities.h"

namespace sc_core {

#ifdef HAS_NO_TIME_TRACE
void sc_trace(sc_trace_file* tf, const sc_time& t, const std::string& name) {
    sc_trace(tf, reinterpret_cast<const sc_core::sc_time::value_type*>(&t), name);
}

#endif
template <> void sc_trace(sc_trace_file* tf, const sc_in<sc_time>& port, const std::string& name) {
    const sc_signal_in_if<sc_time>* iface = nullptr;
    if(sc_get_curr_simcontext()->elaboration_done()) {
        iface = dynamic_cast<const sc_signal_in_if<sc_time>*>(port.get_interface());
    }

    if(iface)
        sc_trace(tf, iface->read(), name);
    else
        port.add_trace_internal(tf, name);
}

template <> void sc_trace(sc_trace_file* tf, const sc_inout<sc_time>& port, const std::string& name) {
    const sc_signal_in_if<sc_time>* iface = nullptr;
    if(sc_get_curr_simcontext()->elaboration_done()) {
        iface = dynamic_cast<const sc_signal_in_if<sc_time>*>(port.get_interface());
    }

    if(iface)
        sc_trace(tf, iface->read(), name);
    else
        port.add_trace_internal(tf, name);
}
} // namespace sc_core
