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

#include <boost/asio.hpp>
#include <boost/thread/thread.hpp>

#include <time.h>
#define DEFINE_EXPORTS_HERE
#include "tcp4tlm_bridge.h"

#define GETCLOCK(X) clock_gettime(CLOCK_REALTIME, X)
namespace scc {

tcp4tlm_bridge::tcp4tlm_bridge(sc_core::sc_module_name name, size_t noOfPorts)
: sc_core::sc_module(name)
, tcp4tlm::server<request_message, response_message>()
, signals("sig_o")
, con_est(false)
, notifyMsgReceived(false)
#ifdef GENERATE_STATISTICS
, rtto()
, txt()
, rxt()
#endif
{
    if(noOfPorts) {
        signals.init(noOfPorts);
    }

    tsckt.register_b_transport(this, &tcp4tlm_bridge::btransport_cb);
    tsckt.register_transport_dbg(this, &tcp4tlm_bridge::transport_dbg_cb);
    SC_THREAD(main_thread);
    SC_THREAD(timing_thread);
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
        LOG(ERR) << "Could not find index for " << rt;
    } else {
        if(histogram.size() <= (unsigned)idx) {
            histogram.resize(idx + 1);
        }

        histogram[idx]++;
    }

    count++;
}
#endif

tcp4tlm_bridge::~tcp4tlm_bridge() {
    if(is_server_running() && is_connection_master.get_value() && !is_shutdown_requested()) {
        end_connection();
    }

    shutdown_server();
}

void tcp4tlm_bridge::start_of_simulation() {
    if(is_connection_master.get_value()) {
        LOG(INFO) << "starting server on port " << this_host_port.get_value();
        start_server(this_host_port.get_value());
    };
}

unsigned tcp4tlm_bridge::transport_dbg_cb(tlm::tlm_generic_payload& gp) {
    sc_core::sc_time delay(sc_core::SC_ZERO_TIME);
    do_access(gp, delay, true);
    return gp.get_response_status() == tlm::TLM_OK_RESPONSE ? gp.get_data_length() : 0;
}

void tcp4tlm_bridge::btransport_cb(tlm::tlm_generic_payload& gp, sc_core::sc_time& delay) { do_access(gp, delay); }

