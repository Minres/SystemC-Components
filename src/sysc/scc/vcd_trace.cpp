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

#include "vcd_trace.h"
#include "utilities.h"
#include <util/ities.h>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <vector>
#include <deque>
#include <unordered_map>

#include "sysc/tracing/sc_tracing_ids.h"
#include <sysc/kernel/sc_simcontext.h>
#include <sysc/kernel/sc_ver.h>
#include <sysc/kernel/sc_event.h>
#include <sysc/datatypes/bit/sc_bit.h>
#include <sysc/datatypes/bit/sc_logic.h>
#include <sysc/datatypes/bit/sc_lv_base.h>
#include <sysc/datatypes/int/sc_signed.h>
#include <sysc/datatypes/int/sc_unsigned.h>
#include <sysc/datatypes/int/sc_int_base.h>
#include <sysc/datatypes/int/sc_uint_base.h>
#include <sysc/datatypes/fx/fx.h>
#include <sysc/utils/sc_report.h> // sc_assert
#include <sysc/utils/sc_string_view.h>

#include <iomanip>
#include <map>
#include <sstream>
#include <stdexcept>
#include <cmath>

namespace scc {
namespace {
thread_local char buf[96];
inline void vcdEmitValueChange(std::ostream& os, std::string const& handle, unsigned bits, const char *val) {
    if(bits==1) {
        os<<*val<<handle<<'\n';
    } else {
        auto start=val;
        for(auto i=1U; *start=='0' && i<bits; ++start, ++i);
        os<<'b'<<start<<' '<<handle<<'\n';
    }
}

inline void vcdEmitValueChange32(std::ostream& os, std::string const& handle, unsigned bits, uint32_t val){
    char *s = buf;
    for (auto i = 0U; i < bits; ++i){
        *s++ = '0' + ((val >> (bits - i - 1)) & 1);
    }
    *s=0;
    vcdEmitValueChange(os, handle, bits, buf);
}

inline void vcdEmitValueChange64(std::ostream& os, std::string const& handle, unsigned bits, uint64_t val){
    char *s = buf;
    for (auto i = 0U; i < bits; ++i){
        *s++ = '0' + ((val >> (bits - i - 1)) & 1);
    }
    *s=0;
    vcdEmitValueChange(os, handle, bits, buf);
}

inline size_t get_buffer_size(int length){
    size_t sz = ( static_cast<size_t>(length) + 4096 ) & (~static_cast<size_t>(4096-1));
    return std::max(1024UL, sz);
}

}

struct vcd_trace {

    vcd_trace(std::string const& nm): name{nm}{}

    virtual void record(std::ostream& os) = 0;

    virtual void update_and_record(std::ostream& os) = 0;

    virtual uintptr_t get_hash() = 0;

    virtual ~vcd_trace(){};

    const std::string name;
    std::string fst_hndl{};
    bool is_alias{false};
    unsigned bits{0};
};

template<typename T, typename OT=T>
struct vcd_trace_t : public vcd_trace {
    vcd_trace_t( const T& object_,
            const std::string& name, int width=-1)
    : vcd_trace( name),
      act_val( object_ ),
      old_val( object_ )
    {bits=width<0?get_bits():width;}

    uintptr_t get_hash() override { return reinterpret_cast<uintptr_t>(&act_val);}

    inline bool changed() { return !is_alias && old_val!=act_val; }

    inline void update() { old_val=act_val; }

    void record(std::ostream& os) override;

    void update_and_record(std::ostream& os) override {update(); record(os);};

