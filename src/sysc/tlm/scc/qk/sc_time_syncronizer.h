#ifndef __SCC_TLM_QK_SC_TIME_SYNCRONIZER_H__
#define __SCC_TLM_QK_SC_TIME_SYNCRONIZER_H__

#include "global_time_keeper.h"
#include <scc/peq.h>

namespace tlm {
namespace scc {
namespace qk {
struct client_deputy {
    ::scc::peq<callback_task> peq;
    bool blocked;
    client_deputy(char const* nm)
    : peq{nm}
    , blocked(false) {}
};

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
        client_deputies.emplace_back();
        auto& client = client_deputies[client_deputies.size() - 1];
        sc_core::sc_spawn(
            [this, &client, idx]() {
                while(true) {
                    client.peq.get()();
                    if(!client.peq.has_next())
                        this->notify_client_blocked(idx, false);
                }
            },
            sc_core::sc_gen_unique_name("peq_cb", false));

        return idx;
    }

    inline void notify_client_blocked(size_t idx, bool is_blocked) {
        assert(idx < client_deputies.size());
        client_deputies[idx].blocked = is_blocked;
        sc_is_free_running =
            !std::any_of(std::begin(client_deputies), std::end(client_deputies), [](client_deputy const& d) { return !d.blocked; });
#ifdef DEBUG_MT_SCHEDULING
        SCCTRACEALL(__PRETTY_FUNCTION__) << (is_blocked ? "blocking" : "unblocking") << " thread " << idx
                                         << ", sc_is_free_running=" << sc_is_free_running;
#endif
    }

private:
    enum class sync_state { INITIAL, PROCESSING, ARMED };
    sc_time_syncronizer(global_time_keeper& gtk);
    void sc_method();
    bool process_pending_tasks();
    sync_state state{sync_state::INITIAL};
    void stage_callback(const sc_core::sc_stage& stage) override;
    inline void notify_task(size_t idx, callback_task&& task, sc_core::sc_time const& d) {
        client_deputies[idx].peq.notify(std::move(task), d);
        notify_client_blocked(idx, true);
    }
    sc_core::sc_time get_min_time() { return sc_core::sc_time::from_value(gtk.get_client_min_time_ticks()); }

    global_time_keeper& gtk;
    sc_core::sc_vector<client_deputy> client_deputies;
    sc_core::sc_process_handle method_handle;
    bool sc_is_free_running{true};
};
} // namespace qk
} // namespace scc
} // namespace tlm
#endif // __SCC_TLM_QK_SC_TIME_SYNCRONIZER_H__
