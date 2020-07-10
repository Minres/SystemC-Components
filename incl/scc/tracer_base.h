/*******************************************************************************
 * Copyright 2019 MINRES Technologies GmbH
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

#ifndef _SCC_TRACER_BASE_H_
#define _SCC_TRACER_BASE_H_

#include "utilities.h"
#include <sysc/tracing/sc_trace.h>
#include <type_traits>

namespace scc {
enum class trace_types: unsigned {
    NONE=0x0, SIGNALS=0x1, PORTS=0x2, SOCKETS=0x4, VARIABLES=0x8, OBJECTS=0x10, ALL=0xff
};

inline trace_types operator | (trace_types lhs, trace_types rhs) {
    return static_cast<trace_types>(static_cast<unsigned>(lhs) | static_cast<unsigned>(rhs));
}

inline trace_types operator & (trace_types lhs, trace_types rhs) {
    return static_cast<trace_types>(static_cast<unsigned>(lhs) & static_cast<unsigned>(rhs));
}

class tracer_base : public sc_core::sc_module {
public:
    tracer_base(const sc_core::sc_module_name& nm)
    : sc_core::sc_module(nm) {}

    tracer_base(const sc_core::sc_module_name& nm, sc_core::sc_trace_file* tf)
    : sc_core::sc_module(nm), trf(tf) {}

    void set_trace_types(trace_types t){
        types_to_trace=t;
    }

protected:
    virtual void descend(const sc_core::sc_object*, bool trace_all = false);

    static void try_trace(sc_core::sc_trace_file* trace_file, const sc_core::sc_object* object, trace_types t);

    sc_core::sc_trace_file* trf{nullptr};

    trace_types types_to_trace{trace_types::ALL};
};

} // namespace scc
#endif /* _SCC_TRACER_BASE_H_ */
