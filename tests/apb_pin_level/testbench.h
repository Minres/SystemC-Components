#ifndef _TESTBENCH_H_
#define _TESTBENCH_H_

#include <apb/pin/initiator.h>
#include <apb/pin/target.h>
#include <scc.h>

using namespace sc_core;

class testbench : public sc_core::sc_module {
public:
    enum { DATA_WIDTH = 32, ADDR_WIDTH = 32 };
    using addr_t = typename apb::pin::initiator<DATA_WIDTH, ADDR_WIDTH>::addr_t;
    using data_t = apb::pin::initiator<DATA_WIDTH, ADDR_WIDTH>::data_t;
    using strb_t = sc_dt::sc_uint<DATA_WIDTH / 8>;
    sc_core::sc_time clk_period{10, sc_core::SC_NS};
    sc_core::sc_clock clk{"clk", clk_period, 0.5, sc_core::SC_ZERO_TIME, true};
    sc_core::sc_signal<bool> rst_n{"rst_n"};
    // initiator side
    tlm::scc::initiator_mixin<tlm::tlm_initiator_socket<DATA_WIDTH>> isck{"isck"};
    apb::pin::initiator<DATA_WIDTH, ADDR_WIDTH> intor_bfm{"intor_bfm"};
    // signal accurate bus
    sc_core::sc_signal<addr_t> PADDR{"PADDR"};
    sc_core::sc_signal<sc_dt::sc_uint<3>> PPROT{"PPROT"};
    sc_core::sc_signal<bool> PNSE{"PNSE"};
    sc_core::sc_signal<bool> PSELx{"PSELx"};
    sc_core::sc_signal<bool> PENABLE{"PENABLE"};
    sc_core::sc_signal<bool> PWRITE{"PWRITE"};
    sc_core::sc_signal<data_t> PWDATA{"PWDATA"};
    sc_core::sc_signal<strb_t> PSTRB{"PSTRB"};
    sc_core::sc_signal<bool> PREADY{"PREADY"};
    sc_core::sc_signal<data_t> PRDATA{"PRDATA"};
    sc_core::sc_signal<bool> PSLVERR{"PSLVERR"};
    sc_core::sc_signal<bool> PWAKEUP{"PWAKEUP"};
    // target side
    apb::pin::target<DATA_WIDTH, ADDR_WIDTH> tgt_bfm{"tgt_bfm"};
    tlm::scc::target_mixin<tlm::tlm_target_socket<scc::LT>> tsck{"tsck"};

public:
    SC_HAS_PROCESS(testbench);
    testbench()
    : testbench("testbench") {}
    testbench(sc_core::sc_module_name nm)
    : sc_core::sc_module(nm) {
        intor_bfm.PCLK_i(clk);
        tgt_bfm.PCLK_i(clk);
        // bfm to signals
        isck(intor_bfm.tsckt);
        intor_bfm.PRESETn_i(rst_n);
        intor_bfm.PADDR_o(PADDR);
        intor_bfm.PPROT_o(PPROT);
        intor_bfm.PNSE_o(PNSE);
        intor_bfm.PSELx_o(PSELx);
        intor_bfm.PENABLE_o(PENABLE);
        intor_bfm.PWRITE_o(PWRITE);
        intor_bfm.PWDATA_o(PWDATA);
        intor_bfm.PSTRB_o(PSTRB);
        intor_bfm.PREADY_i(PREADY);
        intor_bfm.PRDATA_i(PRDATA);
        intor_bfm.PSLVERR_i(PSLVERR);
        intor_bfm.PWAKEUP_o(PWAKEUP);

        tgt_bfm.PRESETn_i(rst_n);
        tgt_bfm.PADDR_i(PADDR);
        tgt_bfm.PPROT_i(PPROT);
        tgt_bfm.PNSE_i(PNSE);
        tgt_bfm.PSELx_i(PSELx);
        tgt_bfm.PENABLE_i(PENABLE);
        tgt_bfm.PWRITE_i(PWRITE);
        tgt_bfm.PWDATA_i(PWDATA);
        tgt_bfm.PSTRB_i(PSTRB);
        tgt_bfm.PREADY_o(PREADY);
        tgt_bfm.PRDATA_o(PRDATA);
        tgt_bfm.PSLVERR_o(PSLVERR);
        tgt_bfm.PWAKEUP_i(PWAKEUP);

        tgt_bfm.isckt(tsck);
        tsck.register_b_transport([this](tlm::tlm_base_protocol_types::tlm_payload_type& trans, sc_core::sc_time& d) {
            if(cb_delegate)
                cb_delegate(trans, d);
        });
    }

    void run1() {}
    void register_b_transport(std::function<void(tlm::tlm_generic_payload&, sc_core::sc_time&)> cb) { cb_delegate = cb; }
    std::function<void(tlm::tlm_generic_payload&, sc_core::sc_time&)> cb_delegate;
};

#endif // _TESTBENCH_H_
