/*
 * sc_main.cpp
 *
 *  Created on:
 *      Author:
 */

#include <scc/report.h>
#include <tlm/scc/tlm_id.h>
#include <tlm/scc/tlm_mm.h>

#include <scc/tlm_target_bfs.h>
#include <scc/tlm_target_bfs_register_base.h>

#include <util/ities.h>

#include <tlm_utils/simple_initiator_socket.h>

#include <array>
#include <map>
#include <vector>
using namespace sc_core;
using namespace sc_dt;
class testbench;

/*
 * @Brief: This class holds the bitfield-registers for the tlm_target_bfs_example
 */
class tlm_target_bfs_register_example : public scc::tlm_target_bfs_register_base<tlm_target_bfs_register_example> {
public:
    /* Registers of tlm_target_bfs_example class */
    std::array<scc::bitfield_register<uint32_t>, 3> registers{{
        {"R0", 0x00} /* a 32 bit register @ peripheral base address + 0x0 */,
        {"R1", 0x04} /* a 32 bit register @ peripheral base address + 0x4 */,
        {"R2", 0x08} /* a 32 bit register @ at peripheral base address + 0x8 */
    }};
    /* Individual named bitfields of tlm_target_bfs_example' registers */
    std::array<scc::bitfield<uint32_t>, 4> bitfields{{
        //{getRegister("R0"), "R0", 0, 32,"regs.R0"} /* a 32-bit field inside R0 register [31:0]*/ ,
        {getRegister("R1"), "BITFIELD_0", 0, 1, "regs.R1.BF0"} /* a 1-bit field inside R1 register [0:0]*/,
        {getRegister("R1"), "BITFIELD_1", 3, 3, "regs.R1.BF1"} /* a 3-bit field inside R1 register [5:3]*/,
        {getRegister("R2"), "BITFIELD_0", 0, 16, "regs.R2.BF0"} /* a 16-bit field inside R1 register [15:0]*/,
        {getRegister("R2"), "BITFIELD_1", 16, 16, "regs.R2.BF1"} /* a 16-bit field inside R1 register [31:16]*/
    }};

    tlm_target_bfs_register_example(sc_core::sc_module_name name)
    : scc::tlm_target_bfs_register_base<tlm_target_bfs_register_example>{name} {};
};

/*
 * @Brief: This class defines the tlm_target_bfs_example.
 */
class tlm_target_bfs_example : public scc::tlm_target_bfs<tlm_target_bfs_register_example, testbench> {
    SC_HAS_PROCESS(tlm_target_bfs_example);

private:
    scc::bitfield_register<uint32_t>& r_io_{regs->getRegister("R0")};
    scc::bitfield<uint32_t>& r_inputconfig_{regs->getBitfield("R1", "BITFIELD_0", "regs.R1.BF0")};
    scc::bitfield<uint32_t>& r_outputconfig_{regs->getBitfield("R1", "BITFIELD_1", "regs.R1.BF1")};
    scc::bitfield<uint32_t>& r_control_{regs->getBitfield("R2", "BITFIELD_0", "regs.R2.BF0")};
    scc::bitfield<uint32_t>& r_status_{regs->getBitfield("R2", "BITFIELD_1", "regs.R2.BF1")};

    void reset() {
        for(auto& bf : regs->bitfields) {
            bf = 0;
        }
    }

    void do_something(void) { r_status_ = 0xAAAA; }
    void do_somethingelse(void) { r_status_ = 0x5555; }
    uint32_t action_on_statusread(void) { return 0; }

public:
    tlm_target_bfs_example(sc_core::sc_module_name name, scc::tlm_target_bfs_params&& params,
                           testbench* owner = nullptr)
    : tlm_target_bfs(name, std::move(params), owner) {
        reset();
        /*
         * Define bitfield specific Read/Write callbacks.
         * with c++14 you can use `auto` inside lamda expressions. This reduces the effort to look up the bitfield's
         * template arguments e.g., `r_control_.setWriteCallback([this](auto&&, uint32_t& valueToWrite) {...});`
         */
        r_control_.setWriteCallback([this](scc::bitfield<uint32_t>&, uint32_t& valueToWrite) {
            static const int CASE_0 = 0;
            r_control_ = valueToWrite;
            if(valueToWrite == CASE_0) {
                do_something();
            } else {
                do_somethingelse();
            }
            SCCINFO() << "r_control_ "
                      << "write: 0x" << std::hex << r_control_ << std::endl;
        });
        r_control_.setReadCallback([this](const scc::bitfield<uint32_t>&) {
            SCCINFO() << "r_control_ "
                      << "read: 0x" << std::hex << r_control_ << std::endl;
            return (r_control_.get());
        });
        r_status_.setReadCallback([this](const scc::bitfield<uint32_t>&) {
            SCCINFO() << "r_status_ "
                      << "read: 0x" << std::hex << r_status_ << std::endl;
            return (action_on_statusread());
        });
        r_io_.setReadCallback([this](const scc::bitfield_register<uint32_t>&, uint32_t& result) {
            SCCINFO() << "r_io_ "
                      << "read: 0x" << std::hex << result << std::endl;
        });
        r_io_.setWriteCallback([this](scc::bitfield_register<uint32_t>&, uint32_t& valueToWrite) {
            SCCINFO() << "r_io_ "
                      << "write: 0x" << std::hex << valueToWrite << std::endl;
        });
    }
};

class testbench : public sc_core::sc_module {
public:
    SC_HAS_PROCESS(testbench);
    sc_core::sc_time clk_period{10, sc_core::SC_NS};
    sc_core::sc_clock clk{"clk", clk_period, 0.5, sc_core::SC_ZERO_TIME, true};
    sc_core::sc_signal<bool> rst{"rst"};

