/*
 * sc_main.cpp
 *
 *  Created on:
 *      Author:
 */

#include <array>
#include <axi/pe/axi_initiator.h>
#include <axi/pe/simple_target.h>
#include <axi/pin/axi4_initiator.h>
#include <axi/pin/axi4_target.h>
#include <axi/scv/recorder_modules.h>
#include <csetjmp>
#include <csignal>
#include <scc.h>

using namespace sc_core;
using namespace axi;
using namespace axi::pe;

class testbench : public sc_core::sc_module {
public:
    using bus_cfg = axi::axi4_cfg</*BUSWIDTH=*/32, /*ADDRWIDTH=*/32, /*IDWIDTH=*/4, /*USERWIDTH=*/1>;

    sc_core::sc_time clk_period{10, sc_core::SC_NS};
    sc_core::sc_clock clk{"clk", clk_period, 0.5, sc_core::SC_ZERO_TIME, true};
    sc_core::sc_signal<bool> rst{"rst"};
    axi::axi_initiator_socket<bus_cfg::BUSWIDTH> intor{"intor"};
    axi::scv::axi_recorder_module<bus_cfg::BUSWIDTH> intor_rec{"intor_rec"};
    axi::pin::axi4_initiator<bus_cfg> intor_bfm{"intor_bfm"};

    axi::aw_ch<bus_cfg, axi::signal_types> aw;
    axi::wdata_ch<bus_cfg, axi::signal_types> wdata;
    axi::b_ch<bus_cfg, axi::signal_types> b;
    axi::ar_ch<bus_cfg, axi::signal_types> ar;
    axi::rresp_ch<bus_cfg, axi::signal_types> rresp;
    axi::pin::axi4_target<bus_cfg> tgt_bfm{"tgt_bfm"};

    axi::scv::axi_recorder_module<bus_cfg::BUSWIDTH> tgt_rec{"tgt_rec"};
    axi::axi_target_socket<bus_cfg::BUSWIDTH> tgt{"tgt"};

private:
    sc_core::sc_attribute<bool> same_id{"same_id", false};

    axi::pe::axi_initiator<bus_cfg::BUSWIDTH> intor_pe;
    axi::pe::simple_target<bus_cfg::BUSWIDTH> tgt_pe;
    unsigned id{0};
    unsigned int ResetCycles{10};
    unsigned int BurstLengthByte{16};
    unsigned int NumberOfIterations{10};
    sc_core::sc_event start_trigger;
    uint8_t resp_cnt{0}, req_cnt{0};

public:
    SC_HAS_PROCESS(testbench);
    testbench(sc_core::sc_module_name nm)
    : sc_core::sc_module(nm)
    , intor_pe("intor_pe", intor)
    , tgt_pe("tgt_pe", tgt) {
        add_attribute(same_id);
        SC_THREAD(run0);
        SC_THREAD(run1);
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
        intor_bfm.bind_w(wdata);
        intor_bfm.bind_b(b);
        intor_bfm.bind_ar(ar);
        intor_bfm.bind_r(rresp);
        // signals to bfm
        tgt_bfm.bind_aw(aw);
        tgt_bfm.bind_w(wdata);
        tgt_bfm.bind_b(b);
        tgt_bfm.bind_ar(ar);
        tgt_bfm.bind_r(rresp);
        // bfm to recorder
        tgt_bfm.isckt(tgt_rec.tsckt);
        // recorder to target
        tgt_rec.isckt(tgt);
        tgt_pe.set_operation_cb([this](axi::axi_protocol_types::tlm_payload_type& trans) -> unsigned {
            auto addr = trans.get_address();
            uint8_t const* src = reinterpret_cast<uint8_t const*>(&addr);
            for(size_t i = 0; i < trans.get_data_length(); ++i) {
                *(trans.get_data_ptr() + i) = i % 2 ? i : resp_cnt;
            }
            resp_cnt++;
            return 0;
        });
    }

    tlm::tlm_generic_payload* prepare_trans(uint64_t start_address, size_t len, unsigned id_offs = 0,
                                            unsigned addr_offs = 0) {
        auto trans = tlm::scc::tlm_mm<>::get().allocate<axi::axi4_extension>(len);
        trans->set_address(start_address);
        tlm::scc::setId(*trans, id);
        auto ext = trans->get_extension<axi::axi4_extension>();
        trans->set_data_length(len);
        trans->set_streaming_width(len);
        ext->set_size(scc::ilog2(std::min<size_t>(len, bus_cfg::BUSWIDTH / 8)));
        sc_assert(len < (bus_cfg::BUSWIDTH / 8) || len % (bus_cfg::BUSWIDTH / 8) == 0);
        auto length = (len * 8 - 1) / bus_cfg::BUSWIDTH;
        if(start_address % (bus_cfg::BUSWIDTH / 8))
            length++;
        ext->set_length(length);
        // ext->set_burst(len * 8 > bus_cfg::buswidth ? axi::burst_e::INCR : axi::burst_e::FIXED);
        ext->set_burst(axi::burst_e::INCR);
        if(same_id.value)
            ext->set_id(0);
        else
            ext->set_id(id | id_offs);
        id = (id + 1) % 8;
        return trans;
    }

