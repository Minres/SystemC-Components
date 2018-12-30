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

    template<typename T>
    struct value_holder_t : public value_registry_if::value_holder {
        const T& value;
        value_holder_t(const std::string& name, const T& value):value_registry_if::value_holder(name), value(value){}
        std::string to_string(){
            std::stringstream ss;
            ss<<value;
            return ss.str();
        }
    };
    template<typename T>
    struct value_holder_masked_t : public value_registry_if::value_holder {
        const T& value;
        const T& mask;
        value_holder_masked_t(const std::string& name, const T& value, int width):value_registry_if::value_holder(name), value(value), mask((1<<width)-1){}
        std::string to_string(){
            std::stringstream ss;
            ss<<(value&mask);
            return ss.str();
        }
    };
    // Constructor
    value_registry_impl(){}

#define DECL_TRACE_METHOD_A(tp) void trace(const tp& object, const std::string& name ) override { \
        holder[name]= new value_holder_t<tp>(name, object); \
    }
#define DECL_TRACE_METHOD_B(tp) void trace(const tp& object, const std::string& name, int width ) override { \
        holder[name]= new value_holder_masked_t<tp>(name, object, width); \
    }

    DECL_TRACE_METHOD_A( sc_event )
    DECL_TRACE_METHOD_A( sc_time )

    DECL_TRACE_METHOD_A( bool )
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

    std::unordered_map<std::string, value_registry_if::value_holder*> holder;

    sc_dt::uint64 dummy=0;
};

value_registry::value_registry(): tracer_base(sc_core::sc_module_name(sc_core::sc_gen_unique_name("value_registrar"))) {
    trf=new value_registry_impl();
}

scc::value_registry::~value_registry() {
    delete dynamic_cast<value_registry_impl*>(trf);
}

std::vector<std::string> scc::value_registry::get_names() {
    auto& holder = dynamic_cast<value_registry_impl*>(trf)->holder;
    std::vector<std::string> keys(holder.size());
    for(auto kv : holder) keys.push_back(kv.first);
    return keys;
}

value_registry_if::value_holder& scc::value_registry::get_value(std::string& name) {
    return *dynamic_cast<value_registry_impl*>(trf)->holder[name];
}

void scc::value_registry::end_of_elaboration() {
    for (auto o : sc_get_top_level_objects(sc_curr_simcontext)) descend(o);
}

