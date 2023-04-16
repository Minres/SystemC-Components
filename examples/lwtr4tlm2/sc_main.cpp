/*
 * sc_main.cpp
 *
 *  Created on:
 *      Author:
 */

#include <array>
#include <tlm/scc/lwtr/tlm2_lwtr.h>
#include <scc_sysc.h>
#include <scc/memory.h>
#include <tlm/scc/initiator_mixin.h>
#include <tlm/scc/lwtr/lwtr4tlm2_extension_registry.h>
#include <tlm/scc/tlm_gp_shared.h>
#include <tlm/scc/tlm_id.h>

using namespace sc_core;

const unsigned SOCKET_WIDTH = 64;

enum class resp_e : uint8_t { OKAY = 0x0, EXOKAY = 0x1, SLVERR = 0x2, DECERR = 0x3 };
namespace lwtr { template<> struct value_converter<::resp_e> {
	static value to_value(::resp_e v) {
		switch(v){
		case ::resp_e::OKAY:return value("OKAY");
		case ::resp_e::EXOKAY:return value("EXOKAY");
		case ::resp_e::SLVERR:return value("SLVERR");
		case ::resp_e::DECERR:return value("DECERR");
		default:return value("ILLEGAL");
		}
	}
};}

struct test_extension : public tlm::tlm_extension<test_extension> {
    test_extension() = default;
    test_extension(const test_extension& o) = default;
    tlm::tlm_extension_base* clone() const override { return new test_extension(*this); }
    void copy_from(tlm::tlm_extension_base const& ext) override {
        auto const* new_ext = dynamic_cast<const test_extension*>(&ext);
        assert(new_ext);
        (*this) = *new_ext;
    }
    uint8_t prot{0};
    resp_e resp{resp_e::OKAY};
};
namespace lwtr { template <class Archive>
void record(Archive &ar, test_extension const& e) {ar & field("prot", e.prot) & field("resp", e.resp);}}

class testbench : public sc_core::sc_module {
public:
    SC_HAS_PROCESS(testbench);
    sc_core::sc_time clk_period{10, sc_core::SC_NS};
    sc_core::sc_clock clk{"clk", clk_period, 0.5, sc_core::SC_ZERO_TIME, true};
    sc_core::sc_signal<bool> rst{"rst"};

private:
    tlm::scc::initiator_mixin<tlm::tlm_initiator_socket<scc::LT>> top_isck{"top_isck"};
    tlm::scc::lwtr::tlm2_lwtr_recorder<scc::LT> itor2mem{"itor2mem"};
    scc::memory<1_GB> mem{"mem"};

    unsigned id{0};
    unsigned int StartAddr{0};
    unsigned int ResetCycles{10};
    unsigned int BurstLengthByte{16};
    unsigned int NumberOfIterations{8};

public:
    testbench(sc_core::sc_module_name nm)
    : sc_core::sc_module(nm) {
        SC_THREAD(run);
        top_isck(itor2mem.ts);
		itor2mem.is(mem.target);
		mem.rd_resp_delay=5_ns;
		mem.wr_resp_delay=2_ns;
    }

    void trace( sc_trace_file* trf ) const override {
        scc::sc_trace(trf, id, std::string(name())+".id"); // trace a local variable
    }

    tlm::tlm_generic_payload* prepare_trans(size_t len) {
        auto trans = tlm::scc::tlm_mm<tlm::tlm_base_protocol_types, false>::get().allocate();
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
        wait(clk.posedge_event());
        rst.write(true);
        wait(clk.posedge_event());
        for(int i = 0; i < NumberOfIterations; ++i) {
            SCCDEBUG("testbench") << "executing transactions in iteration " << i;
            { // 1
                tlm::scc::tlm_gp_shared_ptr trans = prepare_trans(BurstLengthByte);
                trans->set_command(tlm::TLM_READ_COMMAND);
                trans->set_address(StartAddr);
                auto delay = SC_ZERO_TIME;
                top_isck->b_transport(*trans, delay);
                if(trans->get_response_status() != tlm::TLM_OK_RESPONSE)
                    SCCERR() << "Invalid response status" << trans->get_response_string();
            }
            StartAddr += BurstLengthByte;
            wait(5_ns);
            { // 2
                tlm::scc::tlm_gp_shared_ptr trans = prepare_trans(BurstLengthByte);
                trans->set_command(tlm::TLM_WRITE_COMMAND);
                trans->set_address(StartAddr);
                auto delay = 1_ns;
                top_isck->b_transport(*trans, delay);
                if(trans->get_response_status() != tlm::TLM_OK_RESPONSE)
                    SCCERR() << "Invalid response status" << trans->get_response_string();
            }
            StartAddr += BurstLengthByte;
            wait(5_ns);
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
            .logLevel(scc::log::INFO)
            .logAsync(false)
            .coloredOutput(true));
    // clang-format off
    sc_report_handler::set_actions(SC_ERROR, SC_LOG | SC_CACHE_REPORT | SC_DISPLAY);
    lwtr::tx_text_init();
    lwtr::tx_db db("lwtr4tlm2.txlog");
    testbench tb("tb");
    scc::hierarchy_dumper d("lwtr4tlm2.json", scc::hierarchy_dumper::D3JSON);
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
