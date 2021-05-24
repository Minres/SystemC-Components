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
#ifdef WITH_CCI
#include <scc/configurable_tracer.h>
#endif
#include "scc/report.h"
#include <tlm/scc/tlm_id.h>
#include <array>
#include <axi/pe/simple_initiator.h>
#include <axi/pe/simple_target.h>
#include <axi/axi_pin2tlm_adaptor.h>
#include <axi/axi_tlm2pin_adaptor.h>
#include <tlm/scc/tlm_mm.h>

using namespace sc_core;
using namespace sc_dt;
using namespace axi;
using namespace axi::pe;

const unsigned SOCKET_WIDTH = 64;

class testbench : public sc_core::sc_module {
public:
    SC_HAS_PROCESS(testbench);
    sc_core::sc_time clk_period{10, sc_core::SC_NS};
    sc_core::sc_clock clk{"clk", clk_period, 0.5, sc_core::SC_ZERO_TIME, true};
    sc_core::sc_signal<bool> rst{"rst"};
    axi::axi_initiator_socket<SOCKET_WIDTH> intor{"intor"};
#ifdef WITH_SCV
    axi::scv::axi_recorder_module<SOCKET_WIDTH> intor_rec{"intor_rec"};
#endif
    axi::axi_target_socket<SOCKET_WIDTH> tgt{"tgt"};

    // pin level adaptors
    axi::axi_pin2tlm_adaptor<SOCKET_WIDTH> 	pin2tlm_adaptor{"pin2tlm_adaptor"};
    axi::axi_tlm2pin_adaptor<SOCKET_WIDTH>	tlm2pin_adaptor{"tlm2pin_adaptor"};

    sc_signal  <bool>           axi_aw_valid_s    {"axi_aw_valid_s"};
    sc_signal  <bool>           axi_aw_ready_s    {"axi_aw_ready_s"};
    sc_signal  <sc_uint<32>>    axi_aw_id_s       {"axi_aw_id_s"};
    sc_signal  <sc_uint<8> >    axi_aw_len_s      {"axi_aw_len_s"};
    sc_signal  <sc_uint<3> >    axi_aw_size_s     {"axi_aw_size_s"};
    sc_signal  <sc_uint<2> >    axi_aw_burst_s    {"axi_aw_burst_s"};
    sc_signal  <bool>           axi_aw_lock_s     {"axi_aw_lock_s"};
    sc_signal  <sc_uint<4> >    axi_aw_cache_s    {"axi_aw_cache_s"};
    sc_signal  <sc_uint<3> >    axi_aw_prot_s     {"axi_aw_prot_s"};
    sc_signal  <sc_uint<4> >    axi_aw_qos_s      {"axi_aw_qos_s"};
    sc_signal  <sc_uint<4> >    axi_aw_region_s   {"axi_aw_region_s"};
    sc_signal  <bool>           axi_w_valid_s     {"axi_w_valid_s"};
    sc_signal  <bool>           axi_w_ready_s     {"axi_w_ready_s"};
    sc_signal  <bool>           axi_w_last_s      {"axi_w_last_s"};
    sc_signal  <bool>           axi_b_valid_s     {"axi_b_valid_s"};
    sc_signal  <bool>           axi_b_ready_s     {"axi_b_ready_s"};
    sc_signal  <sc_uint<32>>    axi_b_id_s        {"axi_b_id_s"};
    sc_signal  <sc_uint<2> >    axi_b_resp_s      {"axi_b_resp_s"};
    sc_signal  <bool>           axi_ar_valid_s    {"axi_ar_valid_s"};
    sc_signal  <bool>           axi_ar_ready_s    {"axi_ar_ready_s"};
    sc_signal  <sc_uint<32>>    axi_ar_id_s       {"axi_ar_id_s"};
    sc_signal  <sc_uint<8> >    axi_ar_len_s      {"axi_ar_len_s"};
    sc_signal  <sc_uint<3> >    axi_ar_size_s     {"axi_ar_size_s"};
    sc_signal  <sc_uint<2> >    axi_ar_burst_s    {"axi_ar_burst_s"};
    sc_signal  <bool>           axi_ar_lock_s     {"axi_ar_lock_s"};
    sc_signal  <sc_uint<4> >    axi_ar_cache_s    {"axi_ar_cache_s"};
    sc_signal  <sc_uint<3> >    axi_ar_prot_s     {"axi_ar_prot_s"};
    sc_signal  <sc_uint<4> >    axi_ar_qos_s      {"axi_ar_qos_s"};
    sc_signal  <sc_uint<4> >    axi_ar_region_s   {"axi_ar_region_s"};
    sc_signal  <bool>           axi_r_valid_s     {"axi_r_valid_s"};
    sc_signal  <bool>           axi_r_ready_s     {"axi_r_ready_s"};
    sc_signal  <sc_uint<32>>    axi_r_id_s        {"axi_r_id_s"};
    sc_signal  <sc_uint<2> >    axi_r_resp_s      {"axi_r_resp_s"};
    sc_signal  <bool>           axi_r_last_s      {"axi_r_last_s"};
    sc_signal  <sc_uint<32> >   axi_aw_addr_s     {"axi_aw_addr_s"};
    sc_signal  <sc_uint<32> >   axi_ar_addr_s     {"axi_ar_addr_s"};
    sc_signal  <sc_uint<SOCKET_WIDTH/8> >   axi_w_strb_s      {"axi_w_strb_s"};
    sc_signal  <sc_biguint<SOCKET_WIDTH> >   axi_w_data_s  {"axi_w_data_s"};
    sc_signal  <sc_biguint<SOCKET_WIDTH> >   axi_r_data_s  {"axi_r_data_s"};

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
        intor.bind(tlm2pin_adaptor.input_socket);
        pin2tlm_adaptor.output_socket.bind(tgt);
#endif
        pin2tlm_adaptor.clk_i(clk);
        tlm2pin_adaptor.clk_i(clk);
        pin2tlm_adaptor.resetn_i(rst);
        tlm2pin_adaptor.resetn_i(rst);

