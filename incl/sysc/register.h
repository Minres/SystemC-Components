/*******************************************************************************
 * Copyright 2016 MINRES Technologies GmbH
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
/*
 * register.h
 *
 *  Created on: Nov 16, 2016
 *      Author: developer
 */

#ifndef _SYSC_REGISTER_H_
#define _SYSC_REGISTER_H_

#include "utilities.h"
#include "resetable.h"
#include "resource_access_if.h"

#include <functional>
#include <limits>

namespace sysc {

namespace impl{

template<typename T, bool = std::is_integral<T>::value>
struct helper {
};

template<typename T>
struct helper<T, true> {
    using Type = T;
    template<typename Type>
    constexpr Type get_max_uval(){
        return std::numeric_limits<Type>::is_signed?-1:std::numeric_limits<Type>::max();
    }
};

template<typename T>
struct helper<T, false> {
    using Type = typename T::StorageType;
    template<typename Type>
    constexpr Type get_max_uval(){
        return std::numeric_limits<Type>::is_signed?-1:std::numeric_limits<Type>::max();
    }
};

template<typename Type>
constexpr Type get_max_uval(){
    return std::numeric_limits<Type>::is_signed?-1:std::numeric_limits<Type>::max();
}

template<typename DATATYPE>
struct sc_register:
        public sc_core::sc_object,
        public resource_access_if,
        public traceable
        {

    using this_type = struct sc_register<DATATYPE>;
/**
 *
 * @param nm
 * @param storage
 * @param reset_val
 * @param owner
 * @param rdmask
 * @param wrmask
 */
    sc_register(sc_core::sc_module_name nm, DATATYPE& storage, const DATATYPE reset_val, resetable& owner,
            DATATYPE rdmask = get_max_uval<DATATYPE>(), DATATYPE wrmask = get_max_uval<DATATYPE>())
    : sc_core::sc_object(nm)
    , res_val(reset_val)
    , rdmask(rdmask)
    , wrmask(wrmask)
    , storage(storage)
    {
        owner.register_resource(this);
    }
/**
 *
 * @return
 */
    size_t size() const override {return sizeof(DATATYPE);}
/**
 *
 */
    void reset() override {
        DATATYPE r(res_val);
        if(wr_cb) wr_cb(*this, r);
        storage=r;
    }
/**
 *
 * @param data
 * @param length
 * @param offset
 * @return
 */
    bool write(const uint8_t* data, size_t length, uint64_t offset) override {
        assert("Offset out of range" && offset==0);
        if(length!=sizeof(DATATYPE))return false;
        auto temp(*reinterpret_cast<const DATATYPE*>(data));
        if(wr_cb) return wr_cb(*this, temp);
        storage=(temp&wrmask) | (storage&~wrmask);
        return true;
    }
/**
 *
 * @param data
 * @param length
 * @param offset
 * @return
 */
    bool read(uint8_t* data, size_t length, uint64_t offset) const override {
        assert("Offset out of range" && offset==0);
        if(length!=sizeof(DATATYPE))return false;
        auto temp(storage);
        if(rd_cb) return rd_cb(*this, temp);
        *reinterpret_cast<DATATYPE*>(data)=temp&rdmask;
        return true;
    }
/**
 *
 * @param data
 * @param length
 * @param offset
 * @return
 */
    bool write_dbg(const uint8_t* data, size_t length, uint64_t offset) override {
        assert("Offset out of range" && offset==0);
        if(length!=sizeof(DATATYPE))return false;
        storage=*reinterpret_cast<const DATATYPE*>(data);
        return true;
    }
    /**
     *
     * @param data
     * @param length
     * @param offset
     * @return
     */
    bool read_dbg(uint8_t* data, size_t length, uint64_t offset) const override {
        assert("Offset out of range" && offset==0);
        if(length!=sizeof(DATATYPE))return false;
        *reinterpret_cast<DATATYPE*>(data)=storage;
        return true;
    }
    /**
     *
     */
    operator DATATYPE() const {
        return storage;
    }
    /**
     *
     * @return
     */
    DATATYPE get() const {
        return storage;
    }
    /**
     *
     * @param o
     */
    void put(DATATYPE o) const {
        storage=o;
    }
    /**
     *
     * @param other
     * @return
     */
    this_type& operator=(DATATYPE other){
        storage=other;
        return *this;
    }
    /**
     *
     * @param other
     * @return
     */
    this_type& operator|=(DATATYPE other){
        storage|=other;
        return *this;
    }
    /**
     *
     * @param other
     * @return
     */
    this_type& operator&=(DATATYPE other){
        storage&=other;
        return *this;
    }
    /**
     *
     * @param read_cb
     */
    void set_read_cb(std::function<bool(const this_type&, DATATYPE&)> read_cb){
        rd_cb=read_cb;
    }
    /**
     *
     * @param write_cb
     */
    void set_write_cb(std::function<bool(this_type&, DATATYPE&)> write_cb){
        wr_cb=write_cb;
    }
    /**
     *
     * @param trf
     */
    void trace(sc_core::sc_trace_file* trf){
        sc_trace(trf, storage, this->name());
    }

    const DATATYPE res_val;
    const DATATYPE rdmask;
    const DATATYPE wrmask;

private:
    DATATYPE& storage;
    std::function<bool(const this_type&, DATATYPE&)> rd_cb;
    std::function<bool(this_type&, DATATYPE&)> wr_cb;
};

}


template<typename DATATYPE>
using sc_register = impl::sc_register<typename impl::helper<DATATYPE>::Type>;


template<typename DATATYPE, size_t SIZE, size_t START=0>
struct sc_register_indexed: public indexed_resource_access_if {

