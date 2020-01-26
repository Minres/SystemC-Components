/*
 * pool_allocator.h
 *
 *  Created on: 26.01.2020
 *      Author: eyck
 */

#ifndef _UTIL_POOL_ALLOCATOR_H_
#define _UTIL_POOL_ALLOCATOR_H_


namespace util {
template<typename T>
class pool_allocator {
public:
    enum {CHUNK_SIZE=4096};

    void* allocate();

    void free(void* p);

    void resize();

    pool_allocator(const pool_allocator&) = delete;

    pool_allocator(pool_allocator&&) = delete;

    pool_allocator& operator=(const pool_allocator&) = delete;

    pool_allocator& operator=(pool_allocator&&) = delete;

    static pool_allocator& get(){
        static pool_allocator inst;
        return inst;
    }
private:
    pool_allocator() = default;
    using chunk_type = uint8_t[sizeof(T)];
    std::vector<std::array<chunk_type, CHUNK_SIZE>*> chunks;
    std::deque<void*> free_list;
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


}


#endif /* _UTIL_POOL_ALLOCATOR_H_ */
