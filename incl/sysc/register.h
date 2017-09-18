/*
 * register.h
 *
 *  Created on: Nov 16, 2016
 *      Author: developer
 */

#ifndef SYSC_REGISTER_H_
#define SYSC_REGISTER_H_

#include <systemc>

#include "resetable.h"
#include "resource_access_if.h"
#include "traceable.h"

#include <functional>
#include <limits>

namespace sysc {

template<typename DATATYPE>
constexpr DATATYPE get_max_uval(){
	return std::numeric_limits<DATATYPE>::is_signed?-1:std::numeric_limits<DATATYPE>::max();
}

template<typename DATATYPE>
struct sc_register:
		public sc_core::sc_object,
		public resource_access_if,
		public traceable
		{

	using this_reg_type = struct sc_register<DATATYPE>;

	sc_register(sc_core::sc_module_name nm, DATATYPE& storage, const DATATYPE reset_val, resetable& owner,
			DATATYPE rdmask = get_max_uval<DATATYPE>(), DATATYPE wrmask = get_max_uval<DATATYPE>())
	: sc_core::sc_object(nm)
	, storage(storage)
	, res_val(reset_val)
	, rdmask(rdmask)
	, wrmask(wrmask)
    {
        owner.register_resource(this);
    }

    virtual ~sc_register(){ }

    size_t size() const {return sizeof(DATATYPE);}

    void reset(){
        DATATYPE r(res_val);
        if(wr_cb) wr_cb(*this, r);
        storage=r;
    }

    bool write(const uint8_t* data, size_t length){
        if(length!=sizeof(DATATYPE))return false;
        auto temp(*reinterpret_cast<const DATATYPE*>(data));
        if(wr_cb) return wr_cb(*this, temp);
        storage=(temp&wrmask) | (storage&~wrmask);
        return true;
    }

    bool read(uint8_t* data, size_t length) const {
        if(length!=sizeof(DATATYPE))return false;
        auto temp(storage);
        if(rd_cb) return rd_cb(*this, temp);
        *reinterpret_cast<DATATYPE*>(data)=temp&rdmask;
        return true;
    }

    bool write_dbg(const uint8_t* data, size_t length){
        if(length!=sizeof(DATATYPE))return false;
        storage=*reinterpret_cast<const DATATYPE*>(data);
        return true;
    }

    bool read_dbg(uint8_t* data, size_t length) const {
        if(length!=sizeof(DATATYPE))return false;
        *reinterpret_cast<DATATYPE*>(data)=storage;
        return true;
    }

    operator DATATYPE() const {
        return storage;
    }

    DATATYPE get() const {
        return storage;
    }

    void put(DATATYPE o) const {
        storage=o;
    }

    this_reg_type& operator=(DATATYPE other){
        storage=other;
        return *this;
    }

    this_reg_type& operator|=(DATATYPE other){
        storage|=other;
        return *this;
    }

    this_reg_type& operator&=(DATATYPE other){
        storage&=other;
        return *this;
    }

    void add_read_cb(std::function<bool(const this_reg_type&, DATATYPE&)> read_cb){
        rd_cb=read_cb;
    }

    void add_write_cb(std::function<bool(this_reg_type&, DATATYPE&)> write_cb){
        wr_cb=write_cb;
    }

    void trace(sc_core::sc_trace_file* trf){
        sc_trace(trf, storage, this->name());
    }

    const DATATYPE res_val;
    const DATATYPE rdmask;
    const DATATYPE wrmask;
private:
    DATATYPE& storage;
    std::function<bool(const this_reg_type&, DATATYPE&)> rd_cb;
    std::function<bool(this_reg_type&, DATATYPE&)> wr_cb;
};

template<
	typename DATATYPE,
	DATATYPE WRMASK = get_max_uval<DATATYPE>(),
	DATATYPE RDMASK = get_max_uval<DATATYPE>()
	>
struct sc_register_masked: public sc_register<DATATYPE> {

		sc_register_masked(sc_core::sc_module_name nm, DATATYPE& storage, const DATATYPE reset_val, resetable& owner)
		: sc_register<DATATYPE>(nm, storage, reset_val, owner, RDMASK, WRMASK)
	    { }
};
}

#endif /* SYSC_REGISTER_H_ */