    unsigned get_bits() { return sizeof(OT)*8;}
    OT old_val;
    const T& act_val;
};

template<typename T, typename OT>
inline void scc::vcd_trace_t<T, OT>::record(std::ostream& os) {
    if(sizeof(T)<=4)
        vcdEmitValueChange32(os, fst_hndl, bits, old_val);
    else
        vcdEmitValueChange64(os, fst_hndl, bits, old_val);
}

/*
 * bool
 */
template<> void vcd_trace_t<bool, bool>::record(std::ostream& os){
    vcdEmitValueChange(os, fst_hndl, 1, old_val ? "1" : "0");
}

template<> unsigned vcd_trace_t<bool, bool>::get_bits(){ return 1; }
/*
 * sc_dt::sc_bit
 */
template<> void vcd_trace_t<sc_dt::sc_bit, sc_dt::sc_bit>::record(std::ostream& os){
    vcdEmitValueChange(os, fst_hndl, 1, old_val ? "1" : "0");
}

template<> unsigned vcd_trace_t<sc_dt::sc_bit, sc_dt::sc_bit>::get_bits(){ return 1; }
/*
 * sc_dt::sc_logic
 */
template<> void vcd_trace_t<sc_dt::sc_logic, sc_dt::sc_logic>::record(std::ostream& os){
    char buf[2] = {0, 0};
    buf[0]=old_val.to_char();
    vcdEmitValueChange(os, fst_hndl, 1, buf);
}

template<> unsigned vcd_trace_t<sc_dt::sc_logic, sc_dt::sc_logic>::get_bits(){ return 1; }
/*
 * float
 */
template<> void vcd_trace_t<float, float>::record(std::ostream& os){
    vcdEmitValueChange32(os, fst_hndl, 32, *reinterpret_cast<uint32_t*>(&old_val));
}
/*
 * double
 */
template<> void vcd_trace_t<double, double>::record(std::ostream& os){
    vcdEmitValueChange32(os, fst_hndl, 64, *reinterpret_cast<uint64_t*>(&old_val));
}
/*
 * sc_dt::sc_int_base
 */
template<> void vcd_trace_t<sc_dt::sc_int_base, sc_dt::sc_int_base>::record(std::ostream& os){
    static std::vector<char> rawdata(get_buffer_size(old_val.length()));
    char *rawdata_ptr  = &rawdata[0];
    for (int bitindex = old_val.length() - 1; bitindex >= 0; --bitindex) {
        *rawdata_ptr++ = '0'+old_val[bitindex].value();
    }
    vcdEmitValueChange(os, fst_hndl, bits, &rawdata[0]);
}

template<> unsigned vcd_trace_t<sc_dt::sc_int_base, sc_dt::sc_int_base>::get_bits(){ return old_val.length(); }
/*
 * sc_dt::sc_uint_base
 */
template<> void vcd_trace_t<sc_dt::sc_uint_base, sc_dt::sc_uint_base>::record(std::ostream& os){
    static std::vector<char> rawdata(get_buffer_size(old_val.length()));
    char *rawdata_ptr  = &rawdata[0];
    for (int bitindex = old_val.length() - 1; bitindex >= 0; --bitindex) {
        *rawdata_ptr++ = '0'+old_val[bitindex].value();
    }
    vcdEmitValueChange(os, fst_hndl, bits, &rawdata[0]);
}

template<> unsigned vcd_trace_t<sc_dt::sc_uint_base, sc_dt::sc_uint_base>::get_bits(){ return old_val.length(); }
/*
 * sc_dt::sc_signed
 */
template<> void vcd_trace_t<sc_dt::sc_signed, sc_dt::sc_signed>::record(std::ostream& os){
    static std::vector<char> rawdata(get_buffer_size(old_val.length()));
    char *rawdata_ptr  = &rawdata[0];
    for (int bitindex = old_val.length() - 1; bitindex >= 0; --bitindex) {
        *rawdata_ptr++ = '0'+old_val[bitindex].value();
    }
    vcdEmitValueChange(os, fst_hndl, bits, &rawdata[0]);
}

template<> unsigned vcd_trace_t<sc_dt::sc_signed, sc_dt::sc_signed>::get_bits(){ return old_val.length(); }
/*
 * sc_dt::sc_unsigned
 */
template<> void vcd_trace_t<sc_dt::sc_unsigned, sc_dt::sc_unsigned>::record(std::ostream& os){
    static std::vector<char> rawdata(get_buffer_size(old_val.length()));
    char *rawdata_ptr  = &rawdata[0];
    for (int bitindex = old_val.length() - 1; bitindex >= 0; --bitindex) {
        *rawdata_ptr++ = '0'+old_val[bitindex].value();
    }
    vcdEmitValueChange(os, fst_hndl, bits, &rawdata[0]);
}

template<> unsigned vcd_trace_t<sc_dt::sc_unsigned, sc_dt::sc_unsigned>::get_bits(){ return old_val.length(); }
/*
 * sc_dt::sc_fxval
 */
template<> void vcd_trace_t<sc_dt::sc_fxval, sc_dt::sc_fxval>::record(std::ostream& os){
    auto val = old_val.to_double();
    vcdEmitValueChange64(os, fst_hndl, 64, *reinterpret_cast<uint64_t*>(&val));
}

template<> unsigned vcd_trace_t<sc_dt::sc_fxval, sc_dt::sc_fxval>::get_bits(){ return 64; }
/*
 * sc_dt::sc_fxval_fast
 */
template<> void vcd_trace_t<sc_dt::sc_fxval_fast, sc_dt::sc_fxval_fast>::record(std::ostream& os){
    auto val = old_val.to_double();
    vcdEmitValueChange64(os, fst_hndl, 64, *reinterpret_cast<uint64_t*>(&val));
}

template<> unsigned vcd_trace_t<sc_dt::sc_fxval_fast, sc_dt::sc_fxval_fast>::get_bits(){ return 64; }
/*
 * sc_dt::sc_fxnum
 */
template<> void vcd_trace_t<sc_dt::sc_fxnum, double>::record(std::ostream& os){
    vcdEmitValueChange32(os, fst_hndl, 64, *reinterpret_cast<uint64_t*>(&old_val));
}
/*
 * sc_dt::sc_fxnum_fast
 */
template<> void vcd_trace_t<sc_dt::sc_fxnum_fast, double>::record(std::ostream& os){
    vcdEmitValueChange32(os, fst_hndl, 64, *reinterpret_cast<uint64_t*>(&old_val));
}
/*
 * sc_dt::sc_bv_base
 */
template<> void vcd_trace_t<sc_dt::sc_bv_base, sc_dt::sc_bv_base>::record(std::ostream& os){
    auto str = old_val.to_string();
    vcdEmitValueChange(os, fst_hndl, bits, str.c_str());
}

template<> unsigned vcd_trace_t<sc_dt::sc_bv_base, sc_dt::sc_bv_base>::get_bits(){ return old_val.length(); }
/*
 * sc_dt::sc_lv_base
 */
template<> void vcd_trace_t<sc_dt::sc_lv_base, sc_dt::sc_lv_base>::record(std::ostream& os){
    auto str = old_val.to_string();
    vcdEmitValueChange(os, fst_hndl, bits, str.c_str());
}

template<> unsigned vcd_trace_t<sc_dt::sc_lv_base, sc_dt::sc_lv_base>::get_bits(){ return old_val.length(); }
/*******************************************************************************************************
 *
 *******************************************************************************************************/
struct vcd_scope_stack {
    void add_trace(vcd_trace *trace){
        auto hier = util::split(trace->name, '.');
        add_trace_rec(std::begin(hier), std::end(hier), trace);
    }

