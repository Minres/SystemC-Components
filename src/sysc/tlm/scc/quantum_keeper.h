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
struct thread_comms_channel {
    thread_comms_channel(uint64_t my_id, size_t capacity)
    : my_id(my_id)
    , client2time_keeper(capacity)
    , time_keeper2client(capacity) {}
    thread_comms_channel(thread_comms_channel const& o)
    : my_id(o.my_id)
    , client2time_keeper(o.client2time_keeper.capacity())
    , time_keeper2client(o.time_keeper2client.capacity())
    , thread_local_time(o.thread_local_time)
    , thread_blocked(o.thread_blocked) {
        assert(o.client2time_keeper.size() == 0);
        assert(o.time_keeper2client.size() == 0);
    };

    rigtorp::SPSCQueue<comms_entry> client2time_keeper;
    rigtorp::SPSCQueue<uint64_t> time_keeper2client;

    const uint64_t my_id;
    uint64_t thread_local_time;
    bool thread_blocked;
};
///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////
struct sc_time_syncronizer;
struct global_time_keeper {
    friend class sc_time_syncronizer;

    const sc_core::sc_time sc_time_step = 1_ms;

    static global_time_keeper& get() {
        static global_time_keeper keeper;
        return keeper;
    }

    inline void update_sc_time_ticks(uint64_t tick) {
        sc_coms_channel.client2time_keeper.push(std::move(comms_entry{tick, std::move(callback_task())}));
#ifdef DEBUG_MT_SCHDULING
        SCCTRACEALL("global_time_keeper::update_sc_time_ticks") << "sc_time=" << sc_core::sc_time::from_value(tick);
#endif
        update_it.store(true);
        update.notify_all();
    }

    inline uint64_t get_sc_max_time_ticks() {
        uint64_t res = -1;
        while(auto e = sc_coms_channel.time_keeper2client.front()) {
            res = *e;
            sc_coms_channel.time_keeper2client.pop();
        }
        return res;
    }

    inline void update_time_ticks(size_t idx, uint64_t tick) {
#ifdef DEBUG_MT_SCHDULING
        SCCTRACEALL("global_time_keeper::update_time_ticks")
            << "idx=" << idx << ", thread_local_time=" << sc_core::sc_time::from_value(tick);
#endif
        client_coms_channels[idx].client2time_keeper.push(std::move(comms_entry{tick, std::move(callback_task())}));
        update_it.store(true);
        update.notify_all();
    }

    inline void update_time_ticks(size_t idx, uint64_t tick, std::packaged_task<sc_core::sc_time(void)>&& task) {
        client_coms_channels[idx].client2time_keeper.push(std::move(comms_entry{tick, std::move(task)}));
#ifdef DEBUG_MT_SCHDULING
        SCCTRACEALL("global_time_keeper::update_time_ticks(task)")
            << "idx=" << idx << ", thread_local_time=" << sc_core::sc_time::from_value(tick);
#endif
        update_it.store(true);
        update.notify_all();
    }

    inline uint64_t get_max_time_ticks(size_t idx) {
        uint64_t res = -1;
        while(auto e = client_coms_channels[idx].time_keeper2client.front()) {
            res = *e;
            client_coms_channels[idx].time_keeper2client.pop();
        }
        return res;
    }

    inline uint64_t get_window_min_time() { return window_min_time; }

    inline rigtorp::SPSCQueue<std::tuple<size_t, uint64_t, callback_task>>& get_pending_tasks_que() { return pending_tasks; }

protected:
    global_time_keeper();

    global_time_keeper(global_time_keeper const&) = delete;

    global_time_keeper(global_time_keeper&&) = delete;

    ~global_time_keeper();

    size_t get_channel_index();

    // runs in a background thread
    void sync_local_times();
    // will be called from SystemC thread

