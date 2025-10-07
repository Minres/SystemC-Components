#pragma once

#include <sysc/communication/sc_prim_channel.h>
#include <systemc>
#include <thread>

namespace scc {
struct async_thread : sc_core::sc_prim_channel {

    async_thread() {}

    explicit async_thread(const char* nm)
    : sc_core::sc_prim_channel{nm} {}

    void start(std::function<void()> const& f) {
        t1 = std::move(std::thread([this, &f]() {
            f();
            async_request_update();
        }));
    }

    const sc_core::sc_event& thread_finish_event() const { return finish_event; }

private:
    void update() override {
        t1.join();
        finish_event.notify(sc_core::SC_ZERO_TIME);
    }
    std::thread t1;
    sc_core::sc_event finish_event;
};
} // namespace scc