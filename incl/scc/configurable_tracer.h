/*******************************************************************************
 * Copyright 2016 MINRES Technologies GmbH
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
/*
 * tracer.h
 *
 *  Created on: Nov 9, 2016
 *      Author: developer
 */

#ifndef _SCC_CONFIGURABLE_TRACER_H_
#define _SCC_CONFIGURABLE_TRACER_H_

#include "scc/tracer.h"
#include <cci_configuration>

namespace scc {

class configurable_tracer : public scc::tracer {
public:
    /**
     *
     * @param name basename of the trace file(s)
     * @param type type of trace file for transactions
     * @param enable enable VCD (signal based) tracing
     */
    configurable_tracer(std::string &&, file_type, bool = true);

    ~configurable_tracer();

protected:
    void descend(const sc_core::sc_object *) override;
    bool get_trace_enabled(const sc_core::sc_object*, bool =false );
    void augment_object_hierarchical(const sc_core::sc_object*);
    cci::cci_originator cci_originator;
    cci::cci_broker_handle cci_broker;
    std::vector<cci::cci_param_untyped*> params;
};

} /* namespace scc */
#endif /* _SCC_CONFIGURABLE_TRACER_H_ */
