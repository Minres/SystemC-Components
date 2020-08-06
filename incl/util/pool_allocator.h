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
#include <array>
#include <mutex>
#ifdef HAVE_GETENV
#include <cstdlib>
#endif

namespace util {
template <typename T, unsigned CHUNK_SIZE = 4096> class pool_allocator {
public:
    void* allocate();

    void free(void* p);

    void resize();

    pool_allocator(const pool_allocator&) = delete;

    pool_allocator(pool_allocator&&) = delete;

    ~pool_allocator();

    pool_allocator& operator=(const pool_allocator&) = delete;

    pool_allocator& operator=(pool_allocator&&) = delete;

    static pool_allocator& get();

    size_t get_capacity();

    size_t get_free_entries_count();

private:
    pool_allocator() = default;
    using chunk_type = uint8_t[sizeof(T)];
    std::vector<std::array<chunk_type, CHUNK_SIZE>*> chunks;
    std::deque<void*> free_list;
    std::mutex payload_mtx;
};

template <typename T, unsigned CHUNK_SIZE> pool_allocator<T, CHUNK_SIZE>& pool_allocator<T, CHUNK_SIZE>::get() {
    static pool_allocator inst;
    return inst;
}

template <typename T, unsigned CHUNK_SIZE> pool_allocator<T, CHUNK_SIZE>::~pool_allocator() {
#ifdef HAVE_GETENV
    auto* check = getenv("TLM_MM_CHECK");
    if(check && strcasecmp(check, "INFO")) {
        auto diff = get_capacity() - get_free_entries_count();
        if(diff)
            std::cout << __FUNCTION__ << ": detected memory leak upon destruction, " << diff << " of " << get_capacity()
                      << " entries are not free'd" << std::endl;
    }
#endif
}

template <typename T, unsigned CHUNK_SIZE> inline void* pool_allocator<T, CHUNK_SIZE>::allocate() {
    std::lock_guard<std::mutex> lk(payload_mtx);
    if(!free_list.size())
        resize();
    auto ret = free_list.back();
    free_list.pop_back();
    memset(ret, 0, sizeof(T));
    return ret;
}

template <typename T, unsigned CHUNK_SIZE> inline void pool_allocator<T, CHUNK_SIZE>::free(void* p) {
    std::lock_guard<std::mutex> lk(payload_mtx);
    if(p)
        free_list.push_back(p);
}

template <typename T, unsigned CHUNK_SIZE> inline void pool_allocator<T, CHUNK_SIZE>::resize() {
    auto* chunk = new std::array<chunk_type, CHUNK_SIZE>();
    chunks.push_back(chunk);
    for(auto& p : *chunk)
        free_list.push_back(&p[0]);
}

template <typename T, unsigned CHUNK_SIZE> inline size_t pool_allocator<T, CHUNK_SIZE>::get_capacity() {
    return chunks.size() * CHUNK_SIZE;
}

template <typename T, unsigned CHUNK_SIZE> inline size_t pool_allocator<T, CHUNK_SIZE>::get_free_entries_count() {
    return free_list.size();
}

} // namespace util

#endif /* _UTIL_POOL_ALLOCATOR_H_ */
