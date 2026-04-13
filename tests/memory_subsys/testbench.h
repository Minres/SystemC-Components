#ifndef _TESTBENCH_H_
#define _TESTBENCH_H_

#include <array>
#include <cxs/cxs_tlm.h>
#include <limits>
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
struct dmi_probe_target : public sc_core::sc_module {
    static constexpr auto window_size = uint64_t{64};
    tlm::scc::target_mixin<tlm::tlm_target_socket<scc::LT>> target{"target"};
    std::array<uint8_t, window_size> storage{};

    dmi_probe_target()
    : dmi_probe_target(sc_core::sc_gen_unique_name("dmi_probe_target", false)) {}

    explicit dmi_probe_target(sc_core::sc_module_name const& nm)
    : sc_module(nm) {
        target.register_b_transport([](tlm::tlm_generic_payload& gp, sc_core::sc_time&) { gp.set_response_status(tlm::TLM_OK_RESPONSE); });
        target.register_get_direct_mem_ptr([this](tlm::tlm_generic_payload& gp, tlm::tlm_dmi& dmi_data) -> bool {
            auto const start = gp.get_address();
            dmi_data.set_start_address(start);
            dmi_data.set_end_address(start + window_size - 1);
            dmi_data.set_dmi_ptr(storage.data());
            dmi_data.set_granted_access(tlm::tlm_dmi::DMI_ACCESS_READ_WRITE);
            dmi_data.set_read_latency(sc_core::SC_ZERO_TIME);
            dmi_data.set_write_latency(sc_core::SC_ZERO_TIME);
            return true;
        });
    }
};

struct testbench : public sc_core::sc_module {

    using transaction_type = tlm::tlm_base_protocol_types::tlm_payload_type;
    using phase_type = tlm::tlm_base_protocol_types::tlm_phase_type;

    static constexpr uint64_t high_range_size = 4_kB;
    static constexpr uint64_t high_range_base = std::numeric_limits<uint64_t>::max() - (high_range_size - 1);

    sc_core::sc_signal<sc_core::sc_time> clk{"clk"};
    sc_core::sc_signal<bool> rst{"rst"};
    tlm::scc::initiator_mixin<tlm::tlm_initiator_socket<scc::LT>> isck0{"isck0"};
    tlm::scc::initiator_mixin<tlm::tlm_initiator_socket<scc::LT>> isck1{"isck1"};

    scc::router<scc::LT> router{"router", 7, 2};
    scc::memory_tl<1_kB, scc::LT> mem0{"mem0"};
    scc::memory_tl<1_kB, scc::LT> mem1{"mem1"};
    scc::memory_tl<18_MB, scc::LT> mem2{"mem2"};
    scc::memory_tl<24_MB, scc::LT> mem3{"mem3"};
    scc::memory_tl<88_MB, scc::LT> mem4{"mem4"};
    scc::memory<4_GB> mem5{"mem5"};
    dmi_probe_target dmi_probe{"dmi_probe"};

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

        router.bind_target(dmi_probe.target, 6, high_range_base, high_range_size, false);
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
