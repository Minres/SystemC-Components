/*
 * pool_allocator.h
 *
 *  Created on: 26.01.2020
 *      Author: eyck
 */

#ifndef _UTIL_POOL_ALLOCATOR_H_
#define _UTIL_POOL_ALLOCATOR_H_

#include <array>
#include <deque>
#include <mutex>
#include <vector>
#include <unordered_map>
#include <algorithm>
#ifdef HAVE_GETENV
#include <cstdlib>
#endif

namespace util {
template <typename T, unsigned CHUNK_SIZE = 4096> class pool_allocator {
public:
    void* allocate(uint64_t id=0);

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
    std::unordered_map<void*, uint64_t> used_blocks;
#ifdef HAVE_GETENV
    const bool debug_memory{getenv("TLM_MM_CHECK")!=nullptr};
#else
    const bool debug_memory{false};
#endif
};

template <typename T, unsigned CHUNK_SIZE> pool_allocator<T, CHUNK_SIZE>& pool_allocator<T, CHUNK_SIZE>::get() {
    static thread_local pool_allocator inst;
    return inst;
}

template <typename T, unsigned CHUNK_SIZE> pool_allocator<T, CHUNK_SIZE>::~pool_allocator() {
    std::lock_guard<std::mutex> lk(payload_mtx);
#ifdef HAVE_GETENV
    if(debug_memory) {
        auto* check = getenv("TLM_MM_CHECK");
        auto diff = get_capacity() - get_free_entries_count();
        if(diff) {
            std::cerr << __FUNCTION__ << ": detected memory leak upon destruction, " << diff << " of " << get_capacity()
                              << " entries are not free'd" << std::endl;
#ifdef _MSC_VER
            if(check && _stricmp(check, "DEBUG")==0) {
#else
            if(check && strcasecmp(check, "DEBUG")==0) {
#endif
                std::vector<std::pair<void*, uint64_t>> elems(used_blocks.begin(), used_blocks.end());
                std::sort(elems.begin(), elems.end(), [](std::pair<void*, uint64_t> const& a, std::pair<void*, uint64_t> const& b) -> bool {
                    return a.second == b.second? a.first < b.first : a.second < b.second;
                });
                std::cerr<<"The 10 blocks with smallest id are:\n";
                for(size_t i=0; i<std::min(10UL, elems.size()); ++i) {
                    std::cerr<<"\taddr="<<elems[i].first<<", id="<<elems[i].second<<"\n";
                }
            }
        }
    }
#endif
}

template <typename T, unsigned CHUNK_SIZE> inline void* pool_allocator<T, CHUNK_SIZE>::allocate(uint64_t id) {
    std::lock_guard<std::mutex> lk(payload_mtx);
    if(!free_list.size())
        resize();
    auto ret = free_list.back();
    free_list.pop_back();
    memset(ret, 0, sizeof(T));
    if(debug_memory) used_blocks.insert({ret, id});
    return ret;
}

template <typename T, unsigned CHUNK_SIZE> inline void pool_allocator<T, CHUNK_SIZE>::free(void* p) {
    std::lock_guard<std::mutex> lk(payload_mtx);
    if(p) {
        free_list.push_back(p);
        if(debug_memory) used_blocks.erase(p);
    }
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
