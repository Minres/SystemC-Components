#pragma once

#include <atomic>
#include <future>
#include <scc/report.h>
#include <sysc/communication/sc_prim_channel.h>
#include <sysc/kernel/sc_object.h>
#include <sysc/kernel/sc_simcontext.h>
#include <systemc>
#include <thread>

namespace scc {
struct async_thread : sc_core::sc_prim_channel {

    async_thread()
    : sc_core::sc_prim_channel{sc_core::sc_gen_unique_name("async_thread")} {}

    explicit async_thread(const char* nm)
    : sc_core::sc_prim_channel{nm} {}

    ~async_thread() {
        if(active)
            t1.detach();
    }

    void start(std::function<sc_core::sc_time()> const& f) {
        SCCTRACE(SCOBJ) << "Starting new thread";
        t1 = std::move(std::thread([this, f]() {
            try {
                finish_time.store(f().value());
                async_request_update();
            } catch(std::future_error& e) {
            }
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
        SCCTRACE(SCOBJ) << "Finished execution of thread";
    }
    std::thread t1;
    sc_core::sc_event finish_event;
    std::atomic<uint64_t> finish_time;
    bool active{false};
};
} // namespace scc