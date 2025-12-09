#ifndef _TESTBENCH_H_
#define _TESTBENCH_H_

#include <axi/pe/axi_initiator.h>
#include <axi/pe/simple_ace_target.h>
#include <axi/pe/simple_target.h>
#include <axi/pin/ace_initiator.h>
#include <axi/pin/ace_target.h>
#include <axi/scv/recorder_modules.h>
#include <scc.h>

using namespace sc_core;
using namespace axi;
using namespace axi::pe;

class testbench : public sc_core::sc_module, public tlm::scc::pe::intor_bw_b {
public:
    using bus_cfg = axi::ace_cfg</*BUSWIDTH=*/64, /*ADDRWIDTH=*/32, /*IDWIDTH=*/4, /*USERWIDTH=*/1, /*CACHELINE*/ 64>;

    sc_core::sc_time clk_period{10, sc_core::SC_NS};
    sc_core::sc_clock clk{"clk", clk_period, 0.5, sc_core::SC_ZERO_TIME, true};
    sc_core::sc_signal<bool> rst{"rst"};
    // test interface, which is bound to initiator bw_o
    sc_core::sc_export<tlm::scc::pe::intor_bw_b> bw_i{"bw_i"};
    // initiator side
    axi::ace_initiator_socket<bus_cfg::BUSWIDTH> intor{"ace_intor"};
    axi::pin::ace_initiator<bus_cfg> intor_bfm{"ace_intor_bfm"};
    // signal accurate bus
    axi::aw_ace<bus_cfg, axi::signal_types> aw;
    axi::wdata_ace<bus_cfg, axi::signal_types> wdata;
    axi::b_ace<bus_cfg, axi::signal_types> b;
    axi::ar_ace<bus_cfg, axi::signal_types> ar;
    axi::rresp_ace<bus_cfg, axi::signal_types> rresp;
    axi::ac_ace<bus_cfg, axi::signal_types> ac;
    axi::cr_ace<bus_cfg, axi::signal_types> cr;
    axi::cd_ace<bus_cfg, axi::signal_types> cd;

    axi::pin::ace_target<bus_cfg> tgt_bfm{"ace_tgt_bfm"};
    // target side
    axi::ace_target_socket<bus_cfg::BUSWIDTH> tgt_ace{"tgt_ace"};
    axi::axi_target_socket<bus_cfg::BUSWIDTH> tgt_axi{"tgt_axi"};
    // engines
    axi::pe::ace_initiator<bus_cfg::BUSWIDTH> intor_pe;
    axi::pe::simple_target<bus_cfg::BUSWIDTH> axi_tgt_pe;
    axi::pe::simple_ace_target<bus_cfg::BUSWIDTH> ace_tgt_pe;

public:
    SC_HAS_PROCESS(testbench);
    testbench()
    : testbench("testbench") {}
    testbench(sc_core::sc_module_name nm)
    : sc_core::sc_module(nm)
    , intor_pe("ace_intor_pe", intor)
    , ace_tgt_pe("ace_tgt_pe", tgt_ace)
    , axi_tgt_pe("axi_tgt_pe", tgt_axi) {

        bw_i.bind(*this);
        intor_pe.clk_i(clk);
        intor_bfm.clk_i(clk);
        tgt_bfm.clk_i(clk);
        axi_tgt_pe.clk_i(clk);
        ace_tgt_pe.clk_i(clk);
        // pe socket to recorder
        intor(intor_bfm.tsckt);

        // bfm to signals
        intor_bfm.bind_aw(aw);
        intor_bfm.bind_w(wdata);
        intor_bfm.bind_b(b);
        intor_bfm.bind_ar(ar);
        intor_bfm.bind_r(rresp);

        intor_bfm.bind_ac(ac);
        intor_bfm.bind_cr(cr);
        intor_bfm.bind_cd(cd);

        // signals to bfm
        tgt_bfm.bind_aw(aw);
        tgt_bfm.bind_w(wdata);
        tgt_bfm.bind_b(b);
        tgt_bfm.bind_ar(ar);
        tgt_bfm.bind_r(rresp);

        tgt_bfm.bind_ac(ac);
        tgt_bfm.bind_cr(cr);
        tgt_bfm.bind_cd(cd);

        // bfm to ace target
        tgt_bfm.isckt(tgt_ace);
        ace_tgt_pe.isckt_axi(tgt_axi);
        // for updating snooop transaction
        intor_pe.bw_o(bw_i);
    }

    unsigned transport(tlm::tlm_generic_payload& trans) override {
        if(transport_cb)
            return transport_cb(trans);
        else
            return 0;
    }
    std::function<unsigned(tlm::tlm_generic_payload&)> transport_cb;
};

#endif // _TESTBENCH_H_
