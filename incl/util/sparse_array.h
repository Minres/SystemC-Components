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

#include <array>
#include <cassert>

namespace util {

/**
 *  a simple array which allocates memory in configurable chunks (size of 2^lower_width), used for
 *  large sparse arrays. Memory is allocated on demand
 */
template <typename T, uint64_t SIZE, int lower_width = 24> class sparse_array {
public:
    const uint64_t page_addr_mask = (1 << lower_width) - 1;

    const uint64_t page_size = (1 << lower_width);

    const unsigned page_count = 1 + SIZE / page_size;

    const uint64_t page_addr_width = lower_width;

    using page_type = std::array<T, 1 << lower_width>;
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
        T nr = addr >> lower_width;
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
            arr.at(page_nr) = new page_type();
        return *(arr[page_nr]);
    }
    /**
     * check if page for address is allocated
     *
     * @param addr the address to check
     * @return true if the page is allocated
     */
    bool is_allocated(uint32_t addr) {
        assert(addr < SIZE);
        T nr = addr >> lower_width;
        return arr.at(nr) != nullptr;
    }
    /**
     * get the size of the array
     *
     * @return the size
     */
    uint64_t size() { return SIZE; }

protected:
    std::array<page_type*, SIZE / (1 << lower_width) + 1> arr;
};
} // namespace util

#endif /* _SPARSE_ARRAY_H_ */