    std::atomic<bool> stop_it{false};
    std::atomic<bool> update_it{false};
    std::atomic_bool all_threads_blocked{false};
    uint64_t window_min_time;
    // std::atomic<uint64_t> sc_kernel_time{0};
    // std::atomic<uint64_t> sc_kernel_allowed_time{0};
    std::mutex init_mtx, upd_mtx;
    std::condition_variable update;
    thread_comms_channel sc_coms_channel{0, 16};
    std::deque<thread_comms_channel> client_coms_channels;
    rigtorp::SPSCQueue<std::tuple<size_t, uint64_t, callback_task>> pending_tasks{1024};
    std::thread t1;
};
///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////
struct sc_time_syncronizer : sc_core::sc_object, sc_core::sc_stage_callback_if, sc_core::sc_process_host {
    static sc_time_syncronizer& get() {
        static sc_time_syncronizer keeper(global_time_keeper::get());
        return keeper;
    }

    size_t get_channel_index() {
        auto res = gtk.get_channel_index();
        sc_task_que.emplace_back();
        auto& sctq = sc_task_que[sc_task_que.size() - 1];
        sc_core::sc_spawn(
            [this, &sctq]() {
                wait(sc_core::SC_ZERO_TIME);
                while(true) {
                    // auto t = std::move(sctq.get());
                    // t();
                    sctq.get()();
                }
            },
            sc_core::sc_gen_unique_name("peq_cb", false));

        return res;
    }

    inline sc_core::sc_time get_sc_kernel_time() { return sc_core::sc_time::from_value(sc_max_time.load(std::memory_order_seq_cst)); }

private:
    sc_time_syncronizer(global_time_keeper& gtk);
    void method_callback();
    void stage_callback(const sc_core::sc_stage& stage) override;
    sc_core::sc_time get_min_time() { return sc_core::sc_time::from_value(gtk.get_window_min_time()); }
    global_time_keeper& gtk;
    sc_core::sc_vector<::scc::peq<callback_task>> sc_task_que;
    std::atomic_int64_t sc_max_time{0};
    sc_core::sc_process_handle method_handle;
};
///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////
struct quantumkeeper_mt {
    quantumkeeper_mt()
    : gtk_idx(sc_time_syncronizer::get().get_channel_index()) {
        sc_core::sc_spawn_options opt;
        opt.spawn_method();
        opt.set_sensitivity(&keep_alive);
    }

    virtual ~quantumkeeper_mt() {}

    inline void inc(sc_core::sc_time core_inc) {
        local_absolute_time += core_inc;
        auto res = global_time_keeper::get().get_max_time_ticks(gtk_idx);
        if(res != std::numeric_limits<uint64_t>::max())
            local_time_ticks_limit = res;
    }

    inline void check_and_sync(sc_core::sc_time core_inc) {
        assert(tid != std::this_thread::get_id());
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

    inline void execute_on_sysc(std::function<sc_core::sc_time(void)> fct) { execute_on_sysc(fct, local_absolute_time); }

    inline void execute_on_sysc(std::function<sc_core::sc_time(void)> fct, sc_core::sc_time when) {
        callback_task task(fct);
        auto ret = task.get_future();
        global_time_keeper::get().update_time_ticks(gtk_idx, when.value(), std::move(task));
        auto duration = ret.get();
        if(duration.value())
            check_and_sync(duration);
    }

    sc_core::sc_time get_local_time() const {
        assert(tid != std::this_thread::get_id());
        return local_absolute_time - get_current_sc_time();
    }

    sc_core::sc_time get_local_absolute_time() const {
        assert(tid != std::this_thread::get_id());
        return local_absolute_time;
    }

    sc_core::sc_time get_current_sc_time() const { return sc_time_syncronizer::get().get_sc_kernel_time(); }

    void reset() {
        if(tid == std::this_thread::get_id())
            local_absolute_time = get_current_sc_time();
    }

protected:
    size_t gtk_idx;
    sc_core::sc_event keep_alive;
    sc_core::sc_time local_absolute_time;
    uint64_t local_time_ticks_limit{0};
    const std::thread::id tid{std::this_thread::get_id()};
};
} // namespace scc
} // namespace tlm
#endif
