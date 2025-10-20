#include <cci_configuration>
#include <fstream>
#include <interfaces/apb/pin/initiator.h>
#include <interfaces/apb/pin/target.h>
#include <iomanip>
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
    enum { WIDTH = 32 };
    tlm::scc::initiator_mixin<tlm::tlm_initiator_socket<WIDTH>> isck{"isck"};
    apb::pin::initiator<WIDTH> intor{"intor"};
    sc_core::sc_clock PCLK{"PCLK", 1_ns};
    sc_core::sc_signal<bool> PRESETn{"PRESETn"};
    sc_core::sc_signal<sc_dt::sc_uint<32>> PADDR{"PADDR"};
    sc_core::sc_signal<sc_dt::sc_uint<3>> PPROT{"PPROT"};
    sc_core::sc_signal<bool> PNSE{"PNSE"};
    sc_core::sc_signal<bool> PSELx{"PSELx"};
    sc_core::sc_signal<bool> PENABLE{"PENABLE"};
    sc_core::sc_signal<bool> PWRITE{"PWRITE"};
    sc_core::sc_signal<sc_dt::sc_uint<WIDTH>> PWDATA{"PWDATA"};
    sc_core::sc_signal<sc_dt::sc_uint<WIDTH / 8>> PSTRB{"PSTRB"};
    sc_core::sc_signal<bool> PREADY{"PREADY"};
    sc_core::sc_signal<sc_dt::sc_uint<WIDTH>> PRDATA{"PRDATA"};
    sc_core::sc_signal<bool> PSLVERR{"PSLVERR"};

    apb::pin::target<WIDTH, 32> target{"target"};
    tlm::scc::target_mixin<tlm::tlm_target_socket<scc::LT>> tsck{"tsck"};

    testbench(sc_module_name nm)
    : sc_module(nm) {
        SC_HAS_PROCESS(testbench);
        isck(intor.tsckt);
        intor.PCLK_i(PCLK);
        intor.PRESETn_i(PRESETn);
        intor.PADDR_o(PADDR);
        intor.PPROT_o(PPROT);
        intor.PNSE_o(PNSE);
        intor.PSELx_o(PSELx);
        intor.PENABLE_o(PENABLE);
        intor.PWRITE_o(PWRITE);
        intor.PWDATA_o(PWDATA);
        intor.PSTRB_o(PSTRB);
        intor.PREADY_i(PREADY);
        intor.PRDATA_i(PRDATA);
        intor.PSLVERR_i(PSLVERR);
        target.PCLK_i(PCLK);
        target.PRESETn_i(PRESETn);
        target.PADDR_i(PADDR);
        target.PPROT_i(PPROT);
        target.PNSE_i(PNSE);
        target.PSELx_i(PSELx);
        target.PENABLE_i(PENABLE);
        target.PWRITE_i(PWRITE);
        target.PWDATA_i(PWDATA);
        target.PSTRB_i(PSTRB);
        target.PRDATA_o(PRDATA);
        target.PREADY_o(PREADY);
        target.PSLVERR_o(PSLVERR);
        target.isckt(tsck);
        SC_THREAD(run);
        tsck.register_b_transport([this](tlm::tlm_generic_payload& gp, sc_time& delay) {
            gp.set_response_status(tlm::TLM_OK_RESPONSE);
            if(gp.is_write()) {
                SCCDEBUG(SCMOD) << "Received write access to addr 0x" << std::hex << gp.get_address();
                // Print all data bytes with their byte enable values
                if(gp.get_byte_enable_length() > 0) {
                    SCCDEBUG(SCMOD) << "  Data with byte enables:";
                    for(size_t i = 0; i < gp.get_data_length(); ++i) {
                        SCCDEBUG(SCMOD) << "    Byte[" << i << "]: data=0x" << std::hex << std::setw(2) << std::setfill('0')
                                        << (unsigned)gp.get_data_ptr()[i] << " BE=0x" << std::hex << std::setw(2) << std::setfill('0')
                                        << (unsigned)gp.get_byte_enable_ptr()[i]
                                        << (gp.get_byte_enable_ptr()[i] ? " (enabled)" : " (disabled)");
                    }
                } else {
                    SCCDEBUG(SCMOD) << "  Data (no byte enables):";
                    for(size_t i = 0; i < gp.get_data_length(); ++i) {
                        SCCDEBUG(SCMOD) << "    Byte[" << i << "]: data=0x" << std::hex << std::setw(2) << std::setfill('0')
                                        << (unsigned)gp.get_data_ptr()[i];
                    }
                }
            } else {
                memset(gp.get_data_ptr(), 0x55, gp.get_data_length());
                SCCDEBUG(SCMOD) << "Received read access from addr 0x" << std::hex << gp.get_address();
            }
        });
    }

    void run() {
        ///////////////////////////////////////////////////////////////////////////
        // Test data preparation
        ///////////////////////////////////////////////////////////////////////////
        std::array<uint8_t, 8> write_data;
        std::array<uint8_t, 8> read_data;
        std::array<uint8_t, 4> byte_enable;

        write_data[0] = 0xAA;
        write_data[1] = 0xBB;
        write_data[2] = 0xCC;
        write_data[3] = 0xDD;
        write_data[4] = 0xEE;
        write_data[5] = 0xFF;
        write_data[6] = 0x66;
        write_data[7] = 0x77;

        ///////////////////////////////////////////////////////////////////////////
        // Reset sequence
        ///////////////////////////////////////////////////////////////////////////
        PRESETn.write(false);
        for(size_t i = 0; i < 10; ++i)
            wait(PCLK.posedge_event());
        PRESETn.write(true);
        wait(PCLK.posedge_event());

        ///////////////////////////////////////////////////////////////////////////
        // Test 1: Pin-level target - Aligned write (full word, all strobes)
        ///////////////////////////////////////////////////////////////////////////
        PENABLE.write(0);

        for(int i = 0; i < 2; i++) {
            // setup phase
            PSELx.write(1);
            PADDR.write(0x1000);
            PWDATA.write(*reinterpret_cast<uint32_t*>(write_data.data() + i * 4));
            PWRITE.write(1);
            PSTRB.write(0b1111);
            wait(PCLK.posedge_event());

            // access phase
            PENABLE.write(1);
            while(!PREADY.read())
                wait(PREADY.value_changed_event());
            wait(PCLK.posedge_event());

            // cleanup
            PENABLE.write(0);
            PSELx.write(0);
        }

        ///////////////////////////////////////////////////////////////////////////
        // Test 2: Pin-level target - Aligned read (full word)
        ///////////////////////////////////////////////////////////////////////////
        for(int i = 0; i < 2; i++) {
            // setup phase
            PSELx.write(1);
            PADDR.write(0x1004);
            PWRITE.write(0);
            PSTRB.write(0);
            wait(PCLK.posedge_event());

            // access phase
            PENABLE.write(1);
            while(!PREADY.read())
                wait(PREADY.value_changed_event());
            wait(PCLK.posedge_event());

            // cleanup
            PENABLE.write(0);
            PSELx.write(0);

            SCCDEBUG(SCMOD) << std::hex << "read data : " << PRDATA.read();
        }

        ///////////////////////////////////////////////////////////////////////////
        // Test 3: Pin-level target - Write with alternating byte strobes (0b1010)
        ///////////////////////////////////////////////////////////////////////////
        // setup phase
        PSELx.write(1);
        PADDR.write(0x1008);
        PWDATA.write(*reinterpret_cast<uint32_t*>(write_data.data()));
        PWRITE.write(1);
        PSTRB.write(0b1010);
        wait(PCLK.posedge_event());

        // access phase
        PENABLE.write(1);
        while(!PREADY.read())
            wait(PREADY.value_changed_event());
        wait(PCLK.posedge_event());

        // cleanup
        PENABLE.write(0);
        PSELx.write(0);

        wait(PCLK.posedge_event());

        ///////////////////////////////////////////////////////////////////////////
        // Test 4: TLM initiator via pin - Write with byte enables (BE: 0xFF, 0x00, 0x00, 0xFF)
        ///////////////////////////////////////////////////////////////////////////
        tlm::tlm_generic_payload gp;
        sc_time delay;

        byte_enable[0] = 0xFF;
        byte_enable[1] = 0x00;
        byte_enable[2] = 0x00;
        byte_enable[3] = 0xFF;

        gp.set_address(0x1010);
        gp.set_data_length(4);
        gp.set_data_ptr(write_data.data() + 2);
        gp.set_byte_enable_ptr(byte_enable.data());
        gp.set_byte_enable_length(4);
        gp.set_streaming_width(4);
        gp.set_command(tlm::TLM_WRITE_COMMAND);
        delay = SC_ZERO_TIME;
        isck->b_transport(gp, delay);

        ///////////////////////////////////////////////////////////////////////////
        // Test 5: TLM initiator via pin - Aligned read (full word)
        ///////////////////////////////////////////////////////////////////////////
        gp.set_address(0x1014);
        gp.set_data_length(4);
        gp.set_data_ptr(read_data.data());
        gp.set_streaming_width(4);
        gp.set_command(tlm::TLM_READ_COMMAND);
        delay = SC_ZERO_TIME;
        isck->b_transport(gp, delay);
        SCCDEBUG(SCMOD) << std::hex << "read data : " << *(uint32_t*)read_data.data();

        ///////////////////////////////////////////////////////////////////////////
        // Test 6: TLM initiator via pin - Unaligned partial write
        // Note: Unaligned write (0x1016-0x1018) is converted to strobe write (0x1014-0x1018)
        ///////////////////////////////////////////////////////////////////////////
        gp.set_address(0x1016);
        gp.set_data_length(2);
        gp.set_data_ptr(write_data.data() + 2);
        gp.set_byte_enable_ptr(nullptr);
        gp.set_byte_enable_length(0);
        gp.set_streaming_width(2);
        gp.set_command(tlm::TLM_WRITE_COMMAND);
        delay = SC_ZERO_TIME;
        isck->b_transport(gp, delay);

        ///////////////////////////////////////////////////////////////////////////
        // Test 7: Invalid case - Transaction exceeding 32-bit boundary (disabled)
        // Note: This test is disabled as it tests an invalid scenario
        ///////////////////////////////////////////////////////////////////////////
        if(0) {
            gp.set_address(0x1018);
            gp.set_data_length(8);
            gp.set_data_ptr(read_data.data());
            gp.set_streaming_width(8);
            gp.set_command(tlm::TLM_READ_COMMAND);
            delay = SC_ZERO_TIME;
            isck->b_transport(gp, delay);
        }

        sc_stop();
        return;
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
    scc::configurer cfg("apb_bfm.json");
    scc::configurable_tracer trace("apb_bfm", true, true);
    ///////////////////////////////////////////////////////////////////////////
    // create modules/channels and trace
    ///////////////////////////////////////////////////////////////////////////
    testbench tb("tb");

    ///////////////////////////////////////////////////////////////////////////
    // Create VCD trace file
    ///////////////////////////////////////////////////////////////////////////
    sc_trace_file *vcd_trace = sc_create_vcd_trace_file("apb_trace");
    vcd_trace->set_time_unit(1, SC_PS);  // Set time unit to picoseconds

    // Trace clock and reset
    sc_trace(vcd_trace, tb.PCLK, "PCLK");
    sc_trace(vcd_trace, tb.PRESETn, "PRESETn");

    // Trace APB address phase signals
    sc_trace(vcd_trace, tb.PADDR, "PADDR");
    sc_trace(vcd_trace, tb.PSELx, "PSELx");
    sc_trace(vcd_trace, tb.PENABLE, "PENABLE");
    sc_trace(vcd_trace, tb.PWRITE, "PWRITE");
    sc_trace(vcd_trace, tb.PPROT, "PPROT");
    sc_trace(vcd_trace, tb.PNSE, "PNSE");

    // Trace APB data signals
    sc_trace(vcd_trace, tb.PWDATA, "PWDATA");
    sc_trace(vcd_trace, tb.PSTRB, "PSTRB");
    sc_trace(vcd_trace, tb.PRDATA, "PRDATA");

    // Trace APB response signals
    sc_trace(vcd_trace, tb.PREADY, "PREADY");
    sc_trace(vcd_trace, tb.PSLVERR, "PSLVERR");
    cfg.configure();
    cfg.dump_configuration("apb_test.default.json");
    ///////////////////////////////////////////////////////////////////////////
    // run the simulation
    ///////////////////////////////////////////////////////////////////////////
    try {
        sc_core::sc_start();
        if(!sc_core::sc_end_of_simulation_invoked())
            sc_core::sc_stop();
    } catch(sc_report& e) {
        SCCERR() << "Caught sc_report exception during simulation: " << e.what() << ":" << e.get_msg();
    } catch(std::exception& e) {
        SCCERR() << "Caught exception during simulation: " << e.what();
    } catch(...) {
        SCCERR() << "Caught unspecified exception during simulation";
    }

    ///////////////////////////////////////////////////////////////////////////
    // Close VCD trace file
    ///////////////////////////////////////////////////////////////////////////
    sc_close_vcd_trace_file(vcd_trace);

    return 0;
}
