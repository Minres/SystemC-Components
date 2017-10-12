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
 * resettable.h
 *
 *  Created on: Nov 16, 2016
 *      Author: developer
 */

#ifndef _SYSC_RESETTABLE_H_
#define _SYSC_RESETTABLE_H_

#include <vector>
#include "scc/resource_access_if.h"

namespace scc {

class resetable {
public:
    /**
     *
     */
    virtual ~resetable() = default;
    /**
     *
     */
    void reset_start() {
        _in_reset = true;
        for (auto res : resources) res->reset();
    }
    /**
     *
     */
    void reset_stop() {
        for (auto res : resources) res->reset();
        _in_reset = false;
    }
    bool in_reset() { return _in_reset; }
    /**
     *
     * @param res
     */
    void register_resource(resource_access_if *res) { resources.push_back(res); }

protected:
    std::vector<resource_access_if *> resources;
    bool _in_reset = false;
};

} /* namespace scc */

#endif /* _SYSC_RESETTABLE_H_ */
