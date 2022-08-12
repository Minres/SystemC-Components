/*******************************************************************************
 * Copyright 2021 MINRES Technologies GmbH
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

#include "vcd_push_trace.hh"
#include "sc_vcd_trace.h"
#include "trace/vcd_trace.hh"
#include "utilities.h"
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <deque>
#include <iomanip>
#include <limits>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

#define FPRINT(FP, FMTSTR)                                                                                             \
    {                                                                                                                  \
        auto buf = fmt::format(FMTSTR);                                                                                \
        std::fwrite(buf.c_str(), 1, buf.size(), FP);                                                                   \
    }
#define FPRINTF(FP, FMTSTR, ...)                                                                                       \
    {                                                                                                                  \
        auto buf = fmt::format(FMTSTR, __VA_ARGS__);                                                                   \
        std::fwrite(buf.c_str(), 1, buf.size(), FP);                                                                   \
    }

namespace scc {
/*******************************************************************************************************
 *
 *******************************************************************************************************/
vcd_push_trace_file::vcd_push_trace_file(const char* name, std::function<bool()>& enable)
: name(name)
, check_enabled(enable) {
    vcd_out = fopen(fmt::format("{}.vcd", name).c_str(), "w");

#if defined(WITH_SC_TRACING_PHASE_CALLBACKS)
    // remove from hierarchy
    sc_object::detach();
    // register regular (non-delta) callbacks
    sc_object::register_simulation_phase_callback(SC_BEFORE_TIMESTEP);
#else // explicitly register with simcontext
    sc_core::sc_get_curr_simcontext()->add_trace_file(this);
#endif
}

vcd_push_trace_file::~vcd_push_trace_file() {
    if(vcd_out) {
        FPRINTF(vcd_out, "#{}\n", sc_core::sc_time_stamp() / 1_ps);
        fclose(vcd_out);
    }
    for(auto t:all_traces) delete t.trc;
}

template <typename T, typename OT = T> bool changed(trace::vcd_trace* trace) {
    if(reinterpret_cast<trace::vcd_trace_t<T, OT>*>(trace)->changed()) {
        reinterpret_cast<trace::vcd_trace_t<T, OT>*>(trace)->update();
        return true;
    } else
        return false;
}
#define DECL_TRACE_METHOD_A(tp)                                                                                        \
    void vcd_push_trace_file::trace(const tp& object, const std::string& name) {                                       \
        all_traces.emplace_back(this, &changed<tp>, new trace::vcd_trace_t<tp>(object, name));                         \
    }
#define DECL_TRACE_METHOD_B(tp)                                                                                        \
    void vcd_push_trace_file::trace(const tp& object, const std::string& name, int width) {                            \
        all_traces.emplace_back(this, &changed<tp>, new trace::vcd_trace_t<tp>(object, name));                         \
    }
#define DECL_TRACE_METHOD_C(tp, tpo)                                                                                   \
    void vcd_push_trace_file::trace(const tp& object, const std::string& name) {                                       \
        all_traces.emplace_back(this, &changed<tp, tpo>, new trace::vcd_trace_t<tp, tpo>(object, name));               \
    }

#if(SYSTEMC_VERSION >= 20171012)
void vcd_push_trace_file::trace(const sc_core::sc_event& object, const std::string& name) {}
void vcd_push_trace_file::trace(const sc_core::sc_time& object, const std::string& name) {}
#endif
DECL_TRACE_METHOD_A(bool)
DECL_TRACE_METHOD_A(sc_dt::sc_bit)
DECL_TRACE_METHOD_A(sc_dt::sc_logic)

DECL_TRACE_METHOD_B(unsigned char)
DECL_TRACE_METHOD_B(unsigned short)
DECL_TRACE_METHOD_B(unsigned int)
DECL_TRACE_METHOD_B(unsigned long)
#ifdef SYSTEMC_64BIT_PATCHES
DECL_TRACE_METHOD_B(unsigned long long)
#endif
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
DECL_TRACE_METHOD_C(sc_dt::sc_fxnum, sc_dt::sc_fxval)
DECL_TRACE_METHOD_C(sc_dt::sc_fxnum_fast, sc_dt::sc_fxval_fast)

DECL_TRACE_METHOD_A(sc_dt::sc_bv_base)
DECL_TRACE_METHOD_A(sc_dt::sc_lv_base)
#undef DECL_TRACE_METHOD_A
#undef DECL_TRACE_METHOD_B
#undef DECL_TRACE_METHOD_C

void vcd_push_trace_file::trace(const unsigned int& object, const std::string& name, const char** enum_literals) {
    all_traces.emplace_back(this, &changed<unsigned int>, new trace::vcd_trace_enum(object, name, enum_literals));
}

