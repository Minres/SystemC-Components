#ifndef __SCC_TLM_QUANTUMKEEPER_H__
#define __SCC_TLM_QUANTUMKEEPER_H__

#include "rigtorp/SPSCQueue.h"
#include "scc/report.h"
#include "sysc/kernel/sc_wait.h"
#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <deque>
#include <limits>
#include <mutex>
#include <numeric>
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
    sc_core::sc_time get_local_absolute_time() const { return sc_core::sc_time_stamp() + m_local_time; }
};

using time_queue_t = rigtorp::SPSCQueue<uint64_t>;
//! helper class to hold an array of rigtorp::SPSCQueue
struct time_queue_array {
    enum { granularity = 16 };

    void enlarge() {
        auto idx = sz % granularity;
        if(idx == 0)
            array.push_back(static_cast<time_queue_t*>(malloc(sizeof(time_queue_t) * granularity)));
        auto arr = array.back();
        new(arr + idx) time_queue_t(3);
        sz++;
    }

    time_queue_t& operator[](size_t idx) {
        assert(idx < sz);
        auto arr = array[idx / granularity];
        return arr[idx % granularity];
    }

    size_t size() { return sz; }

private:
    std::deque<time_queue_t*> array;
    size_t sz{0};
};

struct global_time_keeper : sc_core::sc_stage_callback_if {

    static global_time_keeper& get() {
        static global_time_keeper keeper;
        return keeper;
    }

    size_t get_index() {
        std::unique_lock<std::mutex> lock{init_mtx};
        thread_local_times.enlarge();
        local_times.emplace_back(std::numeric_limits<uint64_t>::max());
        return local_times.size() - 1;
    }

    void update_time_ticks(size_t idx, uint64_t tick) {
        thread_local_times[idx].push(tick);
        update_it.store(true);
        update.notify_all();
    }

    uint64_t get_max_time_ticks() { return max_time.load(std::memory_order_relaxed); }

    sc_core::sc_time get_max_time() { return sc_core::sc_time::from_value(get_max_time_ticks()); }

    uint64_t get_min_time_ticks() { return min_time.load(std::memory_order_relaxed); }

    sc_core::sc_time get_min_time() { return sc_core::sc_time::from_value(get_min_time_ticks()); }

    uint64_t get_sc_kernel_time_ticks() { return kernel_time_keeper.act_kernel_time.load(std::memory_order_relaxed); }

    sc_core::sc_time get_sc_kernel_time() { return sc_core::sc_time::from_value(get_sc_kernel_time_ticks()); }

    void stop_time_keeper() {
        if(!stop_it.load()) {
            stop_it.store(true);
            update_it.store(true);
            update.notify_all();
            t1.join();
        }
    }

private:
    global_time_keeper()
    : t1{&global_time_keeper::run, this} {
        sc_core::sc_register_stage_callback(*this, sc_core::SC_PRE_TIMESTEP);
        auto idx = get_index(); // assigns idx 0 to the systemc kernel
        assert(idx == 0);
        sc_core::sc_spawn_options opt;
        opt.spawn_method();

        sc_core::sc_spawn(
            []() {
                SCCINFO(sc_core::sc_get_curr_simcontext()->get_curr_proc_info()->process_handle->basename()) << "Triggered method";
                if(sc_core::sc_get_curr_simcontext()->pending_activity_at_current_time())
                    sc_core::next_trigger(sc_core::SC_ZERO_TIME);
                else {
                    sc_core::next_trigger(sc_core::sc_time_to_pending_activity(sc_core::sc_get_curr_simcontext()));
                }
            },
            nullptr, &opt);
    }

    global_time_keeper(global_time_keeper&) = delete;

    global_time_keeper(global_time_keeper&&) = delete;

    void run() {
        while(!stop_it.load(std::memory_order_relaxed)) {
            std::unique_lock<std::mutex> lock{init_mtx};
            update.wait(lock, [this]() -> bool { return update_it.load(std::memory_order_relaxed); });
            bool sc_freerunning = false;
            for(auto i = 1u; i < thread_local_times.size(); ++i) {
                if(auto res = thread_local_times[i].front()) {
                    local_times[i] = *res;
                    thread_local_times[i].pop();
                }
                sc_freerunning &= local_times[i] == std::numeric_limits<uint64_t>::max();
            }
            auto t = sc_freerunning ? min_time.load(std::memory_order_relaxed) + tlm::tlm_global_quantum::instance().get().value()
                                    : *std::min_element(std::begin(local_times), std::end(local_times));
            if(t < std::numeric_limits<uint64_t>::max()) {
                min_time.store(t);
                max_time.store(t + tlm::tlm_global_quantum::instance().get().value());
            }
            update_it.store(false, std::memory_order_acq_rel);
        }
    }

    std::atomic<bool> stop_it{false};
    std::atomic<bool> update_it{false};
    std::atomic<uint64_t> max_time{0};
    std::atomic<uint64_t> min_time{0};
    std::mutex init_mtx, upd_mtx;
    std::condition_variable update;
    time_queue_array thread_local_times;
    std::vector<uint64_t> local_times;
    std::thread t1;
    struct {
        sc_core::sc_time local_absolute_time;
        std::atomic<uint64_t> act_kernel_time{0};
    } kernel_time_keeper;
    // will be called from SystemC thread
    void stage_callback(const sc_core::sc_stage& stage) override {
        sc_core::sc_time next_time;
        if(sc_core::sc_get_curr_simcontext()->next_time(next_time)) {
            kernel_time_keeper.act_kernel_time.store(next_time.value());
            update_time_ticks(0, next_time.value());
            while(get_min_time() < next_time) {
                std::this_thread::sleep_for(std::chrono::microseconds{3});
            }
        }
    }
    std::atomic<uint64_t> sc_time{0};
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
        global_time_keeper::get().update_time_ticks(gtk_idx, local_absolute_time.value());
        // wait until the SystemC thread advanced to the same time then we are
        while(local_absolute_time > global_time_keeper::get().get_sc_kernel_time()) {
            std::this_thread::yield(); // should do the same than __builtin_ia32_pause() or _mm_pause() on MSVC
        }
    }

    sc_core::sc_time get_local_time() const {
        assert(tid != std::this_thread::get_id());
        return sc_core::SC_ZERO_TIME;
    }

    sc_core::sc_time get_local_absolute_time() const {
        assert(tid != std::this_thread::get_id());
        return local_absolute_time;
    }

    sc_core::sc_time get_current_time() const { return global_time_keeper::get().get_sc_kernel_time(); }

    void reset() {
        if(tid == std::this_thread::get_id())
            local_absolute_time = global_time_keeper::get().get_sc_kernel_time();
    }

    bool wait2completion(std::future<bool>&& fu) {
        global_time_keeper::get().update_time_ticks(gtk_idx, local_absolute_time.value());
        global_time_keeper::get().update_time_ticks(gtk_idx, std::numeric_limits<uint64_t>::max());
        auto res = fu.get();
        local_absolute_time = global_time_keeper::get().get_sc_kernel_time();
        return res;
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
