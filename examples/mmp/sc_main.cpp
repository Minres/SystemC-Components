/*
 * sc_main.cpp
 *
 *  Created on:
 *      Author:
 */

#include "scc/report.h"
#include "tlm/tlm_id.h"
#include <array>
#include <vector>
#include <map>
#include <tlm/tlm_mm.h>

#include <mmp/peripheral_base.h>
#include <mmp/peripheral_register_base.h>

#include <util/ities.h>

#include "tlm_utils/simple_initiator_socket.h"

using namespace sc_core;
class testbench;

class MmpExampleRegs : public mmp::peripheral_register_base<MmpExampleRegs> {
public:
  /* Registers of MmpRegs class */
  std::array<mmp::bitfield_register<uint32_t>, 3> registers{{
    {"R0", 0x00} /* a 32 bit register @ peripheral base address + 0x0 */ ,
    {"R1", 0x04} /* a 32 bit register @ peripheral base address + 0x4 */ ,
    {"R2", 0x08} /* a 32 bit register @ at peripheral base address + 0x8 */
  }};
  /* Individual named bitfields of MmpExampleRegs' registers */
  std::array<mmp::bitfield<uint32_t>, 5> bitfields{{
    {getRegister("R0"), "R0", 0, 32,"mmpregs.R0"} /* a 32-bit field inside R0 register [31:0]*/ ,
    {getRegister("R1"), "BITFIELD_0", 0, 1, "mmpregs.R1.BF0"} /* a 1-bit field inside R1 register [0:0]*/ ,
    {getRegister("R1"), "BITFIELD_1", 3, 3, "mmpregs.R1.BF1"} /* a 3-bit field inside R1 register [5:3]*/  ,
    {getRegister("R2"), "BITFIELD_0", 0, 16, "mmpregs.R2.BF0"} /* a 16-bit field inside R1 register [15:0]*/ ,
    {getRegister("R2"), "BITFIELD_1", 16, 16, "mmpregs.R2.BF1"} /* a 16-bit field inside R1 register [31:16]*/
  }};

  MmpExampleRegs(sc_core::sc_module_name name) : mmp::peripheral_register_base<MmpExampleRegs>{name} {};
};

class MmpExample: public mmp::MemoryMappedPeripheralBase<MmpExampleRegs, testbench> {
  SC_HAS_PROCESS(MmpExample);

private:
  mmp::bitfield<uint32_t>& r_io_{regs->getBitfield("R0", "R0", "mmpregs.R0")};
  mmp::bitfield<uint32_t>& r_inputconfig_{regs->getBitfield("R1", "BITFIELD_0", "mmpregs.R1.BF0")};
  mmp::bitfield<uint32_t>& r_outputconfig_{regs->getBitfield("R1", "BITFIELD_1", "mmpregs.R1.BF1")};
  mmp::bitfield<uint32_t>& r_control_{regs->getBitfield("R2", "BITFIELD_0", "mmpregs.R2.BF0")};
  mmp::bitfield<uint32_t>& r_status_{regs->getBitfield("R2", "BITFIELD_1", "mmpregs.R2.BF1")};

  void reset() {
    for (auto& bf : regs->bitfields) {
      bf = 0;
    }
  }

  void do_something(void) { r_status_ = 0xAAAA;}
  void do_somethingelse(void) { r_status_ = 0x5555;}

  uint32_t action_on_statusread(void){return 0;}

public:
  MmpExample(sc_core::sc_module_name name, mmp::PerParams&& params, testbench* owner = nullptr)
  : MemoryMappedPeripheralBase(name, std::move(params), owner) {
    reset();
    r_control_.setWriteCallback([this](mmp::bitfield<uint32_t>&, uint32_t& valueToWrite) {
      static const int CASE_0 = 0;
      r_control_ = valueToWrite;
      if (valueToWrite == CASE_0) {
        do_something();
      } else {
        do_somethingelse();
      }
      SC_REPORT_INFO("", "New Control Value Set");
    });
    r_status_.setReadCallback([this](const mmp::bitfield<uint32_t>&) {
      return(action_on_statusread());
    });
    r_io_.setReadCallback([this](const mmp::bitfield<uint32_t>&) {
      SCCINFO() << "r_io_ " << "read: 0x" << std::hex << r_io_.get() << std::endl;
      return(r_io_.get());
    });
    r_io_.setWriteCallback([this](mmp::bitfield<uint32_t>&, uint32_t& valueToWrite) {
      SCCINFO() << "r_io_ " << "write: 0x" << std::hex << valueToWrite << std::endl;
    });
  }
};

