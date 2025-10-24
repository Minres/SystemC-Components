#ifndef __SCC_TLM_QUANTUMKEEPER_H__
#define __SCC_TLM_QUANTUMKEEPER_H__

#include "rigtorp/SPSCQueue.h"
#include "scc/report.h"
#include "sysc/kernel/sc_wait.h"
#include "util/ities.h"
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <future>
#include <limits>
#include <memory>
#include <mutex>
#include <nonstd/optional.hpp>
#include <scc/peq.h>
#include <scc/utilities.h>
#include <sysc/kernel/sc_simcontext.h>
#include <sysc/kernel/sc_spawn.h>
#include <sysc/kernel/sc_spawn_options.h>
#include <sysc/kernel/sc_time.h>
#include <systemc>
#include <thread>
#include <tlm_core/tlm_2/tlm_quantum/tlm_global_quantum.h>
#include <tlm_utils/tlm_quantumkeeper.h>
#include <vector>

namespace tlm {
namespace scc {

// #define LOGTID << "[" << std::this_thread::get_id() << "]"
#define LOGTID

struct quantumkeeper : public tlm_utils::tlm_quantumkeeper {
    using base = tlm_utils::tlm_quantumkeeper;

    quantumkeeper() {}

    virtual ~quantumkeeper() {}

    inline void check_and_sync(sc_core::sc_time core_inc) {
        // devirtualized inc()
        m_local_time += core_inc;
        if(sc_core::sc_time_stamp() + m_local_time >= m_next_sync_point) {
            // devirtualized sync()
            ::sc_core::wait(m_local_time);
            m_local_time = sc_core::SC_ZERO_TIME;
            m_next_sync_point = sc_core::sc_time_stamp() + compute_local_quantum();
        }
    }
    sc_core::sc_time get_local_absolute_time() const { return base::get_current_time(); }
};

using time_queue_t = rigtorp::SPSCQueue<uint64_t>;
//! helper class to hold an array of rigtorp::SPSCQueue with some cache locality
struct time_queue_array {
    enum { granularity = 16 };

    time_queue_array() = default;

    time_queue_array(size_t initial_capacity)
    : initial_capacity(initial_capacity) {}

    ~time_queue_array() {
        for(auto e : array)
            free(e);
    }

    void enlarge() {
        auto idx = sz % granularity;
        if(idx == 0)
            array.push_back(static_cast<time_queue_t*>(malloc(sizeof(time_queue_t) * granularity)));
        auto arr = array.back();
        new(arr + idx) time_queue_t(initial_capacity);
        sz++;
    }

    time_queue_t& operator[](size_t idx) {
        assert(idx < sz);
        auto arr = array[idx / granularity];
        return arr[idx % granularity];
    }

    size_t size() { return sz; }

private:
    const size_t initial_capacity{4};
    std::deque<time_queue_t*> array;
    size_t sz{0};
};

struct global_time_keeper : sc_core::sc_stage_callback_if {
    using callback_task = std::packaged_task<bool(void)>;

    const sc_core::sc_time sc_time_step = 1_ms;

    static global_time_keeper& get() {
        static global_time_keeper keeper;
        return keeper;
    }

    size_t get_index();

    inline void update_time_ticks(size_t idx, uint64_t tick) {
        thread_local_time_queues[idx].push(tick);
        update_it.store(true);
        update.notify_all();
    }

    inline uint64_t get_max_time_ticks() { return window_max_time.load(std::memory_order_relaxed); }

    inline sc_core::sc_time get_max_time() { return sc_core::sc_time::from_value(get_max_time_ticks()); }

    inline uint64_t get_min_time_ticks() { return window_min_time.load(std::memory_order_relaxed); }

    inline sc_core::sc_time get_min_time() { return sc_core::sc_time::from_value(get_min_time_ticks()); }

    inline uint64_t get_sc_kernel_time_ticks() { return sc_kernel_time.load(std::memory_order_relaxed); }

