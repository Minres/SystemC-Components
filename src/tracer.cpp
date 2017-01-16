/*
 * tracer.cpp
 *
 *  Created on: Nov 9, 2016
 *      Author: developer
 */

#include <sysc/tracer.h>
#include <sysc/scv_tr_db.h>
#include <sysc/utilities.h>

#include <cstring>
#include <iostream>
#include <sstream>

using namespace sc_core;

namespace sysc {

tracer::tracer(std::string&& name, file_type type, bool enable)
:sc_core::sc_module(sc_core::sc_module_name(sc_core::sc_gen_unique_name("tracer")))
, enabled(enable)
, trf(nullptr)
#ifdef WITH_SCV
, txdb(nullptr)
#endif
{
	if(type!=NONE){
	    std::stringstream ss;
	    ss<<name;
		trf =  sc_create_vcd_trace_file(name.c_str());
		trf->set_time_unit(1, SC_NS);
#ifdef WITH_SCV
		switch(type){
		case TEXT:
	        scv_tr_text_init();
	        ss << ".txlog";
	        break;
		case COMPRESSED:
		    scv_tr_compressed_init();
            ss << ".txlgz";
		    break;
		case SQLITE:
		    scv_tr_sqlite_init();
		    break;
		}
		txdb = new scv_tr_db(ss.str().c_str());
		scv_tr_db::set_default_db(txdb);
#endif
	}
}

void tracer::end_of_elaboration(){
    if(enabled) descend(sc_get_top_level_objects(sc_curr_simcontext));
}

tracer::~tracer() {
	if(trf) sc_close_vcd_trace_file(trf);
#ifdef WITH_SCV
	delete txdb;
#endif
}

void tracer::descend(const std::vector<sc_object*>& objects) {
	for(auto obj: objects){
		tracable* t = dynamic_cast<tracable*>(obj);
		if(t) t->trace(trf);
		const char* kind = obj->kind();
		if(strcmp(kind, "sc_signal")==0){
			try_trace_signal(obj);
		}else if(strcmp(kind, "sc_inout")==0 || strcmp(kind, "sc_in")==0 || strcmp(kind, "sc_port")==0){
			try_trace_port(obj);
		}
		descend(obj->get_child_objects());
	}
}

#ifndef GEN_TRACE
#define GEN_TRACE
#endif

#define GEN_TRACE_STD \
		GEN_TRACE(bool); \
		GEN_TRACE(char); \
		GEN_TRACE(unsigned char); \
		GEN_TRACE(short); \
		GEN_TRACE(unsigned short); \
		GEN_TRACE(int); \
		GEN_TRACE(unsigned int); \
		GEN_TRACE(long); \
		GEN_TRACE(unsigned long); \
		GEN_TRACE(long long); \
		GEN_TRACE(unsigned long long); \
		GEN_TRACE( sc_dt::int64 ); \
		GEN_TRACE( sc_dt::uint64 ); \
		GEN_TRACE( sc_core::sc_time ); \
		GEN_TRACE( sc_dt::sc_bit ); \
		GEN_TRACE( sc_dt::sc_logic ); \
		GEN_TRACE( sc_dt::sc_int_base ); \
		GEN_TRACE( sc_dt::sc_uint_base ); \
		GEN_TRACE( sc_dt::sc_signed ); \
		GEN_TRACE( sc_dt::sc_unsigned ); \
		GEN_TRACE( sc_dt::sc_bv_base ); \
		GEN_TRACE( sc_dt::sc_lv_base )

#ifdef SC_INCLUDE_FX
#define GEN_TRACE_FX\
		GEN_TRACE( sc_dt::sc_fxval );\
		GEN_TRACE( sc_dt::sc_fxval_fast );\
		GEN_TRACE( sc_dt::sc_fxnum );\
		GEN_TRACE( sc_dt::sc_fxnum_fast )
#else
#define GEN_TRACE_FX
#endif

void tracer::try_trace_signal(sc_core::sc_object* obj){
#undef GEN_TRACE
#define GEN_TRACE(X){sc_core::sc_signal<X>* sig = dynamic_cast<sc_core::sc_signal<X>*>(obj);if(sig) sc_core::sc_trace( trf, sig->read(), std::string(sig->name()) );}
	GEN_TRACE_STD;
	GEN_TRACE_FX
}

void tracer::try_trace_port(sc_core::sc_object* obj){
#undef GEN_TRACE
#define GEN_TRACE(X){sc_core::sc_in<X>* in_port = dynamic_cast<sc_core::sc_in<X>*>(obj);if(in_port){sc_core::sc_trace(trf, *in_port, std::string(in_port->name()));return;}}
	GEN_TRACE_STD;
	GEN_TRACE_FX
#undef GEN_TRACE
#define GEN_TRACE(X){sc_core::sc_inout<X>* io_port = dynamic_cast<sc_core::sc_inout<X>*>(obj);if(io_port){sc_core::sc_trace(trf, *io_port, std::string(io_port->name()));return;}}
	GEN_TRACE_STD;
	GEN_TRACE_FX
}


} /* namespace sysc */
