/*******************************************************************************
 * Copyright 2017, 2018 MINRES Technologies GmbH
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
 * tracer.cpp
 *
 *  Created on: Nov 9, 2016
 *      Author: developer
 */

#include "scc/tracer_base.h"
#include "scc/traceable.h"
#include <cstring>
#include <systemc>

using namespace sc_core;
using namespace sc_dt;
using namespace scc;

template <typename T, typename std::enable_if<std::is_class<T>::value, int>::type = 0>
inline bool trace_helper(sc_trace_file* trace_file, const sc_object* object) {
    auto ptr = dynamic_cast<const T*>(object);
    if(ptr) {
        sc_core::sc_trace(trace_file, *ptr, object->name());
        return true;
    }
    return false;
}

template <typename T, typename std::enable_if<!std::is_class<T>::value, unsigned>::type = 0>
inline bool trace_helper(sc_trace_file*, const sc_object*) {
    return false;
}

template <typename T> inline bool try_trace_obj(sc_trace_file* trace_file, const sc_object* object) {
    if(trace_helper<sc_core::sc_in<T>>(trace_file, object))
        return true;
    if(trace_helper<sc_core::sc_out<T>>(trace_file, object))
        return true;
    if(trace_helper<sc_core::sc_signal<T, SC_ONE_WRITER>>(trace_file, object))
        return true;
    if(trace_helper<sc_core::sc_signal<T, SC_MANY_WRITERS>>(trace_file, object))
        return true;
    if(trace_helper<sc_core::sc_signal<T, SC_UNCHECKED_WRITERS>>(trace_file, object))
        return true;
    if(trace_helper<T>(trace_file, object))
        return true;
    return false;
}

