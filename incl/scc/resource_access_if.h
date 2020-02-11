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

#ifndef _SYSC_RESOURCE_ACCESS_IF_H_
#define _SYSC_RESOURCE_ACCESS_IF_H_

#include <cstddef>
#include <cstdint>
#include <sysc/kernel/sc_time.h>

namespace scc {
/**
 * interface defining access to a resource
 */
class resource_access_if {
public:
    /**
     * the destructor
     */
    virtual ~resource_access_if() = default;
    /**
     * return the size of the resource
     *
     * @return the size
     */
    virtual std::size_t size() const = 0;
    /**
     * reset the resource
     */
    virtual void reset() = 0;
    // functional accesses
    /**
     * write to the resource
     *
     * @param data data to write
     * @param length length of data to write
     * @param offset offset of the data to write
     * @param d the annotated delay
     * @return true it the access is successful
     */
    virtual bool write(const uint8_t* data, std::size_t length, uint64_t offset = 0,
                       sc_core::sc_time d = sc_core::SC_ZERO_TIME) = 0;
    /**
     * read the data from the resource
     *
     * @param data buffer to read the data into
     * @param length length of data to read
     * @param offset offset of the data to read
     * @param d the annotated delay
     * @return true it the access is successful
     */
    virtual bool read(uint8_t* data, std::size_t length, uint64_t offset = 0,
                      sc_core::sc_time d = sc_core::SC_ZERO_TIME) const = 0;
    // non-functional/debug accesses
    /**
     * debug write to the resource
     *
     * @param data data to write
     * @param length length of data to write
     * @param offset offset of the data to write
     * @return true it the access is successful
     */
    virtual bool write_dbg(const uint8_t* data, std::size_t length, uint64_t offset = 0) = 0;
    /**
     * debug read the data from the resource
     *
     * @param data buffer to read the data into
     * @param length length of data to read
     * @param offset offset of the data to read
     * @return true it the access is successful
     */
    virtual bool read_dbg(uint8_t* data, std::size_t length, uint64_t offset = 0) const = 0;
};
/**
 * interface defining access to an indexed resource
 */
class indexed_resource_access_if {
public:
    using value_type = resource_access_if;
    using pointer = value_type*;
    using const_pointer = const value_type*;
    using reference = value_type&;
    using const_reference = const value_type&;
    using iterator = resource_access_if*;
    using const_iterator = const resource_access_if*;
    /**
     * the destructor
     */
    virtual ~indexed_resource_access_if() = default;
    /**
     * get the size of the resource
     * @return
     */
    virtual std::size_t size() = 0;
    /**
     * Element access.
     *
     * @param idx
     * @return the data
     */
    virtual reference operator[](std::size_t idx) noexcept = 0;
    /**
     * const element access.
     *
     * @param idx
     * @return the data
     */
    virtual const_reference operator[](std::size_t idx) const noexcept = 0;
    /**
     * Element access.
     *
     * @param idx
     * @return the data
     */
    virtual reference at(std::size_t idx) = 0;
    /**
     * const element access.
     *
     * @param idx
     * @return the data
     */
    virtual const_reference at(std::size_t idx) const = 0;
};
} // namespace scc
#endif /* _SYSC_RESOURCE_ACCESS_IF_H_ */
