#ifndef _TESTBENCH_H_
#define _TESTBENCH_H_

#include <cxs/cxs_tlm.h>
#include <scc/cci_util.h>
#include <scc/configurer.h>
#include <scc/memory.h>
#include <scc/observer.h>
#include <scc/router.h>
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
    tlm::scc::initiator_mixin<tlm::tlm_initiator_socket<scc::LT>> isck0{"isck0"};
    tlm::scc::initiator_mixin<tlm::tlm_initiator_socket<scc::LT>> isck1{"isck1"};
    scc::router<scc::LT> router{"router", 6, 2};
    scc::memory_tl<1_kB, scc::LT> mem0{"mem0"};
    scc::memory_tl<1_kB, scc::LT> mem1{"mem1"};
    scc::memory_tl<18_MB, scc::LT> mem2{"mem2"};
    scc::memory_tl<24_MB, scc::LT> mem3{"mem3"};
    scc::memory_tl<88_MB, scc::LT> mem4{"mem4"};
    scc::memory<4_GB> mem5{"mem5"};

    testbench()
    : testbench(sc_core::sc_gen_unique_name("testbench", false)) {}

    testbench(sc_core::sc_module_name const& nm)
    : sc_module(nm) {
        isck0(router.target[0]);
        isck1(router.target[1]);
        router.set_initiator_base(1, 1_MB);
        router.bind_target(mem0.target, 0, 0, 1_kB);
        router.bind_target(mem1.target, 1, 1_kB, 1_kB);
        router.bind_target(mem2.target, 2, 16_MB, 16_MB);
        router.bind_target(mem3.target, 3, 32_MB, 20_MB);
        router.bind_target(mem4.target, 4, 64_MB, 20_MB, false);
        router.initiator[5].bind(mem5.target);
        router.set_default_target(5);
        mem0.clk_i(clk);
        mem1.clk_i(clk);
        mem2.clk_i(clk);
        mem3.clk_i(clk);
        mem4.clk_i(clk);
    }
    void start_of_simulation() { clk = 10_ns; }
};
} // namespace scc
#endif // _TESTBENCH_H_
