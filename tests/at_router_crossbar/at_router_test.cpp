
#include "testbench.h"
#include <catch2/catch_test_macros.hpp>
#include <factory.h>
#include <limits>
#include <sysc/kernel/sc_process_handle.h>
#include <sysc/kernel/sc_time.h>
#include <tlm/scc/tlm_gp_shared.h>
#undef CHECK
#include <algorithm>
#include <catch2/catch_all.hpp>
#include <cstdint>
#include <queue>

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

struct config {
    uint64_t addr = 0;
    unsigned len = 0;
    unsigned socket_nr = std::numeric_limits<unsigned>::max();
    config(uint64_t addr, unsigned len, unsigned socket_nr)
    : addr(addr)
    , len(len)
    , socket_nr(socket_nr) {}
};
void execute(testbench& dut, std::queue<config>& jobs) {
    auto cfg = jobs.front();
    jobs.pop();
    tlm::tlm_generic_payload gp;
    for(auto i = 0u; i < 16; i++) {
        prepare_trans(gp, tlm::TLM_WRITE_COMMAND, cfg.addr + i * cfg.len, 42u, cfg.len);
        auto t = sc_core::SC_ZERO_TIME;
        dut.isck[cfg.socket_nr]->b_transport(gp, t);
        std::cout << "Finished access to 0x" << std::hex << gp.get_address() << "!\n";
    }
    delete[] gp.get_data_ptr();
}

TEST_CASE("1-x-n", "[router][tlm-level]") {
    auto& dut = factory::get<testbench>();
    std::queue<config> jobs;
    unsigned cycles{0};
    sc_start(10 * dut.clk.read());
    jobs.emplace(0x0, 16, 0);
    jobs.emplace(0x400, 16, 0);
    jobs.emplace(0x800, 16, 0);
    jobs.emplace(0xC00, 16, 0);
    std::vector<sc_core::sc_process_handle> processes;
    for(auto i = 0; i < jobs.size(); ++i) {
        processes.push_back(std::move(sc_spawn([&dut, &jobs]() { execute(dut, jobs); })));
    }
    auto all_terminated = false;
    do {
        sc_start(20 * dut.clk.read());
        cycles += 20;
        all_terminated = std::all_of(processes.begin(), processes.end(), [](auto const& process) { return process.terminated(); });
    } while(!all_terminated && cycles < 1000);
    REQUIRE(all_terminated);
    REQUIRE(cycles < 270);
}

TEST_CASE("m-x-1", "[router][tlm-level]") {
    auto& dut = factory::get<testbench>();
    std::queue<config> jobs;
    unsigned cycles{0};
    sc_start(10 * dut.clk.read());
    jobs.emplace(0x0, 16, 0);
    jobs.emplace(0x0, 16, 1);
    std::vector<sc_core::sc_process_handle> processes;
    for(auto i = 0; i < jobs.size(); ++i) {
        processes.push_back(std::move(sc_spawn([&dut, &jobs]() { execute(dut, jobs); })));
    }
    auto all_terminated = false;
    do {
        sc_start(20 * dut.clk.read());
        cycles += 20;
        all_terminated = std::all_of(processes.begin(), processes.end(), [](auto const& process) { return process.terminated(); });
    } while(!all_terminated && cycles < 1000);
    REQUIRE(all_terminated);
    REQUIRE(cycles < 430);
}

TEST_CASE("m-x-n", "[router][tlm-level]") {
    auto& dut = factory::get<testbench>();
    std::queue<config> jobs;
    unsigned cycles{0};
    sc_start(10 * dut.clk.read());
    jobs.emplace(0x0, 16, 0);
    jobs.emplace(0x400, 16, 0);
    jobs.emplace(0x800, 16, 0);
    jobs.emplace(0xC00, 16, 0);
    jobs.emplace(0x0, 16, 1);
    jobs.emplace(0x400, 16, 1);
    jobs.emplace(0x800, 16, 1);
    jobs.emplace(0xC00, 16, 1);
    std::vector<sc_core::sc_process_handle> processes;
    for(auto i = 0; i < jobs.size(); ++i) {
        processes.push_back(std::move(sc_spawn([&dut, &jobs]() { execute(dut, jobs); })));
    }
    auto all_terminated = false;
    do {
        sc_start(20 * dut.clk.read());
        cycles += 20;
        all_terminated = std::all_of(processes.begin(), processes.end(), [](auto const& process) { return process.terminated(); });
    } while(!all_terminated && cycles < 1000);
    REQUIRE(all_terminated);
    REQUIRE(cycles < 290);
}

} // namespace scc
