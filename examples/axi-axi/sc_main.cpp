/*
 * sc_main.cpp
 *
 *  Created on:
 *      Author:
 */

#ifdef WITH_SCV
#include <axi/scv/recorder_modules.h>
#include <scv.h>
#include <scv/scv_tr.h>
#endif
#include "scc/report.h"
#include "tlm/tlm_id.h"
#include <array>
#include <axi/pe/simple_initiator.h>
#include <axi/pe/simple_target.h>
#include <tlm/tlm_mm.h>

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
    axi::axi_initiator_socket<SOCKET_WIDTH> intor{"intor"};
#ifdef WITH_SCV
    scv4axi::axi_recorder_module<SOCKET_WIDTH> intor_rec{"intor_rec"};
#endif
    axi::axi_target_socket<SOCKET_WIDTH> tgt{"tgt"};

    testbench(sc_core::sc_module_name nm)
    : sc_core::sc_module(nm)
    , intor_pe("intor_pe", intor)
    , tgt_pe("tgt_pe", tgt) {
        SC_THREAD(run);
        intor_pe.clk_i(clk);
        tgt_pe.clk_i(clk);
#ifdef WITH_SCV
        intor(intor_rec.tsckt);
        intor_rec.isckt(tgt);
#else
        intor(tgt);
#endif
    }

    tlm::tlm_generic_payload* prepare_trans(size_t len) {
        auto trans = tlm::tlm_mm<>::get().allocate<axi::axi4_extension>();
        setId(*trans, id++);
        auto ext = trans->get_extension<axi::axi4_extension>();
        trans->set_data_length(len);
        trans->set_streaming_width(len);
        ext->set_size(scc::ilog2(std::min<size_t>(len, SOCKET_WIDTH / 8)));
        sc_assert(len < (SOCKET_WIDTH / 8) || len % (SOCKET_WIDTH / 8) == 0);
        ext->set_length((len * 8 - 1) / SOCKET_WIDTH);
        ext->set_burst(len * 8 > SOCKET_WIDTH ? axi::burst_e::INCR : axi::burst_e::FIXED);
        ext->set_id(id);
        return trans;
    }

    void run() {
        unsigned int addr = 0;
        unsigned int NumberOfIterations = 1000;
        rst.write(false);
        for(size_t i = 0; i < 9; ++i)
            wait(clk.posedge_event());
        rst.write(true);
        wait(clk.posedge_event());
        std::array<uint8_t, 256> data;
        wait(10, SC_NS);
        for(int i = 0; i < NumberOfIterations; ++i) {
            SCCDEBUG("testbench") << "executing transactions in iteration " << i;
            { // 1
                auto trans = prepare_trans(16);
                trans->set_command(tlm::TLM_READ_COMMAND);
                trans->set_address(addr);
                trans->set_data_ptr(data.data());
                trans->acquire();
                intor_pe.transport(*trans, false);
                if(trans->get_response_status() != tlm::TLM_OK_RESPONSE)
                    SCCERR() << "Invalid response status" << trans->get_response_string();
                trans->release();
            }
            addr += 8;
            { // 2
                auto trans = prepare_trans(8);
                trans->set_command(tlm::TLM_READ_COMMAND);
                trans->set_address(addr);
                trans->set_data_ptr(data.data());
                trans->acquire();
                intor_pe.transport(*trans, false);
                if(trans->get_response_status() != tlm::TLM_OK_RESPONSE)
                    SCCERR() << "Invalid response status" << trans->get_response_string();
                trans->release();
            }
            addr += 8;
            { // 3
                auto trans = prepare_trans(16);
                trans->set_command(tlm::TLM_WRITE_COMMAND);
                trans->set_address(addr);
                trans->set_data_ptr(data.data());
                trans->acquire();
                intor_pe.transport(*trans, false);
                if(trans->get_response_status() != tlm::TLM_OK_RESPONSE)
                    SCCERR() << "Invalid response status" << trans->get_response_string();
                trans->release();
            }
            addr += 8;
            { // 4
                auto trans = prepare_trans(8);
                trans->set_command(tlm::TLM_WRITE_COMMAND);
                trans->set_address(addr);
                trans->set_data_ptr(data.data());
                trans->acquire();
                intor_pe.transport(*trans, false);
                if(trans->get_response_status() != tlm::TLM_OK_RESPONSE)
                    SCCERR() << "Invalid response status" << trans->get_response_string();
                trans->release();
            }
            addr += 8;
            { // 5
                auto trans = prepare_trans(32);
                trans->set_command(tlm::TLM_READ_COMMAND);
                trans->set_address(addr);
                trans->set_data_ptr(data.data());
                trans->acquire();
                intor_pe.transport(*trans, false);
                if(trans->get_response_status() != tlm::TLM_OK_RESPONSE)
                    SCCERR() << "Invalid response status" << trans->get_response_string();
                trans->release();
            }
            addr += 8;
            { // 6
                auto trans = prepare_trans(32);
                trans->set_command(tlm::TLM_WRITE_COMMAND);
                trans->set_address(addr);
                trans->set_data_ptr(data.data());
                trans->acquire();
                intor_pe.transport(*trans, false);
                if(trans->get_response_status() != tlm::TLM_OK_RESPONSE)
                    SCCERR() << "Invalid response status " << trans->get_response_string();
                trans->release();
            }
            addr += 8;
        }
        wait(100_ns);
        sc_stop();
    }

private:
    axi::pe::simple_axi_initiator<SOCKET_WIDTH> intor_pe;
    axi::pe::simple_target<SOCKET_WIDTH> tgt_pe;
    unsigned id{0};
};

int sc_main(int argc, char* argv[]) {
    sc_report_handler::set_actions(SC_ID_MORE_THAN_ONE_SIGNAL_DRIVER_, SC_DO_NOTHING);
    scc::init_logging(
		      scc::LogConfig()
		      .logLevel(static_cast<scc::log>(7))
		      .logAsync(false)
		      .dontCreateBroker(true)
		      .coloredOutput(true));
    sc_report_handler::set_actions(SC_ERROR, SC_LOG | SC_CACHE_REPORT | SC_DISPLAY);
#ifdef WITH_SCV
    scv_startup();
    scv_tr_text_init();
    scv_tr_db* db = new scv_tr_db("axi_axi_test.txlog");
    scv_tr_db::set_default_db(db);
#endif
    testbench mstr("master");
    sc_core::sc_start(1_ms);
    SCCINFO() << "Finished";
#ifdef WITH_SCV
    delete db;
#endif
    return 0;
}
