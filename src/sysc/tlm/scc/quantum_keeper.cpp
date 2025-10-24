#include "quantum_keeper.h"

namespace tlm {
namespace scc {
global_time_keeper::global_time_keeper()
: t1{&global_time_keeper::sync_local_times, this} {
    t1.detach();
    sc_core::sc_register_stage_callback(*this, sc_core::SC_PRE_TIMESTEP);
    auto idx = get_index(); // assigns idx 0 to the systemc kernel
    assert(idx == 0);
    sc_core::sc_spawn_options opt;
    opt.spawn_method();
    // the lambda runs in SystemC thread
    sc_core::sc_spawn(
        [this]() {
            while(pending_tasks.size()) {
                auto res = pending_tasks.front();
                auto& peq = threaded_tasks[std::get<0>(*res)];
                auto t = std::get<1>(*res);
                if(t.value())
                    peq->notify(std::move(std::get<2>(*res)), t);
                else
                    peq->notify(std::move(std::get<2>(*res)));
            }
            if(sc_core::sc_get_curr_simcontext()->pending_activity_at_current_time())
                sc_core::next_trigger(sc_core::SC_ZERO_TIME);
            else {
                auto t = sc_core::sc_time_to_pending_activity(sc_core::sc_get_curr_simcontext());
                auto min_time = get_min_time();
                auto delay = std::min(t, min_time - sc_core::sc_time_stamp());
                if(!delay.value()) // slow down if there is nothing to do
                    std::this_thread::sleep_for(std::chrono::microseconds(10));
                sc_core::next_trigger(delay);
            }
        },
        nullptr, &opt);
}

global_time_keeper::~global_time_keeper() {
    stop_it.store(true, std::memory_order_acq_rel);
    update.notify_all();
}

size_t global_time_keeper::get_index() {
    std::unique_lock<std::mutex> lock{init_mtx};
    thread_local_time_queues.enlarge();
    local_times.emplace_back(0); // std::numeric_limits<uint64_t>::max());
    threaded_tasks.emplace_back(std::move(util::make_unique<::scc::peq<callback_task>>()));
    auto idx = local_times.size() - 1;
    // the lambda will run in SystemC thread
    sc_core::sc_spawn([this, idx]() {
        wait(sc_core::SC_ZERO_TIME);
        while(true) {
            auto t = std::move(threaded_tasks[idx]->get());
            t();
        }
    });
    return idx;
}

// runs in a background thread
void global_time_keeper::sync_local_times() {
    std::unique_lock<std::mutex> lock{upd_mtx};
    while(true) {
        update.wait_for(lock, std::chrono::milliseconds(1),
                        [this]() -> bool { return update_it.load(std::memory_order_relaxed) || stop_it.load(std::memory_order_relaxed); });
        if(stop_it.load(std::memory_order_relaxed))
            break;
        bool sc_freerunning = true;
        uint64_t min_local_time = std::numeric_limits<uint64_t>::max();
        for(auto i = 0u; i < thread_local_time_queues.size(); ++i) {
            if(auto res = thread_local_time_queues[i].front()) {
                local_times[i] = *res;
                thread_local_time_queues[i].pop();
            }
            if(i) {
                sc_freerunning &= local_times[i] == std::numeric_limits<uint64_t>::max();
                min_local_time = std::min(local_times[i], min_local_time);
            }
        }
        auto t = sc_freerunning ? window_min_time.load(std::memory_order_relaxed) +
                                      std::max(tlm::tlm_global_quantum::instance().get().value(), sc_time_step.value())
                                : min_local_time;
        if(t < std::numeric_limits<uint64_t>::max()) {
            window_min_time.store(t);
            window_max_time.store(t + tlm::tlm_global_quantum::instance().get().value());
        }
        update_it.store(false, std::memory_order_acq_rel);
    }
}

// will be called from SystemC thread
void global_time_keeper::stage_callback(const sc_core::sc_stage& stage) {
    sc_core::sc_time next_time;
    if(sc_core::sc_get_curr_simcontext()->next_time(next_time)) {
        SCCTRACEALL("global_time_keeper::stage_callback") << "updating sc_kernel_time " << next_time;
        sc_kernel_time.store(next_time.value());
        update_time_ticks(0, next_time.value());
        while(get_min_time() < next_time) { // slow down systemc execution to be the slowest
            std::this_thread::sleep_for(std::chrono::microseconds{1});
        }
    }
    SCCTRACEALL("global_time_keeper::stage_callback") << "Advancing SystemC kernel time to " << next_time;
}

} // namespace scc
} // namespace tlm