/*******************************************************************************
 * Copyright 2016, 2018 MINRES Technologies GmbH
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

#ifndef _SYSC_REGISTER_H_
#define _SYSC_REGISTER_H_

#include <memory>

#include "resetable.h"
#include "resource_access_if.h"
#include "util/delegate.h"
#include "utilities.h"
#include <functional>
#include <limits>
#include <sstream>

namespace scc {

namespace impl {
/**
 * some template classes and functions to calculate mask values
 */
template <typename T, bool = std::is_integral<T>::value> class helper {};

template <typename T> class helper<T, true> {
public:
    using Type = T;
    template <typename Type> constexpr Type get_max_uval() const {
        return std::numeric_limits<Type>::is_signed ? -1 : std::numeric_limits<Type>::max();
    }
};

template <typename T> class helper<T, false> {
public:
    using Type = typename T::StorageType;
    template <typename Type> constexpr Type get_max_uval() const {
        return std::numeric_limits<Type>::is_signed ? -1 : std::numeric_limits<Type>::max();
    }
};

template <typename Type> constexpr Type get_max_uval() {
    return std::numeric_limits<Type>::is_signed ? -1 : std::numeric_limits<Type>::max();
}

/**
 * a simple register implementation taking a certain data type. The sc_register does not hold the value itself,
 * this needs to be provided. It only provides som resource access interface and handled callbacks for read and
 * write accesses
 */
template <typename DATATYPE>
class sc_register : public sc_core::sc_object, public resource_access_if, public traceable {
public:
    using this_type = class sc_register<DATATYPE>;
    /**
     * the constructor
     *
     * @param nm the instance name
     * @param storage the storage data structure
     * @param reset_val the reset value
     * @param owner the owning object which needs to implement the resettable interface
     * @param rdmask the SW read mask
     * @param wrmask the SW write mask
     */
    sc_register(sc_core::sc_module_name nm, DATATYPE &storage, const DATATYPE reset_val, resetable &owner,
                DATATYPE rdmask = get_max_uval<DATATYPE>(), DATATYPE wrmask = get_max_uval<DATATYPE>())
    : sc_core::sc_object(nm)
    , res_val(reset_val)
    , rdmask(rdmask)
    , wrmask(wrmask)
    , storage(storage) {
        owner.register_resource(this);
    }
    /**
     * the destructor
     */
    ~sc_register() = default;

