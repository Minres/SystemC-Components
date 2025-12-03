// Validates that handle_operation does not overwrite past the requested length
#include <array>
#include <systemc>
#include <tlm>
#include <gtest/gtest.h>

#include "components/scc/memory.h"
#include "scc/cci_broker.h"

TEST(MemoryBoundaryTest, ReadAcrossPageDoesNotOverwriteBuffer) {
    static scc::cci_broker global_broker("global_broker");
    static auto broker_handle = cci::cci_register_broker(&global_broker);
    constexpr uint64_t kPageSize = 1ull << 24; // matches util::sparse_array default page size
    scc::memory<(kPageSize + 2u)> mem{"mem"};

    std::array<uint8_t, 2> write_data{{0xAAu, 0xBBu}};
    tlm::tlm_generic_payload write;
    sc_core::sc_time delay = sc_core::SC_ZERO_TIME;
    write.set_command(tlm::TLM_WRITE_COMMAND);
    write.set_address(kPageSize - 1); // straddles page boundary
    write.set_data_length(write_data.size());
    write.set_streaming_width(write_data.size());
    write.set_data_ptr(write_data.data());
    mem.handle_operation(write, delay);

    std::array<uint8_t, 4> read_buf{{0xEEu, 0x00u, 0x00u, 0xEEu}};
    tlm::tlm_generic_payload read;
    read.set_command(tlm::TLM_READ_COMMAND);
    read.set_address(kPageSize - 1);
    read.set_data_length(write_data.size());
    read.set_streaming_width(write_data.size());
    read.set_data_ptr(read_buf.data() + 1); // leave guard bytes at both ends
    mem.handle_operation(read, delay);

    EXPECT_EQ(read_buf[0], 0xEEu);       // leading guard untouched
    EXPECT_EQ(read_buf[1], 0xAAu);       // first byte read correctly
    EXPECT_EQ(read_buf[2], 0xBBu);       // second byte read correctly
    EXPECT_EQ(read_buf[3], 0xEEu);       // trailing guard must remain
}
