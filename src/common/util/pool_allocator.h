/*******************************************************************************
 * Copyright 2020-2022 MINRES Technologies GmbH
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

#ifndef _UTIL_POOL_ALLOCATOR_H_
#define _UTIL_POOL_ALLOCATOR_H_

#include <algorithm>
#include <array>
#include <deque>
#include <mutex>
#include <unordered_map>
#include <vector>
#ifdef HAVE_GETENV
#include <cstdlib>
#endif

/**
 * \ingroup scc-common
 */
/**@{*/
//! @brief SCC common utilities
namespace util {
//! a generic pool allocator singleton not being MT-safe
template <typename T, unsigned CHUNK_SIZE = 4096> class pool_allocator {
public:
    /**
     * @fn void allocate*(uint64_t=0)
     * @brief allocate a piece of memory of the given size
     *
     * @param id
     */
    void* allocate(uint64_t id = 0);
    /**
     * @fn void free(void*)
     * @brief pit the memory back into the pool
     *
     * @param p
     */
    void free(void* p);
    /**
     * @fn void resize()
     * @brief add CHUNK_SIZE elements to the pool
     *
     */
    void resize();
    //! deleted constructor
    pool_allocator(const pool_allocator&) = delete;
    //! deleted constructor
    pool_allocator(pool_allocator&&) = delete;
    //! deleted destructor
    ~pool_allocator();
    //! deleted assignment operator
    pool_allocator& operator=(const pool_allocator&) = delete;
    //! deleted assignment operator
    pool_allocator& operator=(pool_allocator&&) = delete;
    //! pool allocator getter
    static pool_allocator& get();
    //! get the number of allocated bytes
    size_t get_capacity();
    //! get the number of free elements
    size_t get_free_entries_count();

private:
    pool_allocator() = default;
    using chunk_type = uint8_t[sizeof(T)];
    std::vector<std::array<chunk_type, CHUNK_SIZE>*> chunks{};
    std::deque<void*> free_list{};
    std::mutex payload_mtx{};
    std::unordered_map<void*, uint64_t> used_blocks{};
#ifdef HAVE_GETENV
    const bool debug_memory{getenv("TLM_MM_CHECK") != nullptr};
#else
    const bool debug_memory{false};
#endif
};

template <typename T, unsigned CHUNK_SIZE> pool_allocator<T, CHUNK_SIZE>& pool_allocator<T, CHUNK_SIZE>::get() {
    thread_local pool_allocator inst;
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
            if(check && _stricmp(check, "DEBUG") == 0) {
#else
            if(check && strcasecmp(check, "DEBUG") == 0) {
#endif
                std::vector<std::pair<void*, uint64_t>> elems(used_blocks.begin(), used_blocks.end());
                std::sort(elems.begin(), elems.end(),
                          [](std::pair<void*, uint64_t> const& a, std::pair<void*, uint64_t> const& b) -> bool {
                              return a.second == b.second ? a.first < b.first : a.second < b.second;
                          });
                std::cerr << "The 10 blocks with smallest id are:\n";
                for(size_t i = 0; i < std::min<decltype(i)>(10UL, elems.size()); ++i) {
                    std::cerr << "\taddr=" << elems[i].first << ", id=" << elems[i].second << "\n";
                }
            }
        }
    }
#endif
    for(auto p:chunks) delete p;
}

template <typename T, unsigned CHUNK_SIZE> inline void* pool_allocator<T, CHUNK_SIZE>::allocate(uint64_t id) {
    std::lock_guard<std::mutex> lk(payload_mtx);
    if(!free_list.size())
        resize();
    auto ret = free_list.back();
    free_list.pop_back();
    memset(ret, 0, sizeof(T));
    if(debug_memory)
        used_blocks.insert({ret, id});
    return ret;
}

template <typename T, unsigned CHUNK_SIZE> inline void pool_allocator<T, CHUNK_SIZE>::free(void* p) {
    std::lock_guard<std::mutex> lk(payload_mtx);
    if(p) {
        free_list.push_back(p);
        if(debug_memory)
            used_blocks.erase(p);
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
/** @} */
#endif /* _UTIL_POOL_ALLOCATOR_H_ */