    inline void randomize(tlm::tlm_generic_payload& gp) {
        auto addr = gp.get_address();
        uint8_t const* src = reinterpret_cast<uint8_t const*>(&addr);
        for(size_t i = 0; i < gp.get_data_length(); ++i) {
            *(gp.get_data_ptr() + i) = i % 2 ? i : req_cnt;
        }
        req_cnt++;
    }
    void run0() {
        unsigned int StartAddr{0x20};
        rst.write(false);
        for(size_t i = 0; i < ResetCycles; ++i)
            wait(clk.posedge_event());
        rst.write(true);
        wait(clk.posedge_event());
        wait(clk.posedge_event());
        start_trigger.notify(sc_core::SC_ZERO_TIME);
        for(int i = 0; i < NumberOfIterations; ++i) {
            SCCDEBUG(SCMOD) << "run0 executing transactions in iteration " << i;
            { // 1
                tlm::scc::tlm_gp_shared_ptr trans = prepare_trans(StartAddr, BurstLengthByte);
                randomize(*trans);
                trans->set_command(tlm::TLM_READ_COMMAND);
                intor_pe.transport(*trans, false);
                if(trans->get_response_status() != tlm::TLM_OK_RESPONSE)
                    SCCERR() << "Invalid response status" << trans->get_response_string();
            }
            StartAddr += BurstLengthByte;
            { // 2
                tlm::scc::tlm_gp_shared_ptr trans = prepare_trans(StartAddr, BurstLengthByte);
                trans->set_command(tlm::TLM_WRITE_COMMAND);
                randomize(*trans);
                intor_pe.transport(*trans, false);
                if(trans->get_response_status() != tlm::TLM_OK_RESPONSE)
                    SCCERR() << "Invalid response status" << trans->get_response_string();
            }
            StartAddr += BurstLengthByte;
        }
        wait(100, SC_NS);
        sc_stop();
    }

    void run1() {
        unsigned int StartAddr{0x1020};
        wait(start_trigger);
        for(int i = 0; i < NumberOfIterations; ++i) {
            SCCDEBUG(SCMOD) << "run1 executing transactions in iteration " << i;
            { // 1
                tlm::scc::tlm_gp_shared_ptr trans = prepare_trans(StartAddr, BurstLengthByte, 0x8);
                randomize(*trans);
                trans->set_command(tlm::TLM_READ_COMMAND);
                intor_pe.transport(*trans, false);
                if(trans->get_response_status() != tlm::TLM_OK_RESPONSE)
                    SCCERR() << "Invalid response status" << trans->get_response_string();
            }
            StartAddr += BurstLengthByte;
            { // 2
                tlm::scc::tlm_gp_shared_ptr trans = prepare_trans(StartAddr, BurstLengthByte, 0x8);
                trans->set_command(tlm::TLM_WRITE_COMMAND);
                randomize(*trans);
                intor_pe.transport(*trans, false);
                if(trans->get_response_status() != tlm::TLM_OK_RESPONSE)
                    SCCERR() << "Invalid response status" << trans->get_response_string();
            }
            StartAddr += BurstLengthByte;
        }
    }
};

jmp_buf env;
void ABRThandler(int sig) { longjmp(env, 1); }

int sc_main(int argc, char* argv[]) {
    sc_report_handler::set_actions(SC_ID_MORE_THAN_ONE_SIGNAL_DRIVER_, SC_DO_NOTHING);
    // clang-format off
    scc::init_logging(
            scc::LogConfig()
            .logLevel(scc::log::INFO)
            .logAsync(false)
            .coloredOutput(true));
    // clang-format on
    sc_report_handler::set_actions(SC_ERROR, SC_LOG | SC_CACHE_REPORT | SC_DISPLAY);
    signal(SIGABRT, ABRThandler);
    signal(SIGSEGV, ABRThandler);
    auto cfg_file = argc == 2 ? argv[1] : "";
    scc::configurer cfg(cfg_file);
#ifdef HAS_CCI
    scc::configurable_tracer trace("axi4_tlm_pin_tlm",
                                   scc::tracer::file_type::NONE, // define the kind of transaction trace
                                   true,                         // enables vcd
                                   true);
#else
    scc::tracer trace("axi4_tlm_pin_tlm",
                      scc::tracer::file_type::NONE, // define the kind of transaction trace
                      true);                        // enables vcd
#endif
    if(setjmp(env) == 0) {
        testbench tb("tb");
        cfg.configure();
        sc_core::sc_start(1_ms);
        auto errcnt = sc_report_handler::get_count(SC_ERROR);
        auto warncnt = sc_report_handler::get_count(SC_WARNING);
        SCCINFO() << "Finished, there were " << errcnt << " error" << (errcnt == 1 ? "" : "s") << " and " << warncnt
                  << " warning" << (warncnt == 1 ? "" : "s");
        return errcnt + warncnt;
    } else {
        return -1;
    }
}
