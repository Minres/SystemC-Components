/*******************************************************************************
 * Copyright (C) 2017, MINRES Technologies GmbH
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * Contributors:
 *       eyck@minres.com - initial API and implementation
 ******************************************************************************/

#ifndef _SPARSE_ARRAY_H_
#define _SPARSE_ARRAY_H_

#include <array>
#include <cassert>

namespace util {

/**
 *  a simple array which allocates memory in 16MB chunks
 */
template <typename T, uint64_t SIZE, int lower_width = 24> struct sparse_array {

    const uint64_t page_addr_mask = (1 << lower_width) - 1;

    const uint64_t page_size = (1 << lower_width);

    const uint64_t page_addr_width = lower_width;

    using page_type = std::array<T, 1 << lower_width>;

    sparse_array() { arr.fill(nullptr); }

    ~sparse_array() {
        for (auto i : arr) delete i;
    }

    T &operator[](uint32_t addr) {
        assert(addr < SIZE);
        T nr = addr >> lower_width;
        if (arr[nr] == nullptr) arr[nr] = new page_type();
        return arr[nr]->at(addr & page_addr_mask);
    }

    page_type &operator()(uint32_t page_nr) {
        assert(page_nr < (SIZE / (1 << lower_width)));
        if (arr[page_nr] == nullptr) arr[page_nr] = new page_type();
        return *(arr[page_nr]);
    }

    uint64_t size() { return SIZE; }

protected:
    std::array<page_type *, SIZE / (1 << lower_width) + 1> arr;
};
}

#endif /* _SPARSE_ARRAY_H_ */
