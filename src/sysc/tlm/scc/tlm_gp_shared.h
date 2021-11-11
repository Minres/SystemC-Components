/*******************************************************************************
 * Copyright (C) 2021, MINRES Technologies GmbH
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 * Contributors:
 *       eyck@minres.com - initial API and implementation
 ******************************************************************************/


#ifndef _SYSC_TLM_TLM_GP_SHARED_H_
#define _SYSC_TLM_TLM_GP_SHARED_H_

#include <tlm>

namespace tlm {
namespace scc {

class tlm_gp_shared_ptr {
    tlm::tlm_generic_payload *ptr { nullptr };
public:
    /// @brief Default constructor, creates a unique_ptr that owns nothing.
    tlm_gp_shared_ptr() noexcept = default;

    /// @brief Takes ownership of the pointer.
    inline tlm_gp_shared_ptr(tlm::tlm_generic_payload *p) noexcept
            : ptr(p) {
        if (ptr && ptr->has_mm())
            ptr->acquire();
    }
    /// @brief Copy constructor.
    inline tlm_gp_shared_ptr(tlm_gp_shared_ptr const& p) noexcept
            : ptr(p.ptr) {
        if (ptr && ptr->has_mm())
            ptr->acquire();
    }
    /// @brief Move constructor.´
    inline tlm_gp_shared_ptr(tlm_gp_shared_ptr &&p) noexcept
            : ptr(std::move(p.ptr)) {
        p.ptr=nullptr;
    }
    /// @brief destructor
    ~tlm_gp_shared_ptr() {
        if (ptr && ptr->has_mm())
            ptr->release();
    }
    /// @brief Copy assignment operator.
    tlm_gp_shared_ptr& operator=(tlm_gp_shared_ptr const & p) noexcept {
        if(ptr && ptr->has_mm())
            ptr->release();
        ptr = p.ptr;
        if (ptr && ptr->has_mm())
            ptr->acquire();
        return *this;
    }
    /// @brief Move assignment operator.
    tlm_gp_shared_ptr& operator=(tlm_gp_shared_ptr &&p) noexcept {
        if(ptr && ptr->has_mm())
            ptr->release();
        ptr = p.ptr;
        p.ptr=nullptr;
        return *this;
    }

    /// @brief raw pointer assignment operator.
    tlm_gp_shared_ptr& operator=(tlm::tlm_generic_payload *p) noexcept {
        if(ptr && ptr->has_mm())
            ptr->release();
        ptr = p;
        if (ptr && ptr->has_mm())
            ptr->acquire();
        return *this;
    }

    /// Dereference the stored pointer.
    inline tlm::tlm_generic_payload& operator*() const  noexcept{
        return *ptr;
    }

    /// Return the stored pointer.
    inline tlm::tlm_generic_payload* operator->() const noexcept {
        return ptr;
    }

    /// Return the stored pointer.
    inline tlm::tlm_generic_payload* get() const noexcept {
        return ptr;
    }

    inline operator bool() const noexcept {
        return ptr!=nullptr;
    }
};
inline std::ostream& operator<<(std::ostream& os, tlm_gp_shared_ptr const& p){
    os<<p.get();
    return os;
}
inline bool operator==(tlm_gp_shared_ptr const& x, tlm_gp_shared_ptr const& y) noexcept {
    return x.get() == y.get();
}

inline bool operator==(tlm_gp_shared_ptr const& x, tlm::tlm_generic_payload* y) noexcept {
    return x.get() == y;
}

inline bool operator!=(tlm_gp_shared_ptr const& x, tlm_gp_shared_ptr const& y) noexcept {
    return x.get() != y.get();
}
}
}
#endif /* _SYSC_TLM_TLM_GP_SHARED_H_ */
