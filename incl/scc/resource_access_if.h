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

namespace scc {

class resource_access_if {
public:
    /**
     *
     */
    virtual ~resource_access_if() = default;
    /**
     *
     * @return
     */
    virtual std::size_t size() const = 0;
    /**
     *
     */
    virtual void reset() = 0;
    // functional accesses
    /**
     *
     * @param data
     * @param length
     * @param offset
     * @return
     */
    virtual bool write(const uint8_t *data, std::size_t length, uint64_t offset = 0) = 0;
    /**
     *
     * @param data
     * @param length
     * @param offset
     * @return
     */
    virtual bool read(uint8_t *data, std::size_t length, uint64_t offset = 0) const = 0;
    // non-functional/debug accesses
    /**
     *
     * @param data
     * @param length
     * @param offset
     * @return
     */
    virtual bool write_dbg(const uint8_t *data, std::size_t length, uint64_t offset = 0) = 0;
    /**
     *
     * @param data
     * @param length
     * @param offset
     * @return
     */
    virtual bool read_dbg(uint8_t *data, std::size_t length, uint64_t offset = 0) const = 0;
};

class indexed_resource_access_if {
public:
    using value_type = resource_access_if;
    using pointer = value_type *;
    using const_pointer = const value_type *;
    using reference = value_type &;
    using const_reference = const value_type &;
    using iterator = resource_access_if *;
    using const_iterator = const resource_access_if *;
    /**
     *
     */
    virtual ~indexed_resource_access_if() = default;
    /**
     *
     * @return
     */
    virtual std::size_t size() = 0;
    // Element access.
    /**
     *
     * @param __n
     * @return
     */
    virtual reference operator[](std::size_t __n) noexcept = 0;
    /**
     *
     * @param __n
     * @return
     */
    virtual const_reference operator[](std::size_t __n) const noexcept = 0;
    /**
     *
     * @param __n
     * @return
     */
    virtual reference at(std::size_t __n) = 0;
    /**
     *
     * @param __n
     * @return
     */
    virtual const_reference at(std::size_t __n) const = 0;
};
}
#endif /* _SYSC_RESOURCE_ACCESS_IF_H_ */
