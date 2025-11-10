#ifndef __SCC_TLM_QUANTUMKEEPER_H__
#define __SCC_TLM_QUANTUMKEEPER_H__

#include "rigtorp/SPSCQueue.h"
#include "sysc/kernel/sc_wait.h"
#include <atomic>
#include <condition_variable>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <future>
#include <limits>
#include <mutex>
#include <nonstd/optional.hpp>
#include <scc/peq.h>
#include <scc/report.h>
#include <scc/utilities.h>
#include <sysc/kernel/sc_object.h>
#include <sysc/kernel/sc_process.h>
#include <sysc/kernel/sc_simcontext.h>
#include <sysc/kernel/sc_spawn.h>
#include <sysc/kernel/sc_spawn_options.h>
#include <sysc/kernel/sc_time.h>
#include <systemc>
#include <thread>
#include <tlm_core/tlm_2/tlm_quantum/tlm_global_quantum.h>
#include <tlm_utils/tlm_quantumkeeper.h>

#if SC_VERSION_MAJOR < 3
#error "Multithreaded quantum keeper is only supported with SystemC 3.0 and newer"
#else
// #define DEBUG_MT_SCHDULING

namespace tlm {
namespace scc {
#ifdef __cpp_lib_hardware_interference_size
static constexpr size_t kCacheLineSize = std::hardware_destructive_interference_size;
#else
static constexpr size_t kCacheLineSize = 64;
#endif
///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////
struct quantumkeeper : public tlm_utils::tlm_quantumkeeper {
    using base = tlm_utils::tlm_quantumkeeper;

    quantumkeeper() {}

    virtual ~quantumkeeper() {}

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
};
///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////
using callback_fct = sc_core::sc_time(void);
using callback_task = std::packaged_task<callback_fct>;

struct comms_entry {
    uint64_t time_tick;
    callback_task task;
};
///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////
/**
 * @brief the communication channel between client threads (incl. SystemC) and the
 * global time keeper
 *
 */
struct thread_comms_channel {
    /**
     * @brief Construct a new thread comms channel object
     *
     * @param my_id
     */
    thread_comms_channel(uint64_t my_id)
    : my_id(my_id)
    , client2time_keeper(7)
    , time_keeper2client(127) {}
    /**
     * @brief Construct a new thread comms channel object
     *
     * @param o
     */
    thread_comms_channel(thread_comms_channel const& o)
    : my_id(o.my_id)
    , client2time_keeper(o.client2time_keeper.capacity())
    , time_keeper2client(o.time_keeper2client.capacity())
    , thread_local_time(o.thread_local_time)
    , thread_blocked(o.thread_blocked.load()) {
        assert(o.client2time_keeper.size() == 0);
        assert(o.time_keeper2client.size() == 0);
    };

    rigtorp::SPSCQueue<comms_entry> client2time_keeper;
    rigtorp::SPSCQueue<uint64_t> time_keeper2client;