        tlm2pin_adaptor.aw_id_o		(axi_aw_id_s);
        tlm2pin_adaptor.aw_addr_o	(axi_aw_addr_s);
        tlm2pin_adaptor.aw_ready_i	(axi_aw_ready_s);
        tlm2pin_adaptor.aw_lock_o	(axi_aw_lock_s);
        tlm2pin_adaptor.aw_valid_o	(axi_aw_valid_s);
        tlm2pin_adaptor.aw_prot_o	(axi_aw_prot_s);
        tlm2pin_adaptor.aw_size_o	(axi_aw_size_s);
        tlm2pin_adaptor.aw_cache_o	(axi_aw_cache_s);
        tlm2pin_adaptor.aw_burst_o	(axi_aw_burst_s);
        tlm2pin_adaptor.aw_qos_o	(axi_aw_qos_s);
        tlm2pin_adaptor.aw_region_o	(axi_aw_region_s);
        tlm2pin_adaptor.aw_len_o	(axi_aw_len_s);
        tlm2pin_adaptor.w_data_o	(axi_w_data_s);
        tlm2pin_adaptor.w_strb_o	(axi_w_strb_s);
        tlm2pin_adaptor.w_last_o	(axi_w_last_s);
        tlm2pin_adaptor.w_valid_o	(axi_w_valid_s);
        tlm2pin_adaptor.w_ready_i	(axi_w_ready_s);
        tlm2pin_adaptor.b_valid_i	(axi_b_valid_s);
        tlm2pin_adaptor.b_ready_o	(axi_b_ready_s);
        tlm2pin_adaptor.b_id_i		(axi_b_id_s);
        tlm2pin_adaptor.b_resp_i	(axi_b_resp_s);
        tlm2pin_adaptor.ar_id_o		(axi_ar_id_s);
        tlm2pin_adaptor.ar_addr_o	(axi_ar_addr_s);
        tlm2pin_adaptor.ar_len_o	(axi_ar_len_s);
        tlm2pin_adaptor.ar_size_o	(axi_ar_size_s);
        tlm2pin_adaptor.ar_burst_o	(axi_ar_burst_s);
        tlm2pin_adaptor.ar_lock_o	(axi_ar_lock_s);
        tlm2pin_adaptor.ar_cache_o	(axi_ar_cache_s);
        tlm2pin_adaptor.ar_prot_o	(axi_ar_prot_s);
        tlm2pin_adaptor.ar_qos_o	(axi_ar_qos_s);
        tlm2pin_adaptor.ar_region_o	(axi_ar_region_s);
        tlm2pin_adaptor.ar_valid_o	(axi_ar_valid_s);
        tlm2pin_adaptor.ar_ready_i	(axi_ar_ready_s);
        tlm2pin_adaptor.r_id_i		(axi_r_id_s);
        tlm2pin_adaptor.r_data_i	(axi_r_data_s);
        tlm2pin_adaptor.r_resp_i	(axi_r_resp_s);
        tlm2pin_adaptor.r_valid_i	(axi_r_valid_s);
        tlm2pin_adaptor.r_ready_o	(axi_r_ready_s);
        tlm2pin_adaptor.r_last_i	(axi_r_last_s);


        pin2tlm_adaptor.aw_ready_o (axi_aw_ready_s);
        pin2tlm_adaptor.aw_id_i    (axi_aw_id_s);
        pin2tlm_adaptor.aw_addr_i  (axi_aw_addr_s);
        pin2tlm_adaptor.aw_lock_i  (axi_aw_lock_s);
        pin2tlm_adaptor.aw_valid_i (axi_aw_valid_s);
        pin2tlm_adaptor.aw_prot_i  (axi_aw_prot_s);
        pin2tlm_adaptor.aw_size_i  (axi_aw_size_s);
        pin2tlm_adaptor.aw_cache_i (axi_aw_cache_s);
        pin2tlm_adaptor.aw_burst_i (axi_aw_burst_s);
        pin2tlm_adaptor.aw_qos_i   (axi_aw_qos_s);
        pin2tlm_adaptor.aw_region_i(axi_aw_region_s);
        pin2tlm_adaptor.aw_len_i   (axi_aw_len_s);