    /**
     *
     * @return
     */
    size_t size() const override { return sizeof(DATATYPE); }
    /**
     * reset the register
     */
    void reset() override {
        DATATYPE r(res_val);
        if (wr_cb) wr_cb(*this, r, sc_core::SC_ZERO_TIME);
        storage = r;
    }
    /**
     * write function from resource_access_if
     *
     * @param data data to write
     * @param length size of data to write
     * @param offset offset within register
     * @param d annotated delay if loosly-timed access
     * @return true if access is successful
     */
    bool write(const uint8_t *data, size_t length, uint64_t offset = 0, sc_core::sc_time d = sc_core::SC_ZERO_TIME) override {
        assert("Access out of range" && offset + length <= sizeof(DATATYPE));
        auto temp(storage);
        auto beg = reinterpret_cast<uint8_t *>(&temp) + offset;
        std::copy(data, data + length, beg);
        if (wr_cb) return wr_cb(*this, temp, d);
        storage = (temp & wrmask) | (storage & ~wrmask);
        return true;
    }
    /**
     * read function from resource_access_if
     *
     * @param data data buffer to read
     * @param length size of data to read
     * @param offset offset within register
     * @param d annotated delay if loosly-timed access
     * @return true if access is successful
     */
    bool read(uint8_t *data, size_t length, uint64_t offset = 0, sc_core::sc_time d = sc_core::SC_ZERO_TIME) const override {
        assert("Access out of range" && offset + length <= sizeof(DATATYPE));
        auto temp(storage);
        if (rd_cb) {
            if (!rd_cb(*this, temp, d)) return false;
        } else
            temp &= rdmask;
        auto beg = reinterpret_cast<uint8_t *>(&temp) + offset;
        std::copy(beg, beg + length, data);
        return true;
    }
    /**
     * debug write function from resource_access_if
     *
     * @param data data to write
     * @param length size of data to write
     * @param offset offset within register
     * @return true if access is successful
     */
    bool write_dbg(const uint8_t *data, size_t length, uint64_t offset=0) override {
        assert("Offset out of range" && offset == 0);
        if (length != sizeof(DATATYPE)) return false;
        storage = *reinterpret_cast<const DATATYPE *>(data);
        return true;
    }
    /**
     * debug read function from resource_access_if
     *
     * @param data data buffer to read
     * @param length size of data to read
     * @param offset offset within register
     * @return true if access is successful
     */
    bool read_dbg(uint8_t *data, size_t length, uint64_t offset=0) const override {
        assert("Offset out of range" && offset == 0);
        if (length != sizeof(DATATYPE)) return false;
        *reinterpret_cast<DATATYPE *>(data) = storage;
        return true;
    }
    /**
     * cast operator
     */
    operator DATATYPE() const { return storage; }
    /**
     * get the storage
     *
     * @return copy of the underlying datatype
     */
    DATATYPE get() const { return storage; }
    /**
     * put value to storage
     *
     * @param data the data to store
     */
    void put(DATATYPE data) const { storage = data; }
    /**
     * copy assignment
     *
     * @param other the data
     * @return
     */
    this_type &operator=(DATATYPE other) {
        storage = other;
        return *this;
    }
    /**
     * unary or
     *
     * @param other
     * @return
     */
    this_type &operator|=(DATATYPE other) {
        storage |= other;
        return *this;
    }
    /**
     * unary and
     *
     * @param other
     * @return
     */
    this_type &operator&=(DATATYPE other) {
        storage &= other;
        return *this;
    }
    /**
     * set the read callback triggered upon a read request without forwarding the annotated time
     * this is primary for backward compatibility
     *
     * @param read_cb
     */
    void set_read_cb(std::function<bool(const this_type &, DATATYPE &)> read_cb) {
        rd_cb = [read_cb](const this_type &reg, DATATYPE &data, sc_core::sc_time delay) { return read_cb(reg, data); };
    }
    /**
     * set the read callback triggered upon a read request
     *
     * @param read_cb
     */
    void set_read_cb(std::function<bool(const this_type &, DATATYPE &, sc_core::sc_time)> read_cb) { rd_cb = read_cb; }
    /**
     * set the write callback triggered upon a write request without forwarding the annotated time
     * this is primary for backward compatibility
     *
     * @param write_cb
     */
    void set_write_cb(std::function<bool(this_type &, DATATYPE &)> write_cb) {
        wr_cb = [write_cb](this_type &reg, DATATYPE &data, sc_core::sc_time delay) { return write_cb(reg, data); };
    }
    /**
     * set the write callback triggered upon a write request
     *
     * @param write_cb
     */
    void set_write_cb(std::function<bool(this_type &, DATATYPE &, sc_core::sc_time)> write_cb) { wr_cb = write_cb; }
    /**
     * trace the register value to the given trace file
     *
     * @param trf
     */
    void trace(sc_core::sc_trace_file *trf) const override { sc_trace(trf, storage, this->name()); }
    //! the reset value
    const DATATYPE res_val;
    //! the SW read mask
    const DATATYPE rdmask;
    //! the SW write mask
    const DATATYPE wrmask;

private:
    DATATYPE &storage;
    std::function<bool(const this_type &, DATATYPE &, sc_core::sc_time)> rd_cb;
    std::function<bool(this_type &, DATATYPE &, sc_core::sc_time)> wr_cb;

    util::delegate<bool(const this_type &, DATATYPE &, sc_core::sc_time)> rd_dlgt;
    util::delegate<bool(this_type &, DATATYPE &, sc_core::sc_time)> wr_dlgt;
};
}
//! import the implementation into the scc namespace
template <typename DATATYPE> using sc_register = impl::sc_register<typename impl::helper<DATATYPE>::Type>;
/**
 * an indexed register aka a register file of a certain type
 */
template <typename DATATYPE, size_t SIZE, size_t START = 0>
class sc_register_indexed : public indexed_resource_access_if {
public:
    using BASE_DATA_TYPE = typename impl::helper<DATATYPE>::Type;

