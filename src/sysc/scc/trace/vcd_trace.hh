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
#ifndef _SCC_TRACE_VCD_TRACE_HH_
#define _SCC_TRACE_VCD_TRACE_HH_

#include "types.hh"
#ifndef FWRITE
#include <cstdio>
#define FWRITE(BUF, SZ, LEN, FP) std::fwrite(BUF, SZ, LEN, FP)
#define FPTR FILE*
#endif
#include <util/ities.h>
#include <scc/utilities.h>
#include <fmt/format.h>
#include <vector>
#include <unordered_map>


namespace scc {
namespace trace {

inline void vcdEmitValueChange(FPTR os, std::string const& handle, unsigned bits, const char *val) {
    auto buf = bits==1?fmt::format("{}{}\n", *val, handle):fmt::format("b{} {}\n", val, handle);
    FWRITE(buf.c_str(), 1, buf.size(), os);
}

inline void vcdEmitValueChange32(FPTR os, std::string const& handle, unsigned bits, uint32_t val){
    auto buf = fmt::format("b{:b} {}\n", val&((1ull<<bits)-1), handle);
    FWRITE(buf.c_str(), 1, buf.size(), os);
}

inline void vcdEmitValueChange64(FPTR os, std::string const& handle, unsigned bits, uint64_t val){
    auto buf = fmt::format("b{:b} {}\n", val&((1ul<<std::min(63u,bits))-1), handle);
    FWRITE(buf.c_str(), 1, buf.size(), os);
}

template<typename T>
inline void vcdEmitValueChangeReal(FPTR os, std::string const& handle, unsigned bits, T val){
    auto buf = fmt::format("r{:.16g} {}\n", val, handle);
    FWRITE(buf.c_str(), 1, buf.size(), os);
}

inline size_t get_buffer_size(int length){
    size_t sz = ( static_cast<size_t>(length) + 4096 ) & (~static_cast<size_t>(4096-1));
    return std::max<uint64_t>(1024UL, sz);
}

/*******************************************************************************************************
 *
 *******************************************************************************************************/
template<typename T>
struct vcd_scope_stack {
    void add_trace(T *trace){
        auto hier = util::split(trace->name, '.');
        add_trace_rec(std::begin(hier), std::end(hier), trace);
    }

    void print(FPTR os, const char *scope_name = "SystemC"){
        auto buf = fmt::format("$scope module {} $end\n", scope_name);
        FWRITE(buf.c_str(), 1, buf.size(), os);
        for (auto& e : m_traces)
            print_variable_declaration_line(os, e.first.c_str(), e.second);
        for (auto& e : m_scopes)
            e.second->print(os,e.first.c_str());
        std::string end = "$upscope $end\n";
        FWRITE(end.c_str(), 1, end.size(), os);
    }

    ~vcd_scope_stack(){
        for (auto& s : m_scopes)
            delete s.second;
    }
private:
    void add_trace_rec(std::vector<std::string>::iterator beg, std::vector<std::string>::iterator const& end, T *trace){
        if(std::distance(beg,  end)==1){
            m_traces.push_back(std::make_pair(*beg, trace));
        } else {
            auto sc = m_scopes.find(*beg);
            if(sc == std::end(m_scopes))
                sc = m_scopes.insert({*beg, new vcd_scope_stack}).first;
            sc->second->add_trace_rec(++beg, end, trace);
        }
    }
    void print_variable_declaration_line(FPTR os, const char* scoped_name, T* trc){
        char buf[2000];
        switch(trc->bits){
        case 0:{
            std::stringstream ss;
            ss << "'" << scoped_name << "' has 0 bits";
            SC_REPORT_ERROR(sc_core::SC_ID_TRACING_OBJECT_IGNORED_, ss.str().c_str() );
            return;
        }
        case 1:
            if(trc->type==WIRE) {
                auto buf = fmt::format("$var wire {} {}  {} $end\n", trc->bits, trc->trc_hndl, scoped_name);
                FWRITE(buf.c_str(), 1, buf.size(), os);
            } else {
                auto buf = fmt::format("$var real {} {} {} $end\n", trc->bits, trc->trc_hndl, scoped_name);
                FWRITE(buf.c_str(), 1, buf.size(), os);
            }
            break;
        default: {
            auto buf = fmt::format("$var wire {} {} {} [{}:0] $end\n", trc->bits, trc->trc_hndl, scoped_name, trc->bits-1);
            FWRITE(buf.c_str(), 1, buf.size(), os);
        }
        }
    }

