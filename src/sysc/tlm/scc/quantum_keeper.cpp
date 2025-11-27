#include "quantum_keeper.h"
#include <atomic>
#include <cstdint>
#include <limits>
#include <scc/report.h>
#include <stdexcept>
#include <sysc/kernel/sc_object.h>
#include <sysc/kernel/sc_simcontext.h>
#include <sysc/kernel/sc_stage_callback_if.h>
#include <sysc/kernel/sc_time.h>
#include <thread>
#include <tlm_utils/tlm_quantumkeeper.h>

namespace tlm {
namespace scc {
///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////
global_time_keeper::global_time_keeper() = default;

global_time_keeper::~global_time_keeper() {
    // shutting down time keeper thread
    stop_it.store(true, std::memory_order_acq_rel);
    update.notify_all();
}
// this function will be called from the systemc thread
void global_time_keeper::start() {
    std::thread t{&global_time_keeper::sync_local_times, this};
    t.detach();
    started = true;
}
// this function will be called from the systemc thread
size_t global_time_keeper::get_channel_index() {
    if(started)
        throw std::runtime_error("global_time_keeper already started");
    client_coms_channels.emplace_back(client_coms_channels.size());
    update_it = true;
    return client_coms_channels.size() - 1;
}

// runs in a background thread
void global_time_keeper::sync_local_times() {
    std::unique_lock<std::mutex> lock{upd_mtx};
    while(true) {
        update.wait_for(lock, std::chrono::milliseconds(1),
                        [this]() -> bool { return update_it.load(std::memory_order_relaxed) || stop_it.load(std::memory_order_relaxed); });
        if(update_it.exchange(false)) {
#ifdef DEBUG_MT_SCHDULING
            SCCTRACEALL("global_time_keeper::sync_local_times") << "update loop";
#endif
            uint64_t min_local_time = std::numeric_limits<uint64_t>::max();
            bool tail = false;
            while(auto res = sc_coms_channel.client2time_keeper.front()) {
                sc_coms_channel.thread_local_time = res->time_tick;
                sc_coms_channel.client2time_keeper.pop();
            }
            for(auto& client_coms_channel : client_coms_channels) {
                while(auto res = client_coms_channel.client2time_keeper.front()) {
                    client_coms_channel.thread_local_time = res->time_tick;
                    if(res->task.valid()) {
                        pending_tasks.emplace(client_coms_channel.my_id, res->time_tick, std::move(res->task));
                    }
                    client_coms_channel.client2time_keeper.pop();
                }
                min_local_time = std::min(client_coms_channel.thread_local_time, min_local_time);
#ifdef DEBUG_MT_SCHDULING
                SCCTRACEALL("global_time_keeper::sync_local_times")
                    << "thread_local_time=" << sc_core::sc_time::from_value(client_coms_channel.thread_local_time);
#endif
            }
            this->all_threads_blocked.store(all_threads_blocked);
            window_min_time = min_local_time;
            sc_coms_channel.time_keeper2client.store(min_local_time);
            auto window_max_time = min_local_time + std::max(tlm::tlm_global_quantum::instance().get().value(), sc_time_step.value());
            for(auto& client_coms_channel : client_coms_channels) {
                client_coms_channel.time_keeper2client.store(window_max_time);
            }
#ifdef DEBUG_MT_SCHDULING
            SCCTRACEALL("global_time_keeper::sync_local_times")
                << "sc_freerunning=" << all_threads_blocked << ", window_min_time=" << window_min_time;
#endif
        } else if(stop_it.load()) {
            break;
        }
    }
}
///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////
sc_time_syncronizer::sc_time_syncronizer(global_time_keeper& gtk)
: sc_core::sc_object("sc_time_syncronizer")
, gtk(gtk) {
    sc_core::sc_register_stage_callback(*this, sc_core::SC_PRE_TIMESTEP | sc_core::SC_POST_END_OF_ELABORATION);
    sc_core::sc_spawn_options opt;
    opt.spawn_method();
    sc_core::sc_spawn([this]() { method_callback(); }, nullptr, &opt);
}

void sc_time_syncronizer::method_callback() {
    auto res = this->gtk.get_max_sc_time_ticks();
    if(res != std::numeric_limits<uint64_t>::max()) {
        sc_max_time.store(res, std::memory_order_seq_cst);
    }
    auto& pending_tasks = this->gtk.pending_tasks;
    if(pending_tasks.size()) {
#ifdef DEBUG_MT_SCHDULING
        SCCTRACEALL(__PRETTY_FUNCTION__) << "updating pending tasks";
#endif
        while(pending_tasks.size()) {
            auto res = pending_tasks.front();
            auto& peq = sc_task_que[std::get<0>(*res)];
            auto t = std::get<1>(*res);
#ifdef DEBUG_MT_SCHDULING
            SCCDEBUG(__PRETTY_FUNCTION__) << "scheduling task at " << t;
#endif
            if(t > sc_core::sc_time_stamp().value())
                peq.notify(std::move(std::get<2>(*res)), sc_core::sc_time::from_value(t) - sc_core::sc_time_stamp());
            else
                peq.notify(std::move(std::get<2>(*res)));
#ifdef DEBUG_MT_SCHDULING
            SCCTRACEALL(__PRETTY_FUNCTION__) << "setting thread_blocked[" << std::get<0>(*res) << "]=1";
#endif
            pending_tasks.pop();
        }
        sc_core::next_trigger(sc_core::SC_ZERO_TIME);
    } else if(sc_core::sc_get_curr_simcontext()->pending_activity_at_current_time()) {
#ifdef DEBUG_MT_SCHDULING
        SCCTRACEALL(__PRETTY_FUNCTION__) << "yield to next delta cycle";
#endif
        sc_core::next_trigger(sc_core::SC_ZERO_TIME);
    } else {
        auto time_to_next_evt = sc_core::sc_time_to_pending_activity(sc_core::sc_get_curr_simcontext());
        auto min_time = get_min_time();
        if(!sc_is_free_running) {
            auto abs_time_to_next_evt = time_to_next_evt + sc_core::sc_time_stamp();
            if(min_time < abs_time_to_next_evt) {
                if(min_time > sc_core::sc_time_stamp()) {
                    sc_core::next_trigger(min_time - sc_core::sc_time_stamp());
                } else {
                    // slow down systemc execution to be the slower than client threads
                    std::this_thread::yield();
                    // std::this_thread::sleep_for(std::chrono::microseconds{1});
                    sc_core::next_trigger(sc_core::SC_ZERO_TIME); // play it again, Sam
                }
            } else {
#ifdef DEBUG_MT_SCHDULING
                SCCTRACEALL(__PRETTY_FUNCTION__) << "advancing SC time lockstepped to " << time_to_next_evt;
#endif
                sc_core::next_trigger(time_to_next_evt);
            }
        } else {
            // all threads are blocked by SystemC, so we can run freely but to avoid starving the kernel we only proceed to the
            // time of the slowest client thread as this one is waiting to be served
            auto next_time_point = sc_core::sc_time_stamp() + time_to_next_evt;
            if(next_time_point.value() == std::numeric_limits<uint64_t>::max()) {
                time_to_next_evt = tlm_utils::tlm_quantumkeeper::get_global_quantum();
                if(time_to_next_evt == sc_core::SC_ZERO_TIME)
                    time_to_next_evt = 1_us;
            }
            sc_core::next_trigger(time_to_next_evt);
        }
    }
}

void sc_time_syncronizer::stage_callback(const sc_core::sc_stage& stage) {
    switch(stage) {
    case sc_core::SC_PRE_TIMESTEP: {
        sc_core::sc_time next_time;
        if(sc_core::sc_get_curr_simcontext()->next_time(next_time)) {
            gtk.update_sc_time_ticks(next_time.value());
        }
#ifdef DEBUG_MT_SCHDULING
        SCCTRACEALL(SCMOD) << "advancing SystemC kernel time to " << next_time << ", get_min_time()=" << get_min_time();
#endif
    } break;
    case sc_core::SC_POST_END_OF_ELABORATION:
        gtk.start();
        break;
    default:
        break;
    }
}

} // namespace scc
} // namespace tlm