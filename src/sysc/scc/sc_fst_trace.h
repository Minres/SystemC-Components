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


#ifndef SCC_FST_TRACE_H
#define SCC_FST_TRACE_H

#include "fstapi.h"
#include <systemc>
#include <sysc/tracing/sc_trace_file_base.h>
#include <functional>

namespace sc_core {
class sc_time;
}
namespace scc {

class fst_trace;
template<class T> class fst_T_trace;

struct fst_trace_file : public sc_core::sc_trace_file_base {

    fst_trace_file(const char *name, std::function<bool()>& enable);

    virtual ~fst_trace_file();

protected:
#define DECL_TRACE_METHOD_A(tp) virtual void trace(const tp& object, const std::string& name);
#define DECL_TRACE_METHOD_B(tp) virtual void trace(const tp& object, const std::string& name, int width);

#if (SYSTEMC_VERSION >= 20171012)
    DECL_TRACE_METHOD_A( sc_core::sc_event )
    DECL_TRACE_METHOD_A( sc_core::sc_time )
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
    DECL_TRACE_METHOD_A( sc_dt::sc_fxnum )
    DECL_TRACE_METHOD_A( sc_dt::sc_fxnum_fast )

    DECL_TRACE_METHOD_A( sc_dt::sc_bv_base )
    DECL_TRACE_METHOD_A( sc_dt::sc_lv_base )
#undef DECL_TRACE_METHOD_A
#undef DECL_TRACE_METHOD_B

    // Output a comment to the trace file
     void write_comment(const std::string& comment);

    // Write trace info for cycle.
     void cycle(bool delta_cycle);

private:
#if WITH_SIM_PHASE_CALLBACKS
    // avoid hidden overload warnings
    virtual void trace( sc_trace_file* ) const;
#endif

    sc_trace_file_base::unit_type previous_time_units_low;
    sc_trace_file_base::unit_type previous_time_units_high;
    std::function<bool()> check_enabled;

    void* m_fst;
    fstHandle* m_symbolp{nullptr};
    char* m_strbuf{nullptr};
};

// ----------------------------------------------------------------------------
sc_core::sc_trace_file *scc_create_fst_trace_file(const char* name, std::function<bool()> enable = std::function<bool()>());

void scc_close_fst_trace_file( sc_core::sc_trace_file* tf );

} // namesoace scc

#endif // SCC_FST_TRACE_H
