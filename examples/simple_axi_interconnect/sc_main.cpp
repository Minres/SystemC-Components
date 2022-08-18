/*
 * sc_main.cpp
 *
 *  Created on:
 *      Author:
 */
#include "simple_axi_interconnect.h"
#include <array>
#include <axi/axi_initiator.h>
#include <axi/axi_target.h>
#include <axi/scv/recorder_modules.h>
#include <scc.h>
// #include <scc/configurable_tracer.h>
#include <scc/memory.h>
#include <scc/hierarchy_dumper.h>
#include <tlm/scc/initiator_mixin.h>
#include <tlm/scc/tlm_gp_shared.h>

#include "generic_extension.h"

using namespace sc_core;
using namespace axi;
using namespace axi::pe;

const unsigned SOCKET_WIDTH = 64;
const unsigned MASTER_NUM = 2;
const unsigned SLAVE_NUM = 2;

class testbench : public sc_core::sc_module {
public:
    SC_HAS_PROCESS(testbench);
    sc_core::sc_time clk_period{10, sc_core::SC_NS};
    sc_core::sc_clock clk{"clk", clk_period, 0.5, sc_core::SC_ZERO_TIME, true};
    sc_core::sc_signal<bool> rst{"rst"};
    axi::axi_initiator<SOCKET_WIDTH> intor_m1{"intor_m1"};
    axi::axi_initiator<SOCKET_WIDTH> intor_m2{"intor_m2"};
    simple_axi_interconnect<SOCKET_WIDTH,MASTER_NUM,SLAVE_NUM> simple_axi_bus{"simple_axi_bus","address.map"};
    axi::axi_target<SOCKET_WIDTH> tgt_s1{"tgt_s1"};
    axi::axi_target<SOCKET_WIDTH> tgt_s2{"tgt_s2"};

private:
    tlm::scc::initiator_mixin<tlm::tlm_initiator_socket<>> top_isck_m1{"top_isck_m1"};
    tlm::scc::initiator_mixin<tlm::tlm_initiator_socket<>> top_isck_m2{"top_isck_m2"};
    scc::memory<1_GB> mem1{"mem1"};
    scc::memory<1_GB> mem2{"mem2"};

    unsigned id{0};
    unsigned int StartAddr{0};
    unsigned int ResetCycles{10};
    unsigned int BurstLengthByte{16};
    unsigned int NumberOfIterations{8};

public:
    testbench(sc_core::sc_module_name nm)
    : sc_core::sc_module(nm) {
        SC_THREAD(run);
        intor_m1.clk_i(clk);
        intor_m2.clk_i(clk);
        top_isck_m1(intor_m1.b_tsck);
        top_isck_m2(intor_m2.b_tsck);
        tgt_s1.clk_i(clk);
        tgt_s2.clk_i(clk);
        simple_axi_bus.clk_i(clk);
        intor_m1.isck.bind(*(simple_axi_bus.target_socket[0]));
        intor_m2.isck.bind(*(simple_axi_bus.target_socket[1]));
        simple_axi_bus.intit_socket[0]->bind(tgt_s1.tsck);
        simple_axi_bus.intit_socket[1]->bind(tgt_s2.tsck);
        tgt_s1.isck(mem1.target);
        tgt_s2.isck(mem2.target);
    }

    tlm::tlm_generic_payload* prepare_trans(size_t len) {
        auto trans = tlm::scc::tlm_mm<tlm::tlm_base_protocol_types, false>::get().allocate<generic_extension<axi::axi_target_socket<SOCKET_WIDTH>>>();
        tlm::scc::tlm_gp_mm::add_data_ptr(len, trans);
        trans->set_data_length(len);
        trans->set_streaming_width(len);
        tlm::scc::setId(*trans, id++);
        return trans;
    }