    using BASE_DATA_TYPE = typename impl::helper<DATATYPE>::Type;

    using value_type = sc_register<DATATYPE>;
    using pointer = value_type*;

    sc_register_indexed(sc_core::sc_module_name nm, std::array<DATATYPE, SIZE>& storage, const DATATYPE reset_val, resetable& owner,
            BASE_DATA_TYPE rdmask = std::numeric_limits<BASE_DATA_TYPE>::is_signed?-1:std::numeric_limits<BASE_DATA_TYPE>::max(),
                    BASE_DATA_TYPE wrmask = std::numeric_limits<BASE_DATA_TYPE>::is_signed?-1:std::numeric_limits<BASE_DATA_TYPE>::max())
    {
        _reg_field=reinterpret_cast<pointer>(malloc(SIZE*sizeof(value_type)));
        for(size_t idx=START; idx<(START+SIZE); ++idx){
            std::stringstream ss;
            ss<<nm<<idx;
            new (_reg_field+idx) sc_register<DATATYPE>(ss.str().c_str(), storage[idx], reset_val, owner, rdmask, wrmask);
        }
    }

    ~sc_register_indexed(){
        for(size_t idx=START; idx<(START+SIZE); ++idx){
            (_reg_field+idx)->~sc_register<DATATYPE>();
        }
        free(_reg_field);
    }

    size_t size() override { return SIZE;};

    void set_read_cb(std::function<bool(const sc_register<DATATYPE>&, DATATYPE&)> read_cb){
        for(std::unique_ptr<sc_register<DATATYPE>> reg: *this) reg->add_read_cb(read_cb);
    }

    void set_write_cb(std::function<bool(sc_register<DATATYPE>&, DATATYPE&)> write_cb){
        for(std::unique_ptr<sc_register<DATATYPE>> reg: *this) reg->add_write_cb(write_cb);
    }

    // Element access.
    indexed_resource_access_if::reference
    operator[](size_t __n) noexcept override
    { return *(_reg_field+__n); }

    indexed_resource_access_if::const_reference
    operator[](size_t __n) const noexcept override
    { return *(_reg_field+__n); }


    reference at(size_t __n) override {
        assert("access out of bound" && __n < SIZE);
        return *(_reg_field+__n);
    }
    const_reference at(size_t __n) const override {
        assert("access out of bound" && __n < SIZE);
        return *(_reg_field+__n);
    }

private:
    value_type* _reg_field;
};

template<
typename DATATYPE,
DATATYPE WRMASK = impl::get_max_uval<DATATYPE>(),
DATATYPE RDMASK = impl::get_max_uval<DATATYPE>()
>
struct sc_register_masked: public sc_register<DATATYPE> {

    sc_register_masked(sc_core::sc_module_name nm, DATATYPE& storage, const DATATYPE reset_val, resetable& owner)
    : sc_register<DATATYPE>(nm, storage, reset_val, owner, RDMASK, WRMASK)
      { }

};
}

#endif /* _SYSC_REGISTER_H_ */
