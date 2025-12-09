#ifndef _TESTBENCH_H_
#define _TESTBENCH_H_

#include <cxs/cxs_tlm.h>
#include <scc/cci_util.h>
#include <scc/configurer.h>
#include <scc/memory.h>
#include <scc/observer.h>
#include <scc/sc_variable.h>
#include <scc/tracer.h>
#include <string>
#include <systemc>
#include <tlm/scc/initiator_mixin.h>

using namespace sc_core;
using namespace sc_dt;
using namespace std;
namespace scc {

const char* sc_gen_unique_name(const char*, bool preserve_first);
struct testbench : public sc_core::sc_module {

    using transaction_type = tlm::tlm_base_protocol_types::tlm_payload_type;
    using phase_type = tlm::tlm_base_protocol_types::tlm_phase_type;

    sc_core::sc_signal<sc_core::sc_time> clk{"clk"};
    sc_core::sc_signal<bool> rst{"rst"};
    tlm::scc::initiator_mixin<tlm::tlm_initiator_socket<scc::LT>> isck{"isck"};
    scc::memory_tl<1_kB, scc::LT> mem{"mem"};

    testbench()
    : testbench(sc_core::sc_gen_unique_name("testbench", false)) {}

    testbench(sc_core::sc_module_name const& nm)
    : sc_module(nm) {
        isck(mem.target);
        mem.clk_i(clk);
    }
    void start_of_simulation() { clk = 10_ns; }
};
} // namespace scc
#endif // _TESTBENCH_H_
