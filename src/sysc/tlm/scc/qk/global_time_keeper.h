#ifndef __SCC_TLM_QK_GLOBAL_TIME_KEEPER_H__
#define __SCC_TLM_QK_GLOBAL_TIME_KEEPER_H__

#include "types.h"
#include <deque>

namespace tlm {
namespace scc {
namespace qk {
///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////
struct sc_time_syncronizer;
/**
 * @brief the global time keeper, a singleton
 * @details The global time keeper receives actual time stamps of all client threads
 * and calculates the max time they are alloed to advance beyond systemc. All calculations are done
 * based on ticks which is the minimal time step a simulator can do. The SystemC thread is special
 * in that it is time wise always the last/slowest thread.
 *
 */
struct global_time_keeper {
    friend class sc_time_syncronizer;
    /**
     * @brief the maximum timestep the simulator is allowed to do if there are no future events
     *
     */
    const sc_core::sc_time sc_time_step = 1_ms;
    /**
     * @brief the singleton getter
     *
     * @return global_time_keeper&
     */
    static global_time_keeper& get() {
        static global_time_keeper keeper;
        return keeper;
    }
    /**
     * @brief Get the minimum time ticks of all client threads (not SystemC)
     *
     * @return uint64_t the absolute number of ticks
     */
    inline uint64_t get_client_min_time_ticks() { return client_min_time; }
    /**
     * @brief Get the maximum time ticks a client thread is allowed to advance
     *
     * @param idx the id of the client thread, to be obtained using get_channel_index()
     * @return uint64_t the absolute number of maximum ticks, if no new information it returns -1
     */
    inline uint64_t get_client_max_time_ticks(size_t idx) { return client_max_time; }
    /**
     * @brief updates the global time keeper with the local time ticks of a clinet thread
     *
     * @param idx the id of the client thread, to be obtained using get_channel_index()
     * @param tick the absolute number of actual ticks
     */
    inline void update_client_time_ticks(size_t idx, uint64_t tick) {
        client_coms_channels[idx].client2time_keeper.push(std::move(comms_entry{tick, std::move(callback_task())}));
        update_it.store(true);
        std::unique_lock<std::mutex> lk(upd_mtx);
        update.notify_all();
    }
    /**
     * @brief updates the global time keeper with the local time ticks of a clinet thread and schedules a task at this new time point
     *
     * @param idx the id of the client thread, to be obtained using get_channel_index()
     * @param tick  the absolute number of actual ticks
     * @param task the task to be executed in the SystemC kernel. It shall return the time it used for execution
     */
    inline void schedule_task(size_t idx, std::packaged_task<sc_core::sc_time(void)>&& task, uint64_t when) {
        client_coms_channels[idx].client2time_keeper.push(std::move(comms_entry{when, std::move(task)}));
        update_it.store(true);
        std::unique_lock<std::mutex> lk(upd_mtx);
        update.notify_all();
    }
    /**
     * @brief Get the maximum sc time ticks a client thread is allowed to advance
     *
     * @return uint64_t  the absolute number of ticks
     */
    inline uint64_t get_sc_time_ticks() { return sc_kernel_time.load(); }
    /**
     * @brief updates the global time keeper with the local time ticks of the SystemC thread
     *
     * @param tick the absolute number of actual SystemC ticks
     */
    inline void update_sc_time_ticks(uint64_t tick) {
        sc_coms_channel.client2time_keeper.push(std::move(comms_entry{tick, std::move(callback_task())}));
        update_it.store(true);
        std::unique_lock<std::mutex> lk(upd_mtx);
        update.notify_all();
    }

protected:
    global_time_keeper();

    global_time_keeper(global_time_keeper const&) = delete;

    global_time_keeper(global_time_keeper&&) = delete;

    ~global_time_keeper();

    void start();

    size_t get_channel_index();

    void sync_local_times();

    std::atomic<bool> stop_it{false};
    std::atomic<bool> update_it{false};
    std::atomic_uint64_t client_min_time;
    std::atomic_uint64_t client_max_time;
    std::atomic_uint64_t sc_kernel_time;
    std::mutex upd_mtx;
    std::condition_variable update;
    thread_comms_channel sc_coms_channel{-1ULL};
    std::deque<thread_comms_channel> client_coms_channels;
    rigtorp::SPSCQueue<std::tuple<size_t, uint64_t, callback_task>> pending_tasks{1024};
    bool started = false;
};
} // namespace qk
} // namespace scc
} // namespace tlm
#endif // __SCC_TLM_QK_GLOBAL_TIME_KEEPER_H__