const unsigned SOCKET_WIDTH = 64;

class testbench : public sc_core::sc_module {
public:
    SC_HAS_PROCESS(testbench);
    sc_core::sc_time clk_period{10, sc_core::SC_NS};
    sc_core::sc_clock clk{"clk", clk_period, 0.5, sc_core::SC_ZERO_TIME, true};
    sc_core::sc_signal<bool> rst{"rst"};

    std::unique_ptr<tlm_utils::simple_initiator_socket_tagged<testbench>> intor_;
    //axi::axi_initiator_socket<SOCKET_WIDTH> intor{"intor"};
    //axi::axi_target_socket<SOCKET_WIDTH> tgt{"tgt"};

    std::map<std::string, std::unique_ptr< mmp::PeripheralBase<testbench>>> duts_{};

    testbench(sc_core::sc_module_name nm)
    : sc_core::sc_module(nm)
    //, intor_pe("intor_pe", intor)
    //, tgt_pe("tgt_pe", tgt)
    {
        SC_THREAD(run);
        intor_ = util::make_unique<tlm_utils::simple_initiator_socket_tagged<testbench>>(
            std::string{"initiator"}.c_str());
        //intor_.register_b_transport(this, &testbench::b_transport, i)
    //    intor_pe.clk_i(clk);
    //    tgt_pe.clk_i(clk);

    //    intor(tgt);
    }

    template <class PERIPHERAL_T>
    void addPeripheral(std::string name, mmp::PerParams&& per_params) {
      auto per = util::make_unique<PERIPHERAL_T>(name.c_str(), std::move(per_params), this);
      per->rst_in_(this->rst);
      intor_->bind(*(per->sock_t_.get()));
      duts_.insert(std::make_pair(name, std::move(per)));
    }

    tlm::tlm_generic_payload* prepare_trans(size_t len) {
      static int id = 0;
      auto trans = tlm::tlm_mm<>::get().allocate(); //new tlm::tlm_generic_payload;
      setId(*trans, id++);
      trans->set_data_length(len);
      trans->set_streaming_width(len);
      trans->set_data_ptr(new uint8_t[len]);
      return trans;
    }

    void run() {
        unsigned int StartAddr          = 0;
        unsigned int ResetCycles        = 10;
        unsigned int NumberOfIterations = 1000;
        rst.write(false);
        for(size_t i = 0; i < ResetCycles; ++i)
            wait(clk.posedge_event());
        rst.write(true);
        wait(clk.posedge_event());

        for(auto& [dut_name, dut_instance]: duts_){
          std::cout << "test " << dut_name << std::endl;

          /* WRITE test */
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

          /* READ test */
          trans = prepare_trans(4);
          trans->set_command(tlm::TLM_READ_COMMAND);
          trans->set_address(0);
          trans->acquire();
        //  sc_core::sc_time t{0, sc_core::SC_NS};
          (*intor_)->b_transport(*trans, t);
          if(trans->get_response_status() != tlm::TLM_OK_RESPONSE)
              SCCERR() << "Invalid response status" << trans->get_response_string();
          uint32_t out = *(uint32_t*)trans->get_data_ptr();
          if(out != in)
            SCCERR() << "Invalid response value " << "out[0x" << std::hex << out << "] != in[0x" << in << "]";
          else
            SCCINFO() << "Read-back successfull " << "out[0x" << std::hex << out << "] == in[0x" << in << "]";
          trans->release();
        }

        wait(100, SC_NS);
        sc_stop();

    }
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

    testbench test("peripheral-test");

    mmp::PerParams per1params{
      /* .base_addr = */  0x1000,
      /* .size  = */      4*sizeof(uint32_t),
      /* .num_irqs = */   0,
      /* .num_regs = */   3
    };
    test.addPeripheral<MmpExample>("per1", std::move(per1params));

    sc_core::sc_start(1_ms);
    SCCINFO() << "Finished";

    return 0;
}