    std::vector<std::pair<std::string,T*> > m_traces{0};
    std::unordered_map<std::string, vcd_scope_stack*> m_scopes{0};
};

struct vcd_trace {
    vcd_trace(std::string const& nm, trace_type t, unsigned bits): name{nm}, bits{bits}, type{t}{}

    virtual void record(FPTR os) = 0;

    virtual void update() = 0;

    virtual uintptr_t get_hash() = 0;

    virtual ~vcd_trace(){};

    const std::string name;
    std::string trc_hndl{};
    bool is_alias{false};
    bool is_triggered{false};
    const unsigned bits;
    const trace_type type;
};

namespace {

inline unsigned get_bits(const char** literals) {
    unsigned nliterals;
    for (nliterals = 0; literals[nliterals]; nliterals++) continue;
    return scc::ilog2(nliterals);
}

struct vcd_trace_enum : public vcd_trace {
    vcd_trace_enum( const unsigned int& object_,
            const std::string& name, const char** literals)
    : vcd_trace( name, trace::WIRE, get_bits(literals))
    , act_val( object_ )
    , old_val( object_ )
    , literals{literals}
    {}

    uintptr_t get_hash() override { return reinterpret_cast<uintptr_t>(&act_val);}

    inline bool changed() { return !is_alias && old_val!=act_val; }

    void update() override { old_val=act_val; }

    void record(FPTR os) override { vcdEmitValueChange64(os, trc_hndl, bits, old_val); }

    unsigned int old_val;
    const unsigned int& act_val;
    const char** literals;
};

template<typename T, typename OT=T>
struct vcd_trace_t : public vcd_trace {
    vcd_trace_t( const T& object_,
            const std::string& name)
    : vcd_trace( name, trace::traits<T>::get_type(), trace::traits<T>::get_bits(object_))
    , act_val( object_ )
    , old_val( object_ )
    {}

    uintptr_t get_hash() override { return reinterpret_cast<uintptr_t>(&act_val);}

    inline bool changed() { return !is_alias && old_val!=act_val; }

    void update() override { old_val=act_val; }

    void record(FPTR os) override;

