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
    using bus_cfg = axi::bfm::axi_cfg</*BUSWIDTH=*/32, /*ADDRWIDTH=*/32, /*IDWIDTH=*/4, /*USERWIDTH=*/0>;

    sc_core::sc_time clk_period{10, sc_core::SC_NS};
    sc_core::sc_clock clk{"clk", clk_period, 0.5, sc_core::SC_ZERO_TIME, true};
    sc_core::sc_signal<bool> rst{"rst"};
    axi::axi_initiator_socket<bus_cfg::BUSWIDTH> intor{"intor"};
    axi::scv::axi_recorder_module<bus_cfg::BUSWIDTH> intor_rec{"intor_rec"};
    axi::bfm::axi4_initiator<bus_cfg> intor_bfm{"into_bfm"};
    sc_core::sc_signal<sc_dt::sc_uint<bus_cfg::IDWIDTH>>       aw_id{"aw_id"};
    sc_core::sc_signal<sc_dt::sc_uint<bus_cfg::ADDRWIDTH>>     aw_addr{"aw_addr"};
    sc_core::sc_signal<bool>                                   aw_ready{"aw_ready"};
    sc_core::sc_signal<bool>                                   aw_valid{"aw_valid"};
    sc_core::sc_signal<sc_dt::sc_uint<3>>                      aw_prot{"aw_prot"};
    sc_core::sc_signal<sc_dt::sc_uint<3>>                      aw_size{"aw_size"};
    sc_core::sc_signal<sc_dt::sc_uint<4>>                      aw_cache{"aw_cache"};
    sc_core::sc_signal<sc_dt::sc_uint<2>>                      aw_burst{"aw_burst"};
    sc_core::sc_signal<sc_dt::sc_uint<4>>                      aw_qos{"aw_qos"};
    sc_core::sc_signal<sc_dt::sc_uint<4>>                      aw_region{"aw_region"};
    sc_core::sc_signal<sc_dt::sc_uint<8>>                      aw_len{"aw_len"};
    sc_core::sc_signal<sc_dt::sc_biguint<bus_cfg::BUSWIDTH>>   w_data{"w_data"};
    sc_core::sc_signal<sc_dt::sc_uint<bus_cfg::BUSWIDTH / 8>>  w_strb{"w_strb"};
    sc_core::sc_signal<bool>                                   w_last{"w_last"};
    sc_core::sc_signal<bool>                                   w_valid{"w_valid"};
    sc_core::sc_signal<bool>                                   w_ready{"w_ready"};
    sc_core::sc_signal<sc_dt::sc_uint<bus_cfg::IDWIDTH>>       w_id{"w_id"};
    sc_core::sc_signal<bool>                                   b_valid{"b_valid"};
    sc_core::sc_signal<bool>                                   b_ready{"b_ready"};
    sc_core::sc_signal<sc_dt::sc_uint<bus_cfg::IDWIDTH>>       b_id{"b_id"};
    sc_core::sc_signal<sc_dt::sc_uint<2>>                      b_resp{"b_resp"};
    sc_core::sc_signal<sc_dt::sc_uint<bus_cfg::IDWIDTH>>       ar_id{"ar_id"};
    sc_core::sc_signal<sc_dt::sc_uint<bus_cfg::ADDRWIDTH>>     ar_addr{"ar_addr"};
    sc_core::sc_signal<sc_dt::sc_uint<8>>                      ar_len{"ar_len"};
    sc_core::sc_signal<sc_dt::sc_uint<3>>                      ar_size{"ar_size"};
    sc_core::sc_signal<sc_dt::sc_uint<2>>                      ar_burst{"ar_burst"};
    sc_core::sc_signal<sc_dt::sc_uint<4>>                      ar_cache{"ar_cache"};
    sc_core::sc_signal<sc_dt::sc_uint<3>>                      ar_prot{"ar_prot"};
    sc_core::sc_signal<sc_dt::sc_uint<4>>                      ar_qos{"ar_qos"};
    sc_core::sc_signal<sc_dt::sc_uint<4>>                      ar_region{"ar_region"};
    sc_core::sc_signal<bool>                                   ar_valid{"ar_valid"};
    sc_core::sc_signal<bool>                                   ar_ready{"ar_ready"};
    sc_core::sc_signal<sc_dt::sc_uint<bus_cfg::IDWIDTH>>       r_id{"r_id"};
    sc_core::sc_signal<sc_dt::sc_biguint<bus_cfg::BUSWIDTH>>   r_data{"r_data"};
    sc_core::sc_signal<sc_dt::sc_uint<2>>                      r_resp{"r_resp"};
    sc_core::sc_signal<bool>                                   r_last{"r_last"};
    sc_core::sc_signal<bool>                                   r_valid{"r_valid"};
    sc_core::sc_signal<bool>                                   r_ready{"r_ready"};
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
        intor_bfm.aw_id(aw_id);
        intor_bfm.aw_addr(aw_addr);
        intor_bfm.aw_ready(aw_ready);
        intor_bfm.aw_valid(aw_valid);
        intor_bfm.aw_prot(aw_prot);
        intor_bfm.aw_size(aw_size);
        intor_bfm.aw_cache(aw_cache);
        intor_bfm.aw_burst(aw_burst);
        intor_bfm.aw_qos(aw_qos);
        intor_bfm.aw_region(aw_region);
        intor_bfm.aw_len(aw_len);
        //intor_bfm.aw_user(aw_user);
        intor_bfm.w_data(w_data);
        intor_bfm.w_strb(w_strb);
        intor_bfm.w_last(w_last);
        intor_bfm.w_id(w_id);
        intor_bfm.w_valid(w_valid);
        intor_bfm.w_ready(w_ready);
        //intor_bfm.w_user(w_user);
        intor_bfm.b_valid(b_valid);
        intor_bfm.b_ready(b_ready);
        intor_bfm.b_id(b_id);
        intor_bfm.b_resp(b_resp);
        //intor_bfm.b_user(b_user);
        intor_bfm.ar_id(ar_id);
        intor_bfm.ar_addr(ar_addr);
        intor_bfm.ar_len(ar_len);
        intor_bfm.ar_size(ar_size);
        intor_bfm.ar_burst(ar_burst);
        intor_bfm.ar_cache(ar_cache);
        intor_bfm.ar_prot(ar_prot);
        intor_bfm.ar_qos(ar_qos);
        intor_bfm.ar_region(ar_region);
        intor_bfm.ar_valid(ar_valid);
        intor_bfm.ar_ready(ar_ready);
        //intor_bfm.ar_user(ar_user);
        intor_bfm.r_id(r_id);
        intor_bfm.r_data(r_data);
        intor_bfm.r_resp(r_resp);
        intor_bfm.r_last(r_last);
        intor_bfm.r_valid(r_valid);
        intor_bfm.r_ready(r_ready);
        //intor_bfm.r_user(r_user);
        // signals to bfm
        tgt_bfm.aw_id(aw_id);
        tgt_bfm.aw_addr(aw_addr);
        tgt_bfm.aw_ready(aw_ready);
        tgt_bfm.aw_valid(aw_valid);
        tgt_bfm.aw_prot(aw_prot);
        tgt_bfm.aw_size(aw_size);
        tgt_bfm.aw_cache(aw_cache);
        tgt_bfm.aw_burst(aw_burst);
        tgt_bfm.aw_qos(aw_qos);
        tgt_bfm.aw_region(aw_region);
        tgt_bfm.aw_len(aw_len);
        //tgt_bfm.aw_user(aw_user);
        tgt_bfm.w_data(w_data);
        tgt_bfm.w_strb(w_strb);
        tgt_bfm.w_last(w_last);
        tgt_bfm.w_id(w_id);
        tgt_bfm.w_valid(w_valid);
        tgt_bfm.w_ready(w_ready);
        //tgt_bfm.w_user(w_user);
        tgt_bfm.b_valid(b_valid);
        tgt_bfm.b_ready(b_ready);
        tgt_bfm.b_id(b_id);
        tgt_bfm.b_resp(b_resp);
        //tgt_bfm.b_user(b_user);
        tgt_bfm.ar_id(ar_id);
        tgt_bfm.ar_addr(ar_addr);
        tgt_bfm.ar_len(ar_len);
        tgt_bfm.ar_size(ar_size);
        tgt_bfm.ar_burst(ar_burst);
        tgt_bfm.ar_cache(ar_cache);
        tgt_bfm.ar_prot(ar_prot);
        tgt_bfm.ar_qos(ar_qos);
        tgt_bfm.ar_region(ar_region);
        tgt_bfm.ar_valid(ar_valid);
        tgt_bfm.ar_ready(ar_ready);
        //tgt_bfm.ar_user(ar_user);
        tgt_bfm.r_id(r_id);
        tgt_bfm.r_data(r_data);
        tgt_bfm.r_resp(r_resp);
        tgt_bfm.r_last(r_last);
        tgt_bfm.r_valid(r_valid);
        tgt_bfm.r_ready(r_ready);
        //tgt_bfm.r_user(r_user);
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
