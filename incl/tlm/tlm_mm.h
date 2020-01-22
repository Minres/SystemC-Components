/*
 * tlm_mm.h
 *
 *  Created on: 17.12.2019
 *      Author: developer
 */

#ifndef _TLM_TLM_MM_H_
#define _TLM_TLM_MM_H_

#include <tlm>

namespace tlm {
template<typename TYPES = tlm_base_protocol_types>
class tlm_mm: public tlm::tlm_mm_interface {
    using gp_t = typename TYPES::tlm_payload_type;
public:
    /**
     * @brief accessor function of the singleton
     * @return
     */
    static tlm_mm& get();
    /**
     * @brief remove the copy constructor
     * @param
     */
    tlm_mm(tlm_mm& ) = delete;
    /**
     * @brief get a plain tlm_payload_type without extensions
     * @return the tlm_payload_type
     */
    gp_t* allocate();
    /**
     * @brief get a tlm_payload_type with registered extension
     * @return the tlm_payload_type
     */
    template<typename PEXT>
    gp_t* allocate(){
        gp_t* ptr = allocate();
        ptr->set_auto_extension(new PEXT);
        return ptr;
    }
    /**
     * @brief return the extension into the memory pool (removing the extensions)
     * @param trans the returning transaction
     */
    void  free(gp_t* trans) override;

private:
    tlm_mm(){}
    struct access{
        gp_t* trans;
        tlm_mm::access* next;
        tlm_mm::access* prev;
    };
    access* free_list = nullptr;
    access* empties = nullptr;
};

template<typename TYPES>
tlm_mm<TYPES>& tlm_mm<TYPES>::get() {
    static tlm_mm<TYPES> mm;
    return mm;
}

template<typename TYPES>
typename tlm_mm<TYPES>::gp_t* tlm_mm<TYPES>::allocate() {
    gp_t* ptr;
    if (free_list) {
        ptr = free_list->trans;
        empties = free_list;
        free_list = free_list->next;
        ptr->reset();
    } else {
        ptr = new gp_t(this);
    }
    ptr->set_mm(this);
    ptr->reset();
    ptr->set_data_ptr(nullptr);
    ptr->set_byte_enable_ptr(nullptr);
    return ptr;
}

template<typename TYPES>
void tlm_mm<TYPES>::free(gp_t* trans) {
    trans->free_all_extensions();
    trans->reset();
    if (!empties) {
        empties = new access;
        empties->next = free_list;
        empties->prev = nullptr;
        if (free_list)
            free_list->prev = empties;
    }
    free_list = empties;
    free_list->trans = trans;
    empties = free_list->prev;
}
}
#endif /* _TLM_TLM_MM_H_ */
