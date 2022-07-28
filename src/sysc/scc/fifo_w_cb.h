/*******************************************************************************
 * Copyright 2019 MINRES Technologies GmbH
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *******************************************************************************/

#pragma once

#include <deque>
#include <functional>
#include <sysc/communication/sc_prim_channel.h>

/** \ingroup scc-sysc
 *  @{
 */
/**@{*/
//! @brief SCC SystemC utilities
namespace scc {
/**
 * @class fifo_w_cb
 * @brief fifo with callbacks
 *
 * A fifo with callbacks upon running empty or being filled. The registered callbacks are triggered if the fifo is empty
 * or if an element is inserted. This can be used to control the sensitivity of processes reading this fifo.
 *
 * @tparam T the type name of the elements to be store in the fifo
 */
template <typename T> class fifo_w_cb : public sc_core::sc_prim_channel {
public:
    fifo_w_cb()
    : sc_core::sc_prim_channel(sc_core::sc_gen_unique_name("fifo_w_cb")) {}

    fifo_w_cb(const char* name)
    : sc_core::sc_prim_channel(name) {}

    virtual ~fifo_w_cb(){};

    void push_back(T& t) {
        in_queue.push_back(t);
        request_update();
    }
    void push_back(const T& t) {
        in_queue.push_back(t);
        request_update();
    }

    T& back() { return in_queue.back(); }
    const T& back() const { return in_queue.back(); }

    void pop_front() {
        out_queue.pop_front();
        if(empty_cb && !out_queue.size())
            empty_cb();
    }

    T& front() { return out_queue.front(); }
    const T& front() const { return out_queue.front(); }

    size_t avail() const { return out_queue.size(); }
    bool empty() const { return out_queue.empty(); }

    void set_avail_cb(std::function<void(void)> f) { avail_cb = f; }
    void set_empty_cb(std::function<void(void)> f) { empty_cb = f; }

    inline sc_core::sc_event const& data_written_event() const { return data_written_evt; }

protected:
    // the update method (does nothing by default)
    virtual void update() {
        if(in_queue.empty())
            return;
        out_queue.insert(out_queue.end(), in_queue.begin(), in_queue.end());
        in_queue.clear();
        if(avail_cb)
            avail_cb();
        data_written_evt.notify(sc_core::SC_ZERO_TIME);
    }

    std::deque<T> in_queue{};
    std::deque<T> out_queue{};
    std::function<void(void)> avail_cb{};
    std::function<void(void)> empty_cb{};
    sc_core::sc_event data_written_evt{};
};

} /* namespace scc */
/** @} */ // end of scc-sysc