#define DECL_REGISTER_METHOD_A(tp)                                                                                     \
    observer::notification_handle* vcd_push_trace_file::observe(const tp& object, const std::string& name) {           \
        all_traces.emplace_back(this, &changed<tp>, new trace::vcd_trace_t<tp>(object, name));                         \
        all_traces.back().trc->is_triggered = true;                                                                    \
        return &all_traces.back();                                                                                     \
    }
#define DECL_REGISTER_METHOD_C(tp, tpo)                                                                                \
    observer::notification_handle* vcd_push_trace_file::observe(const tp& object, const std::string& name) {           \
        all_traces.emplace_back(this, &changed<tp, tpo>, new trace::vcd_trace_t<tp, tpo>(object, name));               \
        all_traces.back().trc->is_triggered = true;                                                                    \
        return &all_traces.back();                                                                                     \
    }
#if(SYSTEMC_VERSION >= 20171012)
observer::notification_handle* vcd_push_trace_file::observe(const sc_core::sc_event& object, const std::string& name) {
    return nullptr;
}
observer::notification_handle* vcd_push_trace_file::observe(const sc_core::sc_time& object, const std::string& name) {
    return nullptr;
}
#endif

DECL_REGISTER_METHOD_A(bool)
DECL_REGISTER_METHOD_A(sc_dt::sc_bit)
DECL_REGISTER_METHOD_A(sc_dt::sc_logic)

DECL_REGISTER_METHOD_A(unsigned char)
DECL_REGISTER_METHOD_A(unsigned short)
DECL_REGISTER_METHOD_A(unsigned int)
DECL_REGISTER_METHOD_A(unsigned long)
#ifdef SYSTEMC_64BIT_PATCHES
DECL_REGISTER_METHOD_A(unsigned long long)
#endif
DECL_REGISTER_METHOD_A(char)
DECL_REGISTER_METHOD_A(short)
DECL_REGISTER_METHOD_A(int)
DECL_REGISTER_METHOD_A(long)
DECL_REGISTER_METHOD_A(sc_dt::int64)
DECL_REGISTER_METHOD_A(sc_dt::uint64)

DECL_REGISTER_METHOD_A(float)
DECL_REGISTER_METHOD_A(double)
DECL_REGISTER_METHOD_A(sc_dt::sc_int_base)
DECL_REGISTER_METHOD_A(sc_dt::sc_uint_base)
DECL_REGISTER_METHOD_A(sc_dt::sc_signed)
DECL_REGISTER_METHOD_A(sc_dt::sc_unsigned)

DECL_REGISTER_METHOD_A(sc_dt::sc_fxval)
DECL_REGISTER_METHOD_A(sc_dt::sc_fxval_fast)
DECL_REGISTER_METHOD_C(sc_dt::sc_fxnum, sc_dt::sc_fxval)
DECL_REGISTER_METHOD_C(sc_dt::sc_fxnum_fast, sc_dt::sc_fxval_fast)

DECL_REGISTER_METHOD_A(sc_dt::sc_bv_base)
DECL_REGISTER_METHOD_A(sc_dt::sc_lv_base)
#undef DECL_REGISTER_METHOD_A
#undef DECL_REGISTER_METHOD_C

bool vcd_push_trace_file::trace_entry::notify() {
    if(!trc->is_alias && compare_and_update(trc))
        that->triggered_traces.push_back(trc);
    return !trc->is_alias;
}

std::string vcd_push_trace_file::obtain_name() {
    const char first_type_used = 'a';
    const int used_types_count = 'z' - 'a' + 1;
    int result;

    result = vcd_name_index;
    char char6 = static_cast<char>(vcd_name_index % used_types_count);

    result = result / used_types_count;
    char char5 = static_cast<char>(result % used_types_count);

    result = result / used_types_count;
    char char4 = static_cast<char>(result % used_types_count);

    result = result / used_types_count;
    char char3 = static_cast<char>(result % used_types_count);

    result = result / used_types_count;
    char char2 = static_cast<char>(result % used_types_count);

    char buf[20];
    std::sprintf(buf, "%c%c%c%c%c", char2 + first_type_used, char3 + first_type_used, char4 + first_type_used,
                 char5 + first_type_used, char6 + first_type_used);
    vcd_name_index++;
    return std::string(buf);
}

void vcd_push_trace_file::write_comment(const std::string& comment) {
    FPRINTF(vcd_out, "$comment\n{}\n$end\n\n", comment);
}