        pin2tlm_adaptor.w_data_i 	(axi_w_data_s );
        pin2tlm_adaptor.w_strb_i 	(axi_w_strb_s );
        pin2tlm_adaptor.w_last_i 	(axi_w_last_s );
        pin2tlm_adaptor.w_valid_i	(axi_w_valid_s);
        pin2tlm_adaptor.w_ready_o	(axi_w_ready_s);

        pin2tlm_adaptor.b_valid_o	(axi_b_valid_s);
        pin2tlm_adaptor.b_ready_i	(axi_b_ready_s);
        pin2tlm_adaptor.b_id_o		(axi_b_id_s);
        pin2tlm_adaptor.b_resp_o	(axi_b_resp_s);

        pin2tlm_adaptor.ar_id_i    	(axi_ar_id_s    );
        pin2tlm_adaptor.ar_addr_i  	(axi_ar_addr_s  );
        pin2tlm_adaptor.ar_len_i   	(axi_ar_len_s   );
        pin2tlm_adaptor.ar_size_i  	(axi_ar_size_s  );
        pin2tlm_adaptor.ar_burst_i 	(axi_ar_burst_s );
        pin2tlm_adaptor.ar_lock_i  	(axi_ar_lock_s  );
        pin2tlm_adaptor.ar_cache_i 	(axi_ar_cache_s );
        pin2tlm_adaptor.ar_prot_i  	(axi_ar_prot_s  );
        pin2tlm_adaptor.ar_qos_i   	(axi_ar_qos_s   );
        pin2tlm_adaptor.ar_region_i	(axi_ar_region_s);
        pin2tlm_adaptor.ar_valid_i 	(axi_ar_valid_s );
        pin2tlm_adaptor.ar_ready_o	(axi_ar_ready_s);

        pin2tlm_adaptor.r_id_o    	(axi_r_id_s    );
        pin2tlm_adaptor.r_data_o  	(axi_r_data_s  );
        pin2tlm_adaptor.r_resp_o  	(axi_r_resp_s  );
        pin2tlm_adaptor.r_last_o 	(axi_r_last_s  );
        pin2tlm_adaptor.r_valid_o 	(axi_r_valid_s );
        pin2tlm_adaptor.r_ready_i	(axi_r_ready_s);
    }

    tlm::tlm_generic_payload* prepare_trans(size_t len) {
        auto trans = tlm::scc::tlm_mm<>::get().allocate<axi::axi4_extension>();
        tlm::scc::setId(*trans, id++);
        auto ext = trans->get_extension<axi::axi4_extension>();
        trans->set_data_length(len);
        trans->set_streaming_width(len);
        ext->set_size(scc::ilog2(std::min<size_t>(len, SOCKET_WIDTH / 8)));
        sc_assert(len < (SOCKET_WIDTH / 8) || len % (SOCKET_WIDTH / 8) == 0);
        ext->set_length((len * 8 - 1) / SOCKET_WIDTH);
        //ext->set_burst(len * 8 > SOCKET_WIDTH ? axi::burst_e::INCR : axi::burst_e::FIXED);
        ext->set_burst(axi::burst_e::INCR);
        ext->set_id(id);
        return trans;
    }

    void run() {
        unsigned int StartAddr          = 0;
        unsigned int ResetCycles        = 10;
	unsigned int BurstLengthByte    = 16;
	unsigned int NumberOfIterations = 1000;
        rst.write(false);
        for(size_t i = 0; i < ResetCycles; ++i)
            wait(clk.posedge_event());
        rst.write(true);
        wait(clk.posedge_event());
        std::array<uint8_t, 256> data;
        wait(clk.posedge_event());
        for(int i = 0; i < NumberOfIterations; ++i) {
            SCCDEBUG("testbench") << "executing transactions in iteration " << i;
            { // 1
                auto trans = prepare_trans(BurstLengthByte);
                trans->set_command(tlm::TLM_READ_COMMAND);
                trans->set_address(StartAddr);
                trans->set_data_ptr(data.data());
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
                trans->set_data_ptr(data.data());
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
#ifdef WITH_CCI
		      .dontCreateBroker(false)
#else
		      .dontCreateBroker(true)
#endif
		      .coloredOutput(true));
    sc_report_handler::set_actions(SC_ERROR, SC_LOG | SC_CACHE_REPORT | SC_DISPLAY);
#ifdef WITH_CCI
    ///////////////////////////////////////////////////////////////////////////
    // set up tracing & transaction recording
    ///////////////////////////////////////////////////////////////////////////
    scc::configurable_tracer trace("axi_pinlevel",
            scc::tracer::file_type::NONE, // bit3-bit1 define the kind of transaction trace
            1, // bit0 enables vcd
            true);
#endif
#ifdef WITH_SCV
    scv_startup();
    scv_tr_text_init();
    scv_tr_db* db = new scv_tr_db("axi_pinlevel.txlog");
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
