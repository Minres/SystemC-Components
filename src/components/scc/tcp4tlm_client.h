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

#ifndef TLM_SCC_TCP4TLM_CLIENT_H_
#define TLM_SCC_TCP4TLM_CLIENT_H_

#include "rigtorp/SPSCQueue.h"
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

struct tcp4tlm_client : public sc_core::sc_module, protected tcp4tlm::client<tcp4tlm::request_message, tcp4tlm::response_message> {
    SC_HAS_PROCESS(tcp4tlm_client);

    cci::cci_param<std::string> other_host_name{"other_host_name", ""};
    cci::cci_param<unsigned> other_host_port{"other_host_port", 0};
    cci::cci_param<unsigned> this_host_port{"this_host_port", 32000u};
    cci::cci_param<bool> wall_time_simulation_speed{"wall_time_simulation_speed", false};
    cci::cci_param<bool> write_no_response{"write_no_response", false};

    tlm_utils::simple_target_socket<tcp4tlm_client, ::scc::LT> tsckt;

    sc_core::sc_vector<sc_core::sc_out<bool>> signals{"signals"};

    tcp4tlm_client(sc_core::sc_module_name name, size_t no_of_ports = 0);

    virtual ~tcp4tlm_client();

    using connection_type = tcp4tlm::connection<tcp4tlm::response_message, tcp4tlm::request_message>;

    void end_connection();

protected:
    void btransport_cb(tlm::tlm_generic_payload&, sc_core::sc_time&);
    unsigned transport_dbg_cb(tlm::tlm_generic_payload&);
    virtual void do_access(tlm::tlm_generic_payload& gp, sc_core::sc_time& delay, bool debug = false);
    void timing_thread();
    void start_of_simulation() override;
    void end_of_simulation() override;
    std::shared_ptr<tcp4tlm::response_message> resp_msg;

private:
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
inline ostream& operator<<(ostream& os, const tcp4tlm_client::statistics& stat) {
    if(stat.print_histogram)
        for(size_t i = 0; i < stat.histogram.size() - 1; ++i)
            os << "\t[" << stat.indexer.getBaseAddr(i) / 1000 << "us;" << stat.indexer.getBaseAddr(i + 1) / 1000 << "us)\t-> "
               << stat.histogram[i] << endl;
    else
        os << stat.min << "," << stat.sum / stat.count << "," << stat.max;
    return os;
}
#endif

inline void tcp4tlm_client::end_connection() {
    SCCINFO(SCMOD) << "Sending shutdown message";
    if(is_remote_connected()) {
        auto smsg = tcp4tlm::make_sync_msg(sc_core::sc_time_stamp().value());
        client_connection().write_data(smsg);
        auto msg = tcp4tlm::make_notify_shutdown_msg();
        client_connection().write_data(msg);
    }
}

} // namespace scc

#endif // TLM_SCC_TCP4TLM_CLIENT_H_
