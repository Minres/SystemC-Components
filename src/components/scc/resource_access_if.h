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
 * @class resource_access_if
 * @brief interface defining access to a resource e.g. a register
 *
 */
class resource_access_if {
public:
    virtual ~resource_access_if() = default;
    /**
     * @fn std::size_t size()const =0
     * @brief return the size of the resource
     *
     * @return the size
     */
    virtual std::size_t size() const = 0;
    /**
     * @fn void reset()=0
     * @brief reset the resource
     *
     */
    virtual void reset() = 0;
    /**
     * @fn bool write(const uint8_t*, std::size_t, uint64_t=0, sc_core::sc_time=sc_core::SC_ZERO_TIME)=0
     * @brief write to the resource
     *
     * @param data  to write
     * @param length  of data to write
     * @param offset  of the data to write
     * @param d the annotated delay
     * @return true it the access is successful
     */
    virtual bool write(const uint8_t* data, std::size_t length, uint64_t offset = 0,
                       sc_core::sc_time d = sc_core::SC_ZERO_TIME) = 0;
    /**
     * @fn bool read(uint8_t*, std::size_t, uint64_t=0, sc_core::sc_time=sc_core::SC_ZERO_TIME)const =0
     * @brief read the data from the resource
     *
     * @param data buffer to read the data into
     * @param length  of data to read
     * @param offset  of the data to read
     * @param d the annotated delay
     * @return true it the access is successful
     */
    virtual bool read(uint8_t* data, std::size_t length, uint64_t offset = 0,
                      sc_core::sc_time d = sc_core::SC_ZERO_TIME) const = 0;
    /**
     * @fn bool write_dbg(const uint8_t*, std::size_t, uint64_t=0)=0
     * @brief debug write to the resource
     *
     * @param data to write
     * @param length of data to write
     * @param offset of the data to write
     * @return true it the access is successful
     */
    virtual bool write_dbg(const uint8_t* data, std::size_t length, uint64_t offset = 0) = 0;
    /**
     * @fn bool read_dbg(uint8_t*, std::size_t, uint64_t=0)const =0
     * @brief debug read the data from the resource
     *
     * @param data buffer to read the data into
     * @param length length of data to read
     * @param offset offset of the data to read
     * @return true it the access is successful
     */
    virtual bool read_dbg(uint8_t* data, std::size_t length, uint64_t offset = 0) const = 0;
};
/**
 * @class indexed_resource_access_if
 * @brief interface defining access to an indexed resource e.g. register file
 *
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

    virtual ~indexed_resource_access_if() = default;
    /**
     * @fn std::size_t size()=0
     * @brief get the size of the resource
     *
     * @return size of the resource in bytes
     */
    virtual std::size_t size() = 0;
    /**
     * @fn reference operator [](std::size_t)=0
     * @brief get value at index
     *
     * @param idx the index
     * @return the value
     */
    virtual reference operator[](std::size_t idx) noexcept = 0;
    /**
     * @fn const_reference operator [](std::size_t)const =0
     * @brief get value at index
     *
     * @param idx the index
     * @return the value
     */
    virtual const_reference operator[](std::size_t idx) const noexcept = 0;
    /**
     * @fn reference operator [](std::size_t)=0
     * @brief get value at index with range checking
     *
     * @param idx the index
     * @return the value
     */
    virtual reference at(std::size_t idx) = 0;
    /**
     * @fn const_reference operator [](std::size_t)const =0
     * @brief get value at index with range checking
     *
     * @param idx the index
     * @return the value
     */
    virtual const_reference at(std::size_t idx) const = 0;
};
} // namespace scc
#endif /* _SYSC_RESOURCE_ACCESS_IF_H_ */
