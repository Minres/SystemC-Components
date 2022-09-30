/*******************************************************************************
 * Copyright 2016-2021 MINRES Technologies GmbH
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
#include "scc/traceable.h"
#include "scc/utilities.h"
#ifdef _MSC_VER
#include <functional>
#else
#include "util/delegate.h"
#endif
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
 * @class sc_register
 * @brief a simple register implementation
 *
 * A simple register implementation taking a certain data type. The sc_register does not hold the value itself,
 * the data storage needs to be provided. It only provides some resource access interface and handled callbacks for read
 * and write accesses
 *
 * @tparam DATATYPE
 */
template <typename DATATYPE>
class sc_register : public sc_core::sc_object, public resource_access_if, public traceable {
public:
    using this_type = sc_register<DATATYPE>;
    /**
     * @fn  sc_register(sc_core::sc_module_name, DATATYPE&, const DATATYPE, resetable&,
     * DATATYPE=get_max_uval<DATATYPE>(), DATATYPE=get_max_uval<DATATYPE>())
     * @brief the constructor
     *
     * @param nm the instance name
     * @param storage the storage data structure
     * @param reset_val the reset value
     * @param owner the owning object which needs to implement the resettable interface
     * @param rdmask the SW read mask
     * @param wrmask the SW write mask
     */
    sc_register(sc_core::sc_module_name nm, DATATYPE& storage, const DATATYPE reset_val, resetable& owner,
                DATATYPE rdmask = get_max_uval<DATATYPE>(), DATATYPE wrmask = get_max_uval<DATATYPE>())
    : sc_core::sc_object(nm)
    , res_val(reset_val)
    , rdmask(rdmask)
    , wrmask(wrmask)
    , storage(storage) {
        owner.register_resource(this);
    }
    /**
     * @fn  ~sc_register()
     * @brief desctructor
     *
     */
    ~sc_register() = default;
    /**
     * @fn size_t size()const
     * @brief get the size of this register in bytes
     *
     * @return
     */
    size_t size() const override { return sizeof(DATATYPE); }
    /**
     * @fn void reset()
     * @brief reset the register
     *
     */
    void reset() override {
        DATATYPE r(res_val);
        if(wr_cb)
            wr_cb(*this, r, sc_core::SC_ZERO_TIME);
        storage = r;
    }
    /**
     * @fn bool write(const uint8_t*, size_t, uint64_t=0, sc_core::sc_time=sc_core::SC_ZERO_TIME)
     * @brief write function from resource_access_if
     *
     * @param data to be written
     * @param length of data to be written in bytes
     * @param offset of the write to the baseaddress of the register
     * @param d offset of the time in a time domain wrt. the SystemC simulator time (e.g. when using loosly-timed
     * modeling)
     * @return true if access is successful
     */
    bool write(const uint8_t* data, size_t length, uint64_t offset = 0,
               sc_core::sc_time d = sc_core::SC_ZERO_TIME) override {
        assert("Access out of range" && offset + length <= sizeof(DATATYPE));
        auto temp(storage);
        auto beg = reinterpret_cast<uint8_t*>(&temp) + offset;
        std::copy(data, data + length, beg);
        if(wr_cb)
            return wr_cb(*this, temp, d);
        storage = (temp & wrmask) | (storage & ~wrmask);
        return true;
    }
    /**
     * @fn bool read(uint8_t*, size_t, uint64_t=0, sc_core::sc_time=sc_core::SC_ZERO_TIME)const
     * @brief read function from resource_access_if
     *
     * @param data to be read
     * @param length of data to be written in bytes
     * @param offset of the write to the baseaddress of the register
     * @param d offset of the time in a time domain wrt. the SystemC simulator time (e.g. when using loosly-timed
     * modeling)
     * @return true if access is successful
     */
    bool read(uint8_t* data, size_t length, uint64_t offset = 0,
              sc_core::sc_time d = sc_core::SC_ZERO_TIME) const override {
        assert("Access out of range" && offset + length <= sizeof(DATATYPE));
        auto temp(storage);
        if(rd_cb) {
            if(!rd_cb(*this, temp, d))
                return false;
        } else
            temp &= rdmask;
        auto beg = reinterpret_cast<uint8_t*>(&temp) + offset;
        std::copy(beg, beg + length, data);
        return true;
    }
    /**
     * @fn bool write_dbg(const uint8_t*, size_t, uint64_t=0)
     * @brief debug write function from resource_access_if
     *
     * This access should not cause any side effect!
     *
     * @param data to be written
     * @param length of data to be written in bytes
     * @param offset of the write to the baseaddress of the register
     * @return true if access is successful
     */
    bool write_dbg(const uint8_t* data, size_t length, uint64_t offset = 0) override {
        assert("Offset out of range" && offset == 0);
        if(length != sizeof(DATATYPE))
            return false;
        storage = *reinterpret_cast<const DATATYPE*>(data);
        return true;
    }
    /**
     * @fn bool read_dbg(uint8_t*, size_t, uint64_t=0)const
     * @brief debug read function from resource_access_if
     *
     * This access should not cause any side effect!
     *
     * @param data to be read
     * @param length of data to be written in bytes
     * @param offset of the write to the baseaddress of the register
     * @return true if access is successful
     */
    bool read_dbg(uint8_t* data, size_t length, uint64_t offset = 0) const override {
        assert("Offset out of range" && offset == 0);
        if(length != sizeof(DATATYPE))
            return false;
        *reinterpret_cast<DATATYPE*>(data) = storage;
        return true;
    }
    /**
     * @fn  operator DATATYPE()const
     * @brief cast operator to get underlying storage
     *
     */
    operator DATATYPE() const { return storage; }
    /**
     * @fn DATATYPE get()const
     * @brief get the underlying storage
     *
     * @return
     */
    DATATYPE get() const { return storage; }
    /**
     * @fn void put(DATATYPE)const
     * @brief write to the underlying storage
     *
     * @param data the new value
     */
    void put(DATATYPE data) const { storage = data; }
    /**
     * @fn this_type& operator =(DATATYPE)
     * @brief assignment operator
     *
     * @param other the new value
     * @return reference to this
     */
    this_type& operator=(DATATYPE other) {
        storage = other;
        return *this;
    }
    /**
     * @fn this_type& operator |=(DATATYPE)
     * @brief unary or
     *
     * @param other the other value
     * @return reference to this
     */
    this_type& operator|=(DATATYPE other) {
        storage |= other;
        return *this;
    }
    /**
     * @fn this_type& operator &=(DATATYPE)
     * @brief unary and
     *
     * @param other the other value
     * @return reference to this
     */
    this_type& operator&=(DATATYPE other) {
        storage &= other;
        return *this;
    }
    /**
     * @fn void set_read_cb(std::function<bool (const this_type&, DATATYPE&)>)
     * @brief set the read callback
     *
     * The read callback is triggered upon a read request without forwarding the annotated time
     * this is primary for backward compatibility
     *
     * @param read_cb the callback functor
     */
    void set_read_cb(std::function<bool(const this_type&, DATATYPE&)> read_cb) {
        rd_cb = [read_cb](const this_type& reg, DATATYPE& data, sc_core::sc_time delay) { return read_cb(reg, data); };
    }
    /**
     * @fn void set_read_cb(std::function<bool (const this_type&, DATATYPE&, sc_core::sc_time)>)
     * @brief set the read callback
     *
     * The read callback functor triggered upon a read request.
     *
     * @param read_cb the callback functor
     */
    void set_read_cb(std::function<bool(const this_type&, DATATYPE&, sc_core::sc_time)> read_cb) { rd_cb = read_cb; }
    /**
     * @fn void set_write_cb(std::function<bool (this_type&, const DATATYPE&)>)
     * @brief set the write callback
     *
     * The write callback functor is triggered upon a write request without forwarding the annotated time
     * this is primary for backward compatibility
     *
     * @param write_cb the callback functor
     */
    void set_write_cb(std::function<bool(this_type&, const DATATYPE&)> write_cb) {
        wr_cb = [write_cb](this_type& reg, DATATYPE& data, sc_core::sc_time delay) { return write_cb(reg, data); };
    }
    /**
     * @fn void set_write_cb(std::function<bool (this_type&, const DATATYPE&, sc_core::sc_time)>)
     * @brief set the write callback
     *
     * The write callback functor is triggered upon a write request.
     *
     * @param write_cb the callback functor
     */
    /**
     * set the write callback triggered upon a write request
     *
     * @param write_cb
     */
    void set_write_cb(std::function<bool(this_type&, const DATATYPE&, sc_core::sc_time)> write_cb) { wr_cb = write_cb; }
    /**
     * @fn void trace(sc_core::sc_trace_file*)const
     * @brief trace the register value to the given trace file
     *
     * @param trf the trace file
     */
    void trace(sc_core::sc_trace_file* trf) const override { sc_core::sc_trace(trf, storage, this->name()); }
    //! \brief the reset value
    const DATATYPE res_val;
    //! \brief the SW read mask
    const DATATYPE rdmask;
    //! \brief the SW write mask
    const DATATYPE wrmask;

private:
    DATATYPE& storage;
    std::function<bool(const this_type&, DATATYPE&, sc_core::sc_time)> rd_cb;
    std::function<bool(this_type&, DATATYPE&, sc_core::sc_time)> wr_cb;

#ifdef _MSC_VER
    std::function<bool(const this_type&, DATATYPE&, sc_core::sc_time)> rd_dlgt;
    std::function<bool(this_type&, DATATYPE&, sc_core::sc_time)> wr_dlgt;
#else
    util::delegate<bool(const this_type&, DATATYPE&, sc_core::sc_time)> rd_dlgt;
    util::delegate<bool(this_type&, DATATYPE&, sc_core::sc_time)> wr_dlgt;
#endif
};
} // namespace impl
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
    using pointer = value_type*;
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
    sc_register_indexed(sc_core::sc_module_name nm, std::array<DATATYPE, SIZE>& storage, const DATATYPE reset_val,
                        resetable& owner,
                        BASE_DATA_TYPE rdmask = std::numeric_limits<BASE_DATA_TYPE>::is_signed
                                                    ? -1
                                                    : std::numeric_limits<BASE_DATA_TYPE>::max(),
                        BASE_DATA_TYPE wrmask = std::numeric_limits<BASE_DATA_TYPE>::is_signed
                                                    ? -1
                                                    : std::numeric_limits<BASE_DATA_TYPE>::max()) {

        _reg_field.init(START + SIZE, [&](const char* name, size_t idx) -> pointer {
            return new sc_register<DATATYPE>(name, storage[idx], reset_val, owner, rdmask, wrmask);
        });
    }

    /**
     * the destructor
     */
    ~sc_register_indexed() override {}
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
    void set_read_cb(std::function<bool(const sc_register<DATATYPE>&, DATATYPE&)> read_cb) {
        for(auto& reg : _reg_field)
            reg.set_read_cb(read_cb);
    }
    /**
     * set the read callback triggered upon a read request
     *
     * @param read_cb
     */
    void set_read_cb(std::function<bool(const sc_register<DATATYPE>&, DATATYPE&, sc_core::sc_time)> read_cb) {
        for(auto& reg : _reg_field)
            reg.set_read_cb(read_cb);
    }
    /**
     * set the write callback triggered upon a write request without forwarding the annotated time
     * this is primary for backward compatibility
     *
     * @param write_cb
     */
    void set_write_cb(std::function<bool(sc_register<DATATYPE>&, DATATYPE const&)> write_cb) {
        for(auto& reg : _reg_field)
            reg.set_write_cb(write_cb);
    }
    /**
     * set the write callback triggered upon a write request
     *
     * @param write_cb
     */
    void set_write_cb(std::function<bool(sc_register<DATATYPE>&, DATATYPE const&, sc_core::sc_time)> write_cb) {
        for(auto& reg : _reg_field)
            reg.set_write_cb(write_cb);
    }
    /**
     * Element access operator
     *
     * @param idx the index
     * @return the data reference at the index
     */
    reference operator[](size_t idx) noexcept override { return _reg_field[idx]; }
    /**
     * const element access operator
     *
     * @param idx
     * @return the data reference at the index
     */
    const_reference operator[](size_t idx) const noexcept override { return _reg_field[idx]; }
    /**
     * Element access operator
     *
     * @param idx the index
     * @return the data reference at the index
     */
    reference at(size_t idx) override {
        assert("access out of bound" && idx < SIZE);
        return _reg_field[idx];
    }
    /**
     * const element access operator
     *
     * @param idx
     * @return the data reference at the index
     */
    const_reference at(size_t idx) const override {
        assert("access out of bound" && idx < SIZE);
        return _reg_field[idx];
    }

private:
    sc_core::sc_vector<value_type> _reg_field;
};
/**
 * alias class to map template argument read an write mask to constructor arguments
 */
template <typename DATATYPE, DATATYPE WRMASK = impl::get_max_uval<DATATYPE>(),
          DATATYPE RDMASK = impl::get_max_uval<DATATYPE>()>
class sc_register_masked : public sc_register<DATATYPE> {
public:
    sc_register_masked(sc_core::sc_module_name nm, DATATYPE& storage, const DATATYPE reset_val, resetable& owner)
    : sc_register<DATATYPE>(nm, storage, reset_val, owner, RDMASK, WRMASK) {}
};
} // namespace scc

#endif /* _SYSC_REGISTER_H_ */
