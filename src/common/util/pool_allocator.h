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

#if defined(_MSC_VER) || defined(__APPLE__)
#define NOEXCEPT
#else
#define NOEXCEPT _GLIBCXX_USE_NOEXCEPT
#endif
/**
 * \ingroup scc-common
 */
/**@{*/
//! @brief SCC common utilities
namespace util {
//! a generic pool allocator singleton not being MT-safe
template <size_t ELEM_SIZE, unsigned CHUNK_SIZE = 4096> class pool_allocator {
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
    using chunk_type = uint8_t[ELEM_SIZE];
    std::vector<std::array<chunk_type, CHUNK_SIZE>*> chunks{};
    std::deque<void*> free_list{};
    std::unordered_map<void*, uint64_t> used_blocks{};
#ifdef HAVE_GETENV
    const bool debug_memory{getenv("TLM_MM_CHECK") != nullptr};
#else
    const bool debug_memory{false};
#endif
};

template <typename T> class stl_pool_allocator {
public:
    typedef T value_type;
    typedef value_type* pointer;
    typedef const value_type* const_pointer;
    typedef value_type& reference;
    typedef const value_type& const_reference;
    typedef std::size_t size_type;
    typedef std::ptrdiff_t difference_type;
    //    convert an allocator<T> to allocator<U> e.g. for std::map from A to _Node<A>
    template <typename U> struct rebind {
        typedef stl_pool_allocator<U> other;
    };

    stl_pool_allocator(const stl_pool_allocator&) noexcept {}

    template <typename T2> stl_pool_allocator(const stl_pool_allocator<T2>&) noexcept {}

    ~stl_pool_allocator() NOEXCEPT {}

    //    address
    pointer address(reference r) { return std::addressof(r); }
    const_pointer address(const_reference r) { return std::addressof(r); }

    pointer allocate(size_type n, const void* = 0) {
        size_type value = 16;
        while(value < n)
            value <<= 2;
        switch(n) {
        case 16:
            return static_cast<T*>(util::pool_allocator<16 * sizeof(T), 65536>::get().allocate());
        case 64:
            return static_cast<T*>(util::pool_allocator<64 * sizeof(T), 16384>::get().allocate());
        case 256:
            return static_cast<T*>(util::pool_allocator<256 * sizeof(T), 4096>::get().allocate());
        case 1024:
            return static_cast<T*>(util::pool_allocator<1024 * sizeof(T), 1024>::get().allocate());
        case 4096:
            return static_cast<T*>(util::pool_allocator<4096 * sizeof(T), 256>::get().allocate());
        case 16384:
            return static_cast<T*>(util::pool_allocator<16384 * sizeof(T), 64>::get().allocate());
        default:
            return static_cast<T*>(::operator new(n * sizeof(T)));
        }
    }

    void deallocate(T* p, size_type n) noexcept {
        size_type value = 16;
        while(value < n)
            value <<= 2;
        switch(n) {
        case 16:
            util::pool_allocator<16 * sizeof(T), 65536>::get().free(p);
            break;
        case 64:
            util::pool_allocator<64 * sizeof(T), 16384>::get().free(p);
            break;
        case 256:
            util::pool_allocator<256 * sizeof(T), 4096>::get().free(p);
            break;
        case 1024:
            util::pool_allocator<1024 * sizeof(T), 1024>::get().free(p);
            break;
        case 4096:
            util::pool_allocator<4096 * sizeof(T), 256>::get().free(p);
            break;
        case 16384:
            util::pool_allocator<16384 * sizeof(T), 64>::get().free(p);
            break;
        default:
            ::operator delete(p);
        }
    }
    size_type max_size() const noexcept { return std::numeric_limits<size_type>::max() / sizeof(T); }

    bool operator==(stl_pool_allocator const&) { return true; }
    bool operator!=(stl_pool_allocator const& oAllocator) { return !operator==(oAllocator); }
};

template <size_t ELEM_SIZE, unsigned CHUNK_SIZE> pool_allocator<ELEM_SIZE, CHUNK_SIZE>& pool_allocator<ELEM_SIZE, CHUNK_SIZE>::get() {
    thread_local pool_allocator inst;
    return inst;
}

template <size_t ELEM_SIZE, unsigned CHUNK_SIZE> pool_allocator<ELEM_SIZE, CHUNK_SIZE>::~pool_allocator() {
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
                std::sort(elems.begin(), elems.end(), [](std::pair<void*, uint64_t> const& a, std::pair<void*, uint64_t> const& b) -> bool {
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
    for(auto p : chunks)
        delete p;
}

template <size_t ELEM_SIZE, unsigned CHUNK_SIZE> inline void* pool_allocator<ELEM_SIZE, CHUNK_SIZE>::allocate(uint64_t id) {
    if(!free_list.size())
        resize();
    auto ret = free_list.back();
    free_list.pop_back();
    memset(ret, 0, ELEM_SIZE);
    if(debug_memory)
        used_blocks.insert({ret, id});
    return ret;
}

template <size_t ELEM_SIZE, unsigned CHUNK_SIZE> inline void pool_allocator<ELEM_SIZE, CHUNK_SIZE>::free(void* p) {
    if(p) {
        free_list.push_back(p);
        if(debug_memory)
            used_blocks.erase(p);
    }
}

template <size_t ELEM_SIZE, unsigned CHUNK_SIZE> inline void pool_allocator<ELEM_SIZE, CHUNK_SIZE>::resize() {
    auto* chunk = new std::array<chunk_type, CHUNK_SIZE>();
    chunks.push_back(chunk);
    for(auto& p : *chunk)
        free_list.push_back(&p[0]);
}

template <size_t ELEM_SIZE, unsigned CHUNK_SIZE> inline size_t pool_allocator<ELEM_SIZE, CHUNK_SIZE>::get_capacity() {
    return chunks.size() * CHUNK_SIZE;
}

template <size_t ELEM_SIZE, unsigned CHUNK_SIZE> inline size_t pool_allocator<ELEM_SIZE, CHUNK_SIZE>::get_free_entries_count() {
    return free_list.size();
}
} // namespace util
/** @} */
#endif /* _UTIL_POOL_ALLOCATOR_H_ */