    inline sc_core::sc_time get_sc_kernel_time() { return sc_core::sc_time::from_value(get_sc_kernel_time_ticks()); }

    inline bool execute_on_sysc(size_t idx, std::packaged_task<bool(void)>&& task, sc_core::sc_time when) {
        auto ret = task.get_future();
        pending_tasks.emplace(idx, when, std::move(task));
        update_time_ticks(idx, when.value());
        return ret.get();
    }

private:
    global_time_keeper();

    global_time_keeper(global_time_keeper const&) = delete;

    global_time_keeper(global_time_keeper&&) = delete;

    ~global_time_keeper();
    // runs in a background thread
    void sync_local_times();
    // will be called from SystemC thread
    void stage_callback(const sc_core::sc_stage& stage) override;

    std::atomic<bool> stop_it{false};
    std::atomic<bool> update_it{false};
    std::atomic<uint64_t> window_max_time{0};
    std::atomic<uint64_t> window_min_time{0};
    std::atomic<uint64_t> sc_kernel_time{0};
    std::mutex init_mtx, upd_mtx;
    std::condition_variable update;
    time_queue_array thread_local_time_queues;
    std::vector<uint64_t> local_times;
    rigtorp::SPSCQueue<std::tuple<size_t, sc_core::sc_time, callback_task>> pending_tasks{1024};
    std::vector<std::unique_ptr<::scc::peq<callback_task>>> threaded_tasks;
    std::thread t1;
};

struct quantumkeeper_mt {
    quantumkeeper_mt()
    : gtk_idx(global_time_keeper::get().get_index()) {
        sc_core::sc_spawn_options opt;
        opt.spawn_method();
        opt.set_sensitivity(&keep_alive);
        sc_core::sc_spawn([this]() { this->keep_alive_cb(); }, "keep_alice", &opt);
    }

    virtual ~quantumkeeper_mt() {}

    void keep_alive_cb() {
        keep_alive.cancel();
        auto& gq = tlm::tlm_global_quantum::instance().get();
        if(gq > sc_core::SC_ZERO_TIME)
            keep_alive.notify(gq);
    }

    inline void check_and_sync(sc_core::sc_time core_inc) {
        assert(tid != std::this_thread::get_id());
        local_absolute_time += core_inc;
        if(local_absolute_time > (global_time_keeper::get().get_sc_kernel_time() + tlm::tlm_global_quantum::instance().get())) {
            global_time_keeper::get().update_time_ticks(gtk_idx, local_absolute_time.value());
            // wait until the SystemC thread advanced to the same time then we are minus the global quantum
            while(local_absolute_time > (global_time_keeper::get().get_sc_kernel_time() + tlm::tlm_global_quantum::instance().get())) {
                std::this_thread::yield(); // should do the same than __builtin_ia32_pause() or _mm_pause() on MSVC
            }
        }
    }

    inline bool execute_on_sysc(std::packaged_task<bool(void)>&& task, sc_core::sc_time when) {
        return global_time_keeper::get().execute_on_sysc(gtk_idx, std::move(task), when);
    }

    sc_core::sc_time get_local_time() const {
        assert(tid != std::this_thread::get_id());
        return sc_core::SC_ZERO_TIME;
    }

    sc_core::sc_time get_local_absolute_time() const {
        assert(tid != std::this_thread::get_id());
        return local_absolute_time;
    }

    sc_core::sc_time get_current_sc_time() const { return global_time_keeper::get().get_sc_kernel_time(); }

    void reset() {
        if(tid == std::this_thread::get_id())
            local_absolute_time = global_time_keeper::get().get_sc_kernel_time();
    }

protected:
    size_t gtk_idx;
    sc_core::sc_event keep_alive;
    sc_core::sc_time local_absolute_time;
    const std::thread::id tid{std::this_thread::get_id()};
};
} // namespace scc
} // namespace tlm
#endif
