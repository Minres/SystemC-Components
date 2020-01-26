/*
 * tlm_mm.h
 *
 *  Created on: 17.12.2019
 *      Author: developer
 */

#ifndef _TLM_TLM_MM_H_
#define _TLM_TLM_MM_H_

#include <util/pool_allocator.h>
#include <tlm>

namespace tlm {

/**
 * a tlm memory manager which can be used as singleton or as local memory manager. It uses the pool_allocator
 * as singleton.
 */
template<typename TYPES = tlm_base_protocol_types>
class tlm_mm: public tlm::tlm_mm_interface {
    using payload_type = typename TYPES::tlm_payload_type;
public:
    /**
     * @brief accessor function of the singleton
     * @return
     */
    static tlm_mm& get();
    /**
     * @brief default constructor
     * @param
     */
    tlm_mm():allocator(util::pool_allocator<payload_type>::get()){}
    /**
     * @brief deleted copy constructor
     * @param
     */
    tlm_mm(const tlm_mm& ) = delete;
    /**
     * @brief deleted move copy constructor
     * @param
     */
    tlm_mm(tlm_mm&& ) = delete;
    /**
     * @brief deleted copy assignment
     * @param
     */
    tlm_mm& operator=(const tlm_mm& other) = delete;
    /**
     * @brief deleted move assignment
     * @param
     */
    tlm_mm& operator=(tlm_mm&& other) = delete;
    /**
     * @brief get a plain tlm_payload_type without extensions
     * @return the tlm_payload_type
     */
    payload_type* allocate();
    /**
     * @brief get a tlm_payload_type with registered extension
     * @return the tlm_payload_type
     */
    template<typename PEXT>
    payload_type* allocate(){
        auto* ptr = allocate();
        ptr->set_auto_extension(new PEXT);
        return ptr;
    }
    /**
     * @brief return the extension into the memory pool (removing the extensions)
     * @param trans the returning transaction
     */
    void  free(payload_type* trans) override;

private:
    util::pool_allocator<payload_type>& allocator;
};

template<typename TYPES>
tlm_mm<TYPES>& tlm_mm<TYPES>::get() {
    static tlm_mm<TYPES> mm;
    return mm;
}

template<typename TYPES>
typename tlm_mm<TYPES>::payload_type* tlm_mm<TYPES>::allocate() {
    auto* ptr = allocator.allocate();
    return new (ptr) payload_type(this);
}

template<typename TYPES>
void tlm_mm<TYPES>::free(payload_type* trans) {
    trans->reset();
    trans->~payload_type();
    allocator.free(trans);
}

}
#endif /* _TLM_TLM_MM_H_ */
