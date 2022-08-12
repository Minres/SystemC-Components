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

#include "fst_trace.hh"
#include "fstapi.h"
#include "trace/types.hh"
#include "utilities.h"
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <deque>
#include <iomanip>
#include <limits>
#include <map>
#include <sstream>
#include <stdexcept>
#include <unordered_map>
#include <util/ities.h>
#include <vector>

namespace scc {
namespace trace {
inline size_t get_buffer_size(int length) {
    size_t sz = (static_cast<size_t>(length) + 4096) & (~static_cast<size_t>(4096 - 1));
    return std::max<decltype(sz)>(1024UL, sz);
}

inline unsigned get_bits(const char** literals) {
    unsigned nliterals;
    for(nliterals = 0; literals[nliterals]; nliterals++)
        continue;
    return scc::ilog2(nliterals);
}

struct fst_trace {

    fst_trace(std::string const& nm, trace_type type, unsigned bits)
    : name{nm}
    , bits{bits}
    , type{type} {}

    virtual void record(void* m_fst) = 0;

    virtual void update_and_record(void* m_fst) = 0;

    virtual uintptr_t get_hash() = 0;

    virtual ~fst_trace(){};

    const std::string name;
    fstHandle fst_hndl{0};
    bool is_alias{false};
    bool is_triggered{false};
    const unsigned bits{0};
    const trace_type type;
};

struct fst_trace_enum : public fst_trace {
    fst_trace_enum(const unsigned int& object_, const std::string& name, const char** literals)
    : fst_trace(name, trace::WIRE, get_bits(literals))
    , act_val(object_)
    , old_val(object_)
    , literals{literals} {}

    uintptr_t get_hash() override { return reinterpret_cast<uintptr_t>(&act_val); }

    inline bool changed() { return !is_alias && old_val != act_val; }

    inline void update() { old_val = act_val; }

    void record(void* os) override { fstWriterEmitValueChange64(os, fst_hndl, bits, old_val); }

    void update_and_record(void* os) override {
        update();
        record(os);
    };

