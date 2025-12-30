#include "sc_time_syncronizer.h"
#include "global_time_keeper.h"
#include <systemc>
#include <tlm_utils/tlm_quantumkeeper.h>

namespace tlm {
namespace scc {
namespace qk {
sc_time_syncronizer::sc_time_syncronizer(global_time_keeper& gtk)
: gtk(gtk) {
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
#ifdef DEBUG_MT_SCHEDULING
        SCCTRACEALL(__PRETTY_FUNCTION__) << "updating pending tasks";
#endif
        while(pending_tasks.size()) {
            auto res = pending_tasks.front();
            auto idx = std::get<0>(*res);
            auto t = std::get<1>(*res);
            auto& peq = sc_task_que[idx];
            if(t > sc_core::sc_time_stamp().value()) {
                auto d = sc_core::sc_time::from_value(t) - sc_core::sc_time_stamp();
#ifdef DEBUG_MT_SCHEDULING
                SCCDEBUG(__PRETTY_FUNCTION__) << "scheduling task from client " << idx << " with t=" << t << " with delay " << d;
#endif
                peq.notify(std::move(std::get<2>(*res)), d);
            } else {
#ifdef DEBUG_MT_SCHEDULING
                SCCDEBUG(__PRETTY_FUNCTION__) << "scheduling task from client " << idx << " with t=" << t << " now";
#endif
                peq.notify(std::move(std::get<2>(*res)), sc_core::SC_ZERO_TIME);
            }
            pending_tasks.pop();
        }
        sc_core::next_trigger(sc_core::SC_ZERO_TIME);
    } else if(sc_core::sc_get_curr_simcontext()->pending_activity_at_current_time()) {
#ifdef DEBUG_MT_SCHEDULING
        SCCTRACEALL(__PRETTY_FUNCTION__) << "yield to next delta cycle";
#endif
        sc_core::next_trigger(sc_core::SC_ZERO_TIME);
    } else {
        auto time_to_next_evt = sc_core::sc_time_to_pending_activity(sc_core::sc_get_curr_simcontext());
        if(!sc_is_free_running) {
            auto min_time = sc_core::sc_time::from_value(gtk.get_max_sc_time_ticks());
            auto abs_time_to_next_evt = sc_core::sc_time_stamp() + time_to_next_evt;
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
#ifdef DEBUG_MT_SCHEDULING
                SCCTRACEALL(__PRETTY_FUNCTION__) << "advancing SC time lockstepped to " << abs_time_to_next_evt;
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
#ifdef DEBUG_MT_SCHEDULING
            SCCTRACEALL(__PRETTY_FUNCTION__) << "advancing SC time free running to " << next_time_point;
#endif
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
#ifdef DEBUG_MT_SCHEDULING
        SCCTRACEALL(__PRETTY_FUNCTION__) << "advancing SystemC kernel time to " << next_time << ", get_min_time()=" << get_min_time();
#endif
    } break;
    case sc_core::SC_POST_END_OF_ELABORATION:
        gtk.start();
        break;
    default:
        break;
    }
}
} // namespace qk
} // namespace scc
} // namespace tlm