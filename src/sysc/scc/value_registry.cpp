/*******************************************************************************
 * Copyright 2018-2022 MINRES Technologies GmbH
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

#include "value_registry.h"
#include <cstring>
#include <sstream>
#include <string>
#include <sysc/datatypes/fx/sc_fxnum.h>
#include <sysc/datatypes/fx/sc_fxval.h>
#include <unordered_map>

using namespace sc_core;
namespace scc {

auto operator<<(std::ostream& os, const sc_event& evt) -> std::ostream& {
#if SYSTEMC_VERSION >= 20181013
    os << evt.triggered();
#endif
    return os;
}

#if SC_VERSION_MAJOR <= 2 && SC_VERSION_MINOR <= 3 && SC_VERSION_PATCH < 2
#define OVERRIDE
#elif defined(NCSC)
#define OVERRIDE
#else
#define OVERRIDE override
#endif
class SC_API value_registry_impl : public sc_trace_file {
public:
    // Constructor
    value_registry_impl() = default;

    auto get_mod4name(const std::string& name) const -> sc_module* {
        sc_module* mod = nullptr;
        sc_object* obj = sc_find_object(name.c_str());
        if(obj)
            return mod;
        auto pos = name.length() - 1;
        do {
            pos = name.find_last_of('.', pos);
            mod = dynamic_cast<sc_module*>(sc_find_object(name.substr(0, pos).c_str()));
        } while(pos > 0 && mod == nullptr);
        return mod;
    }
#define DECL_TRACE_METHOD_A(tp)                                                                                        \
    void trace(const tp& object, const std::string& name) OVERRIDE {                                                   \
        if(sc_core::sc_find_object(name.c_str()) != nullptr) {                                                         \
            if(sc_module* mod = get_mod4name(name)) {                                                                  \
                sc_get_curr_simcontext()->hierarchy_push(mod);                                                         \
                auto* o = new sc_ref_variable<tp>(name, object);                                                       \
                sc_get_curr_simcontext()->hierarchy_pop();                                                             \
                holder[name] = o;                                                                                      \
            }                                                                                                          \
        }                                                                                                              \
    }
#define DECL_TRACE_METHOD_B(tp)                                                                                        \
    void trace(const tp& object, const std::string& name, int width) OVERRIDE {                                        \
        if(sc_core::sc_find_object(name.c_str()) != nullptr) {                                                         \
            if(sc_module* mod = get_mod4name(name)) {                                                                  \
                sc_get_curr_simcontext()->hierarchy_push(mod);                                                         \
                auto* o = new sc_ref_variable_masked<tp>(name, object, width);                                         \
                sc_get_curr_simcontext()->hierarchy_pop();                                                             \
                holder[name] = o;                                                                                      \
            }                                                                                                          \
        }                                                                                                              \
    }                                                                                                                  \
	void trace(const tp& object, const std::string& name) { trace(object, name, sizeof(tp)*8); }

    void trace(const bool& object, const std::string& name) override {
        if(sc_core::sc_find_object(name.c_str()) == nullptr) {
            if(sc_module* mod = get_mod4name(name)) {
                sc_get_curr_simcontext()->hierarchy_push(mod);
                auto* o = new sc_ref_variable<bool>(name.substr(strlen(mod->name()) + 1), object);
                sc_get_curr_simcontext()->hierarchy_pop();
                holder[name] = o;
            }
        }
    }
#if (SYSTEMC_VERSION >= 20171012)
    DECL_TRACE_METHOD_A(sc_event) // NOLINT
    DECL_TRACE_METHOD_A(sc_time)
#endif
    DECL_TRACE_METHOD_A(sc_dt::sc_bit)
    DECL_TRACE_METHOD_A(sc_dt::sc_logic)

    DECL_TRACE_METHOD_B(unsigned char)
    DECL_TRACE_METHOD_B(unsigned short)
    DECL_TRACE_METHOD_B(unsigned int)
    DECL_TRACE_METHOD_B(unsigned long)
    DECL_TRACE_METHOD_B(char)
    DECL_TRACE_METHOD_B(short)
    DECL_TRACE_METHOD_B(int)
    DECL_TRACE_METHOD_B(long)
    DECL_TRACE_METHOD_B(sc_dt::int64)
    DECL_TRACE_METHOD_B(sc_dt::uint64)

    DECL_TRACE_METHOD_A(float)
    DECL_TRACE_METHOD_A(double)
    DECL_TRACE_METHOD_A(sc_dt::sc_int_base)
    DECL_TRACE_METHOD_A(sc_dt::sc_uint_base)
    DECL_TRACE_METHOD_A(sc_dt::sc_signed)
    DECL_TRACE_METHOD_A(sc_dt::sc_unsigned)

    DECL_TRACE_METHOD_A(sc_dt::sc_fxval)
    DECL_TRACE_METHOD_A(sc_dt::sc_fxval_fast)
    DECL_TRACE_METHOD_A(sc_dt::sc_fxnum)
    DECL_TRACE_METHOD_A(sc_dt::sc_fxnum_fast)

    DECL_TRACE_METHOD_A(sc_dt::sc_bv_base)
    DECL_TRACE_METHOD_A(sc_dt::sc_lv_base)

#undef DECL_TRACE_METHOD_A
#undef DECL_TRACE_METHOD_B

    // Trace an enumerated object - where possible output the enumeration
    // literals in the trace file. Enum literals is a null terminated array
    // of null terminated char* literal strings.
    void trace(const unsigned int& object, const std::string& name, const char** enum_literals) override {}

    // Output a comment to the trace file
    void write_comment(const std::string& comment) override {}

    // Set the amount of space before next column
    // (For most formats this does nothing)
    // void space( int n );

    // Also trace transitions between delta cycles if flag is true.
    // void delta_cycles( bool flag );

    // Set time unit.
    void set_time_unit(double v, sc_time_unit tu) override{};

    // Write trace info for cycle
    void cycle(bool delta_cycle) override {}

    // Helper for event tracing
    auto event_trigger_stamp(const sc_event& event) const -> const sc_dt::uint64& { return dummy; }

    // Flush results and close file
    ~value_registry_impl() override {
        for(auto kv : holder)
            delete kv.second;
    }

    std::unordered_map<std::string, sc_variable_b*> holder;

    sc_dt::uint64 dummy = 0;
#ifdef NCSC
    void set_time_unit(int exponent10_seconds) override {}
#endif
};

value_registry::value_registry()
: tracer_base(sc_core::sc_module_name(sc_core::sc_gen_unique_name("value_registrar", true))) {
    trf = new value_registry_impl();
}

scc::value_registry::~value_registry() { delete dynamic_cast<value_registry_impl*>(trf); }

auto scc::value_registry::get_names() const -> std::vector<std::string> {
    auto& holder = dynamic_cast<value_registry_impl*>(trf)->holder;
    std::vector<std::string> keys;
    keys.reserve(holder.size());
    for(auto kv : holder)
        keys.push_back(kv.first);
    return keys;
}

auto scc::value_registry::get_value(std::string name) const -> const sc_variable_b* {
    auto* reg = dynamic_cast<value_registry_impl*>(trf);
    auto it = reg->holder.find(name);
    if(it != reg->holder.end()) {
        std::cerr << "returning value holder ptr" << std::endl;
        return it->second;
    }
    std::cerr << "returning nullptr" << std::endl;
    return nullptr;
}

void scc::value_registry::end_of_elaboration() {
    for(auto o : sc_get_top_level_objects())
        descend(o, true);
}
} // namespace scc