    unsigned int old_val;
    const unsigned int& act_val;
    const char** literals;
};

template <typename T, typename OT = T> struct fst_trace_t : public fst_trace {
    fst_trace_t(const T& object_, const std::string& name, int width = -1)
    : fst_trace(name, trace::traits<T>::get_type(), trace::traits<T>::get_bits(object_))
    , act_val(object_)
    , old_val(object_) {}

    uintptr_t get_hash() override { return reinterpret_cast<uintptr_t>(&act_val); }

    inline bool changed() { return !is_alias && old_val != act_val; }

    inline void update() { old_val = act_val; }

    void record(void* m_fst) override;

    void update_and_record(void* m_fst) override {
        update();
        record(m_fst);
    };

    OT old_val;
    const T& act_val;
};

template <typename T, typename OT> inline void fst_trace_t<T, OT>::record(void* m_fst) {
    if(sizeof(T) <= 4)
        fstWriterEmitValueChange32(m_fst, fst_hndl, bits, old_val);
    else
        fstWriterEmitValueChange64(m_fst, fst_hndl, bits, old_val);
}
template <> void fst_trace_t<bool, bool>::record(void* m_fst) {
    fstWriterEmitValueChange(m_fst, fst_hndl, old_val ? "1" : "0");
}
template <> void fst_trace_t<sc_dt::sc_bit, sc_dt::sc_bit>::record(void* m_fst) {
    fstWriterEmitValueChange(m_fst, fst_hndl, old_val ? "1" : "0");
}
template <> void fst_trace_t<sc_dt::sc_logic, sc_dt::sc_logic>::record(void* m_fst) {
    char buf[2] = {0, 0};
    buf[0] = old_val.to_char();
    fstWriterEmitValueChange(m_fst, fst_hndl, buf);
}
template <> void fst_trace_t<float, float>::record(void* m_fst) {
    double val = old_val;
    fstWriterEmitValueChange(m_fst, fst_hndl, &val);
}
template <> void fst_trace_t<double, double>::record(void* m_fst) {
    fstWriterEmitValueChange(m_fst, fst_hndl, &old_val);
}
template <> void fst_trace_t<sc_dt::sc_int_base, sc_dt::sc_int_base>::record(void* m_fst) {
    static std::vector<char> rawdata(get_buffer_size(old_val.length()));
    char* rawdata_ptr = &rawdata[0];
    for(int bitindex = old_val.length() - 1; bitindex >= 0; --bitindex) {
        *rawdata_ptr++ = '0' + old_val[bitindex].value();
    }
    fstWriterEmitValueChange(m_fst, fst_hndl, &rawdata[0]);
}
template <> void fst_trace_t<sc_dt::sc_uint_base, sc_dt::sc_uint_base>::record(void* m_fst) {
    static std::vector<char> rawdata(get_buffer_size(old_val.length()));
    char* rawdata_ptr = &rawdata[0];
    for(int bitindex = old_val.length() - 1; bitindex >= 0; --bitindex) {
        *rawdata_ptr++ = '0' + old_val[bitindex].value();
    }
    fstWriterEmitValueChange(m_fst, fst_hndl, &rawdata[0]);
}
template <> void fst_trace_t<sc_dt::sc_signed, sc_dt::sc_signed>::record(void* m_fst) {
    static std::vector<char> rawdata(get_buffer_size(old_val.length()));
    char* rawdata_ptr = &rawdata[0];
    for(int bitindex = old_val.length() - 1; bitindex >= 0; --bitindex) {
        *rawdata_ptr++ = '0' + old_val[bitindex].value();
    }
    fstWriterEmitValueChange(m_fst, fst_hndl, &rawdata[0]);
}
template <> void fst_trace_t<sc_dt::sc_unsigned, sc_dt::sc_unsigned>::record(void* m_fst) {
    static std::vector<char> rawdata(get_buffer_size(old_val.length()));
    char* rawdata_ptr = &rawdata[0];
    for(int bitindex = old_val.length() - 1; bitindex >= 0; --bitindex) {
        *rawdata_ptr++ = '0' + old_val[bitindex].value();
    }
    fstWriterEmitValueChange(m_fst, fst_hndl, &rawdata[0]);
}
template <> void fst_trace_t<sc_dt::sc_fxval, sc_dt::sc_fxval>::record(void* m_fst) {
    auto val = old_val.to_double();
    fstWriterEmitValueChange(m_fst, fst_hndl, &val);
}
template <> void fst_trace_t<sc_dt::sc_fxval_fast, sc_dt::sc_fxval_fast>::record(void* m_fst) {
    auto val = old_val.to_double();
    fstWriterEmitValueChange(m_fst, fst_hndl, &val);
}
template <> void fst_trace_t<sc_dt::sc_fxnum, sc_dt::sc_fxval>::record(void* m_fst) {
    fstWriterEmitValueChange32(m_fst, fst_hndl, 64, *reinterpret_cast<uint64_t*>(&old_val));
}
template <> void fst_trace_t<sc_dt::sc_fxnum_fast, sc_dt::sc_fxval_fast>::record(void* m_fst) {
    auto val = old_val.to_double();
    fstWriterEmitValueChange(m_fst, fst_hndl, &val);
}
template <> void fst_trace_t<sc_dt::sc_bv_base, sc_dt::sc_bv_base>::record(void* m_fst) {
    auto str = old_val.to_string();
    auto* cstr = str.c_str();
    auto c = *cstr;
    if(c != '1')
        while(c == *(cstr + 1))
            cstr++;
    fstWriterEmitValueChange(m_fst, fst_hndl, str.c_str());
}
template <> void fst_trace_t<sc_dt::sc_lv_base, sc_dt::sc_lv_base>::record(void* m_fst) {
    auto str = old_val.to_string();
    auto* cstr = str.c_str();
    auto c = *cstr;
    if(c != '1')
        while(c == *(cstr + 1))
            cstr++;
    fstWriterEmitValueChange(m_fst, fst_hndl, str.c_str());
}
} // namespace trace

fst_trace_file::fst_trace_file(const char* name, std::function<bool()>& enable)
: check_enabled(enable) {
    std::stringstream ss;
    ss << name << ".fst";
    m_fst = fstWriterCreate(ss.str().c_str(), 1);
    fstWriterSetPackType(m_fst, FST_WR_PT_LZ4);
    fstWriterSetTimescale(m_fst, 12); // pico seconds 1*10-12
    fstWriterSetFileType(m_fst, FST_FT_VERILOG);
#if defined(WITH_SC_TRACING_PHASE_CALLBACKS)
    // remove from hierarchy
    sc_object::detach();
    // register regular (non-delta) callbacks
    sc_object::register_simulation_phase_callback(SC_BEFORE_TIMESTEP);
#else // explicitly register with simcontext
    sc_core::sc_get_curr_simcontext()->add_trace_file(this);
#endif
}

fst_trace_file::~fst_trace_file() {
    if(m_fst) {
        fstWriterFlushContext(m_fst);
        fstWriterClose(m_fst);
    }
    for(auto t:all_traces) delete t.trc;
}

template <typename T, typename OT = T> bool changed(trace::fst_trace* trace) {
    if(reinterpret_cast<trace::fst_trace_t<T, OT>*>(trace)->changed()) {
        reinterpret_cast<trace::fst_trace_t<T, OT>*>(trace)->update();
        return true;
    } else
        return false;
}
#define DECL_TRACE_METHOD_A(tp)                                                                                        \
    void fst_trace_file::trace(const tp& object, const std::string& name) {                                            \
        all_traces.emplace_back(this, &changed<tp>, new trace::fst_trace_t<tp>(object, name));                         \
    }
#define DECL_TRACE_METHOD_B(tp)                                                                                        \
    void fst_trace_file::trace(const tp& object, const std::string& name, int width) {                                 \
        all_traces.emplace_back(this, &changed<tp>, new trace::fst_trace_t<tp>(object, name));                         \
    }
#define DECL_TRACE_METHOD_C(tp, tpo)                                                                                   \
    void fst_trace_file::trace(const tp& object, const std::string& name) {                                            \
        all_traces.emplace_back(this, &changed<tp, tpo>, new trace::fst_trace_t<tp, tpo>(object, name));               \
    }

#if(SYSTEMC_VERSION >= 20171012)
void fst_trace_file::trace(const sc_core::sc_event& object, const std::string& name) {}
void fst_trace_file::trace(const sc_core::sc_time& object, const std::string& name) {}
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

void fst_trace_file::trace(const unsigned int& object, const std::string& name, const char** enum_literals) {
    // all_traces.emplace_back(this, &changed<unsigned int>, new fst_trace_enum(object, name, enum_literals));
}

#define DECL_REGISTER_METHOD_A(tp)                                                                                     \
    observer::notification_handle* fst_trace_file::observe(const tp& object, const std::string& name) {                \
        all_traces.emplace_back(this, &changed<tp>, new trace::fst_trace_t<tp>(object, name));                         \
        all_traces.back().trc->is_triggered = true;                                                                    \
        return &all_traces.back();                                                                                     \
    }
#define DECL_REGISTER_METHOD_C(tp, tpo)                                                                                \
    observer::notification_handle* fst_trace_file::observe(const tp& object, const std::string& name) {                \
        all_traces.emplace_back(this, &changed<tp, tpo>, new trace::fst_trace_t<tp, tpo>(object, name));               \
        all_traces.back().trc->is_triggered = true;                                                                    \
        return &all_traces.back();                                                                                     \
    }
#if(SYSTEMC_VERSION >= 20171012)
observer::notification_handle* fst_trace_file::observe(const sc_core::sc_event& object, const std::string& name) {
    return nullptr;
}
observer::notification_handle* fst_trace_file::observe(const sc_core::sc_time& object, const std::string& name) {
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

bool fst_trace_file::trace_entry::notify() {
    if(!trc->is_alias && compare_and_update(trc))
        that->triggered_traces.push_back(trc);
    return !trc->is_alias;
}

void fst_trace_file::write_comment(const std::string& comment) {}

void fst_trace_file::init() {
    std::vector<trace_entry*> traces;
    traces.reserve(all_traces.size());
    for(auto& e : all_traces)
        traces.push_back(&e);
    std::sort(std::begin(traces), std::end(traces),
              [](trace_entry const* a, trace_entry const* b) -> bool { return a->trc->name < b->trc->name; });

    std::unordered_map<uintptr_t, fstHandle> alias_map;
    std::deque<std::string> fst_scope;
    for(auto e : traces) {
        auto alias_it = alias_map.find(e->trc->get_hash());
        e->trc->is_alias = alias_it != std::end(alias_map);
        auto hier_tokens = util::split(e->trc->name, '.');
        auto sig_name = hier_tokens.back();
        hier_tokens.pop_back();
        auto cur_it = fst_scope.begin();
        auto tok_it = hier_tokens.begin();
        while(cur_it != std::end(fst_scope) && tok_it != std::end(hier_tokens)) {
            if(*cur_it != *tok_it)
                break;
            ++cur_it;
            ++tok_it;
        }
        auto common_count = std::distance(fst_scope.begin(), cur_it);
        for(auto i = fst_scope.size(); i > common_count; i--) {
            fstWriterSetUpscope(m_fst);
            fst_scope.pop_back();
        }
        for(; tok_it != std::end(hier_tokens); tok_it++) {
            fstWriterSetScope(m_fst, FST_ST_VCD_SCOPE, tok_it->c_str(), nullptr);
            fst_scope.push_back(*tok_it);
        }
        e->trc->fst_hndl =
            fstWriterCreateVar(m_fst, e->trc->type == trace::REAL ? FST_VT_VCD_REAL : FST_VT_VCD_WIRE, FST_VD_IMPLICIT,
                               e->trc->bits, sig_name.c_str(), e->trc->is_alias ? alias_it->second : 0);
        if(!e->trc->is_alias)
            alias_map.insert({e->trc->get_hash(), e->trc->fst_hndl});
    }
    std::copy_if(std::begin(traces), std::end(traces), std::back_inserter(pull_traces),
                 [](trace_entry const* e) { return !(e->trc->is_alias || e->trc->is_triggered); });
    changed_traces.reserve(pull_traces.size());
    triggered_traces.reserve(all_traces.size());
}

void fst_trace_file::cycle(bool delta_cycle) {
    if(delta_cycle)
        return;
    if(last_emitted_ts==std::numeric_limits<uint64_t>::max())
        init();
    if(last_emitted_ts==std::numeric_limits<uint64_t>::max()) {
        uint64_t time_stamp = sc_core::sc_time_stamp().value() / (1_ps).value();
        fstWriterEmitTimeChange(m_fst, time_stamp);
        for(auto& e : all_traces)
            if(!e.trc->is_alias)
                e.trc->update_and_record(m_fst);
        last_emitted_ts = time_stamp;
    } else {
        if(check_enabled && !check_enabled())
            return;
        for(auto e : pull_traces) {
            if(e->compare_and_update(e->trc))
                changed_traces.push_back(e->trc);
        }
        if(triggered_traces.size() || changed_traces.size()) {
            uint64_t time_stamp = sc_core::sc_time_stamp().value() / (1_ps).value();
            if(last_emitted_ts<time_stamp)
                fstWriterEmitTimeChange(m_fst, time_stamp);
            if(triggered_traces.size()) {
                auto end = std::unique(std::begin(triggered_traces), std::end(triggered_traces));
                triggered_traces.erase(end, triggered_traces.end());
                for(auto t : triggered_traces)
                    t->record(m_fst);
                triggered_traces.clear();
            }
            if(changed_traces.size()) {
                for(auto t : changed_traces)
                    t->record(m_fst);
                changed_traces.clear();
            }
            last_emitted_ts = time_stamp;
        }
    }
}

void fst_trace_file::set_time_unit(double v, sc_core::sc_time_unit tu) {}

sc_core::sc_trace_file* create_fst_trace_file(const char* name, std::function<bool()> enable) {
    return new fst_trace_file(name, enable);
}

void close_fst_trace_file(sc_core::sc_trace_file* tf) { delete static_cast<fst_trace_file*>(tf); }

} // namespace scc