    void print(std::ostream& os, const char *scope_name = "SystemC"){
        os<<"$scope module "<<scope_name<<" $end\n";
        for (auto& e : m_traces)
            print_variable_declaration_line(os, e.first.c_str(), e.second);
        for (auto& e : m_scopes)
            e.second->print(os,e.first.c_str());
        os<<"$upscope $end\n";
    }

    ~vcd_scope_stack(){
        for (auto& s : m_scopes)
            delete s.second;
    }
private:
    void add_trace_rec(std::vector<std::string>::iterator beg, std::vector<std::string>::iterator const& end, vcd_trace *trace){
        if(std::distance(beg,  end)==1){
            m_traces.push_back(std::make_pair(*beg, trace));
        } else {
            auto sc = m_scopes.find(*beg);
            if(sc == std::end(m_scopes))
                sc = m_scopes.insert({*beg, new vcd_scope_stack}).first;
            sc->second->add_trace_rec(++beg, end, trace);
        }
    }
    void print_variable_declaration_line(std::ostream& os, const char* scoped_name, vcd_trace* trc){
        char buf[2000];
        switch(trc->bits){
        case 0:{
            std::stringstream ss;
            ss << "'" << scoped_name << "' has 0 bits";
            SC_REPORT_ERROR(sc_core::SC_ID_TRACING_OBJECT_IGNORED_, ss.str().c_str() );
            return;
        }
        case 1:
            os<<"$var wire "<<trc->bits<<" "<<trc->fst_hndl<<"  "<<scoped_name<<" $end\n";
            break;
        default:
            os<<"$var wire "<<trc->bits<<" "<<trc->fst_hndl<<" "<<scoped_name<<" ["<<trc->bits-1<<":0] $end\n";
        }
    }

