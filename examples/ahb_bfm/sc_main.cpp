#include <ahb/pin/initiator.h>
#include <ahb/pin/target.h>
#include <cci_configuration>
#include <fstream>
#include <scc/configurable_tracer.h>
#include <scc/configurer.h>
#include <scc/report.h>
#include <scc/traceable.h>
#include <scc/tracer.h>
#include <tlm/scc/initiator_mixin.h>
#include <tlm/scc/target_mixin.h>

using namespace sc_core;
using namespace scc;

class testbench : public sc_module, public scc::traceable {
public:
    enum { WIDTH = 64 };
    tlm::scc::initiator_mixin<tlm::tlm_initiator_socket<WIDTH>> isck{"isck"};
    ahb::pin::initiator<WIDTH> intor{"intor"};
    sc_core::sc_clock HCLK{"HCLK", 10_ns};
    sc_core::sc_signal<bool> HRESETn{"HRESETn"};
    sc_core::sc_signal<sc_dt::sc_uint<32>> HADDR{"HADDR"};
    sc_core::sc_signal<sc_dt::sc_uint<3>> HBURST{"HBURST"};
    sc_core::sc_signal<bool> HMASTLOCK{"HMASTLOCK"};
    sc_core::sc_signal<sc_dt::sc_uint<4>> HPROT{"HPROT"};
    sc_core::sc_signal<sc_dt::sc_uint<3>> HSIZE{"HSIZE"};
    sc_core::sc_signal<sc_dt::sc_uint<2>> HTRANS{"HTRANS"};
    sc_core::sc_signal<sc_dt::sc_uint<WIDTH>> HWDATA{"HWDATA"};
    sc_core::sc_signal<bool> HWRITE{"HWRITE"};
    sc_core::sc_signal<sc_dt::sc_uint<WIDTH>> HRDATA{"HRDATA"};
    sc_core::sc_signal<bool> HREADY{"HREADY"};
    sc_core::sc_signal<bool> HRESP{"HRESP"};
    sc_core::sc_signal<bool> HSEL{"HSEL"};

    ahb::pin::target<WIDTH, 32> target{"target"};
    tlm::scc::target_mixin<tlm::tlm_target_socket<scc::LT>> tsck{"tsck"};

    testbench(sc_module_name nm)
    : sc_module(nm) {
        SC_HAS_PROCESS(testbench);
        isck(intor.tsckt);
        intor.HCLK_i(HCLK);
        intor.HRESETn_i(HRESETn);
        intor.HADDR_o(HADDR);
        intor.HBURST_o(HBURST);
        intor.HMASTLOCK_o(HMASTLOCK);
        intor.HPROT_o(HPROT);
        intor.HSIZE_o(HSIZE);
        intor.HTRANS_o(HTRANS);
        intor.HWDATA_o(HWDATA);
        intor.HWRITE_o(HWRITE);
        intor.HRDATA_i(HRDATA);
        intor.HREADY_i(HREADY);
        intor.HRESP_i(HRESP);
        target.HCLK_i(HCLK);
        target.HRESETn_i(HRESETn);
        target.HADDR_i(HADDR);
        target.HBURST_i(HBURST);
        target.HMASTLOCK_i(HMASTLOCK);
        target.HPROT_i(HPROT);
        target.HSIZE_i(HSIZE);
        target.HTRANS_i(HTRANS);
        target.HWDATA_i(HWDATA);
        target.HWRITE_i(HWRITE);
        target.HSEL_i(HSEL);
        target.HRDATA_o(HRDATA);
        target.HREADY_o(HREADY);
        target.HRESP_o(HRESP);
        target.isckt(tsck);
        SC_THREAD(run);
        tsck.register_b_transport([this](tlm::tlm_generic_payload& gp, sc_time& delay) {
            gp.set_response_status(tlm::TLM_OK_RESPONSE);
            if(gp.is_write()) {
                SCCDEBUG(SCMOD) << "Received write access to addr 0x" << std::hex << gp.get_address();
            } else {
                memset(gp.get_data_ptr(), 0x55, gp.get_data_length());
                SCCDEBUG(SCMOD) << "Received read access from addr 0x" << std::hex << gp.get_address();
            }
            wait(HCLK.posedge_event());
            wait(HCLK.posedge_event());
        });
    }

    void run() {
        HRESETn.write(false);
        for(size_t i = 0; i < 10; ++i)
            wait(HCLK.posedge_event());
        HRESETn.write(true);
        wait(HCLK.posedge_event());
        HSEL.write(true);
        tlm::tlm_generic_payload gp;
        std::array<uint8_t, 8> data;
        data[0] = 2;
        data[1] = 4;
        sc_time delay;
        gp.set_address(0x1000);
        gp.set_data_length(8);
        gp.set_data_ptr(data.data());
        gp.set_streaming_width(8);
        gp.set_command(tlm::TLM_WRITE_COMMAND);
        delay = SC_ZERO_TIME;
        SCCDEBUG(SCMOD) << "Starting write access to addr 0x" << std::hex << gp.get_address();
        isck->b_transport(gp, delay);
        gp.set_address(0x1020);
        gp.set_data_length(8);
        gp.set_data_ptr(data.data());
        gp.set_streaming_width(8);
        gp.set_command(tlm::TLM_READ_COMMAND);
        delay = SC_ZERO_TIME;
        SCCDEBUG(SCMOD) << "Starting read access from addr 0x" << std::hex << gp.get_address();
        isck->b_transport(gp, delay);
        for(size_t i = 0; i < 10; ++i)
            wait(HCLK.posedge_event());
        sc_stop();
    }
};

int sc_main(int argc, char* argv[]) {
    sc_core::sc_report_handler::set_actions("/IEEE_Std_1666/deprecated", sc_core::SC_DO_NOTHING);
    sc_report_handler::set_actions(SC_ID_MORE_THAN_ONE_SIGNAL_DRIVER_, SC_DO_NOTHING);
    ///////////////////////////////////////////////////////////////////////////
    // configure logging
    ///////////////////////////////////////////////////////////////////////////
    scc::init_logging(LogConfig().logLevel(scc::log::DEBUG).logAsync(false));
    ///////////////////////////////////////////////////////////////////////////
    // set up configuration and tracing
    ///////////////////////////////////////////////////////////////////////////
    scc::configurer cfg("ahb_bfm.json");
    scc::configurable_tracer trace("ahb_bfm", true, true, true);
    ///////////////////////////////////////////////////////////////////////////
    // create modules/channels and trace
    ///////////////////////////////////////////////////////////////////////////
    testbench tb("tb");
    cfg.configure();
    cfg.dump_configuration("ahb_test.default.json");
    ///////////////////////////////////////////////////////////////////////////
    // run the simulation
    ///////////////////////////////////////////////////////////////////////////
    try {
        sc_core::sc_start(1_us);
        if(!sc_core::sc_end_of_simulation_invoked())
            sc_core::sc_stop();
    } catch(sc_report& e) {
        SCCERR() << "Caught sc_report exception during simulation: " << e.what() << ":" << e.get_msg();
    } catch(std::exception& e) {
        SCCERR() << "Caught exception during simulation: " << e.what();
    } catch(...) {
        SCCERR() << "Caught unspecified exception during simulation";
    }
    return 0;
}