    using value_type = sc_register<DATATYPE>;
    using pointer = value_type *;
    /**
     * the constructor
     *
     * @param nm
     * @param storage
     * @param reset_val
     * @param owner
     * @param rdmask
     * @param wrmask
     */
    sc_register_indexed(sc_core::sc_module_name nm, std::array<DATATYPE, SIZE> &storage, const DATATYPE reset_val,
                        resetable &owner,
                        BASE_DATA_TYPE rdmask = std::numeric_limits<BASE_DATA_TYPE>::is_signed
                                                    ? -1
                                                    : std::numeric_limits<BASE_DATA_TYPE>::max(),
                        BASE_DATA_TYPE wrmask = std::numeric_limits<BASE_DATA_TYPE>::is_signed
                                                    ? -1
                                                    : std::numeric_limits<BASE_DATA_TYPE>::max()) {
        _reg_field = reinterpret_cast<pointer>(malloc(SIZE * sizeof(value_type)));
        for (size_t idx = START; idx < (START + SIZE); ++idx) {
            std::stringstream ss;
            ss << nm << idx;
            new (_reg_field + idx)
                sc_register<DATATYPE>(sc_core::sc_module_name(ss.str().c_str()), storage[idx], reset_val, owner, rdmask, wrmask);
        }
    }
    /**
     * the destructor
     */
    ~sc_register_indexed() override {
        free(_reg_field);
    }
    /**
     * get the size of the register file
     *
     * @return the size
     */
    size_t size() override { return SIZE; };
    /**
     * set the read callback triggered upon a read request without forwarding the annotated time
     * this is primary for backward compatibility
     *
     * @param read_cb
     */
    void set_read_cb(std::function<bool(const sc_register<DATATYPE> &, DATATYPE &)> read_cb) {
        for (std::unique_ptr<sc_register<DATATYPE>> reg : *this) reg->add_read_cb(read_cb);
    }
    /**
     * set the read callback triggered upon a read request
     *
     * @param read_cb
     */
    void set_read_cb(std::function<bool(const sc_register<DATATYPE> &, DATATYPE &, sc_core::sc_time)> read_cb) {
        for (std::unique_ptr<sc_register<DATATYPE>> reg : *this) reg->add_read_cb(read_cb);
    }
    /**
     * set the write callback triggered upon a write request without forwarding the annotated time
     * this is primary for backward compatibility
     *
     * @param write_cb
     */
    void set_write_cb(std::function<bool(sc_register<DATATYPE> &, DATATYPE &)> write_cb) {
        for (std::unique_ptr<sc_register<DATATYPE>> reg : *this) reg->add_write_cb(write_cb);
    }
    /**
     * set the write callback triggered upon a write request
     *
     * @param write_cb
     */
    void set_write_cb(std::function<bool(sc_register<DATATYPE> &, DATATYPE &, sc_core::sc_time)> write_cb) {
        for (std::unique_ptr<sc_register<DATATYPE>> reg : *this) reg->add_write_cb(write_cb);
    }
    /**
     * Element access operator
     *
     * @param idx the index
     * @return the data reference at the index
     */
    reference operator[](size_t idx) noexcept override { return *(_reg_field + idx); }
    /**
     * const element access operator
     *
     * @param idx
     * @return the data reference at the index
     */
    const_reference operator[](size_t idx) const noexcept override { return *(_reg_field + idx); }
    /**
     * Element access operator
     *
     * @param idx the index
     * @return the data reference at the index
     */
    reference at(size_t idx) override {
        assert("access out of bound" && idx < SIZE);
        return *(_reg_field + idx);
    }
    /**
     * const element access operator
     *
     * @param idx
     * @return the data reference at the index
     */
   const_reference at(size_t idx) const override {
        assert("access out of bound" && idx < SIZE);
        return *(_reg_field + idx);
    }

private:
    value_type *_reg_field;
};
/**
 * alias class to map template argument read an write mask to constructor arguments
 */
template <typename DATATYPE, DATATYPE WRMASK = impl::get_max_uval<DATATYPE>(),
          DATATYPE RDMASK = impl::get_max_uval<DATATYPE>()>
class sc_register_masked : public sc_register<DATATYPE> {
public:
    sc_register_masked(sc_core::sc_module_name nm, DATATYPE &storage, const DATATYPE reset_val, resetable &owner)
    : sc_register<DATATYPE>(nm, storage, reset_val, owner, RDMASK, WRMASK) {}
};
}

#endif /* _SYSC_REGISTER_H_ */
