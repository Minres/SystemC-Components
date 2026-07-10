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
#include <vector>

#include <time.h>
#define DEFINE_EXPORTS_HERE
#include "bridge.h"

#define GETCLOCK(X) clock_gettime(CLOCK_REALTIME, X)
namespace tlm {
namespace scc {
namespace tcp {

tcp4tlm_bridge::tcp4tlm_bridge(sc_core::sc_module_name name, size_t noOfPorts)
: sc_core::sc_module(name)
, Server()
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
void tcp4tlm2::statistics::updateStat(unsigned long rt) {
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
    if(isServerRunning() && isConnectionMaster.get_value() && !isShutdownRequested()) {
        endConnection();
    }

    shutdownServer();
}

void tcp4tlm_bridge::start_of_simulation() {
    if(isConnectionMaster.get_value()) {
        LOG(INFO) << "starting server on port " << thisHostPort.get_value();
        startServer(thisHostPort.get_value());
    };
}

unsigned tcp4tlm_bridge::transport_dbg_cb(tlm::tlm_generic_payload& gp) {
    sc_core::sc_time delay(sc_core::SC_ZERO_TIME);
    do_access(gp, delay, true);
    return gp.get_response_status() == tlm::TLM_OK_RESPONSE ? gp.get_data_length() : 0;
}

void tcp4tlm_bridge::btransport_cb(tlm::tlm_generic_payload& gp, sc_core::sc_time& delay) { do_access(gp, delay); }

void tcp4tlm_bridge::do_access(tlm::tlm_generic_payload& gp, sc_core::sc_time& delay, bool debug) {
    if(!isConnectionMaster.get_value()) {
        gp.set_response_status(tlm::TLM_GENERIC_ERROR_RESPONSE);
        return;
    }

#ifdef GENERATE_STATISTICS
    static timespec tstart, twser, tmid, tend;
#endif

    if(!isClientConnected()) {
        if(isConnectionMaster.get_value()) {
            impl::connection<RequestMessage, ResponseMessage>::endpoint_t lep = getAcceptor().local_endpoint();
            NotifyEndpointMsg msg = getNotifyEndpointMsg(lep);
            clientConnection().write_data(msg);
        } else {
            while(notifyMsgReceived.load(boost::memory_order_relaxed) != true) {
                boost::this_thread::sleep_for(boost::chrono::milliseconds(100));
            }
        }
    }

    // prepare response
    gp.set_dmi_allowed(false);
    // reset the clock
#ifdef GENERATE_STATISTICS
#define TIMEDIFF(X, Y) X.tv_nsec >= Y.tv_nsec ? X.tv_nsec - Y.tv_nsec : 1000000000 + X.tv_nsec - Y.tv_nsec
    GETCLOCK(&tstart);
    GETCLOCK(&connection_type::getTStamp());
#endif
    gp.set_response_status(tlm::TLM_GENERIC_ERROR_RESPONSE);
    // prepare transfer data
    BusOpMsg bmsg;
    bmsg.index = 0;
    bmsg.time_stamp = sc_core::sc_time_stamp().value();
    bmsg.time_offset = delay.value();
    bmsg.address = gp.get_address();
    bmsg.size = gp.get_data_length();
    bmsg.type = debug ? DEBUG_ACC : NORMAL_ACC;

    if(gp.get_byte_enable_ptr()) {
        bmsg.byte_enable.resize(gp.get_byte_enable_length());
        std::copy(gp.get_byte_enable_ptr(), gp.get_byte_enable_ptr() + gp.get_byte_enable_length(), bmsg.byte_enable.begin());
    } else {
        bmsg.byte_enable.resize(0);
    }

    switch(gp.get_command()) {
    case tlm::TLM_READ_COMMAND: {
        LOG(TRACE) << "Requesting a read @" << sc_core::sc_time_stamp();
        clientConnection().write_data(bmsg);
#ifdef GENERATE_STATISTICS
        twser = connection_type::getTStamp();
        GETCLOCK(&tmid);
#endif
        clientConnection().read_data(resp_msg);

        if(resp_msg->getStatus() != OK || !resp_msg->belongsTo(&bmsg)) {
            break;
        }

        BusDataMsg* mresp = dynamic_cast<BusDataMsg*>(resp_msg.get());

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
        bmsg.noResponse = writeNoResponse.get_value();
        std::copy(gp.get_data_ptr(), gp.get_data_ptr() + gp.get_data_length(), bmsg.data.begin());
        clientConnection().write_data(bmsg);
#ifdef GENERATE_STATISTICS
        twser = connection_type::getTStamp();
        GETCLOCK(&tmid);
#endif

        if(bmsg.noResponse) {
            gp.set_response_status(tlm::TLM_OK_RESPONSE);
        } else {
            clientConnection().read_data(resp_msg);
            gp.set_response_status((resp_msg->getStatus() == OK && resp_msg->belongsTo(&bmsg)) ? tlm::TLM_OK_RESPONSE
                                                                                               : tlm::TLM_GENERIC_ERROR_RESPONSE);
        }
    } break;
    default: // TLM_IGNORE_COMMAND{
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
    host = otherHostName.get_value();
    port = otherHostPort.get_value();
}

void tcp4tlm_bridge::main_thread() {
    if(!noSystemcSync.get_value()) {
        if(isConnectionMaster.get_value()) {
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

                if(isShutdownRequested()) {
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
    if(isServerRunning()) {
        if(isConnectionMaster.get_value()) {
            endConnection();
        } else {
            requestShutdown();
        }
    }

#ifdef GENERATE_STATISTICS
    const char* stream_type = typeid(getAcceptor()) == typeid(boost::asio::ip::tcp::acceptor) ? "tcp" : "stream";
    tlm_genip::addr_decoder indexer;

    for(size_t idx = 0; idx < 10; ++idx) // [0,10000)...[90000,100000),
    {
        indexer.setTargetRange(idx, idx * 10000, 10000);
    }

    for(size_t idx = 10; idx < 20; ++idx) // [100000,200000)...[900000,1000000)
    {
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

inline long long int getTimeOfDayUs() {
    timeval checkpoint;
#if defined __x86_64__
    gettimeofday(&checkpoint, 0); // gettimeofday does not support TZ adjust on Linux.
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
    checkpointUs = getTimeOfDayUs();
    long long int duration = usecsToSleep;

    while(isConnectionMaster.get_value() && limitSimulationSpeed.get_value()) {
        wait(usecsToSleep, sc_core::SC_US);
        actUs = getTimeOfDayUs();
        long long int consumed = actUs - checkpointUs;

        if(consumed > 0 && duration > consumed) {
            struct timespec tv;
            tv.tv_sec = (time_t)(duration - consumed) / 1000000; // Construct the timespec from the number of whole seconds...
            tv.tv_nsec = (long)((duration - consumed) * 1000);   //.. and the remainder in nanoseconds.
            nanosleep(&tv, &tv);
        }

        checkpointUs = getTimeOfDayUs();
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

void tcp4tlm_bridge::init_gp(const BusOpMsg* const msg) {
    gp.set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);
    gp.set_address(msg->address);
    gp.set_streaming_width(msg->size);
    gp.set_data_length(msg->size);
}

void tcp4tlm_bridge::serverReceiveCompleted(con_ptr& con, const RequestMessage* const result) {
    ResponseMessage okmsg(result);

    if(typeid(*result) == typeid(NotifyEndpointMsg)) {
        LOG(TRACE) << "Got NotifyEndpointMsg";
        const NotifyEndpointMsg* const msg = dynamic_cast<const NotifyEndpointMsg* const>(result);

        if(msg->hostname == "0.0.0.0")
        // if remote uses 0.0.0.0 we assume it's running on localhost (although
        // it is listening on all interfaces
        {
            host = "localhost";
        } else {
            host = msg->hostname;
        }

        port = msg->port;
        notifyMsgReceived.store(true, boost::memory_order_relaxed);
        con->write_data(okmsg);
        clientConnection();
        {
            boost::mutex::scoped_lock lock(con_est_mtx);
            con_est = true;
            con_est_sig.notify_all();
        }
    } else if(typeid(*result) == typeid(BusOpMsg)) {
        boost::unique_lock<boost::mutex> lock(gp_mtx);
        {
            boost::mutex::scoped_lock lock(cmd_rec_mxt);
            cmd_rec_sig.notify_all();
        }
        // boost::mutex::scoped_lock lock(gp_mtx);
        const BusOpMsg* const msg = dynamic_cast<const BusOpMsg* const>(result);

        if(msg->data.size() == 0) // we have a read
        {
            LOG(TRACE) << "Got BusOpMsg read";
            init_gp(msg);
            gp.set_command(tlm::TLM_READ_COMMAND);
            BusDataMsg dmsg(msg);
            dmsg.data.resize(msg->size);
            gp.set_data_ptr(&(dmsg.data[0]));

            if(noSystemcSync.get_value()) {
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
                ResponseMessage failmsg(result, FAILURE);
                con->async_write(failmsg);
            }
        } else {
            LOG(TRACE) << "Got BusOpMsg write";
            init_gp(msg);
            gp.set_command(tlm::TLM_WRITE_COMMAND);
            gp.set_data_ptr(const_cast<unsigned char*>(&(msg->data[0])));

            if(noSystemcSync.get_value()) {
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

            if(!msg->noResponse) {
                if(gp.get_response_status() == tlm::TLM_OK_RESPONSE) {
                    con->async_write(okmsg);
                } else {
                    ResponseMessage failmsg(result, FAILURE);
                    con->async_write(failmsg);
                }
            }
        }
    } else if(typeid(*result) == typeid(SyncMsg)) {
        LOG(TRACE) << "Got SyncMsg";

        if(!isConnectionMaster.get_value()) // ignore if we are the simulation master
        {
            boost::unique_lock<boost::mutex> lock(gp_mtx);
            {
                boost::mutex::scoped_lock lock(cmd_rec_mxt);
                cmd_rec_sig.notify_all();
            }
            gp.set_command(tlm::TLM_IGNORE_COMMAND);
            gp_timestamp = sc_core::sc_time((sc_dt::uint64) dynamic_cast<const SyncMsg* const>(result)->time_stamp, false);
            request_bus_access.notify(sc_core::SC_ZERO_TIME);

            while(gp.get_response_status() == tlm::TLM_INCOMPLETE_RESPONSE) {
                gp_sig.wait(lock);
            }
        }

        con->async_write(okmsg);
    } else if(typeid(*result) == typeid(SigOpMsg)) {
        LOG(TRACE) << "Got SigOpMsg";
        const SigOpMsg* const msg = dynamic_cast<const SigOpMsg* const>(result);

        if(signals.size() > msg->index) {
            signals[msg->index] = msg->value;
            con->async_write(okmsg);
        } else {
            ResponseMessage msg(result, DECLINED);
            con->async_write(msg);
        }
    } else if(typeid(*result) == typeid(NotifyShutdownMsg)) {
        LOG(TRACE) << "Got NotifyShutdownMsg";

        if(!isConnectionMaster.get_value()) {
            clientConnection().socket().close();

            if(isServerRunning()) {
                requestShutdown();
            }

            cmd_rec_sig.notify_all();
        }

        return;
    } else {
        LOG(WARN) << "Got an unknown message";
        ResponseMessage msg(result, DECLINED);
        con->async_write(msg);
    }

    // wait for the next command
    con->async_read();
}

void tcp4tlm_bridge::initiate_connection(unsigned short retry_count) {
    if(!isServerRunning()) {
        connect();
        // start the server
        startServer(thisHostPort.get_value());
        // now tell the other end my end-point coordinates
        impl::connection<RequestMessage, ResponseMessage>::endpoint_t lep = getAcceptor().local_endpoint();
        NotifyEndpointMsg msg = getNotifyEndpointMsg(lep);
        LOG(INFO) << "sending coordinates downstream '" << msg.hostname << ":" << msg.port << "' to " << host << ":" << port;
        clientConnection().write_data(msg);
        ResponseMessage* resp;
        clientConnection().read_data(resp);

        if(resp->getStatus() != OK) {
            throw std::exception();
        }

        {
            boost::mutex::scoped_lock lock(con_est_mtx);
            con_est = true;
            con_est_sig.notify_all();
        }
    }
}
} // namespace tcp
} // namespace scc
} // namespace tlm
