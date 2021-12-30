/*
 * sc_main.cpp
 *
 *  Created on:
 *      Author:
 */

#include <axi/bfm/axi4_initiator.h>
#include <axi/bfm/axi4_target.h>
#include <scc.h>
#include <array>
#include <axi/scv/recorder_modules.h>
#include <axi/pe/axi_initiator.h>
#include <axi/pe/simple_target.h>
#include <csetjmp>
#include <csignal>

using namespace sc_core;
using namespace axi;
using namespace axi::pe;

class testbench : public sc_core::sc_module {
public:
    using bus_cfg = axi::axi_cfg</*BUSWIDTH=*/32, /*ADDRWIDTH=*/32, /*IDWIDTH=*/4, /*USERWIDTH=*/1>;

    sc_core::sc_time clk_period{10, sc_core::SC_NS};
    sc_core::sc_clock clk{"clk", clk_period, 0.5, sc_core::SC_ZERO_TIME, true};
    sc_core::sc_signal<bool> rst{"rst"};
    axi::axi_initiator_socket<bus_cfg::BUSWIDTH> intor{"intor"};
    axi::scv::axi_recorder_module<bus_cfg::BUSWIDTH> intor_rec{"intor_rec"};
    axi::bfm::axi4_initiator<bus_cfg> intor_bfm{"into_bfm"};

    axi::aw_ch<bus_cfg, axi::signal_types> aw;
    axi::wdata_ch<bus_cfg, axi::signal_types> wdata;
    axi::b_ch<bus_cfg, axi::signal_types> b;
    axi::ar_ch<bus_cfg, axi::signal_types> ar;
    axi::rresp_ch<bus_cfg, axi::signal_types> rresp;
    axi::bfm::axi4_target<bus_cfg> tgt_bfm{"tgt_bfm"};

    axi::scv::axi_recorder_module<bus_cfg::BUSWIDTH> tgt_rec{"tgt_rec"};
    axi::axi_target_socket<bus_cfg::BUSWIDTH> tgt{"tgt"};

private:
    axi::pe::axi_initiator<bus_cfg::BUSWIDTH> intor_pe;
    axi::pe::simple_target<bus_cfg::BUSWIDTH> tgt_pe;
    unsigned id{0};
    unsigned int StartAddr{0x20};
    unsigned int ResetCycles{10};
    unsigned int BurstLengthByte{16};
    unsigned int NumberOfIterations{1};

public:
    SC_HAS_PROCESS(testbench);
    testbench(sc_core::sc_module_name nm)
    : sc_core::sc_module(nm)
    , intor_pe("intor_pe", intor)
    , tgt_pe("tgt_pe", tgt) {
        SC_THREAD(run);
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
        intor_bfm.bind_wdata(wdata);
        intor_bfm.bind_b(b);
        intor_bfm.bind_ar(ar);
        intor_bfm.bind_rresp(rresp);
        // signals to bfm
        tgt_bfm.bind_aw(aw);
        tgt_bfm.bind_wdata(wdata);
        tgt_bfm.bind_b(b);
        tgt_bfm.bind_ar(ar);
        tgt_bfm.bind_rresp(rresp);
        // bfm to recorder
        tgt_bfm.isckt(tgt_rec.tsckt);
        // recorder to target
        tgt_rec.isckt(tgt);
    }

    tlm::tlm_generic_payload* prepare_trans(size_t len) {
        auto trans = tlm::scc::tlm_mm<>::get().allocate<axi::axi4_extension>(len);
        tlm::scc::setId(*trans, id++);
        auto ext = trans->get_extension<axi::axi4_extension>();
        trans->set_data_length(len);
        trans->set_streaming_width(len);
        ext->set_size(scc::ilog2(std::min<size_t>(len, bus_cfg::BUSWIDTH / 8)));
        sc_assert(len < (bus_cfg::BUSWIDTH / 8) || len % (bus_cfg::BUSWIDTH / 8) == 0);
        ext->set_length((len * 8 - 1) / bus_cfg::BUSWIDTH);
        // ext->set_burst(len * 8 > bus_cfg::buswidth ? axi::burst_e::INCR : axi::burst_e::FIXED);
        ext->set_burst(axi::burst_e::INCR);
        ext->set_id(id);
        return trans;
    }

    inline void randomize(tlm::tlm_generic_payload* gp){
        for(auto i=0U; i<gp->get_data_length(); ++i)
            *(gp->get_data_ptr()+i)=scc::MT19937::uniform();
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
            { // 1
                auto trans = prepare_trans(BurstLengthByte);
                randomize(trans);
                trans->set_command(tlm::TLM_READ_COMMAND);
                trans->set_address(StartAddr);
                trans->acquire();
                intor_pe.transport(*trans, false);
                if(trans->get_response_status() != tlm::TLM_OK_RESPONSE)
                    SCCERR() << "Invalid response status" << trans->get_response_string();
                trans->release();
            }
            StartAddr += BurstLengthByte;
            { // 2
                auto trans = prepare_trans(BurstLengthByte);
                trans->set_command(tlm::TLM_WRITE_COMMAND);
                trans->set_address(StartAddr);
                randomize(trans);
                trans->acquire();
                intor_pe.transport(*trans, false);
                if(trans->get_response_status() != tlm::TLM_OK_RESPONSE)
                    SCCERR() << "Invalid response status" << trans->get_response_string();
                trans->release();
            }
            StartAddr += BurstLengthByte;
        }
        wait(100, SC_NS);
        sc_stop();
    }

};

jmp_buf env;
void  ABRThandler(int sig){
    longjmp(env, 1);
}

int sc_main(int argc, char* argv[]) {
    sc_report_handler::set_actions(SC_ID_MORE_THAN_ONE_SIGNAL_DRIVER_, SC_DO_NOTHING);
    scc::init_logging(
            scc::LogConfig()
            .logLevel(static_cast<scc::log>(7))
            .logAsync(false)
            .coloredOutput(true));
    sc_report_handler::set_actions(SC_ERROR, SC_LOG | SC_CACHE_REPORT | SC_DISPLAY);
    signal(SIGABRT, ABRThandler);
    signal(SIGSEGV, ABRThandler);
#ifdef HAS_CCI
    scc::configurable_tracer trace("axi_axi_test",
                                   scc::tracer::file_type::NONE, // define the kind of transaction trace
                                   true,                         // enables vcd
                                   true);
#else
    scc::tracer trace("axi_axi_test",
                                   scc::tracer::file_type::NONE, // define the kind of transaction trace
                                   true);                        // enables vcd
#endif
    if(setjmp(env) == 0) {
        testbench tb("tb");
        sc_core::sc_start(1_ms);
        SCCINFO() << "Finished";
        return 0;
    } else {
        return -1;
    }
}
