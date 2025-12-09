#ifndef _TESTBENCH_H_
#define _TESTBENCH_H_

#include <axi/pe/axi_initiator.h>
#include <axi/pe/simple_target.h>
#include <axi/pin/axi4_initiator.h>
#include <axi/pin/axi4_target.h>
#include <axi/scv/recorder_modules.h>
#include <scc.h>

using namespace sc_core;
using namespace axi;
using namespace axi::pe;

class testbench : public sc_core::sc_module {
public:
    using bus_cfg = axi::axi4_cfg</*BUSWIDTH=*/64, /*ADDRWIDTH=*/32, /*IDWIDTH=*/4, /*USERWIDTH=*/1>;

    sc_core::sc_time clk_period{10, sc_core::SC_NS};
    sc_core::sc_clock clk{"clk", clk_period, 0.5, sc_core::SC_ZERO_TIME, true};
    sc_core::sc_signal<bool> rst{"rst"};
    // initiator side
    axi::axi_initiator_socket<bus_cfg::BUSWIDTH> intor{"intor"};
    axi::scv::axi_recorder_module<bus_cfg::BUSWIDTH> intor_rec{"intor_rec"};
    axi::pin::axi4_initiator<bus_cfg> intor_bfm{"intor_bfm"};
    // signal accurate bus
    axi::aw_axi<bus_cfg, axi::signal_types> aw;
    axi::wdata_axi<bus_cfg, axi::signal_types> wdata;
    axi::b_axi<bus_cfg, axi::signal_types> b;
    axi::ar_axi<bus_cfg, axi::signal_types> ar;
    axi::rresp_axi<bus_cfg, axi::signal_types> rresp;
    axi::pin::axi4_target<bus_cfg> tgt_bfm{"tgt_bfm"};
    // target side
    axi::scv::axi_recorder_module<bus_cfg::BUSWIDTH> tgt_rec{"tgt_rec"};
    axi::axi_target_socket<bus_cfg::BUSWIDTH> tgt{"tgt"};
    // engines
    axi::pe::axi_initiator<bus_cfg::BUSWIDTH> intor_pe;
    axi::pe::simple_target<bus_cfg::BUSWIDTH> tgt_pe;

public:
    SC_HAS_PROCESS(testbench);
    testbench()
    : testbench("testbench") {}
    testbench(sc_core::sc_module_name nm)
    : sc_core::sc_module(nm)
    , intor_pe("intor_pe", intor)
    , tgt_pe("tgt_pe", tgt) {
        intor_pe.clk_i(clk);
        intor_bfm.clk_i(clk);
        tgt_bfm.clk_i(clk);
        tgt_pe.clk_i(clk);
        // pe socket to recorder
        intor(intor_rec.tsckt);
        // recorder to bfm
        intor_rec.isckt(intor_bfm.tsckt);
        // bfm to signals
        intor_bfm.bind_aw(aw);
        intor_bfm.bind_w(wdata);
        intor_bfm.bind_b(b);
        intor_bfm.bind_ar(ar);
        intor_bfm.bind_r(rresp);
        // signals to bfm
        tgt_bfm.bind_aw(aw);
        tgt_bfm.bind_w(wdata);
        tgt_bfm.bind_b(b);
        tgt_bfm.bind_ar(ar);
        tgt_bfm.bind_r(rresp);
        // bfm to recorder
        tgt_bfm.isckt(tgt_rec.tsckt);
        // recorder to target
        tgt_rec.isckt(tgt);
    }

    void run1() {}
};

#endif // _TESTBENCH_H_
