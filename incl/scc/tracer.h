/*******************************************************************************
 * Copyright 2016, 2018 MINRES Technologies GmbH
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

#ifndef _SCC_TRACER_H_
#define _SCC_TRACER_H_

#include "tracer_base.h"
#ifdef WITH_SCV
#include <scv.h>
#endif
#include <string>
#include <vector>

namespace sc_core {
class sc_object;
class sc_trace_file;
}

namespace scc {
/**
 * a component traversing the SystemC object hierarchy and tracing the objects
 */
class tracer : public tracer_base {
public:
    /**
     * enum defining the transaction trace output type
     */
    enum file_type { NONE, TEXT, COMPRESSED, SQLITE/*, BINARY, LEVEL*/ };
    /**
     * the constructor
     *
     * @param name base name of the trace file(s)
     * @param type type of trace file for transactions
     * @param enable enable VCD (signal and POD) tracing
     */
    tracer(const std::string &&, file_type, bool = true);
    /**
     * the destructor
     */
    virtual ~tracer() override;

protected:
    void end_of_elaboration() override;
    bool enabled;
#ifdef WITH_SCV
    scv_tr_db *txdb;
#endif
};

} /* namespace scc */

#endif /* _SCC_TRACER_H_ */
