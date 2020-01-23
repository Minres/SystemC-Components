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
template<typename T>
class pool_allocator {
public:
    enum {CHUNK_SIZE=4096};

    void* allocate();

    void free(void* p);

    void resize();

private:
    using chunk_type = uint8_t[sizeof(T)];
    std::vector<std::array<chunk_type, CHUNK_SIZE>*> chunks;
    std::deque<void*> free_list;
};

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
     * @brief remove the copy constructor
     * @param
     */
    tlm_mm(tlm_mm& ) = delete;
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
    tlm_mm(){}
    pool_allocator<payload_type> allocator;
};

template<typename T>
inline void* pool_allocator<T>::allocate() {
    if (!free_list.size()) resize();
    auto ret = free_list.back();
    free_list.pop_back();
    memset(ret, 0, sizeof(T));
    return ret;
}

template<typename T>
inline void pool_allocator<T>::free(void *p) {
    if (p) free_list.push_back(p);
}

template<typename T>
inline void pool_allocator<T>::resize() {
    auto* chunk = new std::array<chunk_type, CHUNK_SIZE>();
    chunks.push_back(chunk);
    for (auto& p : *chunk)
        free_list.push_back(&p[0]);
}

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
