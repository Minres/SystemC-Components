#ifndef _TESTBENCH_H_
#define _TESTBENCH_H_

#include <cxs/cxs_tlm.h>
#include <limits>
#include <scc/at_router/nb_router.h>
#include <scc/b2nb_adapter.h>
#include <scc/cci_util.h>
#include <scc/configurer.h>
#include <scc/memory.h>
#include <scc/observer.h>
#include <scc/router.h>
#include <scc/sc_variable.h>
#include <scc/tracer.h>
#include <systemc>
#include <tlm/scc/initiator_mixin.h>
#include <tlm/scc/lwtr/tlm2_lwtr.h>

using namespace sc_core;
using namespace sc_dt;
using namespace std;
namespace scc {

#define WITH_TRACING

const char* sc_gen_unique_name(const char*, bool preserve_first);
struct testbench : public sc_core::sc_module {

    using transaction_type = tlm::tlm_base_protocol_types::tlm_payload_type;
    using phase_type = tlm::tlm_base_protocol_types::tlm_phase_type;

    static constexpr uint64_t high_range_size = 4_kB;
    static constexpr uint64_t high_range_base = std::numeric_limits<uint64_t>::max() - (high_range_size - 1);

    sc_core::sc_signal<sc_core::sc_time> clk{"clk"};
    sc_core::sc_signal<bool> rst{"rst"};

#ifdef WITH_TRACING
    tlm::scc::lwtr::tlm2_lwtr_recorder<32> isck0_rec{"isck0_rec"};
    tlm::scc::lwtr::tlm2_lwtr_recorder<32> isck1_rec{"isck1_rec"};
    tlm::scc::lwtr::tlm2_lwtr_recorder<32> mem0_rec{"mem0_rec"};
    tlm::scc::lwtr::tlm2_lwtr_recorder<32> mem1_rec{"mem1_rec"};
    tlm::scc::lwtr::tlm2_lwtr_recorder<32> mem2_rec{"mem2_rec"};
    tlm::scc::lwtr::tlm2_lwtr_recorder<32> mem3_rec{"mem3_rec"};
#endif
    sc_core::sc_vector<tlm::scc::initiator_mixin<tlm::tlm_initiator_socket<32>>> isck{"isck", 2};
    scc::b2nb_adapter<32> b2nb0{"b2nb0"};
    scc::b2nb_adapter<32> b2nb1{"b2nb1"};
    scc::router<32> router{"router", 4, 2};
    scc::memory_tl<1_kB, 32, 20> mem0{"mem0"};
    scc::memory_tl<1_kB, 32, 20> mem1{"mem1"};
    scc::memory_tl<1_kB, 32, 20> mem2{"mem2"};
    scc::memory_tl<1_kB, 32, 20> mem3{"mem3"};

    inline void configure_memory(scc::memory_tl<1_kB, 32, 20>& m) {
        m.clk_i(clk);
        m.rd_resp_delay.set_value(30_ns);
        m.wr_resp_delay.set_value(500_ns);
    }

    testbench()
    : testbench(sc_core::sc_gen_unique_name("testbench", false)) {}

    testbench(sc_core::sc_module_name const& nm)
    : sc_module(nm) {
        isck[0](b2nb0.tsck);
        isck[1](b2nb1.tsck);
#ifdef WITH_TRACING
        b2nb0.isck(isck0_rec.ts);
        isck0_rec.is(router.target[0]);
        b2nb1.isck(isck1_rec.ts);
        isck1_rec.is(router.target[1]);
        router.bind_target(mem0_rec.ts, 0, 0, 1_kB);
        router.bind_target(mem1_rec.ts, 1, 1_kB, 1_kB);
        router.bind_target(mem2_rec.ts, 2, 2_kB, 1_kB);
        router.bind_target(mem3_rec.ts, 3, 3_kB, 1_kB);
        mem0_rec.is(mem0.target);
        mem1_rec.is(mem1.target);
        mem2_rec.is(mem2.target);
        mem3_rec.is(mem3.target);
#else
        b2nb0.isck(router.target[0]);
        b2nb1.isck(router.target[1]);
        router.bind_target(mem0.target, 0, 0, 1_kB);
        router.bind_target(mem1.target, 1, 1_kB, 1_kB);
        router.bind_target(mem2.target, 2, 2_kB, 1_kB);
        router.bind_target(mem3.target, 3, 3_kB, 1_kB);
#endif
        router.set_initiator_base(1, 1_MB);
        router.set_at_architecture(scc::at_router::hub::create<>);
        router.clk_i(clk);
        configure_memory(mem0);
        configure_memory(mem1);
        configure_memory(mem2);
        configure_memory(mem3);
        b2nb0.tsck.wr_resp_accept_delay_per_beat = 10_ns;
        b2nb0.tsck.rd_resp_accept_delay_per_beat = 10_ns;
        b2nb1.tsck.wr_resp_accept_delay_per_beat = 10_ns;
        b2nb1.tsck.rd_resp_accept_delay_per_beat = 10_ns;
    }
    void start_of_simulation() { clk = 10_ns; }
};
} // namespace scc
#endif // _TESTBENCH_H_
