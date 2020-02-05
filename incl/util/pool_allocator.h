/*
 * pool_allocator.h
 *
 *  Created on: 26.01.2020
 *      Author: eyck
 */

#ifndef _UTIL_POOL_ALLOCATOR_H_
#define _UTIL_POOL_ALLOCATOR_H_

#include <deque>
#include <vector>

namespace util {
template<typename T, unsigned CHUNK_SIZE=4096>
class pool_allocator {
public:

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

    size_t get_capacity();

    size_t get_free_entries_count();
private:
    pool_allocator() = default;
    using chunk_type = uint8_t[sizeof(T)];
    std::vector<std::array<chunk_type, CHUNK_SIZE>*> chunks;
    std::deque<void*> free_list;
};

template<typename T, unsigned CHUNK_SIZE>
inline void* pool_allocator<T, CHUNK_SIZE>::allocate() {
    if (!free_list.size()) resize();
    auto ret = free_list.back();
    free_list.pop_back();
    memset(ret, 0, sizeof(T));
    return ret;
}

template<typename T, unsigned CHUNK_SIZE>
inline void pool_allocator<T, CHUNK_SIZE>::free(void *p) {
    if (p) free_list.push_back(p);
}

template<typename T, unsigned CHUNK_SIZE>
inline void pool_allocator<T, CHUNK_SIZE>::resize() {
    auto* chunk = new std::array<chunk_type, CHUNK_SIZE>();
    chunks.push_back(chunk);
    for (auto& p : *chunk)
        free_list.push_back(&p[0]);
}

template<typename T, unsigned CHUNK_SIZE>
inline size_t pool_allocator<T, CHUNK_SIZE>::get_capacity() {
  return chunks.size()*CHUNK_SIZE;
}

template<typename T, unsigned CHUNK_SIZE>
inline size_t pool_allocator<T, CHUNK_SIZE>::get_free_entries_count() {
  return free_list.size();
}


}

#endif /* _UTIL_POOL_ALLOCATOR_H_ */
