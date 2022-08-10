/*******************************************************************************
 * Copyright 2018-2021 MINRES Technologies GmbH
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

#ifndef _SCC_OBSERVER_H_
#define _SCC_OBSERVER_H_

#include <string>
// needed for typedefs like sc_dt::int64
#include <sysc/kernel/sc_ver.h>
#include <sysc/datatypes/int/sc_nbdefs.h>
#include <sysc/communication/sc_signal_ifs.h>
#include <sysc/kernel/sc_dynamic_processes.h>
#include <sysc/kernel/sc_process_handle.h>
// Some forward declarations
namespace sc_dt {
class sc_bit;
class sc_logic;
class sc_bv_base;
class sc_lv_base;
class sc_signed;
class sc_unsigned;
class sc_int_base;
class sc_uint_base;
class sc_fxval;
class sc_fxval_fast;
class sc_fxnum;
class sc_fxnum_fast;
} // namespace sc_dt

/** \ingroup scc-sysc
 *  @{
 */
/**@{*/
//! @brief SCC SystemC utilities
namespace scc {
/**
 * @brief The interface defining an observer.
 */
struct observer {
    /**
     * @brief A handle to be used be the observed object to notify the observer about a change
     */
    struct notification_handle {
        virtual bool notify() = 0;
        virtual ~notification_handle() {}
    };
    virtual notification_handle* observe(bool const& o, std::string const& nm) = 0;
    virtual notification_handle* observe(sc_dt::sc_bit const& o, std::string const& nm) = 0;
    virtual notification_handle* observe(sc_dt::sc_logic const& o, std::string const& nm) = 0;

    virtual notification_handle* observe(unsigned char const& o, std::string const& nm) = 0;
    virtual notification_handle* observe(unsigned short const& o, std::string const& nm) = 0;
    virtual notification_handle* observe(unsigned int const& o, std::string const& nm) = 0;
    virtual notification_handle* observe(unsigned long const& o, std::string const& nm) = 0;
    virtual notification_handle* observe(char const& o, std::string const& nm) = 0;
    virtual notification_handle* observe(short const& o, std::string const& nm) = 0;
    virtual notification_handle* observe(int const& o, std::string const& nm) = 0;
    virtual notification_handle* observe(long const& o, std::string const& nm) = 0;
    virtual notification_handle* observe(sc_dt::int64 const& o, std::string const& nm) = 0;
    virtual notification_handle* observe(sc_dt::uint64 const& o, std::string const& nm) = 0;

    virtual notification_handle* observe(float const& o, std::string const& nm) = 0;
    virtual notification_handle* observe(double const& o, std::string const& nm) = 0;
    virtual notification_handle* observe(sc_dt::sc_int_base const& o, std::string const& nm) = 0;
    virtual notification_handle* observe(sc_dt::sc_uint_base const& o, std::string const& nm) = 0;
    virtual notification_handle* observe(sc_dt::sc_signed const& o, std::string const& nm) = 0;
    virtual notification_handle* observe(sc_dt::sc_unsigned const& o, std::string const& nm) = 0;

    virtual notification_handle* observe(sc_dt::sc_fxval const& o, std::string const& nm) = 0;
    virtual notification_handle* observe(sc_dt::sc_fxval_fast const& o, std::string const& nm) = 0;
    virtual notification_handle* observe(sc_dt::sc_fxnum const& o, std::string const& nm) = 0;
    virtual notification_handle* observe(sc_dt::sc_fxnum_fast const& o, std::string const& nm) = 0;

