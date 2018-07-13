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
 * tracable.h
 *
 *  Created on: Nov 9, 2016
 *      Author: developer
 */

#ifndef _SCC_TRACABLE_H_
#define _SCC_TRACABLE_H_

namespace sc_core {
class sc_trace_file;
}

namespace scc {

class traceable {
public:
    /**
     *
     */
    virtual ~traceable() = default;
    /**
     *
     * @param trf the tracefile to use
     */
    virtual void trace(sc_core::sc_trace_file *trf) = 0;
};

} /* namespace scc */

#endif /* _SCC_TRACABLE_H_ */
