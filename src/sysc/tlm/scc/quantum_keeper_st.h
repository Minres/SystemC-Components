#ifndef __SCC_TLM_QUANTUMKEEPER_ST_H__
#define __SCC_TLM_QUANTUMKEEPER_ST_H__

#include <systemc>
#include <tlm_utils/tlm_quantumkeeper.h>

namespace tlm {
namespace scc {
///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////
struct quantumkeeper_st : public tlm_utils::tlm_quantumkeeper {
    using base = tlm_utils::tlm_quantumkeeper;

    quantumkeeper_st() {}

    virtual ~quantumkeeper_st() {}
    //! resets quantum keeper to local time 0
    void reset() override { tlm_utils::tlm_quantumkeeper::reset(); }
    //! resets the quantum keeper to the given absolute local time
    inline void reset(sc_core::sc_time const& t) {
        auto now = sc_core::sc_time_stamp();
        if(t > now) {
            m_local_time = t - now;
            m_next_sync_point = t + compute_local_quantum();
        }
    }
    //! increases local time by the given amount and syncs with SystemC kernel if local time is larger than quantum
    inline void check_and_sync(sc_core::sc_time core_inc) {
        // devirtualized inc()
        m_local_time += core_inc;
        if((sc_core::sc_time_stamp() + m_local_time) >= m_next_sync_point) {
            // devirtualized sync()
            ::sc_core::wait(m_local_time);
            m_local_time = sc_core::SC_ZERO_TIME;
            m_next_sync_point = sc_core::sc_time_stamp() + compute_local_quantum();
        }
    }
    sc_core::sc_time get_local_absolute_time() const { return base::get_current_time(); }
    /**
     * @brief execute the given function in the SystemC thread
     *
     * @param fct the function to execute
     */
    inline void execute_on_sysc(std::function<void(void)> fct) { fct(); }
    /**
     * @brief execute the given function in the SystemC thread at a given point in time
     *
     * @param fct the function to execute
     * @param when the time at which simulation time to execute the function in absolute time ticks
     */
    inline void execute_on_sysc(std::function<void(void)>& fct, sc_core::sc_time when) {
        if(when > sc_core::SC_ZERO_TIME)
            wait(when);
        fct();
    }
};
} // namespace scc
} // namespace tlm
#endif // __SCC_TLM_QUANTUMKEEPER_ST_H__
