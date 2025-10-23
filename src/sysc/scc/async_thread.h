#pragma once

#include <atomic>
#include <scc/report.h>
#include <sysc/communication/sc_prim_channel.h>
#include <sysc/kernel/sc_simcontext.h>
#include <systemc>
#include <thread>

namespace scc {
struct async_thread : sc_core::sc_prim_channel {

    async_thread() = default;

    explicit async_thread(const char* nm)
    : sc_core::sc_prim_channel{nm} {}

    ~async_thread() {
        if(active)
            t1.detach();
    }

    void start(std::function<sc_core::sc_time()> const& f) {
        SCCTRACE(__PRETTY_FUNCTION__) << "Starting new thread";
        t1 = std::move(std::thread([this, f]() {
            finish_time.store(f().value(), std::memory_order_acq_rel);
            async_request_update();
        }));
        active = true;
    }

    const sc_core::sc_event& thread_finish_event() const { return finish_event; }

private:
    void update() override {
        t1.join();
        auto end_time = sc_core::sc_time::from_value(finish_time.load());
        finish_event.notify(end_time > sc_core::sc_time_stamp() ? end_time - sc_core::sc_time_stamp() : sc_core::SC_ZERO_TIME);
        active = false;
        SCCTRACE(__PRETTY_FUNCTION__) << "Finished execution of thread";
    }
    std::thread t1;
    sc_core::sc_event finish_event;
    std::atomic<uint64_t> finish_time;
    bool active{false};
};
} // namespace scc