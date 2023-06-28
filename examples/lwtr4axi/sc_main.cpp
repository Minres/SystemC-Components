/*
 * sc_main.cpp
 *
 *  Created on:
 *      Author:
 */

#include <array>
#include "ace_axi_adapt.h"
#include <axi/axi_initiator.h>
#include <axi/axi_target.h>
#include <axi/lwtr/axi_lwtr.h>
#include <axi/lwtr/ace_lwtr.h>
#include <scc.h>
#include <scc/configurable_tracer.h>
#include <scc/memory.h>
#include <scc/hierarchy_dumper.h>
#include <tlm/scc/initiator_mixin.h>
#include <tlm/scc/tlm_gp_shared.h>

using namespace sc_core;
using namespace axi;
using namespace axi::pe;

const unsigned SOCKET_WIDTH = 64;

class testbench : public sc_core::sc_module {
public:
    SC_HAS_PROCESS(testbench);
    sc_core::sc_time clk_period{10, sc_core::SC_NS};
    sc_core::sc_clock clk{"clk", clk_period, 0.5, sc_core::SC_ZERO_TIME, true};
    sc_core::sc_signal<bool> rst{"rst"};
    axi::ace_initiator_socket<SOCKET_WIDTH> intor{"intor"};
    axi::lwtr::ace_lwtr_recorder<SOCKET_WIDTH> ace_rec{"ace_rec"};
    ace_axi_adapt<SOCKET_WIDTH> adapt{"adapt"};
    axi::lwtr::axi_lwtr_recorder<SOCKET_WIDTH> axi_rec{"axi_rec"};
    axi::axi_target_socket<SOCKET_WIDTH> tgt{"tgt"};

    testbench(sc_core::sc_module_name nm)
    : sc_core::sc_module(nm)
    , intor_pe("intor_pe", intor)
    , tgt_pe("tgt_pe", tgt) {
        SC_THREAD(run);
        intor_pe.clk_i(clk);
        tgt_pe.clk_i(clk);
        intor(ace_rec.ts);
        ace_rec.is(adapt.tsckt);
        adapt.isckt(axi_rec.ts);
        axi_rec.is(tgt);
    }

    tlm::tlm_generic_payload* prepare_trans(size_t len, tlm::tlm_command cmd, uint8_t const* data = nullptr) {
        auto trans = tlm::scc::tlm_mm<tlm::tlm_base_protocol_types, false>::get().allocate<axi::ace_extension>();
        tlm::scc::setId(*trans, id++);
        auto ext = trans->get_extension<axi::ace_extension>();
        trans->set_command(cmd);
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
        ext->set_snoop(cmd==tlm::TLM_READ_COMMAND?axi::snoop_e::READ_ONCE:axi::snoop_e::WRITE_UNIQUE);
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
                auto trans = prepare_trans(BurstLengthByte, tlm::TLM_READ_COMMAND);
                SCCDEBUG(SCMOD) << "generating transactions " << trans;
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
                auto trans = prepare_trans(BurstLengthByte, tlm::TLM_WRITE_COMMAND);
                SCCDEBUG(SCMOD) << "generating transactions " << trans;
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
        wait(10_ns);
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
    sc_report_handler::set_actions(SC_ERROR, SC_LOG | SC_CACHE_REPORT | SC_DISPLAY | SC_STOP);
    // clang-format off
    scc::init_logging(
            scc::LogConfig()
            .logLevel(scc::log::INFO)
            .logAsync(false)
            .coloredOutput(true));
    // clang-format off
    ::lwtr::tx_text_init();
    ::lwtr::tx_ftr_init(false);
    ::lwtr::tx_db db("lwtr4axi");
    testbench tb("tb");
    scc::hierarchy_dumper d("lwtr4axi.json", scc::hierarchy_dumper::D3JSON);
    //scc::hierarchy_dumper d("axi_axi_test.elkt", scc::hierarchy_dumper::ELKT);
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
