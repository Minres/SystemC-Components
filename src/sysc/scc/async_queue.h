#pragma once

#include <rigtorp/SPSCQueue.h>
#include <sysc/communication/sc_prim_channel.h>
#include <systemc>

namespace scc {
template <typename T> struct async_source_if : virtual public sc_core::sc_interface {
    virtual bool try_get(T& v) = 0;
    virtual sc_core::sc_event const& data_event() const = 0;
};

template <typename T> struct async_queue : sc_core::sc_prim_channel, async_source_if<T> {

    async_queue(unsigned size = 16)
    : que{size} {}

    explicit async_queue(const char* nm, unsigned size = 16)
    : sc_core::sc_prim_channel{nm}
    , que{size} {}

    void push(T const& v) {
        que.push(v);
        async_request_update();
    }

    void emplace(T&& v) {
        que.emplace(std::move(v));
        pending_update.store(true, std::memory_order_release);
        async_request_update();
    }

    bool try_get(T& v) override {
        if(!que.empty()) {
            if(auto f = que.front()) {
                v = std::move(*f);
                que.pop();
                return true;
            }
        }
        return false;
    }

    const sc_core::sc_event& data_event() const override { return ev_; }

private:
    void update() override {
        if(pending_update.exchange(false, std::memory_order_acq_rel))
            ev_.notify(sc_core::SC_ZERO_TIME);
    }
    rigtorp::SPSCQueue<T> que;
    sc_core::sc_event ev_;
    std::atomic<bool> pending_update{false};
};
} // namespace scc