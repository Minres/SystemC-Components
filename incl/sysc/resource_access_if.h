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
 * resource_access_if.h
 *
 *  Created on: Nov 16, 2016
 *      Author: developer
 */

#ifndef _SYSC_RESOURCE_ACCESS_IF_H_
#define _SYSC_RESOURCE_ACCESS_IF_H_

#include <cstddef>
#include <cstdint>

namespace sysc {

struct resource_access_if {
    virtual ~resource_access_if(){}
    virtual size_t size() const = 0;
    virtual void reset() = 0;
    // functional accesses
    virtual bool write(const uint8_t* data, size_t length) = 0;
    virtual bool read(uint8_t* data, size_t length) const = 0;
    // non-functional/debug accesses
    virtual bool write_dbg(const uint8_t* data, size_t length) = 0;
    virtual bool read_dbg(uint8_t* data, size_t length) const = 0;
};

}

#endif /* _SYSC_RESOURCE_ACCESS_IF_H_ */