    std::unique_ptr<tlm_utils::simple_initiator_socket_tagged<testbench>> intor_;
    std::map<std::string, std::unique_ptr<scc::tlm_target_bfs_base<testbench>>> duts_{};

    testbench(sc_core::sc_module_name nm)
    : sc_core::sc_module(nm) {
        SC_THREAD(run);
        intor_ =
            util::make_unique<tlm_utils::simple_initiator_socket_tagged<testbench>>(std::string{"initiator"}.c_str());
    }

    template <class PERIPHERAL_T> void addPeripheral(std::string name, scc::tlm_target_bfs_params&& per_params) {
        auto per = util::make_unique<PERIPHERAL_T>(name.c_str(), std::move(per_params), this);
        per->rst_in_(this->rst);
        intor_->bind(*(per->sock_t_.get()));
        duts_.insert(std::make_pair(name, std::move(per)));
    }

    tlm::tlm_generic_payload* prepare_trans(size_t len) {
        static int id = 0;
        auto trans = tlm::scc::tlm_mm<>::get().allocate(); // new tlm::tlm_generic_payload;
        tlm::scc::setId(*trans, id++);
        trans->set_data_length(len);
        trans->set_streaming_width(len);
        trans->set_data_ptr(new uint8_t[len]);
        return trans;
    }

    void run() {
        unsigned int StartAddr = 0;
        unsigned int ResetCycles = 10;
        unsigned int NumberOfIterations = 1000;
        rst.write(false);
        for(size_t i = 0; i < ResetCycles; ++i)
            wait(clk.posedge_event());
        rst.write(true);
        wait(clk.posedge_event());

        for(auto& e : duts_) {
            std::cout << "test " << e.first << std::endl;

            /* WRITE test callback-able complete register (R0) */
            auto trans = prepare_trans(4);
            trans->set_command(tlm::TLM_WRITE_COMMAND);
            trans->set_address(0); // r_io_
            trans->acquire();
            sc_core::sc_time t{0, sc_core::SC_NS};
            *((uint32_t*)trans->get_data_ptr()) = 0xDEADBEEF;
            (*intor_)->b_transport(*trans, t);
            if(trans->get_response_status() != tlm::TLM_OK_RESPONSE)
                SCCERR() << "Invalid response status" << trans->get_response_string();
            uint32_t in = *((uint32_t*)trans->get_data_ptr());
            trans->release();

            /* READ test callback-able complete register (R0) */
            trans = prepare_trans(4);
            trans->set_command(tlm::TLM_READ_COMMAND);
            trans->set_address(0);
            trans->acquire();
            (*intor_)->b_transport(*trans, t);
            if(trans->get_response_status() != tlm::TLM_OK_RESPONSE)
                SCCERR() << "Invalid response status" << trans->get_response_string();
            uint32_t out = *(uint32_t*)trans->get_data_ptr();
            if(out != in)
                SCCERR() << "Invalid response value "
                         << "out[0x" << std::hex << out << "] != in[0x" << in << "]";
            else
                SCCINFO() << "Read-back successfull "
                          << "out[0x" << std::hex << out << "] == in[0x" << in << "]";
            trans->release();

            /* WRITE test callback-able bitfield regs.R2.BF0 alias control */
            trans = prepare_trans(2);
            trans->set_command(tlm::TLM_WRITE_COMMAND);
            trans->set_address(8); // r_control_
            trans->acquire();
            *((uint16_t*)trans->get_data_ptr()) = 0x4321;
            (*intor_)->b_transport(*trans, t);
            if(trans->get_response_status() != tlm::TLM_OK_RESPONSE)
                SCCERR() << "Invalid response status" << trans->get_response_string();
            trans->release();

            /* READ test callback-able register. Activates two bitfields status + control */
            trans = prepare_trans(4);
            trans->set_command(tlm::TLM_READ_COMMAND);
            trans->set_address(8); // r_control_ + r_status_
            trans->acquire();
            (*intor_)->b_transport(*trans, t);
            if(trans->get_response_status() != tlm::TLM_OK_RESPONSE)
                SCCERR() << "Invalid response status" << trans->get_response_string();
            SCCINFO() << ">R:8(4) := 0x" << std::hex << *(uint32_t*)trans->get_data_ptr() << std::endl;
            trans->release();

            /* READ test callback-able register. Activate only status bitfield */
            trans = prepare_trans(2);
            trans->set_command(tlm::TLM_READ_COMMAND);
            trans->set_address(10); // r_control_ + r_status_
            trans->acquire();
            (*intor_)->b_transport(*trans, t);
            if(trans->get_response_status() != tlm::TLM_OK_RESPONSE)
                SCCERR() << "Invalid response status" << trans->get_response_string();
            SCCINFO() << ">R:10(2) := 0x" << std::hex << *(uint16_t*)trans->get_data_ptr() << std::endl;
            trans->release();
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
              .dontCreateBroker(true)
              .coloredOutput(true));
    // clang-format off
    sc_report_handler::set_actions(SC_ERROR, SC_LOG | SC_CACHE_REPORT | SC_DISPLAY);

    testbench test("peripheral-test");

    scc::tlm_target_bfs_params_t per1params{/* .base_addr = */ 0x1000,
                                            /* .size  = */ 4 * sizeof(uint32_t),
                                            /* .num_irqs = */ 0,
                                            /* .num_regs = */ 3};
    test.addPeripheral<tlm_target_bfs_example>("per1", std::move(per1params));

    sc_core::sc_start(1_ms);
    SCCINFO() << "Finished";

    return 0;
}
