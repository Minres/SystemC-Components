
#include "testbench.h"
#include <factory.h>
#include <tlm/scc/tlm_gp_shared.h>
#undef CHECK
#include <array>
#include <catch2/catch_all.hpp>
#include <cstdint>
#include <deque>
#include <unordered_map>

using namespace sc_core;
namespace scc {
factory::add<testbench> tb;

template <typename T> void prepare_trans(tlm::tlm_generic_payload& trans, tlm::tlm_command cmd, uint64_t addr, T val) {
    unsigned len = cmd == tlm::TLM_IGNORE_COMMAND ? 0 : sizeof(val);
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

TEST_CASE("simple_read_write_with host memory map", "[memory][tlm-level]") {
    auto& dut = factory::get<testbench>();
    std::array<unsigned char, 256> ref_data;
    auto val = 256;
    for(auto& e : ref_data)
        e = --val;
    dut.rst.write(true);
    sc_start(10 * dut.clk.read());
    dut.rst.write(false);
    sc_start(dut.clk.read());
    {
        tlm::tlm_generic_payload gp;
        sc_core::sc_time t;
        for(uint64_t addr = 0; addr < 1024; addr += sizeof(uint64_t)) {
            prepare_trans(gp, tlm::TLM_WRITE_COMMAND, addr, 123456789ULL);
            dut.isck->b_transport(gp, t);
            delete gp.get_data_ptr();
            sc_start(dut.clk.read());
        }
    }
    {
        tlm::tlm_generic_payload gp;
        prepare_trans(gp, tlm::TLM_READ_COMMAND, 256, 0ULL);
        sc_core::sc_time t;
        dut.isck->b_transport(gp, t);
        uint64_t res;
        memcpy(&res, gp.get_data_ptr(), sizeof(uint64_t));
        REQUIRE(res == 123456789ULL);
        delete gp.get_data_ptr();
    }
    sc_start(dut.clk.read());
    {
        tlm::tlm_generic_payload gp;
        gp.set_command(tlm::TLM_IGNORE_COMMAND);
        gp.set_address(256);
        gp.set_data_ptr(nullptr);
        gp.set_data_length(256);
        gp.set_streaming_width(256);
        gp.set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);
        auto ext = new scc::host_mem_map_extension(ref_data.data());
        gp.set_extension(ext);
        dut.isck->transport_dbg(gp);
    }
    sc_start(dut.clk.read());
    {
        tlm::tlm_generic_payload gp;
        prepare_trans(gp, tlm::TLM_READ_COMMAND, 256, 0ULL);
        sc_core::sc_time t;
        dut.isck->b_transport(gp, t);
        uint64_t res;
        memcpy(&res, gp.get_data_ptr(), sizeof(uint64_t));
        REQUIRE(res == 0xf8f9fafbfcfdfeffULL);
        delete gp.get_data_ptr();
    }
    sc_start(dut.clk.read());
    {
        tlm::tlm_generic_payload gp;
        prepare_trans(gp, tlm::TLM_READ_COMMAND, 384, 0ULL);
        sc_core::sc_time t;
        dut.isck->b_transport(gp, t);
        uint64_t res;
        memcpy(&res, gp.get_data_ptr(), sizeof(uint64_t));
        REQUIRE(res == 0x78797a7b7c7d7e7fULL);
        delete gp.get_data_ptr();
    }
    sc_start(dut.clk.read());
    {
        tlm::tlm_generic_payload gp;
        gp.set_command(tlm::TLM_IGNORE_COMMAND);
        gp.set_address(256);
        gp.set_data_ptr(nullptr);
        gp.set_data_length(256);
        gp.set_streaming_width(256);
        gp.set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);
        auto ext = new scc::host_mem_map_extension(nullptr);
        gp.set_extension(ext);
        dut.isck->transport_dbg(gp);
    }
    sc_start(dut.clk.read());
    {
        tlm::tlm_generic_payload gp;
        prepare_trans(gp, tlm::TLM_READ_COMMAND, 256, 0ULL);
        sc_core::sc_time t;
        dut.isck->b_transport(gp, t);
        uint64_t res;
        memcpy(&res, gp.get_data_ptr(), sizeof(uint64_t));
        REQUIRE(res == 123456789ULL);
        delete gp.get_data_ptr();
    }
    sc_start(dut.clk.read());
    {
        tlm::tlm_generic_payload gp;
        prepare_trans(gp, tlm::TLM_READ_COMMAND, 256, 0ULL);
        sc_core::sc_time t;
        dut.isck->b_transport(gp, t);
        uint64_t res;
        memcpy(&res, gp.get_data_ptr(), sizeof(uint64_t));
        REQUIRE(res == 123456789ULL);
        delete gp.get_data_ptr();
    }
    sc_start(dut.clk.read());
}

} // namespace scc
