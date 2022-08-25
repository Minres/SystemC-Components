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

#include <scc/observer.h>
#include <sysc/tracing/sc_trace.h>
#include <sysc/kernel/sc_ver.h>
#include <deque>
#include <vector>
#include <functional>

namespace sc_core {
class sc_time;
}
/** \ingroup scc-sysc
 *  @{
 */
/**@{*/
//! @brief SCC SystemC utilities
namespace scc {
//! @brief SCC SystemC tracing utilities
namespace trace {
class fst_trace;
}
struct fst_trace_file : public sc_core::sc_trace_file, public observer {

    fst_trace_file(const char *name, std::function<bool()>& enable);

    virtual ~fst_trace_file();

protected:
#define DECL_TRACE_METHOD_A(tp) void trace(const tp& object, const std::string& name) override;
#define DECL_TRACE_METHOD_B(tp) void trace(const tp& object, const std::string& name, int width) override;
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

    void trace( const unsigned int& object,
            const std::string& name,
            const char** enum_literals ) override;

#define DECL_REGISTER_METHOD_A(tp) observer::notification_handle* observe(tp const& o, std::string const& nm) override;
#if (SYSTEMC_VERSION >= 20171012)
    DECL_REGISTER_METHOD_A( sc_core::sc_event )
    DECL_REGISTER_METHOD_A( sc_core::sc_time )
#endif
    DECL_REGISTER_METHOD_A( bool )
    DECL_REGISTER_METHOD_A( sc_dt::sc_bit )
    DECL_REGISTER_METHOD_A( sc_dt::sc_logic )

    DECL_REGISTER_METHOD_A( unsigned char )
    DECL_REGISTER_METHOD_A( unsigned short )
    DECL_REGISTER_METHOD_A( unsigned int )
    DECL_REGISTER_METHOD_A( unsigned long )
    DECL_REGISTER_METHOD_A( char )
    DECL_REGISTER_METHOD_A( short )
    DECL_REGISTER_METHOD_A( int )
    DECL_REGISTER_METHOD_A( long )
    DECL_REGISTER_METHOD_A( sc_dt::int64 )
    DECL_REGISTER_METHOD_A( sc_dt::uint64 )

    DECL_REGISTER_METHOD_A( float )
    DECL_REGISTER_METHOD_A( double )
    DECL_REGISTER_METHOD_A( sc_dt::sc_int_base )
    DECL_REGISTER_METHOD_A( sc_dt::sc_uint_base )
    DECL_REGISTER_METHOD_A( sc_dt::sc_signed )
    DECL_REGISTER_METHOD_A( sc_dt::sc_unsigned )

    DECL_REGISTER_METHOD_A( sc_dt::sc_fxval )
    DECL_REGISTER_METHOD_A( sc_dt::sc_fxval_fast )
    DECL_REGISTER_METHOD_A( sc_dt::sc_fxnum )
    DECL_REGISTER_METHOD_A( sc_dt::sc_fxnum_fast )

    DECL_REGISTER_METHOD_A( sc_dt::sc_bv_base )
    DECL_REGISTER_METHOD_A( sc_dt::sc_lv_base )
#undef DECL_REGISTER_METHOD_A

    // Output a comment to the trace file
    void write_comment(const std::string& comment) override;

    // Write trace info for cycle.
    void cycle(bool delta_cycle) override;

    void set_time_unit( double v, sc_core::sc_time_unit tu ) override;

private:
#if WITH_SC_TRACING_PHASE_CALLBACKS
    // avoid hidden overload warnings
    virtual void trace( sc_trace_file* ) const;
#endif

    void init();
    std::function<bool()> check_enabled;

    void* m_fst{nullptr};
    struct trace_entry: public observer::notification_handle {
        bool (*compare_and_update)(trace::fst_trace*);
        trace::fst_trace* trc;
        fst_trace_file* that;
        bool notify() override;
        trace_entry(fst_trace_file* owner, bool (*compare_and_update)(trace::fst_trace*), trace::fst_trace* trc)
        :compare_and_update{compare_and_update}, trc{trc}, that{owner}{}
        virtual ~trace_entry(){}
    };
    std::deque<trace_entry> all_traces;
    std::vector<trace_entry*> pull_traces;
    std::vector<trace::fst_trace*> changed_traces;
    std::vector<trace::fst_trace*> triggered_traces;
    uint64_t last_emitted_ts{std::numeric_limits<uint64_t>::max()};
};
} // namespace scc
/** @} */ // end of scc-sysc
#endif // SCC_FST_TRACE_H
