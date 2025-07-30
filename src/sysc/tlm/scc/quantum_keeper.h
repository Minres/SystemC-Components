#ifndef __SCC_TLM_QUANTUMKEEPER_H__
#define __SCC_TLM_QUANTUMKEEPER_H__

#include "sysc/kernel/sc_wait.h"
#include <atomic>
#include <condition_variable>
#include <sysc/kernel/sc_simcontext.h>
#include <sysc/kernel/sc_spawn.h>
#include <sysc/kernel/sc_spawn_options.h>
#include <sysc/kernel/sc_time.h>
#include <systemc>
#include <thread>
#include <tlm_utils/tlm_quantumkeeper.h>

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
};

struct quantumkeeper_mt : /*tlm_utils::tlm_quantumkeeper,*/ sc_core::sc_stage_callback_if {
    quantumkeeper_mt() {
        sc_core::sc_register_stage_callback(*this, /*sc_core::SC_POST_UPDATE |*/ sc_core::SC_PRE_TIMESTEP);
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

    void stage_callback(const sc_core::sc_stage& stage) override {
        sc_core::sc_time next_time;
        auto res = sc_core::sc_get_curr_simcontext()->next_time(next_time);
        if(res) {
            act_kernel_time.store(next_time.value());
            if(next_time + local_absolute_time < tlm::tlm_global_quantum::instance().get()) {
                running.store(true);
                cv.notify_all();
            }
        }
    }
    // TODO: should overload inc and check from base class
    inline void check_and_sync(sc_core::sc_time core_inc) {
        local_absolute_time += core_inc;
        if(tid == std::this_thread::get_id()) {
            if(local_absolute_time >= m_next_sync_point) {
                keep_alive.cancel();
                ::sc_core::wait(local_absolute_time - sc_core::sc_time_stamp());
                local_absolute_time = sc_core::sc_time_stamp();
                m_next_sync_point = local_absolute_time + tlm::tlm_global_quantum::instance().compute_local_quantum();
                keep_alive_cb();
            }
        } else {
            if(local_absolute_time.value() > tlm::tlm_global_quantum::instance().get().value()) {
                running.store(false);
                std::unique_lock<std::mutex> lock(mtx);
                cv.wait_for(lock, timeout, [this] { return running.load(); });
            }
        }
    }

    sc_core::sc_time get_current_time() const { return local_absolute_time; }

    sc_core::sc_time get_local_time() const { return local_absolute_time - sc_core::sc_time_stamp(); }
    void reset() {
        local_absolute_time = sc_core::sc_time_stamp();
        m_next_sync_point = local_absolute_time + tlm::tlm_global_quantum::instance().compute_local_quantum();
    }

protected:
    sc_core::sc_event keep_alive;
    sc_core::sc_time local_absolute_time, m_next_sync_point;
    const std::thread::id tid{std::this_thread::get_id()};
    std::mutex mtx;
    std::condition_variable cv;
    std::chrono::milliseconds timeout{1};
    std::atomic<uint64_t> act_kernel_time{0};
    std::atomic<bool> running{false};
};
} // namespace scc
} // namespace tlm
#endif
