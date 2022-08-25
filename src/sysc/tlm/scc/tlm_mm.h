/*******************************************************************************
 * Copyright 2020 MINRES Technologies GmbH
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

#ifndef _TLM_TLM_MM_H_
#define _TLM_TLM_MM_H_

#include <tlm>
#include <util/pool_allocator.h>

//! @brief SystemC TLM
namespace tlm {
//! @brief SCC TLM utilities
namespace scc {

struct tlm_gp_mm : public tlm_extension<tlm_gp_mm> {
    virtual ~tlm_gp_mm() {}

    void copy_from(tlm_extension_base const& from) override {
        if(auto ext = dynamic_cast<tlm_gp_mm const*>(&from))
            memcpy(ext->data_ptr, data_ptr, std::min(ext->data_size, data_size));
    }

    tlm_gp_mm* clone() const override { return tlm_gp_mm::create(data_size); }

    size_t const data_size;
    uint8_t* const data_ptr;
    uint8_t* const be_ptr;

    static tlm_gp_mm* create(size_t sz, bool be = false);

    template <typename TYPES = tlm_base_protocol_types>
    static typename TYPES::tlm_payload_type* add_data_ptr(size_t sz, typename TYPES::tlm_payload_type& gp,
                                                          bool be = false) {
        return add_data_ptr(sz, &gp, be);
    }
    template <typename TYPES = tlm_base_protocol_types>
    static typename TYPES::tlm_payload_type* add_data_ptr(size_t sz, typename TYPES::tlm_payload_type* gp,
                                                          bool be = false);

protected:
    tlm_gp_mm(size_t sz, uint8_t* data_ptr, uint8_t* be_ptr)
    : data_size(sz)
    , data_ptr(data_ptr)
    , be_ptr(be_ptr) {}
};

template <size_t SZ, bool BE = false> struct tlm_gp_mm_t : public tlm_gp_mm {

    friend tlm_gp_mm;

    virtual ~tlm_gp_mm_t() {}

    void free() override { util::pool_allocator<tlm_gp_mm_t<SZ, BE>>::get().free(this); }

protected:
    tlm_gp_mm_t(size_t sz)
    : tlm_gp_mm(sz, data, BE ? be : nullptr) {}
    uint8_t data[SZ];
    uint8_t be[BE ? SZ : 0];
};

struct tlm_gp_mm_v : public tlm_gp_mm {

    friend tlm_gp_mm;

    virtual ~tlm_gp_mm_v() { delete data_ptr; }

protected:
    tlm_gp_mm_v(size_t sz)
    : tlm_gp_mm(sz, new uint8_t[sz], nullptr) {}
};

inline tlm_gp_mm* tlm::scc::tlm_gp_mm::create(size_t sz, bool be) {
    if(sz > 4096) {
        return new tlm_gp_mm_v(sz);
    } else if(sz > 1024) {
        if(be) {
            return new(util::pool_allocator<tlm_gp_mm_t<4096, true>>::get().allocate()) tlm_gp_mm_t<4096, true>(sz);
        } else {
            return new(util::pool_allocator<tlm_gp_mm_t<4096, false>>::get().allocate()) tlm_gp_mm_t<4096, false>(sz);
        }
    } else if(sz > 256) {
        if(be) {
            return new(util::pool_allocator<tlm_gp_mm_t<1024, true>>::get().allocate()) tlm_gp_mm_t<1024, true>(sz);
        } else {
            return new(util::pool_allocator<tlm_gp_mm_t<1024, false>>::get().allocate()) tlm_gp_mm_t<1024, false>(sz);
        }
    } else if(sz > 64) {
        if(be) {
            return new(util::pool_allocator<tlm_gp_mm_t<256, true>>::get().allocate()) tlm_gp_mm_t<256, true>(sz);
        } else {
            return new(util::pool_allocator<tlm_gp_mm_t<256, false>>::get().allocate()) tlm_gp_mm_t<256, false>(sz);
        }
    } else if(sz > 16) {
        if(be) {
            return new(util::pool_allocator<tlm_gp_mm_t<64, true>>::get().allocate()) tlm_gp_mm_t<64, true>(sz);
        } else {
            return new(util::pool_allocator<tlm_gp_mm_t<64, false>>::get().allocate()) tlm_gp_mm_t<64, false>(sz);
        }
    } else if(be) {
        return new(util::pool_allocator<tlm_gp_mm_t<16, true>>::get().allocate()) tlm_gp_mm_t<16, true>(sz);
    } else {
        return new(util::pool_allocator<tlm_gp_mm_t<16, false>>::get().allocate()) tlm_gp_mm_t<16, false>(sz);
    }
}

template <typename TYPES>
inline typename TYPES::tlm_payload_type*
tlm::scc::tlm_gp_mm::add_data_ptr(size_t sz, typename TYPES::tlm_payload_type* gp, bool be) {
    auto* ext = create(sz, be);
    gp->set_auto_extension(ext);
    gp->set_data_ptr(ext->data_ptr);
    gp->set_data_length(sz);
    gp->set_byte_enable_ptr(ext->be_ptr);
    if(be)
        gp->set_byte_enable_length(sz);
    return gp;
}

template <typename EXT> struct tlm_ext_mm : public EXT {

    friend tlm_gp_mm;

    ~tlm_ext_mm() {}

    void free() override { util::pool_allocator<tlm_ext_mm<EXT>>::get().free(this); }

    EXT* clone() const override { return create(*this); }

    template <typename... Args> static EXT* create(Args... args) {
        return new(util::pool_allocator<tlm_ext_mm<EXT>>::get().allocate()) tlm_ext_mm<EXT>(args...);
    }

protected:
    template <typename... Args>
    tlm_ext_mm(Args... args)
    : EXT(args...) {}
};
/**
 * @class tlm_mm
 * @brief a tlm memory manager
 *
 * This memory manager can be used as singleton or as local memory manager. It uses the pool_allocator
 * as singleton to maximize reuse
 */
