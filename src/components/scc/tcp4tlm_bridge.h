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

#ifndef TLM_SCC_TCP_BRIDGE_H_
#define TLM_SCC_TCP_BRIDGE_H_

#include "tcp4tlm/client.h"
#include "tcp4tlm/messages.h"
#include "tcp4tlm/server.h"
#include <boost/asio.hpp>
#include <boost/atomic.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <cci_configuration>
#include <scc/report.h>
#include <scc/utilities.h>
#include <systemc>
#include <tlm>
#include <tlm_utils/simple_initiator_socket.h>
#include <tlm_utils/simple_target_socket.h>
#include <tlm_utils/tlm_quantumkeeper.h>

namespace scc {

struct tcp4tlm_bridge : public sc_core::sc_module,
                        protected tcp4tlm::server<request_message, response_message>,
                        protected tcp4tlm::client<request_message, response_message> {
    SC_HAS_PROCESS(tcp4tlm_bridge);

    cci::cci_param<bool> is_connection_master{"is_connection_master", false};
    cci::cci_param<std::string> other_host_name{"other_host_name", ""};
    cci::cci_param<unsigned> other_host_port{"other_host_port", 0};
    cci::cci_param<unsigned> this_host_port{"this_host_port", 0};
    cci::cci_param<bool> limit_simulation_speed{"limit_simulation_speed", true};
    cci::cci_param<bool> write_no_response{"write_no_response", false};
    cci::cci_param<bool> no_systemc_sync{"no_systemc_sync", false};

    tlm_utils::simple_target_socket<tcp4tlm_bridge, ::scc::LT> tsckt;
    tlm_utils::simple_initiator_socket<tcp4tlm_bridge, ::scc::LT> isckt;

    sc_core::sc_vector<sc_core::sc_out<bool>> signals;

    tcp4tlm_bridge(sc_core::sc_module_name name, size_t noOfPorts = 0);

    virtual ~tcp4tlm_bridge();

    typedef tcp4tlm::connection<response_message, request_message> connection_type;
    typedef boost::shared_ptr<connection_type> con_ptr;

    virtual void server_send_completed(con_ptr& con, bool established = false) {
        if(established) {
            con->async_read();
        }
    }

    virtual void server_receive_completed(con_ptr& con, const request_message* const result);

    void initiate_connection(unsigned short retry_count = 0);

    bool is_connection_established() { return con_est; }

    void wait4connection() {
        if(!con_est)
            LOG(TRACE) << "waiting for connection";
        boost::unique_lock<boost::mutex> lock(con_est_mtx);
        while(!con_est) {
            con_est_sig.wait(lock);
        }
    }

    void wait4command() {
        SCCTRACE(SCMOD) << "waiting for command";
        boost::unique_lock<boost::mutex> lock(cmd_rec_mxt);
        cmd_rec_sig.wait(lock);
    }

    void wait4sync() {
        SCCTRACE(SCMOD) << "waiting for sync";
        boost::unique_lock<boost::mutex> lock(sync_mtx);
        sync_sig.wait(lock);
    }

    void end_connection();

protected:
    sc_core::sc_event request_bus_access;
    boost::mutex gp_mtx, con_est_mtx, cmd_rec_mxt, sync_mtx;
    boost::condition_variable gp_sig, con_est_sig, cmd_rec_sig, sync_sig;
    tlm::tlm_generic_payload gp;
    sc_core::sc_time gp_timestamp, gp_timeoffset;
    bool con_est;
    unsigned long long sync;
    void btransport_cb(tlm::tlm_generic_payload&, sc_core::sc_time&);
    unsigned transport_dbg_cb(tlm::tlm_generic_payload&);
    virtual void do_access(tlm::tlm_generic_payload& gp, sc_core::sc_time& delay, bool debug = false);
    void main_thread();
    void timing_thread();
    virtual void end_of_elaboration();
    virtual void start_of_simulation();
    virtual void end_of_simulation();
    boost::shared_ptr<response_message> resp_msg;
    boost::atomic_bool notifyMsgReceived;
    tlm_utils::tlm_quantumkeeper quantumkeeper;

private:
    void init_gp(const bus_op_msg* const msg);
#ifdef BOOST_ASIO_HAS_LOCAL_SOCKETS
    notify_endpoint_msg get_notify_endpoint_msg(boost::asio::local::stream_protocol::endpoint& ep) {
        return notify_endpoint_msg(ep.path(), 0);
    };
#endif
    notify_endpoint_msg get_notify_endpoint_msg(boost::asio::ip::tcp::endpoint& ep) {
        boost::asio::ip::address addr = ep.address();
        return notify_endpoint_msg(addr.is_unspecified() ? boost::asio::ip::host_name() : addr.to_string(), ep.port());
    }
#ifdef GENERATE_STATISTICS
    std::vector<unsigned long> rtto, txt, rxt;

public:
    struct statistics {
        std::vector<unsigned long> histogram;
        unsigned long min, max;
        unsigned long long sum;
        tlm_genip::addr_decoder_if& indexer;
        unsigned long count;
        bool print_histogram;
        statistics(tlm_genip::addr_decoder_if& indexer_, size_t hsize, unsigned long initval)
        : histogram(hsize, 0)
        , min(initval)
        , max(initval)
        , sum(initval)
        , indexer(indexer_)
        , count(1)
        , print_histogram(false) {}
        void updateStat(unsigned long rt);
    };
#endif
};

#ifdef GENERATE_STATISTICS
inline ostream& operator<<(ostream& os, const tcp4tlm_bridge::statistics& stat) {
    if(stat.print_histogram)
        for(size_t i = 0; i < stat.histogram.size() - 1; ++i)
            os << "\t[" << stat.indexer.getBaseAddr(i) / 1000 << "us;" << stat.indexer.getBaseAddr(i + 1) / 1000 << "us)\t-> "
               << stat.histogram[i] << endl;
    else
        os << stat.min << "," << stat.sum / stat.count << "," << stat.max;
    return os;
}
#endif

inline void tcp4tlm_bridge::end_connection() {
    SCCINFO(SCMOD) << "Server ends the connection to Client";
    if(is_client_connected()) {
        sync_msg smsg(sc_core::sc_time_stamp().value());
        client_connection().write_data(smsg);
        response_message* resp;
        client_connection().read_data(resp);
        notify_shutdown_msg msg;
        client_connection().write_data(msg);
    }
    request_shutdown();
}

} // namespace scc

#endif // TLM_SCC_TCP_BRIDGE_H_
