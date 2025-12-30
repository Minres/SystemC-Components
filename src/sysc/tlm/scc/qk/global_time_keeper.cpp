#include "global_time_keeper.h"
#include <tlm_utils/tlm_quantumkeeper.h>

namespace tlm {
namespace scc {
namespace qk {
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
#ifdef DEBUG_MT_SCHEDULING
            SCCTRACEALL("global_time_keeper::sync_local_times") << "update loop";
#endif
            uint64_t min_local_time = std::numeric_limits<uint64_t>::max();
            bool tail = false;
            while(auto res = sc_coms_channel.client2time_keeper.front()) {
                sc_coms_channel.thread_local_time = res->time_tick;
                sc_coms_channel.client2time_keeper.pop();
            }
            for(size_t i = 0; i < client_coms_channels.size(); ++i) {
                auto& client_coms_channel = client_coms_channels[i];
                bool has_task = false;
                bool has_entries = false;
                while(auto res = client_coms_channel.client2time_keeper.front()) {
                    has_entries = true;
                    client_coms_channel.thread_local_time = res->time_tick;
                    if(res->task.valid()) {
#ifdef DEBUG_MT_SCHEDULING
                        SCCTRACEALL("global_time_keeper::sync_local_times")
                            << "forwarding task of client " << client_coms_channel.my_id << " with timestamp t=" << res->time_tick;
#endif
                        pending_tasks.emplace(client_coms_channel.my_id, res->time_tick, std::move(res->task));
                        has_task = true;
                    }
                    client_coms_channel.client2time_keeper.pop();
                }
                if(has_entries)
                    client_coms_channel.waiting4sc = has_task;
                if(!client_coms_channel.waiting4sc)
                    min_local_time = std::min(client_coms_channel.thread_local_time, min_local_time);
#ifdef DEBUG_MT_SCHEDULING
                if(has_entries) {
                    SCCTRACEALL("global_time_keeper::sync_local_times")
                        << "thread_local_time[" << i << "]=" << sc_core::sc_time::from_value(client_coms_channel.thread_local_time)
                        << (client_coms_channel.waiting4sc ? " (waiting)" : " (running)");
                }
#endif
            }
            window_min_time = min_local_time;
            sc_coms_channel.time_keeper2client.store(min_local_time);
            auto window_max_time = min_local_time + std::max(tlm::tlm_global_quantum::instance().get().value(), sc_time_step.value());
            for(auto& client_coms_channel : client_coms_channels) {
                client_coms_channel.time_keeper2client.store(window_max_time);
            }
#ifdef DEBUG_MT_SCHEDULING
            SCCTRACEALL("global_time_keeper::sync_local_times") << "window_min_time=" << window_min_time;
#endif
        } else if(stop_it.load()) {
            break;
        }
    }
}

} // namespace qk
} // namespace scc
} // namespace tlm