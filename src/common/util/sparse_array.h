/*******************************************************************************
 * Copyright 2017 MINRES Technologies GmbH
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

#ifndef _SPARSE_ARRAY_H_
#define _SPARSE_ARRAY_H_

#include "ities.h"
#include <array>
#include <cassert>

/**
 * \ingroup scc-common
 */
/**@{*/
//! @brief SCC common utilities
namespace util {

/**
 *  @brief a sparse array suitable for large sizes with compile time constants for performance
 *
 *  a simple array which allocates memory in configurable chunks (size of 2^PAGE_ADDR_BITS), used for
 *  large sparse arrays. Memory is allocated on demand
 */
template <typename T, uint64_t SIZE, unsigned PAGE_ADDR_BITS = 24> class sparse_array {
public:
    static_assert(SIZE > 0, "sparse_array size must be greater than 0");

    static constexpr uint64_t page_addr_mask = (1 << PAGE_ADDR_BITS) - 1;

    static constexpr uint64_t page_size = (1 << PAGE_ADDR_BITS);

    static constexpr uint64_t page_count = (SIZE + page_size - 1) / page_size;

    static constexpr uint64_t page_addr_width = PAGE_ADDR_BITS;

    using page_type = std::array<T, 1 << PAGE_ADDR_BITS>;
    /**
     * the default constructor
     */
    sparse_array() { arr.fill(nullptr); }
    /**
     * the destructor
     */
    ~sparse_array() {
        for(auto i : arr)
            delete i;
    }
    /**
     * element access operator
     *
     * @param addr address to access
     * @return the data type reference
     */
    T& operator[](uint32_t addr) {
        assert(addr < SIZE);
        T nr = addr >> PAGE_ADDR_BITS;
        if(arr[nr] == nullptr)
            arr[nr] = new page_type();
        return arr[nr]->at(addr & page_addr_mask);
    }
    /**
     * page fetch operator
     *
     * @param page_nr the page number ot fetch
     * @return reference to page
     */
    page_type& operator()(uint64_t page_nr) {
        assert(page_nr < page_count);
        if(arr[page_nr] == nullptr)
            arr.at(page_nr) = new page_type();
        return *(arr[page_nr]);
    }
    /**
     * check if page for address is allocated
     *
     * @param addr the address to check
     * @return true if the page is allocated
     */
    bool is_allocated(uint64_t addr) {
        assert(addr < SIZE);
        uint64_t nr = addr >> PAGE_ADDR_BITS;
        return arr.at(nr) != nullptr;
    }
    /**
     * get the size of the array
     *
     * @return the size
     */
    uint64_t size() { return SIZE; }

protected:
    std::array<page_type*, SIZE / (1 << PAGE_ADDR_BITS) + 1> arr;
};
/**
 *  @brief a sparse array suitable for large sizes as specialization of the compile time fixed one
 *
 *  a simple array which allocates memory in configurable chunks (size of 2^PAGE_ADDR_BITS), used for
 *  large sparse arrays. Memory is allocated on demand
 */
template <typename T> class sparse_array<T, 0, 24> {
public:
    const uint64_t mem_size;

    const uint64_t page_addr_mask;

    const uint64_t page_size;

    const unsigned page_count;

    const uint64_t page_addr_width;

    using page_type = std::vector<T>;
    /**
     * the default constructor
     */
    sparse_array(uint64_t SIZE, unsigned PAGE_ADDR_BITS = 24)
    : mem_size{SIZE}
    , page_addr_mask{(1ULL << PAGE_ADDR_BITS) - 1}
    , page_size{(1ULL << PAGE_ADDR_BITS)}
    , page_count{(SIZE + page_size - 1) / page_size}
    , page_addr_width{PAGE_ADDR_BITS}
    , arr{SIZE / (1 << PAGE_ADDR_BITS) + 1} {
        assert(SIZE > 0 && "sparse_array size must be greater than 0");
        arr.fill(nullptr);
    }
    /**
     * the destructor
     */
    ~sparse_array() {
        for(auto i : arr)
            delete i;
    }
    /**
     * element access operator
     *
     * @param addr address to access
     * @return the data type reference
     */
    T& operator[](uint32_t addr) {
        assert(addr < mem_size);
        T nr = addr >> page_addr_width;
        if(arr[nr] == nullptr)
            arr[nr] = new page_type();
        return arr[nr]->at(addr & page_addr_mask);
    }
    /**
     * page fetch operator
     *
     * @param page_nr the page number ot fetch
     * @return reference to page
     */
    page_type& operator()(uint32_t page_nr) {
        assert(page_nr < page_count);
        if(arr[page_nr] == nullptr)
            arr.at(page_nr) = new page_type(page_size);
        return *(arr[page_nr]);
    }
    /**
     * check if page for address is allocated
     *
     * @param addr the address to check
     * @return true if the page is allocated
     */
    bool is_allocated(uint32_t addr) {
        assert(addr < mem_size);
        T nr = addr >> page_addr_width;
        return arr.at(nr) != nullptr;
    }
    /**
     * get the size of the array
     *
     * @return the size
     */
    uint64_t size() { return mem_size; }

protected:
    std::vector<page_type*> arr;
};

} // namespace util
/** @}*/
#endif /* _SPARSE_ARRAY_H_ */
