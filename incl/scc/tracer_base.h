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

namespace scc {

class tracer_base : public sc_core::sc_module {
public:
    tracer_base(const sc_core::sc_module_name& nm)
    : sc_core::sc_module(nm)
    , trf(nullptr) {}

protected:
    virtual void descend(const sc_core::sc_object*, bool trace_all = false);

    static void try_trace(sc_core::sc_trace_file* trace_file, const sc_object* object);

    sc_core::sc_trace_file* trf;
};

} // namespace scc
#endif /* _SCC_TRACER_BASE_H_ */
