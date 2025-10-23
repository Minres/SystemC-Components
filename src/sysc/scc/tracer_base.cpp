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

#include "tracer_base.h"
#include "observer.h"
#include "sc_variable.h"
#include "traceable.h"
#include <cstring>
#include <systemc>

#define SC_TRACE_NS ::scc::

using namespace sc_core;
using namespace sc_dt;

namespace scc {
using sc_trace_file = sc_core::sc_trace_file;
using sc_object = sc_core::sc_object;

template <typename T> inline auto try_trace_obj(sc_trace_file* trace_file, const sc_object* object, trace_types types_to_trace) -> bool {
    if((types_to_trace & trace_types::PORTS) == trace_types::PORTS) {
        if(auto const* ptr = dynamic_cast<sc_core::sc_in<T> const*>(object)) {
            if(auto* if_ptr = ptr->get_interface(0)) {
                SC_TRACE_NS sc_trace(trace_file, *if_ptr, object->name());
                return true;
            }
        }
        if(auto const* ptr = dynamic_cast<sc_core::sc_inout<T> const*>(object)) {
            if(auto* if_ptr = ptr->get_interface(0)) {
                SC_TRACE_NS sc_trace(trace_file, *if_ptr, object->name());
                return true;
            }
        }
    }
    if((types_to_trace & trace_types::SIGNALS) == trace_types::SIGNALS) {
        if(auto const* ptr = dynamic_cast<sc_core::sc_signal_inout_if<T> const*>(object)) {
            SC_TRACE_NS sc_trace(trace_file, *ptr, object->name());
            return true;
        }
    }
    return false;
}

template <size_t SIZE> struct ForLoop {
    template <template <size_t> class Func>
    static bool iterate(sc_trace_file* trace_file, const sc_object* object, trace_types types_to_trace) {
        if(ForLoop<SIZE - (SIZE > 128 ? 8 : 1)>::template iterate<Func>(trace_file, object, types_to_trace))
            return true;
        else
            return Func<SIZE>()(trace_file, object, types_to_trace);
    }
};

template <> struct ForLoop<1> {
    template <template <size_t> class Func>
    static bool iterate(sc_trace_file* trace_file, const sc_object* object, trace_types types_to_trace) {
        return Func<1>()(trace_file, object, types_to_trace);
    }
};

template <size_t size> struct sc_uint_tester {
    bool operator()(sc_trace_file* trace_file, const sc_object* object, trace_types types_to_trace) {
        return try_trace_obj<sc_uint<size>>(trace_file, object, types_to_trace);
    }
};

template <size_t size> struct sc_int_tester {
    bool operator()(sc_trace_file* trace_file, const sc_object* object, trace_types types_to_trace) {
        return try_trace_obj<sc_int<size>>(trace_file, object, types_to_trace);
    }
};

template <size_t size> struct sc_biguint_tester {
    bool operator()(sc_trace_file* trace_file, const sc_object* object, trace_types types_to_trace) {
        return try_trace_obj<sc_biguint<size>>(trace_file, object, types_to_trace);
    }
};

template <size_t size> struct sc_bigint_tester {
    bool operator()(sc_trace_file* trace_file, const sc_object* object, trace_types types_to_trace) {
        return try_trace_obj<sc_bigint<size>>(trace_file, object, types_to_trace);
    }
};

template <size_t size> struct sc_bv_tester {
    bool operator()(sc_trace_file* trace_file, const sc_object* object, trace_types types_to_trace) {
        return try_trace_obj<sc_bv<size>>(trace_file, object, types_to_trace);
    }
};

template <size_t size> struct sc_lv_tester {
    bool operator()(sc_trace_file* trace_file, const sc_object* object, trace_types types_to_trace) {
        return try_trace_obj<sc_lv<size>>(trace_file, object, types_to_trace);
    }
};

void tracer_base::try_trace(sc_trace_file* trace_file, const sc_object* object, trace_types types_to_trace) {
    if(try_trace_obj<bool>(trace_file, object, types_to_trace))
        return;

    if(try_trace_obj<char>(trace_file, object, types_to_trace))
        return;
    if(try_trace_obj<unsigned char>(trace_file, object, types_to_trace))
        return;

    if(try_trace_obj<short>(trace_file, object, types_to_trace))
        return;
    if(try_trace_obj<unsigned short>(trace_file, object, types_to_trace))
        return;

    if(try_trace_obj<int>(trace_file, object, types_to_trace))
        return;
    if(try_trace_obj<unsigned int>(trace_file, object, types_to_trace))
        return;

    if(try_trace_obj<long>(trace_file, object, types_to_trace))
        return;
    if(try_trace_obj<unsigned long>(trace_file, object, types_to_trace))
        return;

    if(try_trace_obj<long long>(trace_file, object, types_to_trace))
        return;
    if(try_trace_obj<unsigned long long>(trace_file, object, types_to_trace))
        return;

    if(try_trace_obj<float>(trace_file, object, types_to_trace))
        return;
    if(try_trace_obj<double>(trace_file, object, types_to_trace))
        return;
#if(SYSTEMC_VERSION >= 20171012)
    if(try_trace_obj<sc_core::sc_time>(trace_file, object, types_to_trace))
        return;
#endif
    if(try_trace_obj<sc_bit>(trace_file, object, types_to_trace))
        return;
    if(try_trace_obj<sc_logic>(trace_file, object, types_to_trace))
        return;
#if defined(FULL_TRACE_TYPE_LIST)
    if(ForLoop<64>::iterate<sc_uint_tester>(trace_file, object, types_to_trace))
        return;
    if(ForLoop<64>::iterate<sc_int_tester>(trace_file, object, types_to_trace))
        return;
    if(ForLoop<1024>::iterate<sc_biguint_tester>(trace_file, object, types_to_trace))
        return;
    if(ForLoop<1024>::iterate<sc_bigint_tester>(trace_file, object, types_to_trace))
        return;
    if(ForLoop<1024>::iterate<sc_bv_tester>(trace_file, object, types_to_trace))
        return;
    if(ForLoop<1024>::iterate<sc_lv_tester>(trace_file, object, types_to_trace))
        return;
#else
    if(ForLoop<31>::iterate<sc_uint_tester>(trace_file, object, types_to_trace))
        return;
    if(try_trace_obj<sc_uint<32>>(trace_file, object, types_to_trace))
        return;
    if(try_trace_obj<sc_uint<40>>(trace_file, object, types_to_trace))
        return;
    if(try_trace_obj<sc_uint<48>>(trace_file, object, types_to_trace))
        return;
    if(try_trace_obj<sc_uint<56>>(trace_file, object, types_to_trace))
        return;
    if(try_trace_obj<sc_uint<64>>(trace_file, object, types_to_trace))
        return;

    if(ForLoop<31>::iterate<sc_int_tester>(trace_file, object, types_to_trace))
        return;
    if(try_trace_obj<sc_int<32>>(trace_file, object, types_to_trace))
        return;
    if(try_trace_obj<sc_int<40>>(trace_file, object, types_to_trace))
        return;
    if(try_trace_obj<sc_int<48>>(trace_file, object, types_to_trace))
        return;
    if(try_trace_obj<sc_int<56>>(trace_file, object, types_to_trace))
        return;
    if(try_trace_obj<sc_int<64>>(trace_file, object, types_to_trace))
        return;

    if(try_trace_obj<sc_biguint<32>>(trace_file, object, types_to_trace))
        return;
    if(try_trace_obj<sc_biguint<64>>(trace_file, object, types_to_trace))
        return;
    if(try_trace_obj<sc_biguint<128>>(trace_file, object, types_to_trace))
        return;
    if(try_trace_obj<sc_biguint<256>>(trace_file, object, types_to_trace))
        return;
    if(try_trace_obj<sc_biguint<384>>(trace_file, object, types_to_trace))
        return;
    if(try_trace_obj<sc_biguint<512>>(trace_file, object, types_to_trace))
        return;
    if(try_trace_obj<sc_biguint<1024>>(trace_file, object, types_to_trace))
        return;

    if(try_trace_obj<sc_bigint<32>>(trace_file, object, types_to_trace))
        return;
    if(try_trace_obj<sc_bigint<64>>(trace_file, object, types_to_trace))
        return;
    if(try_trace_obj<sc_bigint<128>>(trace_file, object, types_to_trace))
        return;
    if(try_trace_obj<sc_bigint<256>>(trace_file, object, types_to_trace))
        return;
    if(try_trace_obj<sc_bigint<384>>(trace_file, object, types_to_trace))
        return;
    if(try_trace_obj<sc_bigint<512>>(trace_file, object, types_to_trace))
        return;
    if(try_trace_obj<sc_bigint<1024>>(trace_file, object, types_to_trace))
        return;

    if(ForLoop<31>::iterate<sc_bv_tester>(trace_file, object, types_to_trace))
        return;
    if(try_trace_obj<sc_bv<32>>(trace_file, object, types_to_trace))
        return;
    if(try_trace_obj<sc_bv<40>>(trace_file, object, types_to_trace))
        return;
    if(try_trace_obj<sc_bv<48>>(trace_file, object, types_to_trace))
        return;
    if(try_trace_obj<sc_bv<56>>(trace_file, object, types_to_trace))
        return;
    if(try_trace_obj<sc_bv<64>>(trace_file, object, types_to_trace))
        return;
    if(try_trace_obj<sc_bv<128>>(trace_file, object, types_to_trace))
        return;
    if(try_trace_obj<sc_bv<256>>(trace_file, object, types_to_trace))
        return;
    if(try_trace_obj<sc_bv<384>>(trace_file, object, types_to_trace))
        return;
    if(try_trace_obj<sc_bv<512>>(trace_file, object, types_to_trace))
        return;
    if(try_trace_obj<sc_bv<1024>>(trace_file, object, types_to_trace))
        return;

    if(ForLoop<31>::iterate<sc_lv_tester>(trace_file, object, types_to_trace))
        return;
    if(try_trace_obj<sc_lv<32>>(trace_file, object, types_to_trace))
        return;
    if(try_trace_obj<sc_lv<40>>(trace_file, object, types_to_trace))
        return;
    if(try_trace_obj<sc_lv<48>>(trace_file, object, types_to_trace))
        return;
    if(try_trace_obj<sc_lv<56>>(trace_file, object, types_to_trace))
        return;
    if(try_trace_obj<sc_lv<64>>(trace_file, object, types_to_trace))
        return;
    if(try_trace_obj<sc_lv<128>>(trace_file, object, types_to_trace))
        return;
    if(try_trace_obj<sc_lv<256>>(trace_file, object, types_to_trace))
        return;
    if(try_trace_obj<sc_lv<384>>(trace_file, object, types_to_trace))
        return;
    if(try_trace_obj<sc_lv<512>>(trace_file, object, types_to_trace))
        return;
    if(try_trace_obj<sc_lv<1024>>(trace_file, object, types_to_trace))
        return;
#endif
}

std::string tracer_base::get_name() { return sc_core::sc_gen_unique_name("$$$scc_tracer$$$", true); }

void tracer_base::descend(const sc_object* obj, bool trace_all) {
    if(obj == this)
        return;
    const char* kind = obj->kind();
    if(strcmp(kind, "tlm_signal") == 0) {
        obj->trace(trf);
    } else if(strcmp(kind, "sc_vector") == 0) {
        for(auto o : obj->get_child_objects())
            descend(o, trace_all);
    } else if((strcmp(kind, "sc_module") == 0 && trace_all) || dynamic_cast<const traceable*>(obj)) {
        obj->trace(trf);
        for(auto o : obj->get_child_objects())
            descend(o, trace_all);
    } else if(strcmp(kind, "sc_variable") == 0) {
        if((types_to_trace & trace_types::VARIABLES) == trace_types::VARIABLES)
            obj->trace(trf);
    } else if(const auto* tr = dynamic_cast<const scc::traceable*>(obj)) {
        if(tr->is_trace_enabled())
            obj->trace(trf);
        for(auto o : obj->get_child_objects())
            descend(o, tr->is_trace_enabled());
    } else {
        try_trace(trf, obj, types_to_trace);
    }
}

namespace {
tracer_base* find_tracer_base(sc_core::sc_object* obj) {
    if(auto p = dynamic_cast<tracer_base*>(obj))
        return p;
    for(auto* o : obj->get_child_objects()) {
        if(auto p = find_tracer_base(o))
            return p;
    }
    return nullptr;
}
} // namespace

static char const* const default_trace_enable_name = "scc_tracer.default_trace_enable";
tracer_base::tracer_base(const sc_core::sc_module_name& nm, sc_core::sc_trace_file* tf, bool owned)
  : sc_core::sc_module(nm)
  , cci_broker(cci::cci_get_broker())
  , trf(tf) {
    default_trace_enable_handle = cci_broker.get_param_handle(default_trace_enable_name);
    if(!default_trace_enable_handle.is_valid()) {
        default_trace_enable = scc::make_unique<cci::cci_param<bool>>(
            default_trace_enable_name, false, "the default for tracing if no attribute is configured", cci::CCI_ABSOLUTE_NAME);
        default_trace_enable_handle = cci_broker.get_param_handle(default_trace_enable_name);
    }
}

bool tracer_base::get_default_trace_enable() {
    auto cci_broker = cci::cci_get_broker();
    auto default_trace_enable_handle = cci_broker.get_param_handle(default_trace_enable_name);
    if(default_trace_enable_handle.is_valid())
        return default_trace_enable_handle.get_cci_value().get<bool>();
    else
        return false; // if the parameter does not exist, no tracer has been instantiated
}

void tracer_base::set_default_trace_enable(bool v) {
    auto cci_broker = cci::cci_get_broker();
    auto default_trace_enable_handle = cci_broker.get_param_handle(default_trace_enable_name);
    if(default_trace_enable_handle.is_valid())
        default_trace_enable_handle.get_cci_value().set<bool>(v);
}

} // namespace scc
