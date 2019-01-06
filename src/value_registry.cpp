/*
 * trace_file.cpp
 *
 *  Created on: 30.12.2018
 *      Author: eyck
 */

#include <scc/value_registry.h>
#include <sysc/datatypes/fx/sc_fxval.h>
#include <sysc/datatypes/fx/sc_fxnum.h>
#include <string>
#include <sstream>
#include <unordered_map>

using namespace sc_core;
using namespace scc;

std::ostream& operator<<(std::ostream& os, const sc_event& evt){
    os<<evt.triggered();
    return os;
}


class SC_API value_registry_impl: public sc_trace_file {
public:
    // Constructor
    value_registry_impl(){}

    sc_module* get_mod4name(const std::string& name) const {
        sc_module* mod = nullptr;
        sc_object* obj = sc_find_object(name.c_str());
        if(obj) return mod;
        auto pos = name.length()-1;
        do {
            pos = name.find_last_of('.', pos);
            mod = dynamic_cast<sc_module*>(sc_find_object(name.substr(0, pos).c_str()));
        }while(pos>0 && mod == nullptr);
        return mod;
    }
#define DECL_TRACE_METHOD_A(tp) void trace(const tp& object, const std::string& name ) override { \
        if(sc_core::sc_find_object(name.c_str()) != nullptr) {  \
            if(sc_module* mod = get_mod4name(name)){ \
                sc_get_curr_simcontext()->hierarchy_push(mod); \
                auto* o = new sc_variable_t<tp>(name, object); \
                sc_get_curr_simcontext()->hierarchy_pop(); \
                holder[name]=o; \
            } \
        } \
    }
#define DECL_TRACE_METHOD_B(tp) void trace(const tp& object, const std::string& name, int width ) override { \
        if(sc_core::sc_find_object(name.c_str()) != nullptr) {  \
            if(sc_module* mod = get_mod4name(name)){ \
                sc_get_curr_simcontext()->hierarchy_push(mod); \
                auto* o = new sc_variable_masked_t<tp>(name, object, width); \
                sc_get_curr_simcontext()->hierarchy_pop(); \
                holder[name]=o; \
            } \
        } \
    }

    DECL_TRACE_METHOD_A( sc_event )
    DECL_TRACE_METHOD_A( sc_time )

    //DECL_TRACE_METHOD_A( bool )
    void trace(const bool& object, const std::string& name ) override {
            if(sc_core::sc_find_object(name.c_str()) == nullptr) {
                if(sc_module* mod = get_mod4name(name)){
                    sc_get_curr_simcontext()->hierarchy_push(mod);
                    auto* o = new sc_variable_t<bool>(name.substr(strlen(mod->name())+1), object);
                    sc_get_curr_simcontext()->hierarchy_pop();
                    holder[name]=o;
                }
            }
        }
    DECL_TRACE_METHOD_A( sc_dt::sc_bit )
    DECL_TRACE_METHOD_A( sc_dt::sc_logic )

    DECL_TRACE_METHOD_B( unsigned char )
    DECL_TRACE_METHOD_B( unsigned short )
    DECL_TRACE_METHOD_B( unsigned int )
    DECL_TRACE_METHOD_B( unsigned long )
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
    DECL_TRACE_METHOD_A( sc_dt::sc_fxnum )
    DECL_TRACE_METHOD_A( sc_dt::sc_fxnum_fast )

    DECL_TRACE_METHOD_A( sc_dt::sc_bv_base )
    DECL_TRACE_METHOD_A( sc_dt::sc_lv_base )

#undef DECL_TRACE_METHOD_A
#undef DECL_TRACE_METHOD_B

    // Trace an enumerated object - where possible output the enumeration
    // literals in the trace file. Enum literals is a null terminated array
    // of null terminated char* literal strings.
    void trace( const unsigned int& object, const std::string& name, const char** enum_literals ) override {

    }

    // Output a comment to the trace file
    void write_comment( const std::string& comment ) override { }

    // Set the amount of space before next column
    // (For most formats this does nothing)
    // void space( int n );

    // Also trace transitions between delta cycles if flag is true.
    // void delta_cycles( bool flag );

    // Set time unit.
    void set_time_unit( double v, sc_time_unit tu ) override {};

    // Write trace info for cycle
    void cycle( bool delta_cycle ) override {}

    // Helper for event tracing
    const sc_dt::uint64& event_trigger_stamp( const sc_event& event ) const { return dummy; }

    // Flush results and close file
    virtual ~value_registry_impl(){  for(auto kv : holder) delete kv.second; }

    std::unordered_map<std::string, sc_variable*> holder;

    sc_dt::uint64 dummy=0;

};

value_registry::value_registry(): tracer_base(sc_core::sc_module_name(sc_core::sc_gen_unique_name("value_registrar", true))) {
    trf=new value_registry_impl();
}

scc::value_registry::~value_registry() {
    delete dynamic_cast<value_registry_impl*>(trf);
}

std::vector<std::string> scc::value_registry::get_names() const {
    auto& holder = dynamic_cast<value_registry_impl*>(trf)->holder;
    std::vector<std::string> keys;
    keys.reserve(holder.size());
    for(auto kv : holder) keys.push_back(kv.first);
    return keys;
}

const sc_variable* scc::value_registry::get_value(std::string name) const {
    auto* reg=dynamic_cast<value_registry_impl*>(trf);
    auto it=reg->holder.find(name);
    if(it!=reg->holder.end()){
        std::cerr<<"returning value holder ptr"<<std::endl;
        return it->second;
    }
    std::cerr<<"returning nullptr"<<std::endl;
    return nullptr;
}

void scc::value_registry::end_of_elaboration() {
    for (auto o : sc_get_top_level_objects(sc_curr_simcontext))
        descend(o, true);
}