    void run() {
        rst.write(false);
        for(size_t i = 0; i < ResetCycles; ++i)
            wait(clk.posedge_event());
        rst.write(true);
        wait(clk.posedge_event());
        wait(clk.posedge_event());
        for(int i = 0; i < NumberOfIterations; ++i) {
            SCCDEBUG("testbench") << "executing transactions in iteration " << i;
            { // 1 write
                tlm::scc::tlm_gp_shared_ptr trans = prepare_trans(BurstLengthByte);
                trans->set_command(tlm::TLM_WRITE_COMMAND);
                trans->set_address(StartAddr);
                unsigned char* data_ptr = trans->get_data_ptr();
                std::memcpy(data_ptr,&StartAddr,sizeof(StartAddr));
                generic_extension<axi::axi_target_socket<SOCKET_WIDTH>> *generic_ext = trans->get_extension<generic_extension<axi::axi_target_socket<SOCKET_WIDTH>>>();
                if (generic_ext != NULL)
                {
                    generic_ext->set_bw_if_ptr(simple_axi_bus.target_socket[0]); // assign pointer master1
                }
                auto delay = SC_ZERO_TIME;
                top_isck_m1->b_transport(*trans, delay);
                if(trans->get_response_status() != tlm::TLM_OK_RESPONSE)
                    SCCERR() << "Invalid response status" << trans->get_response_string();
            }
            // wait(100, SC_NS);
            { // 2 read and compare
                tlm::scc::tlm_gp_shared_ptr trans = prepare_trans(BurstLengthByte);
                trans->set_command(tlm::TLM_READ_COMMAND);
                trans->set_address(StartAddr);
                unsigned char* data_ptr = trans->get_data_ptr();
                generic_extension<axi::axi_target_socket<SOCKET_WIDTH>> *generic_ext = trans->get_extension<generic_extension<axi::axi_target_socket<SOCKET_WIDTH>>>();
                if (generic_ext != NULL)
                {
                    generic_ext->set_bw_if_ptr(simple_axi_bus.target_socket[0]); // assign pointer master1
                }
                auto delay = SC_ZERO_TIME;
                top_isck_m1->b_transport(*trans, delay);
                if(*((unsigned int*)data_ptr) != StartAddr){
                    SCCERR() << "FAIL data: "<< *((unsigned int*)data_ptr) << " expected: " << StartAddr;
                }
                if(trans->get_response_status() != tlm::TLM_OK_RESPONSE)
                    SCCERR() << "Invalid response status" << trans->get_response_string();
            }
            StartAddr += BurstLengthByte*64;
        }
        wait(100, SC_NS);
        sc_stop();
    }
};

int sc_main(int argc, char* argv[]) {
    sc_report_handler::set_actions(SC_ID_MORE_THAN_ONE_SIGNAL_DRIVER_, SC_DO_NOTHING);
    // clang-format off
    scc::init_logging(
            scc::LogConfig()
            .logLevel(static_cast<scc::log>(7))
            .logAsync(false)
            .coloredOutput(true));
    // clang-format off
    sc_report_handler::set_actions(SC_ERROR, SC_LOG | SC_CACHE_REPORT | SC_DISPLAY);
#ifdef HAS_CCI
    scc::configurable_tracer trace("axi_axi_test",
                                   scc::tracer::file_type::TEXT, // define the kind of transaction trace
                                   true,                         // enables vcd
                                   true);
#else
    scc::tracer trace("axi_axi_test",
                      scc::tracer::file_type::NONE, // define the kind of transaction trace
                      true);                        // enables vcd
#endif
    testbench tb("tb");
    scc::hierarchy_dumper d("axi_axi.elkt", scc::hierarchy_dumper::ELKT);
    try {
        sc_core::sc_start(1_ms);
        SCCINFO() << "Finished";
    } catch(sc_core::sc_report& e) {
        SCCERR() << "Caught sc_report exception during simulation: " << e.what() << ":" << e.get_msg();
    } catch(std::exception& e) {
        SCCERR() << "Caught exception during simulation: " << e.what();
    } catch(...) {
        SCCERR() << "Caught unspecified exception during simulation";
    }
    if(sc_is_running()) {
        SCCERR() << "Simulation timeout!"; // calls sc_stop
    }
    auto errcnt = sc_report_handler::get_count(SC_ERROR);
    auto warncnt = sc_report_handler::get_count(SC_WARNING);
    SCCINFO() << "Finished, there were " << errcnt << " error" << (errcnt == 1 ? "" : "s") << " and " << warncnt
              << " warning" << (warncnt == 1 ? "" : "s");
    return errcnt + warncnt;
}