    std::vector<std::pair<std::string,vcd_trace*> > m_traces{0};
    std::unordered_map<std::string, vcd_scope_stack*> m_scopes{0};
};
/*******************************************************************************************************
 *
 *******************************************************************************************************/
vcd_trace_file::vcd_trace_file(const char *name, std::function<bool()> &enable):name(name)
{
    std::stringstream ss;
    ss<<name<<".vcd";
    vcd_out.open(ss.str(), std::ios_base::out|std::ios_base::binary);
    if(!vcd_out.is_open())
        throw std::runtime_error("Could not open file");
#if defined(WITH_SIM_PHASE_CALLBACKS)
    // remove from hierarchy
    sc_object::detach();
    // register regular (non-delta) callbacks
    sc_object::register_simulation_phase_callback( SC_BEFORE_TIMESTEP );
#else // explicitly register with simcontext
    sc_core::sc_get_curr_simcontext()->add_trace_file( this );
#endif
}

vcd_trace_file::~vcd_trace_file() {
    if (vcd_out.is_open()){
        vcd_out.close();
    }
}

template<typename T, typename OT=T>
bool changed(vcd_trace* trace) {
    if(reinterpret_cast<vcd_trace_t<T, OT>*>(trace)->changed()){
        reinterpret_cast<vcd_trace_t<T, OT>*>(trace)->update();
        return true;
    } else
        return false;
}
#define DECL_TRACE_METHOD_A(tp) void vcd_trace_file::trace(const tp& object, const std::string& name)\
        {all_traces.emplace_back(&changed<tp>, new vcd_trace_t<tp>(object, name));}
#define DECL_TRACE_METHOD_B(tp) void vcd_trace_file::trace(const tp& object, const std::string& name, int width)\
        {all_traces.emplace_back(&changed<tp>, new vcd_trace_t<tp>(object, name));}
#define DECL_TRACE_METHOD_C(tp, tpo) void vcd_trace_file::trace(const tp& object, const std::string& name)\
        {all_traces.emplace_back(&changed<tp, tpo>, new vcd_trace_t<tp, tpo>(object, name));}

#if (SYSTEMC_VERSION >= 20171012)
void vcd_trace_file::trace(const sc_core::sc_event& object, const std::string& name){}
void vcd_trace_file::trace(const sc_core::sc_time& object, const std::string& name){}
#endif
DECL_TRACE_METHOD_A( bool )
DECL_TRACE_METHOD_A( sc_dt::sc_bit )
DECL_TRACE_METHOD_A( sc_dt::sc_logic )

DECL_TRACE_METHOD_B( unsigned char )
DECL_TRACE_METHOD_B( unsigned short )
DECL_TRACE_METHOD_B( unsigned int )
DECL_TRACE_METHOD_B( unsigned long )
#ifdef SYSTEMC_64BIT_PATCHES
DECL_TRACE_METHOD_B( unsigned long long)
#endif
DECL_TRACE_METHOD_B( char )
DECL_TRACE_METHOD_B( short )
DECL_TRACE_METHOD_B( int )
DECL_TRACE_METHOD_B( long )
DECL_TRACE_METHOD_B( sc_dt::int64 )
DECL_TRACE_METHOD_B( sc_dt::uint64 )

DECL_TRACE_METHOD_A( float )
DECL_TRACE_METHOD_A( double )
DECL_TRACE_METHOD_A( sc_dt::sc_int_base )
DECL_TRACE_METHOD_A( sc_dt::sc_uint_base )
DECL_TRACE_METHOD_A( sc_dt::sc_signed )
DECL_TRACE_METHOD_A( sc_dt::sc_unsigned )

DECL_TRACE_METHOD_A( sc_dt::sc_fxval )
DECL_TRACE_METHOD_A( sc_dt::sc_fxval_fast )
DECL_TRACE_METHOD_C( sc_dt::sc_fxnum, double)
DECL_TRACE_METHOD_C( sc_dt::sc_fxnum_fast, double)

DECL_TRACE_METHOD_A( sc_dt::sc_bv_base )
DECL_TRACE_METHOD_A( sc_dt::sc_lv_base )
#undef DECL_TRACE_METHOD_A
#undef DECL_TRACE_METHOD_B

void vcd_trace_file::trace(const unsigned int &object, const std::string &name, const char **enum_literals) {
}

std::string vcd_trace_file::obtain_name() {
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
    std::sprintf(buf, "%c%c%c%c%c",
            char2 + first_type_used,
            char3 + first_type_used,
            char4 + first_type_used,
            char5 + first_type_used,
            char6 + first_type_used);
    vcd_name_index++;
    return std::string(buf);
}

void vcd_trace_file::write_comment(const std::string &comment) {
    vcd_out<<"$comment\n"<<comment<<"\n$end\n\n";
}

void vcd_trace_file::init() {
    std::sort(std::begin(all_traces), std::end(all_traces), [](trace_entry const& a, trace_entry const& b)->bool {return a.trc->name<b.trc->name;});
    std::unordered_map<uintptr_t, std::string> alias_map;

    vcd_scope_stack scope;
    for(auto& e:all_traces){
        auto alias_it = alias_map.find(e.trc->get_hash());
        e.trc->is_alias=alias_it!=std::end(alias_map);
        e.trc->fst_hndl = e.trc->is_alias?alias_it->second:obtain_name();
        if(!e.trc->is_alias)
            alias_map.insert({e.trc->get_hash(), e.trc->fst_hndl});
        scope.add_trace(e.trc);
    }
    std::copy_if(std::begin(all_traces), std::end(all_traces),
                std::back_inserter(active_traces),
                [](trace_entry const& e) { return !e.trc->is_alias; });
    //date:
    char buf[200];
    time_t long_time;
    time(&long_time);
    struct tm* p_tm = localtime(&long_time);
    strftime(buf, 199, "%b %d, %Y       %H:%M:%S", p_tm);
    vcd_out<<"$date\n     "<<buf<<"\n$end\n\n";
    //version:
    vcd_out<<"$version\n "<<sc_core::sc_version()<<"\n$end\n\n";
    //timescale:
    vcd_out<<"$timescale\n     "<<1_ps<<"\n$end\n\n" ;
    std::stringstream ss;
    ss<<"tracing "<<active_traces.size()<<" distinct traces out of "<<all_traces.size()<<" traces";
    write_comment(ss.str());
    scope.print(vcd_out);
}

std::string vcd_trace_file::prune_name(std::string const& orig_name) {
    static bool warned = false;
    bool braces_removed = false;
    std::string hier_name=orig_name;
    for (unsigned int i = 0; i< hier_name.length(); i++) {
        if (hier_name[i] == '[') {
            hier_name[i] = '(';
            braces_removed = true;
        }
        else if (hier_name[i] == ']') {
            hier_name[i] = ')';
            braces_removed = true;
        }
    }

    if(braces_removed && !warned){
        std::stringstream ss;
        ss << name << ":\n"
                "\tTraced objects found with name containing [], which may be\n"
                "\tinterpreted by the waveform viewer in unexpected ways.\n"
                "\tSo the [] is automatically replaced by ().";

        SC_REPORT_WARNING( sc_core::SC_ID_TRACING_OBJECT_NAME_FILTERED_, ss.str().c_str() );
    }
    return hier_name;
}

void vcd_trace_file::cycle(bool delta_cycle) {
    if(delta_cycle) return;
    if(!initialized) {
        init();
        initialized=true;
        vcd_out<<"$enddefinitions  $end\n\n$dumpvars\n";
        for(auto& e: active_traces)
            e.trc->update_and_record(vcd_out);
        vcd_out<<"$end\n\n";
    } else {
        std::ostringstream os;
        for(auto e: active_traces)
            if(e.compare_and_update(e.trc))
                e.trc->record(os);
        if(os.str().length()) {
            vcd_out<<'#'<<sc_core::sc_time_stamp()/1_ps<<'\n'<<os.str()<<'\n';
        }
    }
}

void vcd_trace_file::set_time_unit(double v, sc_core::sc_time_unit tu) {
}

sc_core::sc_trace_file* create_vcd_trace_file(const char *name, std::function<bool()> enable) {
    return  new vcd_trace_file(name, enable);
}

void close_vcd_trace_file(sc_core::sc_trace_file *tf) {
    delete static_cast<vcd_trace_file*>(tf);
}

}
