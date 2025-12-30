#ifndef __SCC_TLM_QK_SC_TIME_SYNCRONIZER_H__
#define __SCC_TLM_QK_SC_TIME_SYNCRONIZER_H__

#include "global_time_keeper.h"
#include <scc/peq.h>

namespace tlm {
namespace scc {
namespace qk {
/**
 *the SystemC time keeper, a singleton in the SystemC thread
 */
struct sc_time_syncronizer : sc_core::sc_stage_callback_if, sc_core::sc_process_host {
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
        auto idx = gtk.get_channel_index();
        sc_task_que.emplace_back();
        blocked_channels.emplace_back(true);
        auto& sctq = sc_task_que[sc_task_que.size() - 1];
        sc_core::sc_spawn(
            [this, &sctq, idx]() {
                wait(sc_core::SC_ZERO_TIME);
                while(true) {
                    // while(!sctq.has_next()) {
                    //     sc_core::wait(sctq.event());
                    // }
                    // this->notify_client_blocked(idx, true);
                    sctq.get()();
                    // this->notify_client_blocked(idx, false);
                }
            },
            sc_core::sc_gen_unique_name("peq_cb", false));

        return idx;
    }
    /**
     * @brief Get the current sc kernel time
     *
     * @return sc_core::sc_time
     */
    inline sc_core::sc_time get_sc_kernel_time() { return sc_core::sc_time::from_value(sc_max_time.load(std::memory_order_seq_cst)); }

    inline void notify_client_blocked(size_t idx, bool is_blocked) {
        assert(idx < blocked_channels.size());
        blocked_channels[idx] = is_blocked;
        sc_is_free_running = !std::any_of(std::begin(blocked_channels), std::end(blocked_channels), [](bool b) { return !b; });
#ifdef DEBUG_MT_SCHEDULING
        SCCTRACEALL(__PRETTY_FUNCTION__) << (is_blocked ? "blocking" : "unblocking") << " thread " << idx
                                         << ", sc_is_free_running=" << sc_is_free_running;
#endif
    }

private:
    sc_time_syncronizer(global_time_keeper& gtk);
    void method_callback();
    void stage_callback(const sc_core::sc_stage& stage) override;
    sc_core::sc_time get_min_time() { return sc_core::sc_time::from_value(gtk.get_min_time_ticks()); }
    global_time_keeper& gtk;
    sc_core::sc_vector<::scc::peq<callback_task>> sc_task_que;
    std::atomic_int64_t sc_max_time{0};
    sc_core::sc_process_handle method_handle;
    std::vector<bool> blocked_channels;
    bool sc_is_free_running{true};
};
} // namespace qk
} // namespace scc
} // namespace tlm
#endif // __SCC_TLM_QK_SC_TIME_SYNCRONIZER_H__
