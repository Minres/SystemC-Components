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

#ifndef _SCC_TRACABLE_H_
#define _SCC_TRACABLE_H_

namespace sc_core {
class sc_trace_file;
}

namespace scc {
/**
 * interface defining a traceable component, this overlaps with the trace function of sc_core::sc_object
 * in fact it is a signalling interface
 */
class traceable {
public:
    /**
     * the destructor
     */
    virtual ~traceable() = default;
    /**
     * trace the elements of the object to the trace file
     *
     * @param trf the tracefile to use
     */
    virtual void trace(sc_core::sc_trace_file *trf) const = 0;
};

} /* namespace scc */

#endif /* _SCC_TRACABLE_H_ */