    OT old_val;
    const T& act_val;
};

template<typename T, typename OT>
void vcd_trace_t<T, OT>::record(FPTR os) {
    if(sizeof(T)<=4)
        vcdEmitValueChange32(os, trc_hndl, bits, old_val);
    else
        vcdEmitValueChange64(os, trc_hndl, bits, old_val);
}
template<> void vcd_trace_t<sc_core::sc_time, sc_core::sc_time>::record(FPTR os){
    vcdEmitValueChange64(os, trc_hndl, bits, old_val.value());
}
template<> void vcd_trace_t<bool, bool>::record(FPTR os){
    vcdEmitValueChange(os, trc_hndl, 1, old_val ? "1" : "0");
}
template<> void vcd_trace_t<sc_dt::sc_bit, sc_dt::sc_bit>::record(FPTR os){
    vcdEmitValueChange(os, trc_hndl, 1, old_val ? "1" : "0");
}
template<> void vcd_trace_t<sc_dt::sc_logic, sc_dt::sc_logic>::record(FPTR os){
    char buf[2] = {0, 0};
    buf[0]=old_val.to_char();
    vcdEmitValueChange(os, trc_hndl, 1, buf);
}
template<> void vcd_trace_t<float, float>::record(FPTR os){
    vcdEmitValueChangeReal(os, trc_hndl, 32, old_val);
}
template<> void vcd_trace_t<double, double>::record(FPTR os){
    vcdEmitValueChangeReal(os, trc_hndl, 64, old_val);
}
template<> void vcd_trace_t<sc_dt::sc_int_base, sc_dt::sc_int_base>::record(FPTR os){
    static std::vector<char> rawdata(get_buffer_size(old_val.length()));
    char *rawdata_ptr  = &rawdata[0];
    for (int bitindex = old_val.length() - 1; bitindex >= 0; --bitindex) {
        *rawdata_ptr++ = '0'+old_val[bitindex].value();
    }
    vcdEmitValueChange(os, trc_hndl, bits, &rawdata[0]);
}
template<> void vcd_trace_t<sc_dt::sc_uint_base, sc_dt::sc_uint_base>::record(FPTR os){
    static std::vector<char> rawdata(get_buffer_size(old_val.length()));
    char *rawdata_ptr  = &rawdata[0];
    for (int bitindex = old_val.length() - 1; bitindex >= 0; --bitindex) {
        *rawdata_ptr++ = '0'+old_val[bitindex].value();
    }
    vcdEmitValueChange(os, trc_hndl, bits, &rawdata[0]);
}
template<> void vcd_trace_t<sc_dt::sc_signed, sc_dt::sc_signed>::record(FPTR os){
    static std::vector<char> rawdata(get_buffer_size(old_val.length()));
    char *rawdata_ptr  = &rawdata[0];
    bool is_started=false;
    int bitindex = old_val.length() - 1;
    *rawdata_ptr++ = '0'+old_val[bitindex--].value();
    for (; bitindex >= 0; --bitindex) {
        auto c = '0'+old_val[bitindex].value();
        if(is_started)
            *rawdata_ptr++ = c;
        else if(c=='1' || *(rawdata_ptr-1)!=c ) {
            *rawdata_ptr++ = c;
            is_started=true;
        }
    }
    vcdEmitValueChange(os, trc_hndl, bits, &rawdata[0]);
}
template<> void vcd_trace_t<sc_dt::sc_unsigned, sc_dt::sc_unsigned>::record(FPTR os){
    static std::vector<char> rawdata(get_buffer_size(old_val.length()));
    char *rawdata_ptr  = &rawdata[0];
    bool is_started=false;
    int bitindex = old_val.length() - 1;
    *rawdata_ptr++ = '0'+old_val[bitindex--].value();
    for (; bitindex >= 0; --bitindex) {
        auto c = '0'+old_val[bitindex].value();
        if(is_started)
            *rawdata_ptr++ = c;
        else if(c=='1' || *(rawdata_ptr-1)!=c ) {
            *rawdata_ptr++ = c;
            is_started=true;
        }
    }
    vcdEmitValueChange(os, trc_hndl, bits, &rawdata[0]);
}
template<> void vcd_trace_t<sc_dt::sc_fxval, sc_dt::sc_fxval>::record(FPTR os){
    vcdEmitValueChangeReal(os, trc_hndl, bits, old_val);
}
template<> void vcd_trace_t<sc_dt::sc_fxval_fast, sc_dt::sc_fxval_fast>::record(FPTR os){
    vcdEmitValueChangeReal(os, trc_hndl, bits, old_val);
}
template<> void vcd_trace_t<sc_dt::sc_fxnum, sc_dt::sc_fxval>::record(FPTR os){
    vcdEmitValueChangeReal(os, trc_hndl, bits, old_val);
}
template<> void vcd_trace_t<sc_dt::sc_fxnum_fast, sc_dt::sc_fxval_fast>::record(FPTR os){
    vcdEmitValueChangeReal(os, trc_hndl, bits, old_val);
}
template<> void vcd_trace_t<sc_dt::sc_bv_base, sc_dt::sc_bv_base>::record(FPTR os){
    auto str = old_val.to_string();
    auto* cstr = str.c_str();
    auto c=*cstr;
    if(c!='1') while(c == *(cstr+1)) cstr++;
    vcdEmitValueChange(os, trc_hndl, bits, cstr);
}
template<> void vcd_trace_t<sc_dt::sc_lv_base, sc_dt::sc_lv_base>::record(FPTR os){
    auto str = old_val.to_string();
    auto* cstr = str.c_str();
    auto c=*cstr;
    if(c!='1') while(c == *(cstr+1)) cstr++;
    vcdEmitValueChange(os, trc_hndl, bits, cstr);
}
} // namespace anonymous
} // namespace trace
} // namespace scc
#endif //_SCC_TRACE_VCD_TRACE_HH_