void tcp4tlm_bridge::do_access(tlm::tlm_generic_payload& gp, sc_core::sc_time& delay, bool debug) {
    if(!is_connection_master.get_value()) {
        gp.set_response_status(tlm::TLM_GENERIC_ERROR_RESPONSE);
        return;
    }

#ifdef GENERATE_STATISTICS
    static timespec tstart, twser, tmid, tend;
#endif

    if(!is_client_connected()) {
        if(is_connection_master.get_value()) {
            tcp4tlm::connection<request_message, response_message>::endpoint_t lep = get_acceptor().local_endpoint();
            notify_endpoint_msg msg = get_notify_endpoint_msg(lep);
            client_connection().write_data(msg);
        } else {
            while(notifyMsgReceived.load(boost::memory_order_relaxed) != true) {
                boost::this_thread::sleep_for(boost::chrono::milliseconds(100));
            }
        }
    }

    gp.set_dmi_allowed(false);
#ifdef GENERATE_STATISTICS
#define TIMEDIFF(X, Y) X.tv_nsec >= Y.tv_nsec ? X.tv_nsec - Y.tv_nsec : 1000000000 + X.tv_nsec - Y.tv_nsec
    GETCLOCK(&tstart);
    GETCLOCK(&connection_type::get_t_stamp());
#endif
    gp.set_response_status(tlm::TLM_GENERIC_ERROR_RESPONSE);
    bus_op_msg bmsg;
    bmsg.index = 0;
    bmsg.time_stamp = sc_core::sc_time_stamp().value();
    bmsg.time_offset = delay.value();
    bmsg.address = gp.get_address();
    bmsg.size = gp.get_data_length();
    bmsg.type = debug ? debug_acc : normal_acc;

    if(gp.get_byte_enable_ptr()) {
        bmsg.byte_enable.resize(gp.get_byte_enable_length());
        std::copy(gp.get_byte_enable_ptr(), gp.get_byte_enable_ptr() + gp.get_byte_enable_length(), bmsg.byte_enable.begin());
    } else {
        bmsg.byte_enable.resize(0);
    }

    switch(gp.get_command()) {
    case tlm::TLM_READ_COMMAND: {
        LOG(TRACE) << "Requesting a read @" << sc_core::sc_time_stamp();
        client_connection().write_data(bmsg);
#ifdef GENERATE_STATISTICS
        twser = connection_type::get_t_stamp();
        GETCLOCK(&tmid);
#endif
        client_connection().read_data(resp_msg);

        if(resp_msg->get_status() != ok || !resp_msg->belongs_to(&bmsg)) {
            break;
        }

        bus_data_msg* mresp = dynamic_cast<bus_data_msg*>(resp_msg.get());

        if(mresp == NULL || mresp->data.size() != gp.get_data_length()) {
            break;
        }

        unsigned char* end = std::copy(mresp->data.begin(), mresp->data.end(), gp.get_data_ptr());
        assert((unsigned)(end - gp.get_data_ptr()) == gp.get_data_length());
        gp.set_response_status(tlm::TLM_OK_RESPONSE);
    } break;
    case tlm::TLM_WRITE_COMMAND: {
        LOG(TRACE) << "Requesting a write @" << sc_core::sc_time_stamp();
        bmsg.data.resize(gp.get_data_length());
        bmsg.no_response = write_no_response.get_value();
        std::copy(gp.get_data_ptr(), gp.get_data_ptr() + gp.get_data_length(), bmsg.data.begin());
        client_connection().write_data(bmsg);
#ifdef GENERATE_STATISTICS
        twser = connection_type::get_t_stamp();
        GETCLOCK(&tmid);
#endif

        if(bmsg.no_response) {
            gp.set_response_status(tlm::TLM_OK_RESPONSE);
        } else {
            client_connection().read_data(resp_msg);
            gp.set_response_status((resp_msg->get_status() == ok && resp_msg->belongs_to(&bmsg)) ? tlm::TLM_OK_RESPONSE
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

void tcp4tlm_bridge::end_of_elaboration() {
    host = other_host_name.get_value();
    port = other_host_port.get_value();
}

void tcp4tlm_bridge::main_thread() {
    if(!no_systemc_sync.get_value()) {
        if(is_connection_master.get_value()) {
            wait4connection();

            while(true) {
                wait(request_bus_access);
                {
                    sc_core::sc_time delay(sc_core::SC_ZERO_TIME);
                    boost::unique_lock<boost::mutex> lock(gp_mtx);
                    isckt->b_transport(gp, delay);
                    gp_sig.notify_all();
                }
            }
        } else {
            wait4connection();

            while(true) {
                wait4command();

                if(is_shutdown_requested()) {
                    break;
                }

                if(sc_core::sc_time_stamp() < gp_timestamp) {
                    quantumkeeper.set(gp_timestamp - sc_core::sc_time_stamp());
                    quantumkeeper.sync();
                }

                boost::unique_lock<boost::mutex> lock(gp_mtx);

                if(gp.get_command() != tlm::TLM_IGNORE_COMMAND) {
                    isckt->b_transport(gp, gp_timeoffset);
                }

                gp_sig.notify_all();
            }

            sc_core::sc_stop();
        }
    }
}

void tcp4tlm_bridge::end_of_simulation() {
    if(is_server_running()) {
        if(is_connection_master.get_value()) {
            end_connection();
        } else {
            request_shutdown();
        }
    }

#ifdef GENERATE_STATISTICS
    const char* stream_type = typeid(get_acceptor()) == typeid(boost::asio::ip::tcp::acceptor) ? "tcp" : "stream";
    tlm_genip::addr_decoder indexer;

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

void tcp4tlm_bridge::timing_thread() {
    const unsigned long usecsToSleep = 1000;
    wait(sc_core::SC_ZERO_TIME);
    wait4connection();
#if defined __x86_64__
    long long int checkpointUs, actUs;
    checkpointUs = get_time_of_day_us();
    long long int duration = usecsToSleep;

    while(is_connection_master.get_value() && limit_simulation_speed.get_value()) {
        wait(usecsToSleep, sc_core::SC_US);
        actUs = get_time_of_day_us();
        long long int consumed = actUs - checkpointUs;

        if(consumed > 0 && duration > consumed) {
            struct timespec tv;
            tv.tv_sec = (time_t)(duration - consumed) / 1000000;
            tv.tv_nsec = (long)((duration - consumed) * 1000);
            nanosleep(&tv, &tv);
        }

        checkpointUs = get_time_of_day_us();
    }

#else
    boost::posix_time::ptime checkpoint = boost::posix_time::microsec_clock::local_time();
    boost::posix_time::time_duration duration = boost::posix_time::microsec(usecsToSleep);

    while(isConnectionMaster.get_value() && limitSimulationSpeed.get_value()) {
        wait(usecsToSleep, sc_core::SC_US);
        boost::posix_time::time_duration consumed = boost::posix_time::microsec_clock::local_time() - checkpoint;

        if(duration > consumed) {
            boost::this_thread::sleep(duration - consumed);
        }

        checkpoint = boost::posix_time::microsec_clock::local_time();
    }

#endif
}

void tcp4tlm_bridge::init_gp(const bus_op_msg* const msg) {
    gp.set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);
    gp.set_address(msg->address);
    gp.set_streaming_width(msg->size);
    gp.set_data_length(msg->size);
}

void tcp4tlm_bridge::server_receive_completed(con_ptr& con, const request_message* const result) {
    response_message okmsg(result);

    if(typeid(*result) == typeid(notify_endpoint_msg)) {
        LOG(TRACE) << "Got notify_endpoint_msg";
        const notify_endpoint_msg* const msg = dynamic_cast<const notify_endpoint_msg* const>(result);

        if(msg->hostname == "0.0.0.0") {
            host = "localhost";
        } else {
            host = msg->hostname;
        }

        port = msg->port;
        notifyMsgReceived.store(true, boost::memory_order_relaxed);
        con->write_data(okmsg);
        client_connection();
        {
            boost::mutex::scoped_lock lock(con_est_mtx);
            con_est = true;
            con_est_sig.notify_all();
        }
    } else if(typeid(*result) == typeid(bus_op_msg)) {
        boost::unique_lock<boost::mutex> lock(gp_mtx);
        {
            boost::mutex::scoped_lock lock(cmd_rec_mxt);
            cmd_rec_sig.notify_all();
        }
        const bus_op_msg* const msg = dynamic_cast<const bus_op_msg* const>(result);

        if(msg->data.size() == 0) {
            LOG(TRACE) << "Got bus_op_msg read";
            init_gp(msg);
            gp.set_command(tlm::TLM_READ_COMMAND);
            bus_data_msg dmsg(msg);
            dmsg.data.resize(msg->size);
            gp.set_data_ptr(&(dmsg.data[0]));

            if(no_systemc_sync.get_value()) {
                sc_core::sc_time delay(sc_core::SC_ZERO_TIME);
                isckt->b_transport(gp, delay);
            } else {
                gp_timestamp = sc_core::sc_time((sc_dt::uint64)msg->time_stamp, false);
                gp_timeoffset = sc_core::sc_time((sc_dt::uint64)msg->time_offset, false);
                request_bus_access.notify(sc_core::SC_ZERO_TIME);

                while(gp.get_response_status() == tlm::TLM_INCOMPLETE_RESPONSE) {
                    gp_sig.wait(lock);
                }
            }

            if(gp.get_response_status() == tlm::TLM_OK_RESPONSE) {
                con->async_write(dmsg);
            } else {
                response_message failmsg(result, failure);
                con->async_write(failmsg);
            }
        } else {
            LOG(TRACE) << "Got bus_op_msg write";
            init_gp(msg);
            gp.set_command(tlm::TLM_WRITE_COMMAND);
            gp.set_data_ptr(const_cast<unsigned char*>(&(msg->data[0])));

            if(no_systemc_sync.get_value()) {
                sc_core::sc_time delay(sc_core::SC_ZERO_TIME);
                isckt->b_transport(gp, delay);
            } else {
                gp_timestamp = sc_core::sc_time((sc_dt::uint64)msg->time_stamp, false);
                gp_timeoffset = sc_core::sc_time((sc_dt::uint64)msg->time_offset, false);
                request_bus_access.notify(sc_core::SC_ZERO_TIME);

                while(gp.get_response_status() == tlm::TLM_INCOMPLETE_RESPONSE) {
                    gp_sig.wait(lock);
                }
            }

            if(!msg->no_response) {
                if(gp.get_response_status() == tlm::TLM_OK_RESPONSE) {
                    con->async_write(okmsg);
                } else {
                    response_message failmsg(result, failure);
                    con->async_write(failmsg);
                }
            }
        }
    } else if(typeid(*result) == typeid(sync_msg)) {
        LOG(TRACE) << "Got sync_msg";

        if(!is_connection_master.get_value()) {
            boost::unique_lock<boost::mutex> lock(gp_mtx);
            {
                boost::mutex::scoped_lock lock(cmd_rec_mxt);
                cmd_rec_sig.notify_all();
            }
            gp.set_command(tlm::TLM_IGNORE_COMMAND);
            gp_timestamp = sc_core::sc_time((sc_dt::uint64) dynamic_cast<const sync_msg* const>(result)->time_stamp, false);
            request_bus_access.notify(sc_core::SC_ZERO_TIME);

            while(gp.get_response_status() == tlm::TLM_INCOMPLETE_RESPONSE) {
                gp_sig.wait(lock);
            }
        }

        con->async_write(okmsg);
    } else if(typeid(*result) == typeid(sig_op_msg)) {
        LOG(TRACE) << "Got sig_op_msg";
        const sig_op_msg* const msg = dynamic_cast<const sig_op_msg* const>(result);

        if(signals.size() > msg->index) {
            signals[msg->index] = msg->value;
            con->async_write(okmsg);
        } else {
            response_message msg(result, declined);
            con->async_write(msg);
        }
    } else if(typeid(*result) == typeid(notify_shutdown_msg)) {
        LOG(TRACE) << "Got notify_shutdown_msg";

        if(!is_connection_master.get_value()) {
            client_connection().socket().close();

            if(is_server_running()) {
                request_shutdown();
            }

            cmd_rec_sig.notify_all();
        }

        return;
    } else {
        LOG(WARN) << "Got an unknown message";
        response_message msg(result, declined);
        con->async_write(msg);
    }

    con->async_read();
}

void tcp4tlm_bridge::initiate_connection(unsigned short retry_count) {
    if(!is_server_running()) {
        connect();
        start_server(this_host_port.get_value());
        tcp4tlm::connection<request_message, response_message>::endpoint_t lep = get_acceptor().local_endpoint();
        notify_endpoint_msg msg = get_notify_endpoint_msg(lep);
        LOG(INFO) << "sending coordinates downstream '" << msg.hostname << ":" << msg.port << "' to " << host << ":" << port;
        client_connection().write_data(msg);
        response_message* resp;
        client_connection().read_data(resp);

        if(resp->get_status() != ok) {
            throw std::exception();
        }

        {
            boost::mutex::scoped_lock lock(con_est_mtx);
            con_est = true;
            con_est_sig.notify_all();
        }
    }
}

} // namespace scc
