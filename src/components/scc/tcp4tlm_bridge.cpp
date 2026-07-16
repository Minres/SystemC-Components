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

#include "tcp4tlm_bridge.h"
#include "scc/report.h"
#include "scc/tcp4tlm/messages.h"
#include "tlm/scc/tlm_extensions.h"
#include "tlm/scc/tlm_gp_shared.h"
#include <algorithm>
#include <boost/asio.hpp>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <ctime>
#include <mutex>
#include <sysc/kernel/sc_module.h>
#include <sysc/kernel/sc_simcontext.h>
#include <sysc/kernel/sc_time.h>
#include <thread>

#define GETCLOCK(X) clock_gettime(CLOCK_REALTIME, X)
namespace scc {

using namespace std::chrono_literals;

tcp4tlm_bridge::tcp4tlm_bridge(sc_core::sc_module_name name, size_t no_of_ports)
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
    tsckt.register_b_transport(this, &tcp4tlm_bridge::btransport_cb);
    tsckt.register_transport_dbg(this, &tcp4tlm_bridge::transport_dbg_cb);
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
void tcp4tlm_bridge::statistics::updateStat(unsigned long rt) {
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

tcp4tlm_bridge::~tcp4tlm_bridge() {
    if(is_server_running() && is_connection_server.get_value() && !is_shutdown_requested()) {
        end_connection();
    }
    shutdown_server();
}

void tcp4tlm_bridge::end_of_elaboration() {
    client::host = other_host_name.get_value();
    client::port = other_host_port.get_value();
}

void tcp4tlm_bridge::start_of_simulation() {
    if(is_connection_server.get_value()) {
        SCCINFO(SCMOD) << "starting server on port " << this_host_port.get_value();
        start_server(this_host_port.get_value());
    } else {
        connect();
        start_server(this_host_port.get_value());
        tcp4tlm::connection<tcp4tlm::request_message, tcp4tlm::response_message>::endpoint_t lep = get_acceptor().local_endpoint();
        auto msg = get_notify_endpoint_msg(lep);
        const auto* endpoint = msg.root()->payload_as_NotifyEndpointMsg();
        SCCTRACE(SCMOD) << "sending coordinates downstream '" << (endpoint->hostname() ? endpoint->hostname()->str() : std::string{}) << ":"
                        << endpoint->port() << "' to " << host << ":" << port;
        client_connection().write_data(msg);
        std::shared_ptr<tcp4tlm::response_message> resp;
        client_connection().read_data(resp);
        if(tcp4tlm::get_status(resp ? resp->root() : nullptr) != tcp4tlm::ok) {
            throw std::exception();
        }
    }
}

void tcp4tlm_bridge::end_of_simulation() {
    if(is_server_running()) {
        if(is_connection_server.get_value() && is_connected) {
            end_connection();
        }
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

unsigned tcp4tlm_bridge::transport_dbg_cb(tlm::tlm_generic_payload& gp) {
    sc_core::sc_time delay(sc_core::SC_ZERO_TIME);
    do_access(gp, delay, true);
    return gp.get_response_status() == tlm::TLM_OK_RESPONSE ? gp.get_data_length() : 0;
}

void tcp4tlm_bridge::btransport_cb(tlm::tlm_generic_payload& gp, sc_core::sc_time& delay) { do_access(gp, delay); }

void tcp4tlm_bridge::do_access(tlm::tlm_generic_payload& gp, sc_core::sc_time& delay, bool debug) {
#ifdef GENERATE_STATISTICS
    static timespec tstart, twser, tmid, tend;
#endif
    if(!is_remote_connected())
        SCCFATAL(SCMOD) << "No remote connected";
    gp.set_dmi_allowed(false);
#ifdef GENERATE_STATISTICS
#define TIMEDIFF(X, Y) X.tv_nsec >= Y.tv_nsec ? X.tv_nsec - Y.tv_nsec : 1000000000 + X.tv_nsec - Y.tv_nsec
    GETCLOCK(&tstart);
    GETCLOCK(&connection_type::get_t_stamp());
#endif
    gp.set_response_status(tlm::TLM_GENERIC_ERROR_RESPONSE);
    std::vector<uint8_t> byte_enable;
    if(gp.get_byte_enable_ptr()) {
        byte_enable.resize(gp.get_byte_enable_length());
        std::copy(gp.get_byte_enable_ptr(), gp.get_byte_enable_ptr() + gp.get_byte_enable_length(), byte_enable.begin());
    }
    switch(gp.get_command()) {
    case tlm::TLM_READ_COMMAND: {
        SCCTRACE(SCMOD) << "Requesting a read @" << sc_core::sc_time_stamp();
        auto bmsg =
            tcp4tlm::make_bus_op_msg(sc_core::sc_time_stamp().value(), delay.value(), debug ? tcp4tlm::debug_acc : tcp4tlm::normal_acc, 0,
                                     gp.get_address(), gp.get_data_length(), false, {}, byte_enable);
        client_connection().write_data(bmsg);
#ifdef GENERATE_STATISTICS
        twser = connection_type::get_t_stamp();
        GETCLOCK(&tmid);
#endif
        client_connection().read_data(resp_msg);
        const auto* response = resp_msg ? resp_msg->root() : nullptr;
        if(tcp4tlm::get_status(response) != tcp4tlm::ok || !tcp4tlm::belongs_to(response, bmsg.root())) {
            break;
        }
        const auto* mresp = response->payload_as_BusDataMsg();
        if(mresp == nullptr || mresp->data() == nullptr || mresp->data()->size() != gp.get_data_length()) {
            break;
        }
        unsigned char* end = std::copy(mresp->data()->begin(), mresp->data()->end(), gp.get_data_ptr());
        assert(static_cast<unsigned>(end - gp.get_data_ptr()) == gp.get_data_length());
        gp.set_response_status(tlm::TLM_OK_RESPONSE);
    } break;
    case tlm::TLM_WRITE_COMMAND: {
        SCCTRACE(SCMOD) << "Requesting a write @" << sc_core::sc_time_stamp();
        std::vector<uint8_t> data(gp.get_data_length());
        std::copy(gp.get_data_ptr(), gp.get_data_ptr() + gp.get_data_length(), data.begin());
        auto bmsg =
            tcp4tlm::make_bus_op_msg(sc_core::sc_time_stamp().value(), delay.value(), debug ? tcp4tlm::debug_acc : tcp4tlm::normal_acc, 0,
                                     gp.get_address(), gp.get_data_length(), write_no_response.get_value(), data, byte_enable);
        client_connection().write_data(bmsg);
#ifdef GENERATE_STATISTICS
        twser = connection_type::get_t_stamp();
        GETCLOCK(&tmid);
#endif

        if(write_no_response.get_value()) {
            gp.set_response_status(tlm::TLM_OK_RESPONSE);
        } else {
            client_connection().read_data(resp_msg);
            const auto* response = resp_msg ? resp_msg->root() : nullptr;
            gp.set_response_status((tcp4tlm::get_status(response) == tcp4tlm::ok && tcp4tlm::belongs_to(response, bmsg.root()))
                                       ? tlm::TLM_OK_RESPONSE
                                       : tlm::TLM_GENERIC_ERROR_RESPONSE);
        }
    } break;
    default:
        break;
    }
#ifdef GENERATE_STATISTICS
    GETCLOCK(&tend);
    txt.push_back(TIMEDIFF(tmid, tstart));
    rtto.push_back(TIMEDIFF(tmid, twser));
    rxt.push_back(TIMEDIFF(tend, tmid));
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

void tcp4tlm_bridge::timing_thread() {
    wait(sc_core::SC_ZERO_TIME);
    // wait until the client connects. We cannot do this in start_of_simulation as we
    // would (potentially) block the server start of other bridges
    if(is_connection_server.get_value()) {
        std::unique_lock<std::mutex> lock(con_est_mtx);
        while(!con_est.load()) {
            con_est_sig.wait_for(lock, 100ms, [this]() { return con_est.load(); });
        }
    }
    // now deal with the timing
    const auto usecs_to_sleep = 1000LL;
#if defined __x86_64__
    if(!is_connection_server.get_value())
        return;
    if(wall_time_simulation_speed.get_value()) {
        SCCDEBUG(SCMOD) << "Running in wall time mode";
        auto duration = usecs_to_sleep;
        auto checkpoint_us = get_time_of_day_us();
        while(true) {
            wait(usecs_to_sleep, sc_core::SC_US);
            auto act_us = get_time_of_day_us();
            auto consumed = act_us - checkpoint_us;
            if(consumed > 0 && duration > consumed) {
                struct timespec tv;
                tv.tv_sec = static_cast<time_t>(duration - consumed) / 1000000;
                tv.tv_nsec = static_cast<decltype(tv.tv_nsec)>((duration - consumed) * 1000);
                nanosleep(&tv, &tv);
            }
            checkpoint_us = get_time_of_day_us();
        }
    } else {
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
#else
    std::posix_time::ptime checkpoint = std::posix_time::microsec_clock::local_time();
    std::posix_time::time_duration duration = std::posix_time::microsec(usecsToSleep);
    if(!is_connection_server.get_value() || !limit_simulation_speed.get_value())
        return;
    while(true) {
        wait(usecsToSleep, sc_core::SC_US);
        std::posix_time::time_duration consumed = std::posix_time::microsec_clock::local_time() - checkpoint;
        if(duration > consumed) {
            std::this_thread::sleep(duration - consumed);
        }
        checkpoint = std::posix_time::microsec_clock::local_time();
    }
#endif
}

tlm::scc::tlm_gp_shared_ptr tcp4tlm_bridge::init_gp(const tcp4tlm::BusOpMsg* const msg) {
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
    return gp;
}

void tcp4tlm_bridge::server_receive_completed(con_ptr& con, const tcp4tlm::request_message* const result) {
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
        SCCTRACE(SCMOD) << "Got notify_endpoint_msg";
        const auto* msg = request->payload_as_NotifyEndpointMsg();
        if(msg->hostname() != nullptr && msg->hostname()->str() == "0.0.0.0") {
            host = "localhost";
        } else {
            host = msg->hostname() ? msg->hostname()->str() : std::string{};
        }
        port = msg->port();
        con->write_data(okmsg);
        client_connection();
        con_est = true;
        con_est_sig.notify_all();
    } break;
    case tcp4tlm::RequestPayload_BusOpMsg: {
        const auto* msg = request->payload_as_BusOpMsg();
        auto time_point = sc_core::sc_time::from_value(msg->time_stamp());
        callback_task task([this, msg, con]() {
            auto gp = init_gp(msg);
            auto delay = sc_core::sc_time::from_value(msg->time_offset());
            isckt->b_transport(*gp, delay);
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
        next_time_stamp.push(time_point);
        fut.wait();
        fut.get();
    } break;
    case tcp4tlm::RequestPayload_SyncMsg: {
        SCCTRACE(SCMOD) << "Got sync_msg";
        // TODO: checkif this is correct
        if(!is_connection_server.get_value()) {
            const auto* msg = request->payload_as_SyncMsg();
            auto time_point = sc_core::sc_time::from_value(msg->time_stamp());
            next_time_stamp.push(time_point);
        }
        con->async_write(okmsg);
    } break;
    case tcp4tlm::RequestPayload_SigOpMsg: {
        SCCTRACE(SCMOD) << "Got sig_op_msg";
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
        SCCTRACE(SCMOD) << "Got notify_shutdown_msg";
        client_connection().socket().close();
        is_connected = false;
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

void tcp4tlm_bridge::process_task_que() {
    timed_task res;
    while(true) {
        wait(task_que.data_event());
        auto success = task_que.try_get(res);
        if(success) {
            SCCTRACEALL(SCMOD) << "Got a task @" << res.timepoint;
            if(no_systemc_sync.get_value() || sc_core::sc_time_stamp() > res.timepoint) {
                res.t();
            } else {
                auto time_point = res.timepoint - sc_core::sc_time_stamp();
                timed_task_que.notify(std::move(res.t), time_point);
            }
        }
    }
}

void tcp4tlm_bridge::process_timed_task_que() {
    while(true) {
        wait(timed_task_que.event());
        auto task = timed_task_que.get();
        SCCTRACEALL(SCMOD) << "Executing a task";
        task();
    }
}

} // namespace scc
