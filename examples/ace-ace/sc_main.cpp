/*
 * sc_main.cpp
 *
 *  Created on:
 *      Author:
 */
#ifndef SC_INCLUDE_DYNAMIC_PROCESSES
#define SC_INCLUDE_DYNAMIC_PROCESSES
#endif
#include <array>
#include <axi/pe/simple_initiator.h>
#include <axi/pe/simple_target.h>
#include <axi/pe/simple_ace_target.h>
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
    axi::axi_target_socket<SOCKET_WIDTH> tgt{"tgt"};
    axi::ace_target_socket<SOCKET_WIDTH> tgt_ace{"tgt_ace"};

    testbench(sc_core::sc_module_name nm)
    : sc_core::sc_module(nm)
    , intor_pe("ace_intor_pe", intor)
    , axi_tgt_pe("axi_tgt_pe", tgt)
    , ace_tgt_pe("target_ace", tgt_ace) {
        SC_THREAD(run);
        intor_pe.clk_i(clk);
        axi_tgt_pe.clk_i(clk);
        ace_tgt_pe.clk_i(clk);
        intor(tgt_ace);
        ace_tgt_pe.isckt_axi(tgt);

        // here set snoop_cb of initiator and prepare snoop data
        intor_pe.set_snoop_cb([this](axi::axi_protocol_types::tlm_payload_type& trans) -> unsigned {
            SCCTRACE(SCMOD)<<" enter snoop_cb of tgt_pe(simple target) in sc_main";
            auto addr = trans.get_address();
            uint8_t const* src = reinterpret_cast<uint8_t const*>(&addr);
            for(size_t i = 0; i < trans.get_data_length(); ++i) {
                *(trans.get_data_ptr() + i) = i % 2 ? i : 0x1111;
            }
            return 0;
        });

    }

    tlm::tlm_generic_payload* prepare_trans_ace(size_t len, uint8_t const* data = nullptr) {
        auto trans = tlm::scc::tlm_mm<tlm::tlm_base_protocol_types, false>::get().allocate<axi::ace_extension>();
        tlm::scc::setId(*trans, id++);
        auto ext = trans->get_extension<axi::ace_extension>();
        //  Transaction length (in bytes)=m_length:Total number of bytes of the transaction.
        trans->set_data_length(len);
        // ask Eyck?? what's difference between set_data_ptr() and set_address()
        trans->set_data_ptr(new uint8_t[len]);
        if(data)
            for(auto i = 0U; i < len; ++i)
                *(trans->get_data_ptr() + i) = *(data + i);
        // no clear definition??? check later??
        trans->set_streaming_width(len);
        // arsize, one signal in ARchannel(size, the number of bytes in each data transfer in a read transaction.)
        //len= BurstLengthByte=16
        ext->set_size(scc::ilog2(std::min<size_t>(len, SOCKET_WIDTH / 8)));
        // check whether trans is byte aligned
        sc_assert(len < (SOCKET_WIDTH / 8) || len % (SOCKET_WIDTH / 8) == 0);
        // arlen (Length, the exact number of data transfers in a read transaction.)  -1 because last beat in response??
        ext->set_length((len * 8 - 1) / SOCKET_WIDTH);

        ext->set_burst(axi::burst_e::INCR);
        ext->set_id(id);

        ext->set_snoop(axi::snoop_e::READ_SHARED); // set it so that is_data_less return true???

        return trans;
    }

    void run() {
        unsigned int StartAddr = 0;
        unsigned int ResetCycles = 10;
        unsigned int BurstLengthByte = 16;
        unsigned int NumberOfIterations = 1;
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
                auto trans = prepare_trans_ace(BurstLengthByte);

                SCCDEBUG(SCMOD) << "generating transactions " << trans;
                trans->set_command(tlm::TLM_READ_COMMAND);
                trans->set_address(StartAddr);
                trans->set_data_ptr(data.data());
                trans->acquire();
                // before prepare the trans, here use tranport() of initiator
                 intor_pe.transport(*trans, false);
                if(trans->get_response_status() != tlm::TLM_OK_RESPONSE)
                    SCCERR() << "Invalid response status" << trans->get_response_string();
                trans->release();
                StartAddr += BurstLengthByte;
            }
            { // 2
                auto trans = prepare_trans_ace(BurstLengthByte);
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
            {   // start snoop
                auto trans = prepare_trans_ace(BurstLengthByte);
                SCCDEBUG(SCMOD) << "start snoop ";
                SCCDEBUG(SCMOD) << "generating transactions " << trans;
                trans->set_command(tlm::TLM_READ_COMMAND);
                trans->set_address(StartAddr);
                trans->set_data_ptr(data.data());
                trans->acquire();
                ace_tgt_pe.snoop(*trans);
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
    axi::pe::simple_target<SOCKET_WIDTH> axi_tgt_pe;
    axi::pe::simple_ace_target<SOCKET_WIDTH> ace_tgt_pe;


    unsigned id{0};
};

int sc_main(int argc, char* argv[]) {
    sc_report_handler::set_actions(SC_ID_MORE_THAN_ONE_SIGNAL_DRIVER_, SC_DO_NOTHING);
    // clang-format off
    scc::init_logging(
            scc::LogConfig()
            .logLevel(scc::log::TRACEALL)
            .logAsync(false)
            .coloredOutput(true));
    // clang-format on
    sc_report_handler::set_actions(SC_ERROR, SC_LOG | SC_CACHE_REPORT | SC_DISPLAY);
    scc::configurable_tracer trace("ace_axi_test",
                                   scc::tracer::file_type::TEXT, // define the kind of transaction trace
                                   true,                         // enables vcd
                                   true);
    testbench mstr("master");
    sc_core::sc_start(10_ms);
    SCCINFO() << "Finished";
    return 0;
}
