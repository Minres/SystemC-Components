
#define SC_INCLUDE_FX
#include <cxs/cxs_tlm.h>
#include <scc/cci_util.h>
#include <scc/configurer.h>
#include <scc/observer.h>
#include <scc/sc_variable.h>
#include <scc/tracer.h>
#include <string>
#include <systemc>

using namespace sc_dt;
using namespace std;
using namespace cxs;
/*
 * to be tested:
 * PHITSIZE: 256, messages: 4*12bytes, 4*16bytes, 4x20bytes, 2x32bytes, 2x48bytes, 2x64bytes
 */
enum { PHIT_WIDTH = 256 };
struct testbench : public sc_core::sc_module,
                   public tlm::nw::tlm_network_fw_transport_if<cxs_packet_types>,
                   public tlm::nw::tlm_network_bw_transport_if<cxs_packet_types> {

    using transaction_type = cxs_packet_types::tlm_payload_type;
    using phase_type = cxs_packet_types::tlm_phase_type;

    sc_core::sc_clock clk{"clk", 1_ns};
    sc_core::sc_signal<bool> rst{"rst"};
    cxs_pkt_initiator_socket<> isck{"isck"};
    cxs_transmitter<PHIT_WIDTH> tx{"tx"};
    cxs_channel<PHIT_WIDTH> cxs_chan{"cxs_chan"};
    cxs_receiver<PHIT_WIDTH> rx{"rx"};
    cxs_pkt_target_socket<> tsck{"tsck"};

    testbench(sc_core::sc_module_name const& nm)
    : sc_module(nm) {
        isck(*this);
        tsck(*this);
        isck(tx.tsck);
        tx.clk_i(clk);
        tx.isck(cxs_chan.tsck);
        cxs_chan.isck(rx.tsck);
        rx.clk_i(clk);
        rx.rst_i(rst);
        rx.isck(tsck);
        SC_HAS_PROCESS(testbench);
        SC_THREAD(run);
        cxs_chan.channel_delay.set_value(100_ns);
        rx.max_credit.set_value(15);
    }

    void run() {
        rst = true;
        for(auto i = 0u; i < 11u; ++i)
            wait(clk.posedge_event());
        rst = false;
        cxs::cxs_packet_payload trans;
        sc_core::sc_time t = sc_core::SC_ZERO_TIME;
        isck->b_transport(trans, t);
        wait(t);
        cxs_pkt_shared_ptr trans_ptr = cxs_pkt_mm::get().allocate();
        trans_ptr->get_data().resize(16);
        auto phase{tlm::nw::REQUEST};
        auto status = isck->nb_transport_fw(*trans_ptr, phase, t);
        sc_assert(phase == tlm::nw::CONFIRM);
        wait(target_received_evt);
        wait(1_us);
        sc_core::sc_stop();
    }

    void b_transport(transaction_type& trans, sc_core::sc_time& t) override {
        SCCINFO(SCMOD) << "Received blocking transaction at local time " << (sc_core::sc_time_stamp() + t);
    }

    tlm::tlm_sync_enum nb_transport_fw(transaction_type& trans, phase_type& phase, sc_core::sc_time& t) override {
        if(phase == tlm::nw::REQUEST) {
            SCCINFO(SCMOD) << "Received non-blocking transaction with phase " << phase.get_name();
            target_received_evt.notify(sc_core::SC_ZERO_TIME);
            phase = tlm::nw::CONFIRM;
            return tlm::TLM_UPDATED;
        }
        throw std::runtime_error("illegal request in forward path");
    }

    tlm::tlm_sync_enum nb_transport_bw(transaction_type& trans, phase_type& phase, sc_core::sc_time& t) override {
        if(phase == tlm::nw::CONFIRM) {
            confirmation_evt.notify(sc_core::SC_ZERO_TIME);
            return tlm::TLM_ACCEPTED;
        }
        throw std::runtime_error("illegal response in backward path");
    }

    unsigned int transport_dbg(transaction_type& trans) override { return 0; }

    sc_core::sc_event confirmation_evt;
    sc_core::sc_event target_received_evt;
};

int sc_main(int sc_argc, char* sc_argv[]) {
    scc::init_logging(scc::log::DEBUG);
    scc::configurer cfg("");
    scc::tracer trc("cxs_tlm");
    testbench tb("tb");
    sc_core::sc_start();
    SCCINFO("sc_main") << "End Simulation.";

    return sc_core::sc_report_handler::get_count(sc_core::SC_ERROR) + sc_core::sc_report_handler::get_count(sc_core::SC_WARNING);
} // End of 'sc_main'
