#ifndef _TESTBENCH_H_
#define _TESTBENCH_H_

#include <ahb/pin/initiator.h>
#include <ahb/pin/target.h>
#include <scc.h>

using namespace sc_core;

class testbench : public sc_core::sc_module {
public:
    enum { DWIDTH = 32 };
    sc_core::sc_time clk_period{10, sc_core::SC_NS};
    sc_core::sc_clock clk{"clk", clk_period, 0.5, sc_core::SC_ZERO_TIME, true};
    sc_core::sc_signal<bool> rst_n{"rst_n"};
    // initiator side
    tlm::scc::initiator_mixin<tlm::tlm_initiator_socket<DWIDTH>> isck{"isck"};
    ahb::pin::initiator<DWIDTH> intor_bfm{"intor_bfm"};
    // signal accurate bus
    sc_core::sc_signal<sc_dt::sc_uint<32>> HADDR{"HADDR"};
    sc_core::sc_signal<sc_dt::sc_uint<3>> HBURST{"HBURST"};
    sc_core::sc_signal<bool> HMASTLOCK{"HMASTLOCK"};
    sc_core::sc_signal<sc_dt::sc_uint<4>> HPROT{"HPROT"};
    sc_core::sc_signal<sc_dt::sc_uint<3>> HSIZE{"HSIZE"};
    sc_core::sc_signal<sc_dt::sc_uint<2>> HTRANS{"HTRANS"};
    sc_core::sc_signal<sc_dt::sc_uint<DWIDTH>> HWDATA{"HWDATA"};
    sc_core::sc_signal<bool> HWRITE{"HWRITE"};
    sc_core::sc_signal<sc_dt::sc_uint<DWIDTH>> HRDATA{"HRDATA"};
    sc_core::sc_signal<bool> HREADY{"HREADY"};
    sc_core::sc_signal<bool> HRESP{"HRESP"};
    sc_core::sc_signal<bool> HSEL{"HSEL"};
    // target side
    ahb::pin::target<DWIDTH, 32> tgt_bfm{"tgt_bfm"};
    tlm::scc::target_mixin<tlm::tlm_target_socket<scc::LT>> tsck{"tsck"};

public:
    SC_HAS_PROCESS(testbench);
    testbench()
    : testbench("testbench") {}
    testbench(sc_core::sc_module_name nm)
    : sc_core::sc_module(nm) {
        intor_bfm.HCLK_i(clk);
        tgt_bfm.HCLK_i(clk);
        // bfm to signals
        isck(intor_bfm.tsckt);
        intor_bfm.HRESETn_i(rst_n);
        intor_bfm.HADDR_o(HADDR);
        intor_bfm.HBURST_o(HBURST);
        intor_bfm.HMASTLOCK_o(HMASTLOCK);
        intor_bfm.HPROT_o(HPROT);
        intor_bfm.HSIZE_o(HSIZE);
        intor_bfm.HTRANS_o(HTRANS);
        intor_bfm.HWDATA_o(HWDATA);
        intor_bfm.HWRITE_o(HWRITE);
        intor_bfm.HRDATA_i(HRDATA);
        intor_bfm.HREADY_i(HREADY);
        intor_bfm.HRESP_i(HRESP);
        // signals to bfm
        tgt_bfm.HRESETn_i(rst_n);
        tgt_bfm.HADDR_i(HADDR);
        tgt_bfm.HBURST_i(HBURST);
        tgt_bfm.HMASTLOCK_i(HMASTLOCK);
        tgt_bfm.HPROT_i(HPROT);
        tgt_bfm.HSIZE_i(HSIZE);
        tgt_bfm.HTRANS_i(HTRANS);
        tgt_bfm.HWDATA_i(HWDATA);
        tgt_bfm.HWRITE_i(HWRITE);
        tgt_bfm.HSEL_i(HSEL);
        tgt_bfm.HRDATA_o(HRDATA);
        tgt_bfm.HREADY_o(HREADY);
        tgt_bfm.HRESP_o(HRESP);
        tgt_bfm.isckt(tsck);
    }

    void run1() {}
};

#endif // _TESTBENCH_H_
