/*
 * sc_main.cpp
 *
 *  Created on:
 *      Author:
 */

#define SC_INCLUDE_DYNAMIC_PROCESSES

#include <ace_axi_adapt.h>
#include <array>
#include <axi/pe/simple_initiator.h>
#include <axi/pe/simple_target.h>
#include <axi/scv/recorder_modules.h>
#include <cstdint>
#include <scc.h>

using namespace sc_core;
using namespace axi;
using namespace axi::pe;
using namespace sc_core;

const unsigned SOCKET_WIDTH = 64;

class testbench : public sc_core::sc_module {
public:
    SC_HAS_PROCESS(testbench);
    sc_core::sc_time clk_period{10, sc_core::SC_NS};
    sc_core::sc_clock clk{"clk", clk_period, 0.5, sc_core::SC_ZERO_TIME, true};
    sc_core::sc_signal<bool> rst{"rst"};
    axi::ace_initiator_socket<SOCKET_WIDTH> intor{"intor"};
    // axi::scv::ace_recorder_module<SOCKET_WIDTH> intor_rec{"intor_rec"};
    axi::scv::axi_recorder_module<SOCKET_WIDTH> intor_rec{"intor_rec"};
    axi::axi_target_socket<SOCKET_WIDTH> tgt{"tgt"};
    ace_axi_adapt<SOCKET_WIDTH> Adapter1{"Adapter1"};
    testbench(sc_core::sc_module_name nm)
    : sc_core::sc_module(nm)
    , intor_pe("intor_pe", intor)
    , tgt_pe("tgt_pe", tgt) {
        SC_THREAD(run);
        intor_pe.clk_i(clk);
        tgt_pe.clk_i(clk);
        // tgt_pe.Clk(clk);
        // intor(intor_rec.tsckt);
        intor(Adapter1.tsckt);
        // intor_rec.isckt(Adapter1.tsckt);
        Adapter1.isckt(intor_rec.tsckt);
        // intor_rec.isckt(tgt_pe.axi);
        intor_rec.isckt(tgt);
    }

    tlm::tlm_generic_payload* prepare_trans(size_t len, uint8_t const* data = nullptr) {
        auto trans = tlm::scc::tlm_mm<tlm::tlm_base_protocol_types, false>::get().allocate<axi::axi4_extension>();
        tlm::scc::setId(*trans, id++);
        auto ext = trans->get_extension<axi::axi4_extension>();
        trans->set_data_length(len);
        trans->set_data_ptr(new uint8_t[len]);
        if(data)
            for(auto i = 0U; i < len; ++i)
                *(trans->get_data_ptr() + i) = *(data + i);
        trans->set_streaming_width(len);
        ext->set_size(scc::ilog2(std::min<size_t>(len, SOCKET_WIDTH / 8)));
        sc_assert(len < (SOCKET_WIDTH / 8) || len % (SOCKET_WIDTH / 8) == 0);
        ext->set_length((len * 8 - 1) / SOCKET_WIDTH);
        // ext->set_burst(len * 8 > SOCKET_WIDTH ? axi::burst_e::INCR : axi::burst_e::FIXED);
        ext->set_burst(axi::burst_e::INCR);
        ext->set_id(id);
        return trans;
    }

    void run() {
        unsigned int StartAddr = 0;
        unsigned int ResetCycles = 10;
        unsigned int BurstLengthByte = 16;
        unsigned int NumberOfIterations = 1000;
        rst.write(false);
        for(size_t i = 0; i < ResetCycles; ++i)
            wait(clk.posedge_event());
        rst.write(true);
        wait(clk.posedge_event());
        std::array<uint8_t, 256> data;
        wait(10, SC_NS);
        for(int i = 0; i < NumberOfIterations; ++i) {
            SCCDEBUG("testbench") << "executing transactions in iteration " << i;
            { // 1
                auto trans = prepare_trans(BurstLengthByte);
                SCCDEBUG(SCMOD) << "generating transactions " << trans;
                trans->set_command(tlm::TLM_READ_COMMAND);
                trans->set_address(StartAddr);
                trans->set_data_ptr(data.data());
                trans->acquire();
                intor_pe.transport(*trans, false);
                if(trans->get_response_status() != tlm::TLM_OK_RESPONSE)
                    SCCERR() << "Invalid response status" << trans->get_response_string();
                trans->release();
                StartAddr += BurstLengthByte;
            }
            { // 2
                auto trans = prepare_trans(BurstLengthByte);
                SCCDEBUG(SCMOD) << "generating transactions " << trans;
                trans->set_command(tlm::TLM_WRITE_COMMAND);
                trans->set_address(BurstLengthByte);
                trans->set_data_ptr(data.data());
                trans->acquire();
                intor_pe.transport(*trans, false);
                if(trans->get_response_status() != tlm::TLM_OK_RESPONSE)
                    SCCERR() << "Invalid response status" << trans->get_response_string();
                trans->release();
                StartAddr += BurstLengthByte;
            }
        }
        wait(100_ns);
        sc_stop();
    }

private:
    axi::pe::simple_ace_initiator<SOCKET_WIDTH> intor_pe;
    axi::pe::simple_target<SOCKET_WIDTH> tgt_pe;
    // Target_axi<SOCKET_WIDTH> tgt_pe;
    unsigned id{0};
};

int sc_main(int argc, char* argv[]) {
    sc_report_handler::set_actions(SC_ID_MORE_THAN_ONE_SIGNAL_DRIVER_, SC_DO_NOTHING);
    // clang-format off
    scc::init_logging(
            scc::LogConfig()
            .logLevel(scc::log::DEBUG)
            .logAsync(false)
            .coloredOutput(true));
    // clang-format on
    sc_report_handler::set_actions(SC_ERROR, SC_LOG | SC_CACHE_REPORT | SC_DISPLAY);
#ifdef HAS_CCI
    scc::configurable_tracer trace("ace_ace_test",
                                   scc::tracer::file_type::NONE, // define the kind of transaction trace
                                   true,                         // enables vcd
                                   true);
#else
    scc::tracer trace("ace_ace_test",
                      scc::tracer::file_type::NONE, // define the kind of transaction trace
                      true);                        // enables vcd
#endif
    testbench mstr("master");
    sc_core::sc_start(10_ms);
    SCCINFO() << "Finished";
    return 0;
}
