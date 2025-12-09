#ifndef _TESTBENCH_H_
#define _TESTBENCH_H_

#include <cxs/cxs_tlm.h>
#include <cxs/scv/tlm_recording.h>
#include <scc/cci_util.h>
#include <scc/configurer.h>
#include <scc/fifo_w_cb.h>
#include <scc/observer.h>
#include <scc/sc_variable.h>
#include <scc/tracer.h>
#include <systemc>
#include <tlm/nw/initiator_mixin.h>
#include <tlm/nw/target_mixin.h>

using namespace sc_core;
using namespace sc_dt;
using namespace std;
namespace cxs {

const char* sc_gen_unique_name(const char*, bool preserve_first);
template <unsigned PHIT_WIDTH> struct testbench : public sc_core::sc_module {

    using transaction_type = cxs_packet_types::tlm_payload_type;
    using phase_type = cxs_packet_types::tlm_phase_type;

    sc_core::sc_clock clk{"clk", 1_ns};
    sc_core::sc_signal<bool> rst{"rst"};
    tlm::nw::initiator_mixin<cxs_pkt_initiator_socket<>, cxs_packet_types> isck{"isck"};
    cxs_transmitter<PHIT_WIDTH> tx{"tx"};
    tlm::nw::scv::tlm_recorder_module<CXS_CMD, PHIT_WIDTH, cxs_flit_types> tx_rec{"tx_rec"};
    cxs_channel<PHIT_WIDTH> cxs_chan{"cxs_chan"};
    tlm::nw::scv::tlm_recorder_module<CXS_CMD, PHIT_WIDTH, cxs_flit_types> rx_rec{"rx_rec"};
    cxs_receiver<PHIT_WIDTH> rx{"rx"};
    tlm::nw::target_mixin<cxs_pkt_target_socket<>, false, cxs_packet_types> tsck{"tsck"};

    testbench()
    : testbench(sc_core::sc_gen_unique_name("testbench", false)) {}

    testbench(sc_core::sc_module_name const& nm)
    : sc_module(nm) {
        isck.register_nb_transport_bw(
            [this](transaction_type& trans, phase_type& phase, sc_core::sc_time& t) { return this->nb_transport_fw(trans, phase, t); });
        tsck.register_nb_transport_fw(
            [this](transaction_type& trans, phase_type& phase, sc_core::sc_time& t) { return this->nb_transport_fw(trans, phase, t); });
        isck(tx.tsck);
        tx.clk_i(clk);
        tx.rst_i(rst);
        cxs_chan.tx_clk_i(clk);
        cxs_chan.rx_clk_i(clk);
#if 0
        tx.isck(cxs_chan.tsck);
        cxs_chan.isck(rx.tsck);
#else
        tx.isck(tx_rec.ts);
        tx_rec.is(cxs_chan.tsck);
        cxs_chan.isck(rx_rec.ts);
        rx_rec.is(rx.tsck);
#endif
        rx.clk_i(clk);
        rx.rst_i(rst);
        rx.isck(tsck);
        tx.clock_period.set_value(1_ns);
        cxs_chan.tx_clock_period.set_value(1_ns);
        cxs_chan.channel_delay.set_value(100_ns);
        cxs_chan.rx_clock_period.set_value(1_ns);
        rx.clock_period.set_value(1_ns);
        rx.max_credit.set_value(15);
    }

    tlm::tlm_sync_enum nb_transport_fw(transaction_type& trans, phase_type& phase, sc_core::sc_time& t) {
        if(phase == tlm::nw::REQUEST) {
            SCCDEBUG(SCMOD) << "Received non-blocking transaction with phase " << phase.get_name();
            recv.push_back(&trans);
            phase = tlm::nw::CONFIRM;
            return tlm::TLM_UPDATED;
        }
        throw std::runtime_error("illegal request in forward path");
    }

    tlm::tlm_sync_enum nb_transport_bw(transaction_type& trans, phase_type& phase, sc_core::sc_time& t) {
        if(phase == tlm::nw::CONFIRM) {
            confirmation_evt.notify(sc_core::SC_ZERO_TIME);
            return tlm::TLM_ACCEPTED;
        }
        throw std::runtime_error("illegal response in backward path");
    }

    sc_core::sc_event confirmation_evt;
    scc::fifo_w_cb<cxs_pkt_shared_ptr> recv;
};
} // namespace cxs
#endif // _TESTBENCH_H_