    const uint64_t my_id;
    uint64_t thread_local_time;
    std::atomic_bool thread_blocked;
};
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
    inline uint64_t get_min_time_ticks() { return window_min_time; }
    /**
     * @brief Get the maximum time ticks a client thread is allowed to advance
     *
     * @param idx the id of the client thread, to be obtained using get_channel_index()
     * @return uint64_t the absolute number of maximum ticks, if no new information it returns -1
     */
    inline uint64_t get_max_time_ticks(size_t idx) {
        uint64_t res = -1;
        while(auto e = client_coms_channels[idx].time_keeper2client.front()) {
            res = *e;
            client_coms_channels[idx].time_keeper2client.pop();
        }
        return res;
    }
    /**
     * @brief updates the global time keeper with the local time ticks of a clinet thread
     *
     * @param idx the id of the client thread, to be obtained using get_channel_index()
     * @param tick the absolute number of actual ticks
     */
    inline void update_time_ticks(size_t idx, uint64_t tick) {
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
    inline void update_time_ticks(size_t idx, uint64_t tick, std::packaged_task<sc_core::sc_time(void)>&& task) {
        client_coms_channels[idx].client2time_keeper.push(std::move(comms_entry{tick, std::move(task)}));
        update_it.store(true);
        std::unique_lock<std::mutex> lk(upd_mtx);
        update.notify_all();
    }
    /**
     * @brief Get the maximum sc time ticks a client thread is allowed to advance
     *
     * @return uint64_t  the absolute number of ticks
     */
    inline uint64_t get_max_sc_time_ticks() {
        uint64_t res = -1;
        while(auto e = sc_coms_channel.time_keeper2client.front()) {
            res = *e;
            sc_coms_channel.time_keeper2client.pop();
        }
        return res;
    }
    /**
     * @brief updates the global time keeper with the local time ticks of the SystemC thread
     *
     * @param tick the absolute number of actual SystemC ticks
     */
    inline void update_sc_time_ticks(uint64_t tick) {
        sc_coms_channel.client2time_keeper.push(std::move(comms_entry{tick, std::move(callback_task())}));
#ifdef DEBUG_MT_SCHDULING
        SCCTRACEALL("global_time_keeper::update_sc_time_ticks") << "sc_time=" << sc_core::sc_time::from_value(tick);
#endif
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
    std::atomic_bool all_threads_blocked{false};
    std::atomic_uint64_t window_min_time;
    std::mutex upd_mtx;
    std::condition_variable update;
    thread_comms_channel sc_coms_channel{0};
    std::deque<thread_comms_channel> client_coms_channels;
    rigtorp::SPSCQueue<std::tuple<size_t, uint64_t, callback_task>> pending_tasks{1024};
    bool started = false;
};
///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////
/**
 *the SystemC time keeper, a singleton in the SystemC thread
 */
struct sc_time_syncronizer : sc_core::sc_object, sc_core::sc_stage_callback_if, sc_core::sc_process_host {
    ~sc_time_syncronizer() = default;
    /**
     * @brief the singleton getter
     *
     * @return sc_time_syncronizer&
     */
    static sc_time_syncronizer& get() {
        static sc_time_syncronizer keeper(global_time_keeper::get());
        return keeper;
    }
    /**
     * @brief Get the channel index
     *
     * @details it creates all needed infrastructure to handle the data exchange between client threads and the SystemC thread
     *
     * @return size_t the index
     */
    size_t get_channel_index() {
        auto res = gtk.get_channel_index();
        sc_task_que.emplace_back();
        auto& sctq = sc_task_que[sc_task_que.size() - 1];
        sc_core::sc_spawn(
            [this, &sctq]() {
                wait(sc_core::SC_ZERO_TIME);
                while(true) {
                    sctq.get()();
                }
            },
            sc_core::sc_gen_unique_name("peq_cb", false));

        return res;
    }
    /**
     * @brief Get the current sc kernel time
     *
     * @return sc_core::sc_time
     */
    inline sc_core::sc_time get_sc_kernel_time() { return sc_core::sc_time::from_value(sc_max_time.load(std::memory_order_seq_cst)); }

private:
    sc_time_syncronizer(global_time_keeper& gtk);
    void method_callback();
    void stage_callback(const sc_core::sc_stage& stage) override;
    sc_core::sc_time get_min_time() { return sc_core::sc_time::from_value(gtk.get_min_time_ticks()); }
    global_time_keeper& gtk;
    sc_core::sc_vector<::scc::peq<callback_task>> sc_task_que;
    std::atomic_int64_t sc_max_time{0};
    sc_core::sc_process_handle method_handle;
};
///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////
/**
 * the multi threading quantum keeper
 */
struct quantumkeeper_mt {
    /**
     * @brief Construct a new quantumkeeper mt object
     *
     */
    quantumkeeper_mt()
    : gtk_idx(sc_time_syncronizer::get().get_channel_index()) {
        sc_core::sc_spawn_options opt;
        opt.spawn_method();
        opt.set_sensitivity(&keep_alive);
    }
    /**
     * @brief Destroy the quantumkeeper mt object
     *
     */
    virtual ~quantumkeeper_mt() = default;
    /**
     * @brief increments the local time and updates the global time keeper with the new time
     *
     * @param core_inc the amount to increment the local time
     */
    inline void inc(sc_core::sc_time core_inc) {
        local_absolute_time += core_inc;
        auto res = global_time_keeper::get().get_max_time_ticks(gtk_idx);
        if(res != std::numeric_limits<uint64_t>::max())
            local_time_ticks_limit = res;
    }
    /**
     * @brief checks the local time against the global time keeper and holds the thread until it is within the quantum
     *
     * @param core_inc the amount to increment the local time
     */
    inline void check_and_sync(sc_core::sc_time core_inc) {
        inc(core_inc);
        global_time_keeper::get().update_time_ticks(gtk_idx, local_absolute_time.value());
        // wait until the SystemC thread advanced to the same time then we are minus the global quantum
        while(local_absolute_time.value() > local_time_ticks_limit) {
            std::this_thread::yield(); // should do the same than __builtin_ia32_pause() or _mm_pause() on MSVC
            auto res = global_time_keeper::get().get_max_time_ticks(gtk_idx);
            if(res != std::numeric_limits<uint64_t>::max())
                local_time_ticks_limit = res;
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
        callback_task task([&fct]() {
            auto t0 = sc_core::sc_time_stamp();
            fct();
            return t0 - sc_core::sc_time_stamp();
        });
        auto ret = task.get_future();
        global_time_keeper::get().update_time_ticks(gtk_idx, when.value(), std::move(task));
        auto duration = ret.get();
        if(duration.value())
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
    sc_core::sc_time get_current_sc_time() const { return sc_time_syncronizer::get().get_sc_kernel_time(); }
    /**
     * @brief resets the local time of this thread to the SystemC kernel time
     *
     */
    void reset() { local_absolute_time = get_current_sc_time(); }

protected:
    size_t gtk_idx;
    sc_core::sc_event keep_alive;
    sc_core::sc_time local_absolute_time;
    uint64_t local_time_ticks_limit{0};
};
} // namespace scc
} // namespace tlm
#endif
#endif
