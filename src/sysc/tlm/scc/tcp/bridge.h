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

#include <boost/asio.hpp>
#include <cci_configuration>
#include <systemc>
#include <tlm.h>
#include <tlm_utils/simple_initiator_socket.h>
#include <tlm_utils/simple_target_socket.h>
#include <tlm_utils/tlm_quantumkeeper.h>
//#include <addr_decoder_ext.h>
#include <scc/report.h>
#include <vector>

#include <boost/atomic.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>

#include "impl/messages.h"

#include "impl/Client.h"
#include "impl/Server.h"
namespace tlm {
namespace scc {
namespace tcp {

struct tcp4tlm_bridge : public sc_core::sc_module,
                protected impl::Server<RequestMessage, ResponseMessage>,
                protected impl::Client<RequestMessage, ResponseMessage> {

    SC_HAS_PROCESS(bridge);

    cci::cci_param<bool> isConnectionMaster{"isConnectionMaster", false};
    cci::cci_param<std::string> otherHostName{"otherHostName", ""};
    cci::cci_param<unsigned> otherHostPort{"otherHostPort", 0};
    cci::cci_param<unsigned> thisHostPort{"thisHostPort", 0};
    cci::cci_param<bool> limitSimulationSpeed{"limitSimulationSpeed", true};
    cci::cci_param<bool> writeNoResponse{"writeNoResponse", false};
    cci::cci_param<bool> noSystemcSync{"noSystemcSync", false};

    tlm_utils::simple_target_socket<tcp4tlm_bridge> tsckt;
    tlm_utils::simple_initiator_socket<tcp4tlm_bridge> isckt;

    sc_core::sc_vector<sc_core::sc_out<bool>> signals;

    tcp4tlm_bridge(sc_core::sc_module_name name, size_t noOfPorts = 0);

    virtual ~tcp4tlm_bridge();

    typedef impl::connection<ResponseMessage, RequestMessage> connection_type;
    typedef boost::shared_ptr<connection_type> con_ptr;

    virtual void serverSendCompleted(con_ptr& con, bool established = false) {
        // if connection is just established wait for something from the client
        if(established) {
            con->async_read();
        }
    }

    virtual void serverReceiveCompleted(con_ptr& con, const RequestMessage* const result);

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

    void endConnection();

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
    boost::shared_ptr<ResponseMessage> resp_msg;
    boost::atomic_bool notifyMsgReceived;
    tlm_utils::tlm_quantumkeeper quantumkeeper;

private:
    void init_gp(const BusOpMsg* const msg);
#ifdef BOOST_ASIO_HAS_LOCAL_SOCKETS
    NotifyEndpointMsg getNotifyEndpointMsg(boost::asio::local::stream_protocol::endpoint& ep) { return NotifyEndpointMsg(ep.path(), 0); };
#endif
    NotifyEndpointMsg getNotifyEndpointMsg(boost::asio::ip::tcp::endpoint& ep) {
        boost::asio::ip::address addr = ep.address();
        return NotifyEndpointMsg(addr.is_unspecified() ? boost::asio::ip::host_name() : addr.to_string(), ep.port());
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
inline ostream& operator<<(ostream& os, const bridge::statistics& stat) {
    if(stat.print_histogram)
        for(size_t i = 0; i < stat.histogram.size() - 1; ++i)
            os << "\t[" << stat.indexer.getBaseAddr(i) / 1000 << "us;" << stat.indexer.getBaseAddr(i + 1) / 1000 << "us)\t-> "
               << stat.histogram[i] << endl;
    else
        os << stat.min << "," << stat.sum / stat.count << "," << stat.max;
    return os;
}
#endif

inline void tcp4tlm_bridge::endConnection() {
    SCCINFO(SCMOD) << "Server ends the connection to Client";
    if(isClientConnected()) {
        SyncMsg smsg(sc_core::sc_time_stamp().value());
        clientConnection().write_data(smsg);
        ResponseMessage* resp;
        clientConnection().read_data(resp);
        NotifyShutdownMsg msg;
        clientConnection().write_data(msg);
    }
    requestShutdown();
}
} // namespace tcp
} // namespace scc
} // namespace tlm

#endif // TLM_SCC_TCP_BRIDGE_H_
