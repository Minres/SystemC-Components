#ifndef __SCC_TLM_QK_TYPES_H__
#define __SCC_TLM_QK_TYPES_H__

#include <future>
#include <rigtorp/SPSCQueue.h>
#include <scc/report.h>

// #define DEBUG_MT_SCHEDULING

namespace tlm {
namespace scc {
namespace qk {
#ifdef __cpp_lib_hardware_interference_size
static constexpr size_t kCacheLineSize = std::hardware_destructive_interference_size;
#else
static constexpr size_t kCacheLineSize = 64;
#endif
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
/**
 * @brief the communication channel between client threads (incl. SystemC) and the
 * global time keeper
 *
 */
struct thread_comms_channel {
    /**
     * @brief Construct a new thread comms channel object
     *
     * @param my_id
     */
    thread_comms_channel(uint64_t my_id)
    : client2time_keeper(7)
    , my_id(my_id) {}
    /**
     * @brief Construct a new thread comms channel object
     *
     * @param o
     */
    thread_comms_channel(thread_comms_channel const& o)
    : client2time_keeper(o.client2time_keeper.capacity())
    , my_id(o.my_id)
    , thread_local_time(o.thread_local_time) {
        assert(o.client2time_keeper.size() == 0);
    };

    rigtorp::SPSCQueue<comms_entry> client2time_keeper;

    const uint64_t my_id;
    bool waiting4sc{true};
    uint64_t thread_local_time{0};
};
} // namespace qk
} // namespace scc
} // namespace tlm
#endif // __SCC_TLM_QK_TYPES_H__