void tracer_base::try_trace(sc_trace_file* trace_file, const sc_object* object) {
    if(try_trace_obj<bool>(trace_file, object))
        return;

    if(try_trace_obj<char>(trace_file, object))
        return;
    if(try_trace_obj<unsigned char>(trace_file, object))
        return;

    if(try_trace_obj<short>(trace_file, object))
        return;
    if(try_trace_obj<unsigned short>(trace_file, object))
        return;

    if(try_trace_obj<int>(trace_file, object))
        return;
    if(try_trace_obj<unsigned int>(trace_file, object))
        return;

    if(try_trace_obj<long>(trace_file, object))
        return;
    if(try_trace_obj<unsigned long>(trace_file, object))
        return;

    if(try_trace_obj<long long>(trace_file, object))
        return;
    if(try_trace_obj<unsigned long long>(trace_file, object))
        return;

    if(try_trace_obj<float>(trace_file, object))
        return;
    if(try_trace_obj<double>(trace_file, object))
        return;
#if(SYSTEMC_VERSION >= 20171012)
    if(try_trace_obj<sc_time>(trace_file, object))
        return;
#endif
    if(try_trace_obj<sc_bit>(trace_file, object))
        return;
    if(try_trace_obj<sc_logic>(trace_file, object))
        return;

    if(try_trace_obj<sc_uint<1>>(trace_file, object))
        return;
    if(try_trace_obj<sc_uint<2>>(trace_file, object))
        return;
    if(try_trace_obj<sc_uint<3>>(trace_file, object))
        return;
    if(try_trace_obj<sc_uint<4>>(trace_file, object))
        return;
    if(try_trace_obj<sc_uint<8>>(trace_file, object))
        return;
    if(try_trace_obj<sc_uint<16>>(trace_file, object))
        return;
    if(try_trace_obj<sc_uint<32>>(trace_file, object))
        return;
    if(try_trace_obj<sc_uint<64>>(trace_file, object))
        return;

    if(try_trace_obj<sc_int<1>>(trace_file, object))
        return;
    if(try_trace_obj<sc_int<2>>(trace_file, object))
        return;
    if(try_trace_obj<sc_int<3>>(trace_file, object))
        return;
    if(try_trace_obj<sc_int<4>>(trace_file, object))
        return;
    if(try_trace_obj<sc_int<8>>(trace_file, object))
        return;
    if(try_trace_obj<sc_int<16>>(trace_file, object))
        return;
    if(try_trace_obj<sc_int<32>>(trace_file, object))
        return;
    if(try_trace_obj<sc_int<64>>(trace_file, object))
        return;

    if(try_trace_obj<sc_biguint<32>>(trace_file, object))
        return;
    if(try_trace_obj<sc_biguint<64>>(trace_file, object))
        return;
    if(try_trace_obj<sc_biguint<128>>(trace_file, object))
        return;
    if(try_trace_obj<sc_biguint<256>>(trace_file, object))
        return;
    if(try_trace_obj<sc_biguint<512>>(trace_file, object))
        return;
    if(try_trace_obj<sc_biguint<1024>>(trace_file, object))
        return;

    if(try_trace_obj<sc_bigint<32>>(trace_file, object))
        return;
    if(try_trace_obj<sc_bigint<64>>(trace_file, object))
        return;
    if(try_trace_obj<sc_bigint<128>>(trace_file, object))
        return;
    if(try_trace_obj<sc_bigint<256>>(trace_file, object))
        return;
    if(try_trace_obj<sc_bigint<512>>(trace_file, object))
        return;
    if(try_trace_obj<sc_bigint<1024>>(trace_file, object))
        return;

    if(try_trace_obj<sc_bv<1>>(trace_file, object))
        return;
    if(try_trace_obj<sc_bv<2>>(trace_file, object))
        return;
    if(try_trace_obj<sc_bv<3>>(trace_file, object))
        return;
    if(try_trace_obj<sc_bv<4>>(trace_file, object))
        return;
    if(try_trace_obj<sc_bv<8>>(trace_file, object))
        return;
    if(try_trace_obj<sc_bv<16>>(trace_file, object))
        return;
    if(try_trace_obj<sc_bv<32>>(trace_file, object))
        return;
    if(try_trace_obj<sc_bv<64>>(trace_file, object))
        return;
    if(try_trace_obj<sc_bv<128>>(trace_file, object))
        return;
    if(try_trace_obj<sc_bv<256>>(trace_file, object))
        return;
    if(try_trace_obj<sc_bv<512>>(trace_file, object))
        return;

    if(try_trace_obj<sc_lv<1>>(trace_file, object))
        return;
    if(try_trace_obj<sc_lv<2>>(trace_file, object))
        return;
    if(try_trace_obj<sc_lv<3>>(trace_file, object))
        return;
    if(try_trace_obj<sc_lv<4>>(trace_file, object))
        return;
    if(try_trace_obj<sc_lv<8>>(trace_file, object))
        return;
    if(try_trace_obj<sc_lv<16>>(trace_file, object))
        return;
    if(try_trace_obj<sc_lv<32>>(trace_file, object))
        return;
    if(try_trace_obj<sc_lv<64>>(trace_file, object))
        return;
    if(try_trace_obj<sc_lv<128>>(trace_file, object))
        return;
    if(try_trace_obj<sc_lv<256>>(trace_file, object))
        return;
    if(try_trace_obj<sc_lv<512>>(trace_file, object))
        return;
}

void tracer_base::descend(const sc_object* obj, bool trace_all) {
    if(obj == this)
        return;
    const char* kind = obj->kind();
    if(strcmp(kind, "tlm_signal") == 0) {
        obj->trace(trf);
        return;
    } else if(strcmp(kind, "sc_vector") == 0) {
        for(auto o : obj->get_child_objects())
            descend(o, trace_all);
        return;
    } else if((strcmp(kind, "sc_module") == 0 && trace_all) || dynamic_cast<const traceable*>(obj)) {
        obj->trace(trf);
        for(auto o : obj->get_child_objects())
            descend(o, trace_all);
    } else
        try_trace(trf, obj);
}
