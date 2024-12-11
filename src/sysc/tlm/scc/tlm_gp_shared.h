/*******************************************************************************
 * Copyright 2021-2022 MINRES Technologies GmbH
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

#ifndef _SYSC_TLM_TLM_GP_SHARED_H_
#define _SYSC_TLM_TLM_GP_SHARED_H_

#include <tlm>

//! @brief SystemC TLM
namespace tlm {
//! @brief SCC TLM utilities
namespace scc {

template <typename T> class tlm_payload_shared_ptr {
    T* ptr{nullptr};

public:
    /// @brief Default constructor, creates a unique_ptr that owns nothing.
    tlm_payload_shared_ptr() noexcept = default;

    /// @brief Takes ownership of the pointer.
    inline tlm_payload_shared_ptr(T* p) noexcept
    : ptr(p) {
        if(ptr && ptr->has_mm())
            ptr->acquire();
    }
    /// @brief Copy constructor.
    inline tlm_payload_shared_ptr(tlm_payload_shared_ptr const& p) noexcept
    : ptr(p.ptr) {
        if(ptr && ptr->has_mm())
            ptr->acquire();
    }
    /// @brief Move constructor.´
    inline tlm_payload_shared_ptr(tlm_payload_shared_ptr&& p) noexcept
    : ptr(std::move(p.ptr)) {
        p.ptr = nullptr;
    }
    /// @brief destructor
    ~tlm_payload_shared_ptr() {
        if(ptr && ptr->has_mm())
            ptr->release();
    }
    /// @brief Copy assignment operator.
    tlm_payload_shared_ptr& operator=(tlm_payload_shared_ptr const& p) noexcept {
        if(ptr && ptr->has_mm())
            ptr->release();
        ptr = p.ptr;
        if(ptr && ptr->has_mm())
            ptr->acquire();
        return *this;
    }
    /// @brief Move assignment operator.
    tlm_payload_shared_ptr& operator=(tlm_payload_shared_ptr&& p) noexcept {
        if(ptr && ptr->has_mm())
            ptr->release();
        ptr = p.ptr;
        p.ptr = nullptr;
        return *this;
    }

    /// @brief raw pointer assignment operator.
    tlm_payload_shared_ptr& operator=(tlm::tlm_generic_payload* p) noexcept {
        if(ptr && ptr->has_mm())
            ptr->release();
        ptr = p;
        if(ptr && ptr->has_mm())
            ptr->acquire();
        return *this;
    }

    /// Dereference the stored pointer.
    inline T& operator*() const noexcept { return *ptr; }

    /// Return the stored pointer.
    inline T* operator->() const noexcept { return ptr; }

    /// Return the stored pointer.
    inline T* get() const noexcept { return ptr; }

    inline operator bool() const noexcept { return ptr != nullptr; }
};

template <typename T> inline std::ostream& operator<<(std::ostream& os, tlm_payload_shared_ptr<T> const& p) {
    os << p.get();
    return os;
}

template <typename T> inline bool operator==(tlm_payload_shared_ptr<T> const& x, tlm_payload_shared_ptr<T> const& y) noexcept {
    return x.get() == y.get();
}

template <typename T> inline bool operator==(tlm_payload_shared_ptr<T> const& x, T* y) noexcept { return x.get() == y; }

template <typename T> inline bool operator!=(tlm_payload_shared_ptr<T> const& x, tlm_payload_shared_ptr<T> const& y) noexcept {
    return x.get() != y.get();
}

using tlm_gp_shared_ptr = tlm_payload_shared_ptr<tlm::tlm_generic_payload>;
} // namespace scc
} // namespace tlm
#endif /* _SYSC_TLM_TLM_GP_SHARED_H_ */
