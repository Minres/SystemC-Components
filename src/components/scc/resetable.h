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

#ifndef _SYSC_RESETTABLE_H_
#define _SYSC_RESETTABLE_H_

#include "resource_access_if.h"
#include <vector>

namespace scc {
/**
 * @class resetable
 * @brief base class for components having a reset
 *
 * This class forms a reset domain which is a set of components (sc_modules) having the same reset.
 * sc_modules using sc_registers should inherit from this class as well so that all registers get the respective reset.
 *
 */
class resetable {
public:
    virtual ~resetable() = default;
    /**
     * @fn void reset_start()
     * @brief distributes the begin of the reset to all registered components and set the reset state
     *
     */
    virtual void reset_start() {
        _in_reset = true;
        for(auto res : resources)
            res->reset();
    }
    /**
     * @fn void reset_stop()
     * @brief distributes the end of the reset to all registered components and finishes the reset state
     *
     */
    virtual void reset_stop() {
        for(auto res : resources)
            res->reset();
        _in_reset = false;
    }
    /**
     * @fn bool in_reset()
     * @brief get the current state of this reset domain
     *
     * @return true if reset state is active
     */
    bool in_reset() { return _in_reset; }
    /**
     * @fn void register_resource(resource_access_if*)
     * @brief register a resource with this reset domain
     *
     * @param res the resource belonging to this reset domain
     */
    void register_resource(resource_access_if* res) { resources.push_back(res); }

protected:
    std::vector<resource_access_if*> resources;
    bool _in_reset = false;
};

} /* namespace scc */

#endif /* _SYSC_RESETTABLE_H_ */
