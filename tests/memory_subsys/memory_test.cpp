
#include "testbench.h"
#include <factory.h>
#include <tlm/scc/tlm_gp_shared.h>
#undef CHECK
#include <array>
#include <catch2/catch_all.hpp>
#include <cstdint>

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

template <typename T> void do_dmi_access(T& isck, uint64_t address, uint64_t expected_size) {
    tlm::tlm_generic_payload gp;
    tlm::tlm_dmi dmi;
    gp.set_address(address);
    auto res = isck->get_direct_mem_ptr(gp, dmi);
    REQUIRE(res == true);
    REQUIRE(dmi.get_start_address() == address);
    REQUIRE(dmi.get_end_address() == (address + expected_size - 1));
    REQUIRE(dmi.is_read_write_allowed());
}

TEST_CASE("dmi_access", "[memory][tlm-level]") {
    auto& dut = factory::get<testbench>();
    std::array<unsigned char, 256> ref_data;
    auto val = 256;
    for(auto& e : ref_data)
        e = --val;
    dut.rst.write(true);
    sc_start(10 * dut.clk.read());
    dut.rst.write(false);
    sc_start(dut.clk.read());

    do_dmi_access(dut.isck0, 0, 1_kB);
    do_dmi_access(dut.isck0, 1_kB, 1_kB);
    do_dmi_access(dut.isck0, 16_MB, 16_MB);
    do_dmi_access(dut.isck0, 32_MB, 16_MB);
    do_dmi_access(dut.isck0, 48_MB, 4_MB);
    do_dmi_access(dut.isck0, 64_MB, 16_MB);
    do_dmi_access(dut.isck0, 80_MB, 4_MB);
    do_dmi_access(dut.isck1, 0 - 1_MB, 1_kB);
    do_dmi_access(dut.isck1, 1_kB - 1_MB, 1_kB);
    do_dmi_access(dut.isck1, 16_MB - 1_MB, 16_MB);
    do_dmi_access(dut.isck1, 32_MB - 1_MB, 16_MB);
    do_dmi_access(dut.isck1, 48_MB - 1_MB, 4_MB);
    do_dmi_access(dut.isck0, 128_MB, 16_MB);
    sc_start(dut.clk.read());
}

TEST_CASE("page_boundary_check", "[memory][tlm-level]") {
    auto& dut = factory::get<testbench>();
    constexpr uint64_t kPageSize = dut.mem3.getPageSize();

    std::array<uint8_t, 2> write_data{{0xAAu, 0xBBu}};
    tlm::tlm_generic_payload write;
    sc_core::sc_time delay = sc_core::SC_ZERO_TIME;
    write.set_command(tlm::TLM_WRITE_COMMAND);
    write.set_address(kPageSize - 1); // straddles page boundary
    write.set_data_length(write_data.size());
    write.set_streaming_width(write_data.size());
    write.set_data_ptr(write_data.data());
    dut.mem3.handle_operation(write, delay);

    std::array<uint8_t, 4> read_buf{{0xEEu, 0x00u, 0x00u, 0xEEu}};
    tlm::tlm_generic_payload read;
    read.set_command(tlm::TLM_READ_COMMAND);
    read.set_address(kPageSize - 1);
    read.set_data_length(write_data.size());
    read.set_streaming_width(write_data.size());
    read.set_data_ptr(read_buf.data() + 1); // leave guard bytes at both ends
    dut.mem3.handle_operation(read, delay);

    REQUIRE(read_buf[0] == 0xEEu); // leading guard untouched
    REQUIRE(read_buf[1] == 0xAAu); // first byte read correctly
    REQUIRE(read_buf[2] == 0xBBu); // second byte read correctly
    REQUIRE(read_buf[3] == 0xEEu); // trailing guard must remain
}

} // namespace scc