    virtual notification_handle* observe(sc_dt::sc_bv_base const& o, std::string const& nm) = 0;
    virtual notification_handle* observe(sc_dt::sc_lv_base const& o, std::string const& nm) = 0;
#if (SYSTEMC_VERSION >= 20171012)
    virtual notification_handle* observe(sc_core::sc_time const& o, std::string const& nm) = 0;
    virtual notification_handle* observe(sc_core::sc_event const& o, std::string const& nm) = 0;
#endif
    virtual ~observer() {}
};

#define DECL_REGISTER_METHOD_A(tp)                                                                                     \
		inline observer::notification_handle* observe(observer* obs, tp const& o, std::string const& nm) {                 \
	return obs->observe(o, nm);                                                                                    \
}
#if (SYSTEMC_VERSION >= 20171012)
DECL_REGISTER_METHOD_A( sc_core::sc_event )
DECL_REGISTER_METHOD_A( sc_core::sc_time )
#endif
DECL_REGISTER_METHOD_A(bool)
DECL_REGISTER_METHOD_A(sc_dt::sc_bit)
DECL_REGISTER_METHOD_A(sc_dt::sc_logic)

DECL_REGISTER_METHOD_A(unsigned char)
DECL_REGISTER_METHOD_A(unsigned short)
DECL_REGISTER_METHOD_A(unsigned int)
DECL_REGISTER_METHOD_A(unsigned long)
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
DECL_REGISTER_METHOD_A(sc_dt::sc_fxnum)
DECL_REGISTER_METHOD_A(sc_dt::sc_fxnum_fast)

DECL_REGISTER_METHOD_A(sc_dt::sc_bv_base)
DECL_REGISTER_METHOD_A(sc_dt::sc_lv_base)

#undef DECL_REGISTER_METHOD_A

template< class T > inline
void sc_trace(sc_core::sc_trace_file* tf, const sc_core::sc_signal_in_if<T>& object, const char* name ) {
    if(auto* obs = dynamic_cast<observer*>(tf)) {
        if(auto* handle = obs->observe(object.read(), std::string(name))){
            sc_core::sc_spawn_options scopts;
            scopts.spawn_method();
            scopts.set_sensitivity(&object.default_event());
            sc_core::sc_spawn([handle](){
                if(!handle->notify()) // suspend if trace is an alias
                    sc_core::sc_get_current_process_handle().disable();
            }, nullptr, &scopts);
        }
    } else
        sc_core::sc_trace( tf, object.read(), name );
}

template <class T> inline
void sc_trace(sc_core::sc_trace_file* tf, const sc_core::sc_signal_in_if<T>& object,const std::string& name) {
    sc_trace(tf, object, name.c_str());
}

template <class T> inline
void sc_trace(sc_core::sc_trace_file* tf, const sc_core::sc_in<T>& port, char const* name) {
    const sc_core::sc_signal_in_if<T>* iface = 0;
    if (sc_core::sc_get_curr_simcontext()->elaboration_done() )
        iface = dynamic_cast<const sc_core::sc_signal_in_if<T>*>( port.get_interface() );
    if ( iface )
        if(auto* obs = dynamic_cast<observer*>(tf)) {
            if(auto* handle = obs->observe(port.read(), name)){
                sc_core::sc_spawn_options scopts;
                scopts.spawn_method();
                scopts.set_sensitivity(&port.default_event());
                sc_core::sc_spawn([handle](){
                    if(!handle->notify()) // suspend if trace is an alias
                        sc_core::sc_get_current_process_handle().disable();
                }, nullptr, &scopts);
            }
        } else
            sc_trace( tf, iface->read(), name );
    else
        port.add_trace_internal( tf, name );
}

template <class T> inline
void sc_trace(sc_core::sc_trace_file* tf, const sc_core::sc_in<T>& port, const std::string& name) {
    sc_trace(tf, port, name.c_str());
}

template <class T> inline
void sc_trace( sc_core::sc_trace_file* tf, const sc_core::sc_inout<T>& port, char const* name ) {
    const sc_core::sc_signal_in_if<T>* iface = 0;
    if (sc_core::sc_get_curr_simcontext()->elaboration_done() )
        iface = dynamic_cast<const sc_core::sc_signal_in_if<T>*>( port.get_interface() );
    if ( iface )
        if(auto* obs = dynamic_cast<observer*>(tf)) {
            if(auto* handle = obs->observe(port.read(), name)){
                sc_core::sc_spawn_options scopts;
                scopts.spawn_method();
                scopts.set_sensitivity(&port.default_event());
                sc_core::sc_spawn([handle](){
                    if(!handle->notify()) // suspend if trace is an alias
                        sc_core::sc_get_current_process_handle().disable();
                }, nullptr, &scopts);
            }
        } else
            sc_trace( tf, iface->read(), name );
    else
        port.add_trace_internal( tf, name );
}

template <class T> inline
void sc_trace(sc_core::sc_trace_file* tf, const sc_core::sc_inout<T>& port, const std::string& name) {
    sc_trace(tf, port, name.c_str());
}

#define DEFN_TRACE_FUNC_REF_A(tp)                                             \
		inline void sc_trace( sc_core::sc_trace_file* tf, const tp& object, const std::string& name )      \
		{ sc_core::sc_trace(tf, object, name); }

#define DEFN_TRACE_FUNC_PTR_A(tp)                                             \
		inline void sc_trace( sc_core::sc_trace_file* tf, const tp* object, const std::string& name )      \
		{ sc_core::sc_trace(tf, object, name); }

#define DEFN_TRACE_FUNC_A(tp)                                                 \
		DEFN_TRACE_FUNC_REF_A(tp)                                                     \
		DEFN_TRACE_FUNC_PTR_A(tp)

#define DEFN_TRACE_FUNC_REF_B(tp)                                             \
		inline void sc_trace( sc_core::sc_trace_file* tf, const tp& object, const std::string& name, int width) \
		{ sc_core::sc_trace(tf, object, name, width); } \
        inline void sc_trace( sc_core::sc_trace_file* tf, const tp& object, const std::string& name) \
        { sc_core::sc_trace(tf, object, name, sizeof(tp)*8); }

#define DEFN_TRACE_FUNC_PTR_B(tp)                                             \
		inline void sc_trace( sc_core::sc_trace_file* tf, const tp* object, const std::string& name, int width) \
		{ sc_core::sc_trace(tf, object, name, width); } \
        inline void sc_trace( sc_core::sc_trace_file* tf, const tp* object, const std::string& name) \
        { sc_core::sc_trace(tf, object, name, sizeof(tp)*8); }


#define DEFN_TRACE_FUNC_B(tp)                                                 \
		DEFN_TRACE_FUNC_REF_B(tp)                                                     \
		DEFN_TRACE_FUNC_PTR_B(tp)


DEFN_TRACE_FUNC_A( sc_core::sc_event )
DEFN_TRACE_FUNC_A( sc_core::sc_time )

DEFN_TRACE_FUNC_A( bool )
DEFN_TRACE_FUNC_A( float )
DEFN_TRACE_FUNC_A( double )

DEFN_TRACE_FUNC_B( unsigned char )
DEFN_TRACE_FUNC_B( unsigned short )
DEFN_TRACE_FUNC_B( unsigned int )
DEFN_TRACE_FUNC_B( unsigned long )
#ifdef SYSTEMC_64BIT_PATCHES
DEFN_TRACE_FUNC_B( unsigned long long)
#endif
DEFN_TRACE_FUNC_B( char )
DEFN_TRACE_FUNC_B( short )
DEFN_TRACE_FUNC_B( int )
DEFN_TRACE_FUNC_B( long )
#ifdef SYSTEMC_64BIT_PATCHES
DEFN_TRACE_FUNC_B( long long)
#endif
DEFN_TRACE_FUNC_B( sc_dt::int64 )
DEFN_TRACE_FUNC_B( sc_dt::uint64 )

DEFN_TRACE_FUNC_A( sc_dt::sc_bit )
DEFN_TRACE_FUNC_A( sc_dt::sc_logic )

DEFN_TRACE_FUNC_A( sc_dt::sc_int_base )
DEFN_TRACE_FUNC_A( sc_dt::sc_uint_base )
DEFN_TRACE_FUNC_A( sc_dt::sc_signed )
DEFN_TRACE_FUNC_A( sc_dt::sc_unsigned )

DEFN_TRACE_FUNC_A( sc_dt::sc_bv_base )
DEFN_TRACE_FUNC_A( sc_dt::sc_lv_base )

#ifdef SC_INCLUDE_FX

DEFN_TRACE_FUNC_A( sc_dt::sc_fxval )
DEFN_TRACE_FUNC_A( sc_dt::sc_fxval_fast )
DEFN_TRACE_FUNC_A( sc_dt::sc_fxnum )
DEFN_TRACE_FUNC_A( sc_dt::sc_fxnum_fast )

#endif // SC_INCLUDE_FX

#undef DEFN_TRACE_FUNC_REF_A
#undef DEFN_TRACE_FUNC_PTR_A
#undef DEFN_TRACE_FUNC_A

#undef DEFN_TRACE_FUNC_REF_B
#undef DEFN_TRACE_FUNC_PTR_B
#undef DEFN_TRACE_FUNC_B

} /* namespace scc */
/** @} */ // end of scc-sysc
#endif /* _SCC_OBSERVER_H_ */
