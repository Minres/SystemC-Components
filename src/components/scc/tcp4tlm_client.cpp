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

#include "tcp4tlm_client.h"
#include "scc/report.h"
#include "scc/tcp4tlm/messages.h"
#include "tlm/scc/tlm_extensions.h"
#include "tlm/scc/tlm_gp_shared.h"
#include <algorithm>
#include <atomic>
#include <boost/asio.hpp>
#include <cstdint>
#include <cstring>
#include <ctime>
#include <limits>
#include <sysc/kernel/sc_module.h>
#include <sysc/kernel/sc_simcontext.h>
#include <sysc/kernel/sc_time.h>
#include <thread>

#define GETCLOCK(X) clock_gettime(CLOCK_REALTIME, X)
namespace scc {

using namespace std::chrono_literals;

tcp4tlm_client::tcp4tlm_client(sc_core::sc_module_name name, size_t no_of_ports)
: sc_core::sc_module(name)
#ifdef GENERATE_STATISTICS
, rtto()
, txt()
, rxt()
#endif
{
    tsckt.register_b_transport(this, &tcp4tlm_client::btransport_cb);
    tsckt.register_transport_dbg(this, &tcp4tlm_client::transport_dbg_cb);
    SC_THREAD(timing_thread);
#ifdef GENERATE_STATISTICS
    rtto.reserve(100000);
    txt.reserve(100000);
    rxt.reserve(100000);
#endif
}

#ifdef GENERATE_STATISTICS
void tcp4tlm_client::statistics::updateStat(unsigned long rt) {
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

tcp4tlm_client::~tcp4tlm_client() { end_connection(); }

void tcp4tlm_client::start_of_simulation() {
    client::host = other_host_name.get_value();
    client::port = other_host_port.get_value();
    connect();
    auto msg = tcp4tlm::make_notify_endpoint_msg("", std::numeric_limits<uint16_t>::max());
    const auto* endpoint = msg.root()->payload_as_NotifyEndpointMsg();
    SCCTRACE(SCMOD) << "sending coordinates downstream '" << (endpoint->hostname() ? endpoint->hostname()->str() : std::string{}) << ":"
                    << endpoint->port() << "' to " << host << ":" << port;
    client_connection().write_data(msg);
    std::shared_ptr<tcp4tlm::response_message> resp;
    client_connection().read_data(resp);
    SCCTRACE(SCMOD) << "got response, start simulating";
    if(tcp4tlm::get_status(resp ? resp->root() : nullptr) != tcp4tlm::ok) {
        throw std::exception();
    }
}

void tcp4tlm_client::end_of_simulation() {
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

unsigned tcp4tlm_client::transport_dbg_cb(tlm::tlm_generic_payload& gp) {
    sc_core::sc_time delay(sc_core::SC_ZERO_TIME);
    do_access(gp, delay, true);
    return gp.get_response_status() == tlm::TLM_OK_RESPONSE ? gp.get_data_length() : 0;
}

void tcp4tlm_client::btransport_cb(tlm::tlm_generic_payload& gp, sc_core::sc_time& delay) { do_access(gp, delay); }

void tcp4tlm_client::do_access(tlm::tlm_generic_payload& gp, sc_core::sc_time& delay, bool debug) {
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

void tcp4tlm_client::timing_thread() {
    if(no_systemc_sync.get_value())
        return;
    wait(sc_core::SC_ZERO_TIME);
    while(true) {
        wait(1_ms);
        auto smsg = tcp4tlm::make_sync_msg(sc_core::sc_time_stamp().value());
        client_connection().write_data(smsg);
    }
}

} // namespace scc
