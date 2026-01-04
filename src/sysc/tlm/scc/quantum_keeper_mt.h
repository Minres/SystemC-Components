#ifndef __SCC_TLM_QUANTUMKEEPER_MT_H__
#define __SCC_TLM_QUANTUMKEEPER_MT_H__

#include "qk/sc_time_syncronizer.h"
#include <scc/async_thread.h>
#include <sysc/kernel/sc_simcontext.h>

#if SC_VERSION_MAJOR < 3
#error "Multithreaded quantum keeper is only supported with SystemC 3.0 and newer"
#else
#define DEBUG_MT_SCHEDULING

namespace tlm {
namespace scc {
/**
 * the multi threading quantum keeper
 */
struct quantumkeeper_mt : sc_core::sc_object {
    /**
     * @brief Construct a new quantumkeeper mt object
     *
     */
    quantumkeeper_mt()
    : sc_core::sc_object("qk")
    , gtk_idx(qk::sc_time_syncronizer::get().get_channel_index()) {
        sc_core::sc_spawn_options opt;
        opt.spawn_method();
        opt.set_sensitivity(&keep_alive);
    }
    /**
     * @brief Destroy the quantumkeeper mt object
     *
     */
    virtual ~quantumkeeper_mt() = default;

    void run_thread(std::function<sc_core::sc_time()> f) {
        qk::sc_time_syncronizer::get().notify_client_blocked(gtk_idx, false);
        reset(sc_core::sc_time_stamp());
        thread_executor.start([this, f]() {
            this->check_and_sync(sc_core::SC_ZERO_TIME);
            auto ret = f();
            qk::sc_time_syncronizer::get().notify_client_blocked(gtk_idx, true);
            return ret;
        });
        wait(thread_executor.thread_finish_event());
    }
    /**
     * @brief increments the local time and updates the global time keeper with the new time
     *
     * @param core_inc the amount to increment the local time
     */
    inline void inc(sc_core::sc_time const& core_inc) {
        local_absolute_time += core_inc;
        local_time_ticks_limit = qk::global_time_keeper::get().get_client_max_time_ticks(gtk_idx);
    }
    /**
     * @brief checks the local time against the global time keeper and holds the thread until it is within the quantum
     *
     * @param core_inc the amount to increment the local time
     */
    inline void check_and_sync(sc_core::sc_time const& core_inc) {
        inc(core_inc);
        qk::global_time_keeper::get().update_client_time_ticks(gtk_idx, local_absolute_time.value());
        // wait until the SystemC thread advanced to the same time then we are minus the global quantum
        while(local_absolute_time.value() > local_time_ticks_limit) {
            std::this_thread::yield(); // should do the same than __builtin_ia32_pause() or _mm_pause() on MSVC
            local_time_ticks_limit = qk::global_time_keeper::get().get_client_max_time_ticks(gtk_idx);
        }
    }
    /**
     * @brief execute the given function in the SystemC thread
     *
     * @param fct the function to execute
     */
    inline void execute_on_sysc(std::function<void(void)> fct) { execute_on_sysc(fct, local_absolute_time); }
    /**
     * @brief execute the given function in the SystemC thread at a given point in time
     *
     * @param fct the function to execute
     * @param when the time at which simulation time to execute the function in absolute time ticks
     */
    inline void execute_on_sysc(std::function<void(void)>& fct, sc_core::sc_time when) {
        qk::callback_task task([this, &fct]() {
            auto t0 = sc_core::sc_time_stamp();
            fct();
            return sc_core::sc_time_stamp() - t0;
        });
        auto ret = task.get_future();
        qk::global_time_keeper::get().schedule_task(gtk_idx, std::move(task), when.value());
        auto duration = ret.get();
        check_and_sync(duration);
    }
    /**
     * @brief Get the local time of this thread
     *
     * @return sc_core::sc_time the local time relative to the SystemC kernel
     */
    sc_core::sc_time get_local_time() const { return local_absolute_time - get_current_sc_time(); }
    /**
     * @brief Get the local absolute time of this thread
     *
     * @return sc_core::sc_time the local time
     */
    sc_core::sc_time get_local_absolute_time() const { return local_absolute_time; }
    /**
     * @brief Get the current sc time of this thread
     *
     * @return sc_core::sc_time the time of the SystemC kernel
     */
    sc_core::sc_time get_current_sc_time() const { return sc_core::sc_time::from_value(qk::global_time_keeper::get().get_sc_time_ticks()); }
    /**
     * @brief resets the local time of this thread to the SystemC kernel time
     *
     */
    void reset() { reset(get_current_sc_time()); }
    /**
     * @brief esets the local time of this thread to the given absolute time
     *
     * @param abs_time the absolute time to set
     */
    void reset(sc_core::sc_time const& abs_time) { local_absolute_time = sc_core::sc_time_stamp(); }

    void unblock_thread() { qk::global_time_keeper::get().update_client_time_ticks(gtk_idx, local_absolute_time.value()); }

protected:
    size_t gtk_idx;
    ::scc::async_thread thread_executor;
    sc_core::sc_event keep_alive;
    sc_core::sc_time local_absolute_time;
    uint64_t local_time_ticks_limit{0};
};
} // namespace scc
} // namespace tlm
#endif
#endif // __SCC_TLM_QUANTUMKEEPER_MT_H__
