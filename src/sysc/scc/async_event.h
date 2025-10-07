#pragma once
#include <atomic>
#include <sysc/communication/sc_prim_channel.h>
#include <sysc/kernel/sc_time.h>
#include <systemc>
#include <thread>

namespace scc {
struct async_event : sc_core::sc_prim_channel {

    async_event() = default;

    async_event(char const* nm)
    : sc_core::sc_prim_channel{nm} {}

    void notify() {
        if(std::this_thread::get_id() == id)
            ev_.notify(sc_core::SC_ZERO_TIME);
        else {
            pending_update.store(true, std::memory_order_release);
            async_request_update();
        }
    }

    void update() override {
        if(pending_update.exchange(false, std::memory_order_acq_rel)) {
            ev_.notify(sc_core::SC_ZERO_TIME);
        }
    }
    const sc_core::sc_event& operator()() const { return ev_; }
    const sc_core::sc_event& event() const { return ev_; }

private:
    const std::thread::id id{std::this_thread::get_id()};
    sc_core::sc_event ev_;
    std::atomic<bool> pending_update{false};
};
} // namespace scc