
#include "testbench.h"
#include <factory.h>
#include <sysc/kernel/sc_time.h>
#include <tlm/scc/tlm_gp_shared.h>
#undef CHECK
#include <array>
#include <catch2/catch_all.hpp>
#include <cstdint>

using namespace sc_core;
namespace scc {
factory::add<testbench> tb;

template <typename T> void prepare_trans(tlm::tlm_generic_payload& trans, tlm::tlm_command cmd, uint64_t addr, T val, unsigned len) {
    unsigned char* data = len ? new unsigned char[len] : nullptr;
    if(cmd == tlm::TLM_WRITE_COMMAND) {
        memcpy(data, &val, len);
        trans.set_command(cmd);
    }
    if(cmd == tlm::TLM_READ_COMMAND) {
        memset(data, 0, len);
        trans.set_command(tlm::TLM_READ_COMMAND);
    }
    trans.set_address(addr);
    trans.set_data_ptr(data);
    trans.set_data_length(len);
    trans.set_streaming_width(len);
    trans.set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);
}

template <typename T> void prepare_trans(tlm::tlm_generic_payload& trans, tlm::tlm_command cmd, uint64_t addr, T val) {
    unsigned len = cmd == tlm::TLM_IGNORE_COMMAND ? 0 : sizeof(val);
    tlm::tlm_generic_payload gp0;
    prepare_trans(gp0, tlm::TLM_WRITE_COMMAND, addr, val, len);
}

TEST_CASE("simple access", "[router][tlm-level]") {
    auto& dut = factory::get<testbench>();
    uint64_t addr = 0x100;
    unsigned len = 16;
    auto f = [&dut, &addr, &len]() {
        tlm::tlm_generic_payload gp0;
        prepare_trans(gp0, tlm::TLM_WRITE_COMMAND, addr, 42u, len);
        auto t = sc_core::SC_ZERO_TIME;
        dut.isck0->b_transport(gp0, t);
        wait(20_ns);
        tlm::tlm_generic_payload gp1;
        prepare_trans(gp1, tlm::TLM_READ_COMMAND, addr, len);
        t = sc_core::SC_ZERO_TIME;
        dut.isck1->b_transport(gp0, t);
        wait(20_ns);
        delete[] gp0.get_data_ptr();
        delete[] gp1.get_data_ptr();
    };
    unsigned cycles{0};
    sc_start(sc_core::SC_ZERO_TIME);
    sc_start(10 * dut.clk.read());
    auto run1 = sc_spawn(f);
    while(cycles < 1000 && !run1.terminated()) {
        sc_start(20 * dut.clk.read());
        cycles += 20;
    }
}

} // namespace scc
