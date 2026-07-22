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

#ifndef TLM_SCC_TCP4TLM_BRIDGE_H_
#define TLM_SCC_TCP4TLM_BRIDGE_H_

#include "rigtorp/SPSCQueue.h"
#include "scc/async_event.h"
#include "scc/async_queue.h"
#include "scc/peq.h"
#include "tcp4tlm/client.h"
#include "tcp4tlm/messages.h"
#include "tcp4tlm/server.h"
#include "tlm/scc/tlm_gp_shared.h"
#include "tlm/scc/tlm_mm.h"
#include <atomic>
#include <boost/asio.hpp>
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
                        protected tcp4tlm::server<tcp4tlm::request_message, tcp4tlm::response_message>,
                        protected tcp4tlm::client<tcp4tlm::request_message, tcp4tlm::response_message> {
    SC_HAS_PROCESS(tcp4tlm_bridge);

    /*! this governs the sequence of connection:
     * - if false: it first starts the server, connects to remote and sends a NotifyEndpointMsg
     * - if true:  it starts the server, waits for a NotifyEndpointMsg and then connects the client
     */
    cci::cci_param<bool> is_connection_server{"is_connection_server", false};
    cci::cci_param<std::string> other_host_name{"other_host_name", ""};
    cci::cci_param<unsigned> other_host_port{"other_host_port", 0};
    cci::cci_param<unsigned> this_host_port{"this_host_port", 32000u};
    cci::cci_param<bool> wall_time_simulation_speed{"wall_time_simulation_speed", false};
    cci::cci_param<bool> write_no_response{"write_no_response", false};
    cci::cci_param<bool> no_systemc_sync{"no_systemc_sync", false};

    tlm_utils::simple_target_socket<tcp4tlm_bridge, ::scc::LT> tsckt;

    tlm_utils::simple_initiator_socket<tcp4tlm_bridge, ::scc::LT> isckt;

    sc_core::sc_vector<sc_core::sc_out<bool>> signals{"signals"};

    tcp4tlm_bridge(sc_core::sc_module_name name, size_t no_of_ports = 0);

    virtual ~tcp4tlm_bridge();

    typedef tcp4tlm::connection<tcp4tlm::response_message, tcp4tlm::request_message> connection_type;
    typedef std::shared_ptr<connection_type> con_ptr;

    void server_send_completed(con_ptr& con, bool established = false) override {
        if(established) {
            con->async_read();
        }
    }

    void server_receive_completed(con_ptr& con, const tcp4tlm::request_message* const result) override;

    void start_server() {
        using namespace std::chrono_literals;
        server::start_server(this_host_port.get_value());
    }

    void wait4connection() {
        using namespace std::chrono_literals;
        // wait until con_ext becomes true. To mitigate lost notifications the wait_for wuns in a loop
        std::unique_lock<std::mutex> lock(con_est_mtx);
        while(!con_est.load())
            con_est_sig.wait_for(lock, 100ms, [this]() { return con_est.load(); });
    }

    bool is_connection_established() { return con_est; }

    void end_connection();

    sc_core::sc_event const& get_shutdown_event() const { return shutdown_evt; }

protected:
    using callback_task = std::packaged_task<bool(void)>;
    struct timed_task {
        callback_task t;
        sc_core::sc_time timepoint;
    };
    scc::async_queue<timed_task> task_que;
    scc::peq<callback_task> timed_task_que;
    std::mutex con_est_mtx; //, gp_mtx, sync_mtx;
    std::condition_variable con_est_sig;
    std::atomic<bool> con_est{false};
    sc_core::sc_event shutdown_evt;
    rigtorp::SPSCQueue<sc_core::sc_time> next_time_stamp;
    unsigned long long sync;
    void btransport_cb(tlm::tlm_generic_payload&, sc_core::sc_time&);
    unsigned transport_dbg_cb(tlm::tlm_generic_payload&);
    virtual void do_access(tlm::tlm_generic_payload& gp, sc_core::sc_time& delay, bool debug = false);
    void timing_thread();
    void process_task_que();
    void process_timed_task_que();
    void end_of_elaboration() override;
    void start_of_simulation() override;
    void end_of_simulation() override;
    std::shared_ptr<tcp4tlm::response_message> resp_msg;
    tlm_utils::tlm_quantumkeeper quantumkeeper;
    tlm::scc::tlm_mm<tlm::tlm_base_protocol_types, false> mm;

private:
    tlm::scc::tlm_gp_shared_ptr init_gp(const tcp4tlm::BusOpMsg* const msg);
#ifdef BOOST_ASIO_HAS_LOCAL_SOCKETS
    tcp4tlm::request_message get_notify_endpoint_msg(boost::asio::local::stream_protocol::endpoint& ep) {
        return tcp4tlm::make_notify_endpoint_msg(ep.path(), 0);
    };
#endif
    tcp4tlm::request_message get_notify_endpoint_msg(boost::asio::ip::tcp::endpoint& ep) {
        boost::asio::ip::address addr = ep.address();
        return tcp4tlm::make_notify_endpoint_msg(addr.is_unspecified() ? boost::asio::ip::host_name() : addr.to_string(), ep.port());
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
    SCCINFO(SCMOD) << "Sending shutdown message";
    if(is_remote_connected()) {
        auto smsg = tcp4tlm::make_sync_msg(sc_core::sc_time_stamp().value());
        client_connection().write_data(smsg);
        auto msg = tcp4tlm::make_notify_shutdown_msg();
        client_connection().write_data(msg);
    }
}

} // namespace scc

#endif // TLM_SCC_TCP4TLM_BRIDGE_H_