void vcd_push_trace_file::init() {
    std::vector<trace_entry*> traces;
    traces.reserve(all_traces.size());
    for(auto& e : all_traces)
        traces.push_back(&e);
    std::sort(std::begin(traces), std::end(traces),
              [](trace_entry const* a, trace_entry const* b) -> bool { return a->trc->name < b->trc->name; });

    std::unordered_map<uintptr_t, std::string> alias_map;
    trace::vcd_scope_stack<trace::vcd_trace> scope;
    for(auto e : traces) {
        auto alias_it = alias_map.find(e->trc->get_hash());
        e->trc->is_alias = alias_it != std::end(alias_map);
        e->trc->trc_hndl = e->trc->is_alias ? alias_it->second : obtain_name();
        if(!e->trc->is_alias)
            alias_map.insert({e->trc->get_hash(), e->trc->trc_hndl});
        scope.add_trace(e->trc);
    }
    std::copy_if(std::begin(traces), std::end(traces), std::back_inserter(pull_traces),
                 [](trace_entry const* e) { return !(e->trc->is_alias || e->trc->is_triggered); });
    changed_traces.reserve(pull_traces.size());
    triggered_traces.reserve(traces.size());
    // date:
    char tbuf[200];
    time_t long_time;
    time(&long_time);
    struct tm* p_tm = localtime(&long_time);
    strftime(tbuf, 199, "%b %d, %Y       %H:%M:%S", p_tm);
    FPRINTF(vcd_out, "$date\n     {}\n$end\n\n", tbuf);
    // version:
    FPRINTF(vcd_out, "$version\n {}\n$end\n\n", sc_core::sc_version());
    // timescale:
    FPRINTF(vcd_out, "$timescale\n     {}\n$end\n\n", (1_ps).to_string());
    std::stringstream ss;
    ss << "tracing " << pull_traces.size() << " distinct traces out of " << all_traces.size() << " traces";
    write_comment(ss.str());
    scope.print(vcd_out);
}

std::string vcd_push_trace_file::prune_name(std::string const& orig_name) {
    static bool warned = false;
    bool braces_removed = false;
    std::string hier_name = orig_name;
    for(unsigned int i = 0; i < hier_name.length(); i++) {
        if(hier_name[i] == '[') {
            hier_name[i] = '(';
            braces_removed = true;
        } else if(hier_name[i] == ']') {
            hier_name[i] = ')';
            braces_removed = true;
        }
    }

    if(braces_removed && !warned) {
        std::stringstream ss;
        ss << name
           << ":\n"
              "\tTraced objects found with name containing [], which may be\n"
              "\tinterpreted by the waveform viewer in unexpected ways.\n"
              "\tSo the [] is automatically replaced by ().";

        SC_REPORT_WARNING(sc_core::SC_ID_TRACING_OBJECT_NAME_FILTERED_, ss.str().c_str());
    }
    return hier_name;
}

void vcd_push_trace_file::cycle(bool delta_cycle) {
    if(delta_cycle)
        return;
    if(last_emitted_ts==std::numeric_limits<uint64_t>::max()) {
        init();
        FPRINT(vcd_out, "$enddefinitions  $end\n\n$dumpvars\n");
        for(auto& e : all_traces)
            if(!e.trc->is_alias) {
                e.compare_and_update(e.trc);
                e.trc->record(vcd_out);
            }
        FPRINT(vcd_out, "$end\n\n");
        last_emitted_ts = sc_core::sc_time_stamp().value() / (1_ps).value();
    } else {
        if(check_enabled && !check_enabled())
            return;
        for(auto e : pull_traces) {
            if(e->compare_and_update(e->trc))
                changed_traces.push_back(e->trc);
        }
        if(triggered_traces.size() || changed_traces.size()) {
            uint64_t time_stamp = sc_core::sc_time_stamp().value() / (1_ps).value();
            FPRINTF(vcd_out, "#{}\n", time_stamp);
            auto end = std::unique(std::begin(triggered_traces), std::end(triggered_traces));
            triggered_traces.erase(end, triggered_traces.end());
            if(triggered_traces.size()) {
                auto end = std::unique(std::begin(triggered_traces), std::end(triggered_traces));
                for(auto it = triggered_traces.begin(); it != end; ++it)
                    (*it)->record(vcd_out);
                triggered_traces.clear();
            }
            if(changed_traces.size()) {
                for(auto t : changed_traces)
                    t->record(vcd_out);
                changed_traces.clear();
            }
            last_emitted_ts = time_stamp;
        }
    }
}

void vcd_push_trace_file::set_time_unit(double v, sc_core::sc_time_unit tu) {}

sc_core::sc_trace_file* create_vcd_push_trace_file(const char* name, std::function<bool()> enable) {
    return new vcd_push_trace_file(name, enable);
}

void close_vcd_push_trace_file(sc_core::sc_trace_file* tf) { delete static_cast<vcd_push_trace_file*>(tf); }
} // namespace scc