template <typename TYPES = tlm_base_protocol_types, bool CLEANUP_DATA = true>
class tlm_mm : public tlm::tlm_mm_interface {
    using payload_type = typename TYPES::tlm_payload_type;

public:
    /**
     * @brief accessor function of the singleton
     * @return
     */
    static tlm_mm& get();

    tlm_mm()
    : allocator(util::pool_allocator<payload_type>::get()) {}

    tlm_mm(const tlm_mm&) = delete;

    tlm_mm(tlm_mm&&) = delete;

    tlm_mm& operator=(const tlm_mm& other) = delete;

    tlm_mm& operator=(tlm_mm&& other) = delete;

    ~tlm_mm() = default;
    /**
     * @brief get a plain tlm_payload_type without extensions
     * @return the tlm_payload_type
     */
    payload_type* allocate();
    /**
     * @brief get a tlm_payload_type with registered extension
     * @return the tlm_payload_type
     */
    template <typename PEXT> payload_type* allocate() {
        auto* ptr = allocate();
        ptr->set_auto_extension(new PEXT);
        return ptr;
    }
    /**
     * @brief get a plain tlm_payload_type without extensions but initialized data and byte enable
     * @return the tlm_payload_type
     */
    payload_type* allocate(size_t sz, bool be = false);
    /**
     * @brief get a tlm_payload_type with registered extension and initialize data pointer
     *
     * @return the tlm_payload_type
     */
    template <typename PEXT> payload_type* allocate(size_t sz, bool be = false) {
        auto* ptr = allocate(sz, be);
        ptr->set_auto_extension(tlm_ext_mm<PEXT>::create());
        return ptr;
    }
    /**
     * @brief return the extension into the memory pool (removing the extensions)
     * @param trans the returning transaction
     */
    void free(tlm::tlm_generic_payload* trans) override;

private:
    util::pool_allocator<payload_type>& allocator;
};

template <typename TYPES, bool CLEANUP_DATA> inline tlm_mm<TYPES, CLEANUP_DATA>& tlm_mm<TYPES, CLEANUP_DATA>::get() {
    static tlm_mm<TYPES, CLEANUP_DATA> mm;
    return mm;
}

template <typename TYPES, bool CLEANUP_DATA>
inline typename tlm_mm<TYPES, CLEANUP_DATA>::payload_type* tlm_mm<TYPES, CLEANUP_DATA>::allocate() {
    auto* ptr = allocator.allocate(sc_core::sc_time_stamp().value());
    return new(ptr) payload_type(this);
}

template <typename TYPES, bool CLEANUP_DATA>
inline typename tlm_mm<TYPES, CLEANUP_DATA>::payload_type* tlm_mm<TYPES, CLEANUP_DATA>::allocate(size_t sz, bool be) {
    return sz?tlm_gp_mm::add_data_ptr(sz, allocate(), be) : allocate();
}

template <typename TYPES, bool CLEANUP_DATA> void tlm_mm<TYPES, CLEANUP_DATA>::free(tlm::tlm_generic_payload* trans) {
    if(CLEANUP_DATA && !trans->get_extension<tlm_gp_mm>()) {
        if(trans->get_data_ptr())
            delete[] trans->get_data_ptr();
        trans->set_data_ptr(nullptr);
        if(trans->get_byte_enable_ptr())
            delete[] trans->get_byte_enable_ptr();
        trans->set_byte_enable_ptr(nullptr);
    }
    trans->reset();
    trans->~tlm_generic_payload();
    allocator.free(trans);
}

} // namespace scc
} // namespace tlm

#endif /* _TLM_TLM_MM_H_ */
