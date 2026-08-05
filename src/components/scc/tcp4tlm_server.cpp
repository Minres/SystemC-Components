/*******************************************************************************
 * Copyright 2026 MINRES Technologies GmbH
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

#include "tcp4tlm_server.h"
#include <algorithm>
#include <atomic>
#include <boost/asio.hpp>
#include <cstdint>
#include <cstring>
#include <ctime>
#include <scc/report.h>
#include <scc/tcp4tlm/messages.h>
#include <scc/wall_time_speed_limiter.h>
#include <sysc/kernel/sc_module.h>
#include <sysc/kernel/sc_simcontext.h>
#include <sysc/kernel/sc_time.h>
#include <thread>
#include <tlm/scc/tlm_extensions.h>
#include <tlm/scc/tlm_gp_shared.h>

#define GETCLOCK(X) clock_gettime(CLOCK_REALTIME, X)
namespace scc {

using namespace std::chrono_literals;

tcp4tlm_server::tcp4tlm_server(sc_core::sc_module_name name, size_t no_of_ports)
: sc_core::sc_module(name)
, tcp4tlm::server<tcp4tlm::request_message, tcp4tlm::response_message>(2)
, signals{"signals", no_of_ports}
, next_time_stamp(16)
#ifdef GENERATE_STATISTICS
, rtto()
, txt()
, rxt()
#endif
{
    SC_THREAD(timing_thread);
    SC_THREAD(process_task_que);
    SC_THREAD(process_timed_task_que);
#ifdef GENERATE_STATISTICS
    rtto.reserve(100000);
    txt.reserve(100000);
    rxt.reserve(100000);
#endif
}

#ifdef GENERATE_STATISTICS
void tcp4tlm_server::statistics::updateStat(unsigned long rt) {
    if(rt > max) {
        max = rt;
    }

    if(rt < min) {
        min = rt;
    }

    sum += rt;
    int idx = indexer.getIndexFromAddr(rt);

    if(idx < 0) {
        SCCERR(SCMOD) << "Could not find index for " << rt;
    } else {
        if(histogram.size() <= static_cast<unsigned>(idx)) {
            histogram.resize(idx + 1);
        }

        histogram[idx]++;
    }

    count++;
}
#endif

tcp4tlm_server::~tcp4tlm_server() {
    if(is_server_running()) {
        SCCTRACE(SCMOD) << "[" << __FUNCTION__ << "] shutting down server";
        shutdown_server();
    }
}

void tcp4tlm_server::before_end_of_elaboration() {
    if(wall_time_simulation_speed.get_value())
        scc::wall_time_speed_limiter::get(); // initialize module
}

void tcp4tlm_server::start_of_simulation() {
    SCCINFO(SCMOD) << "starting server on port " << this_host_port.get_value();
    server::start_server(this_host_port.get_value());
}

void tcp4tlm_server::end_of_simulation() {
    if(is_server_running()) {
        SCCTRACE(SCMOD) << "[" << __FUNCTION__ << "] shutting down server";
        request_shutdown();
    }
#ifdef GENERATE_STATISTICS
    const char* stream_type = typeid(get_acceptor()) == typeid(boost::asio::ip::tcp::acceptor) ? "tcp" : "stream";
    util::range_lut indexer;
    for(size_t idx = 0; idx < 10; ++idx) {
        indexer.setTargetRange(idx, idx * 10000, 10000);
    }
    for(size_t idx = 10; idx < 20; ++idx) {
        indexer.setTargetRange(idx, 100000 * (idx - 9), 100000);
    }
    for(size_t idx = 20; idx < 30; ++idx) {
        indexer.setTargetRange(idx, 1000000 * (idx - 19), 1000000);
    }
    for(size_t idx = 30; idx < 40; ++idx) {
        indexer.setTargetRange(idx, 10000000 * (idx - 29), 10000000);
    }
    statistics stat_tx(indexer, 40, txt[0]), stat_send(indexer, 40, rtto[0]), stat_rx(indexer, 30, rxt[0]);
    for(size_t i = 0; i < txt.size(); ++i) {
        stat_tx.updateStat(txt[i]);
        stat_send.updateStat(rtto[i]);
        stat_rx.updateStat(rxt[i]);
    }
    cout << "Statistics for " << stream_type << " socket based communication" << endl;
    cout << "Send times for " << txt.size() << " transactions in ns for writing (min,avg,max):     " << stat_tx << endl;
    cout << "Transmit times for " << txt.size() << " transactions in ns for writing (min,avg,max): " << stat_send << endl;
    cout << "Receive times for " << txt.size() << " transactions in ns for reading (min,avg,max):  " << stat_rx << endl;
    stat_tx.print_histogram = true;
    cout << "Send times histogram:" << endl << stat_tx;
    stat_rx.print_histogram = true;
    cout << "Receive times histogram:" << endl << stat_rx;
#endif
}

inline long long int get_time_of_day_us() {
    timeval checkpoint;
#if defined __x86_64__
    gettimeofday(&checkpoint, 0);
    return checkpoint.tv_sec * 100000 + checkpoint.tv_usec;
#else
    return 0;
#endif
}

void tcp4tlm_server::timing_thread() {
    wait(sc_core::SC_ZERO_TIME);
    // wait until the client connects. We cannot do this in start_of_simulation as we
    // would (potentially) block the server start of other bridges
    wait4connection();
    // now deal with the timing
    if(!wall_time_simulation_speed.get_value()) {
        SCCDEBUG(SCMOD) << "Running in simulated time mode";
        while(true) {
            while(next_time_stamp.empty()) {
                wait(sc_core::SC_ZERO_TIME);
                std::this_thread::yield();
            }
            auto next = *next_time_stamp.front();
            SCCTRACEALL(SCMOD) << "Got time stamp, advancing to " << next;
            next_time_stamp.pop();
            if(next > sc_core::sc_time_stamp()) {
                wait(next - sc_core::sc_time_stamp());
            }
        }
    }
}

tlm::scc::tlm_gp_shared_ptr tcp4tlm_server::init_gp(const tcp4tlm::BusOpMsg* const msg) {
    tlm::scc::tlm_gp_shared_ptr gp = mm.get().allocate<tlm::scc::data_buffer>();
    auto ext = gp->get_extension<tlm::scc::data_buffer>();
    gp->set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);
    gp->set_address(msg->address());
    gp->set_streaming_width(msg->size());
    gp->set_data_length(msg->size());
    ext->set_size(msg->size());
    gp->set_data_ptr(ext->get_buf_ptr());
    if(msg->data() == nullptr || msg->data()->size() == 0) {
        gp->set_command(tlm::TLM_READ_COMMAND);
    } else {
        gp->set_command(tlm::TLM_WRITE_COMMAND);
        std::memcpy(gp->get_data_ptr(), msg->data()->Data(), gp->get_data_length());
    }
    if(msg->byte_enable() != nullptr && msg->byte_enable()->size() > 0) {
        gp->set_byte_enable_length(msg->byte_enable()->size());
        gp->set_byte_enable_ptr(new uint8_t[msg->byte_enable()->size()]); // TODO: this might result in a memeory leak
        std::memcpy(gp->get_byte_enable_ptr(), msg->data()->Data(), gp->get_data_length());
    }
    return gp;
}

void tcp4tlm_server::server_receive_completed(con_ptr& con, const tcp4tlm::request_message* const result) {
    const auto* request = result ? result->root() : nullptr;
    if(request == nullptr) {
        auto msg = tcp4tlm::make_response(uint32_t{0}, tcp4tlm::declined);
        con->async_write(msg);
        con->async_read();
        return;
    }
    auto okmsg = tcp4tlm::make_response(request);
    switch(request->payload_type()) {
    case tcp4tlm::RequestPayload_NotifyEndpointMsg: {
        SCCTRACE(SCMOD) << "Got NotifyEndpointMsg";
        const auto* msg = request->payload_as_NotifyEndpointMsg();
        callback_task task([this, msg, con]() {
            auto okmsg = tcp4tlm::make_response(msg->id());
            con->async_write(okmsg);
            return true;
        });
        timed_task tup{std::move(task), sc_core::SC_ZERO_TIME};
        task_que.emplace(std::move(tup));
        con_est.store(true, std::memory_order_acq_rel);
        con_est_sig.notify_all();
    } break;
    case tcp4tlm::RequestPayload_BusOpMsg: {
        SCCTRACE(SCMOD) << "Got BusOpMsg";
        const auto* msg = request->payload_as_BusOpMsg();
        auto time_point = sc_core::sc_time::from_value(msg->time_stamp());
        callback_task task([this, msg, con]() {
            auto gp = init_gp(msg);
            auto delay = sc_core::sc_time::from_value(msg->time_offset());
            if(msg->type() == scc::tcp4tlm::BusAccessType::BusAccessType_DEBUG_ACC)
                isckt->transport_dbg(*gp);
            else
                isckt->b_transport(*gp, delay);
            if(gp->get_byte_enable_ptr()) {
                delete[] gp->get_byte_enable_ptr();
                gp->set_byte_enable_ptr(nullptr);
            }
            if(gp->is_read()) {
                auto* ext = gp->get_extension<tlm::scc::data_buffer>();
                auto dmsg = tcp4tlm::make_bus_data_msg(msg->id(), ext->data(),
                                                       gp->get_response_status() == tlm::TLM_OK_RESPONSE ? tcp4tlm::ok : tcp4tlm::failure);
                con->async_write(dmsg);
            } else if(gp->get_response_status() != tlm::TLM_OK_RESPONSE) {
                auto failmsg = tcp4tlm::make_response(msg->id(), tcp4tlm::failure);
                con->async_write(failmsg);
            } else if(!msg->no_response()) {
                auto okmsg = tcp4tlm::make_response(msg->id());
                con->async_write(okmsg);
            }
            return true;
        });
        std::future<bool> fut = task.get_future();
        timed_task tup{std::move(task), time_point};
        task_que.emplace(std::move(tup));
        if(!wall_time_simulation_speed.get_value())
            next_time_stamp.push(time_point);
        fut.wait();
        fut.get();
    } break;
    case tcp4tlm::RequestPayload_SyncMsg: {
        SCCTRACE(SCMOD) << "Got SyncMsg";
        if(!wall_time_simulation_speed.get_value()) {
            const auto* msg = request->payload_as_SyncMsg();
            auto time_point = sc_core::sc_time::from_value(msg->time_stamp());
            next_time_stamp.push(time_point);
        }
        // no response
    } break;
    case tcp4tlm::RequestPayload_SigOpMsg: {
        SCCTRACE(SCMOD) << "Got SigOpMsg";
        const auto* msg = request->payload_as_SigOpMsg();
        if(signals.size() > msg->index()) {
            callback_task task([this, &msg]() {
                signals[msg->index()] = msg->value();
                return true;
            });
            std::future<bool> fut = task.get_future();
            timed_task tup{std::move(task), sc_core::SC_ZERO_TIME};
            task_que.emplace(std::move(tup));
            fut.wait();
            fut.get();
            con->async_write(okmsg);
        } else {
            auto declined_msg = tcp4tlm::make_response(request, tcp4tlm::declined);
            con->async_write(declined_msg);
        }
    } break;
    case tcp4tlm::RequestPayload_NotifyShutdownMsg: {
        SCCTRACE(SCMOD) << "Got NotifyShutdownMsg";
        if(is_server_running()) {
            request_shutdown();
        }
        callback_task task([this]() {
            this->shutdown_evt.notify(sc_core::SC_ZERO_TIME);
            return true;
        });
        std::future<bool> fut = task.get_future();
        timed_task tup{std::move(task), sc_core::SC_ZERO_TIME};
        task_que.emplace(std::move(tup));
        fut.wait();
        fut.get();
        return;
    }
    default: {
        SCCWARN(SCMOD) << "Got an unhandled message";
        auto msg = tcp4tlm::make_response(request, tcp4tlm::declined);
        con->async_write(msg);
    } break;
    }
    con->async_read();
}

void tcp4tlm_server::process_task_que() {
    timed_task res;
    while(true) {
        while(task_que.try_get(res)) {
            SCCTRACEALL(SCMOD) << "Got a task @" << res.timepoint;
            if(wall_time_simulation_speed.get_value() || sc_core::sc_time_stamp() >= res.timepoint) {
                res.t();
            } else {
                auto time_point = res.timepoint - sc_core::sc_time_stamp();
                timed_task_que.notify(std::move(res.t), time_point);
            }
        }
        wait(task_que.data_event());
    }
}

void tcp4tlm_server::process_timed_task_que() {
    while(true) {
        wait(timed_task_que.event());
        auto task = timed_task_que.get();
        SCCTRACEALL(SCMOD) << "Executing a task";
        task();
    }
}
} // namespace scc
