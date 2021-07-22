/*******************************************************************************
 * Copyright 2020 MINRES Technologies GmbH
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

#ifndef _TLM_TLM_MM_H_
#define _TLM_TLM_MM_H_

#include <tlm>
#include <util/pool_allocator.h>

namespace tlm {
namespace scc {
/**
 * @class tlm_mm
 * @brief a tlm memory manager
 *
 * This memory manager can be used as singleton or as local memory manager. It uses the pool_allocator
 * as singleton to maximize reuse
 */
template <typename TYPES = tlm_base_protocol_types> class tlm_mm : public tlm::tlm_mm_interface {
    using payload_type = typename TYPES::tlm_payload_type;

public:
    /**
     * @brief accessor function of the singleton
     * @return
     */
    static tlm_mm& get();

    tlm_mm()
    : allocator(util::pool_allocator<payload_type>::get()) {}

    tlm_mm(const tlm_mm&) = delete;

    tlm_mm(tlm_mm&&) = delete;

    tlm_mm& operator=(const tlm_mm& other) = delete;

    tlm_mm& operator=(tlm_mm&& other) = delete;

    ~tlm_mm() = default;
    /**
     * @brief get a plain tlm_payload_type without extensions
     * @return the tlm_payload_type
     */
    payload_type* allocate();
    /**
     * @brief get a tlm_payload_type with registered extension
     * @return the tlm_payload_type
     */
    template <typename PEXT> payload_type* allocate() {
        auto* ptr = allocate();
        ptr->set_auto_extension(new PEXT);
        return ptr;
    }
    /**
     * @brief return the extension into the memory pool (removing the extensions)
     * @param trans the returning transaction
     */
    void free(tlm::tlm_generic_payload* trans) override;

private:
    util::pool_allocator<payload_type>& allocator;
};

template <typename TYPES> tlm_mm<TYPES>& tlm_mm<TYPES>::get() {
    static tlm_mm<TYPES> mm;
    return mm;
}

template <typename TYPES> typename tlm_mm<TYPES>::payload_type* tlm_mm<TYPES>::allocate() {
    auto* ptr = allocator.allocate();
    return new(ptr) payload_type(this);
}

template <typename TYPES> void tlm_mm<TYPES>::free(tlm::tlm_generic_payload* trans) {
    if(trans->get_data_ptr())
        delete[] trans->get_data_ptr();
    trans->set_data_ptr(nullptr);
    trans->reset();
    trans->~tlm_generic_payload();
    allocator.free(trans);
}

} // namespace scc
} // namespace tlm
#endif /* _TLM_TLM_MM_H_ */
