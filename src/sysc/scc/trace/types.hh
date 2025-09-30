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

#ifndef _SCC_TRACE_TYPES_HH_
#define _SCC_TRACE_TYPES_HH_

#include <type_traits>
#include <systemc>
#include <sysc/datatypes/fx/fx.h>
#include <sysc/tracing/sc_tracing_ids.h>

namespace scc {
namespace trace {

enum trace_type {WIRE, REAL};
/*******************************************************************************************************
 *
 *******************************************************************************************************/
template<typename T> struct traits {
    static inline trace_type get_type() { return std::is_floating_point<T>::value?REAL:WIRE;}
    static inline unsigned get_bits(T const&) { return std::is_floating_point<T>::value?1:sizeof(T)*8;}
};

template<> inline trace_type traits<sc_dt::sc_fxval>::get_type(){ return REAL;}
template<> inline trace_type traits<sc_dt::sc_fxval_fast>::get_type(){ return REAL;}
template<> inline trace_type traits<sc_dt::sc_fxnum>::get_type(){ return REAL;}
template<> inline trace_type traits<sc_dt::sc_fxnum_fast>::get_type(){ return REAL;}

template<> inline unsigned traits<bool>::get_bits(bool const&){ return 1; }
template<> inline unsigned traits<sc_dt::sc_bit>::get_bits(sc_dt::sc_bit const&){ return 1; }
template<> inline unsigned traits<sc_dt::sc_logic>::get_bits(sc_dt::sc_logic const&){ return 1; }
template<> inline unsigned traits<sc_dt::sc_bv_base>::get_bits(sc_dt::sc_bv_base const& o){ return o.length(); }
template<> inline unsigned traits<sc_dt::sc_lv_base>::get_bits(sc_dt::sc_lv_base const& o){ return o.length(); }
template<> inline unsigned traits<sc_dt::sc_int_base>::get_bits(sc_dt::sc_int_base const& o){ return o.length(); }
template<> inline unsigned traits<sc_dt::sc_uint_base>::get_bits(sc_dt::sc_uint_base const& o){ return o.length(); }
template<> inline unsigned traits<sc_dt::sc_signed>::get_bits(sc_dt::sc_signed const& o){ return o.length(); }
template<> inline unsigned traits<sc_dt::sc_unsigned>::get_bits(sc_dt::sc_unsigned const& o){ return o.length(); }
template<> inline unsigned traits<sc_dt::sc_fxval>::get_bits(sc_dt::sc_fxval const&){ return 1; }
template<> inline unsigned traits<sc_dt::sc_fxval_fast>::get_bits(sc_dt::sc_fxval_fast const&){ return 1; }
//FIXME: printing fxnum/fxnum_fast as real is just a workaround
// should return act_val.wl();
template<> inline unsigned traits<sc_dt::sc_fxnum>::get_bits(sc_dt::sc_fxnum const&){return 1; }
template<> inline unsigned traits<sc_dt::sc_fxnum_fast>::get_bits(sc_dt::sc_fxnum_fast const& o){ return 1; }
}
}


#endif /* _SCC_TRACE_TYPES_HH_ */
