
#include "testbench.h"
#include <factory.h>
#include <tlm/scc/tlm_gp_shared.h>
#undef CHECK
#include <catch2/catch_all.hpp>
#include <unordered_map>

using namespace sc_core;
using tlm_gp_shared_ptr_vec = std::vector<tlm::scc::tlm_gp_shared_ptr>;

factory::add<testbench> tb;

//// DataTransfer()
//// ==============
// void DataTransfer(uint64_t Start_Address, unsigned axsize, unsigned axlen, unsigned Data_Bus_Bytes, axi::burst_e Mode, bool IsWrite) {
//     auto Number_Bytes = 2u<<axsize;
//     auto Burst_Length = axlen+1;
//// Data_Bus_Bytes is the number of 8-bit byte lanes in the bus
//// Mode is the AXI transfer mode
//// IsWrite is TRUE for a write, and FALSE for a read
// auto addr = Start_Address; // Variable for current address
// auto Aligned_Address = (unsigned(addr/Number_Bytes) * Number_Bytes);
// auto aligned = (Aligned_Address == addr); // Check whether addr is aligned to nbytes
// auto dtsize = Number_Bytes * Burst_Length; // Maximum total data transaction size
// auto Lower_Wrap_Boundary = 0ULL;
// auto Upper_Wrap_Boundary = 0ULL
// if(Mode == axi::burst_e::WRAP){
//     Lower_Wrap_Boundary = (uint64_t(addr/dtsize) * dtsize);
//     // addr must be aligned for a wrapping burst
//     Upper_Wrap_Boundary = Lower_Wrap_Boundary + dtsize;
// }
// for(unsigned i=0; i<Burst_Length; ++i) {
//     auto n = i+1;
//     auto Lower_Byte_Lane = addr - (uint64_t(addr/Data_Bus_Bytes)) * Data_Bus_Bytes;
//     if(aligned){
//         auto Upper_Byte_Lane = Lower_Byte_Lane + Number_Bytes - 1;
//     } else {
//         auto Upper_Byte_Lane = Aligned_Address + Number_Bytes - 1
//                 - (uint64_t(addr/Data_Bus_Bytes)) * Data_Bus_Bytes;
//     }
//     // Peform data transfer
//     if(IsWrite)
//         dwrite(addr, low_byte, high_byte);
//     else
//         dread(addr, low_byte, high_byte);
//     // Increment address if necessary
//     if(Mode != axi::burst_e::FIXED) {
//         if(aligned){
//             addr = addr + Number_Bytes;
//             if(Mode == axi::burst_e::WRAP){
//                     // WRAP mode is always aligned
//                     if(addr >= Upper_Wrap_Boundary) addr = Lower_Wrap_Boundary;
//             }
//         } else {
//             addr = Aligned_Address + Number_Bytes;
//             aligned = true; // All transfers after the first are aligned
//         }
//     }
// }
// return;
// }
bool is_equal(tlm::tlm_generic_payload const& a, tlm::tlm_generic_payload const& b) {
    auto ret = true;
    ret &= a.get_command() == b.get_command();
    ret &= a.get_address() == b.get_address();
    ret &= a.get_data_length() == b.get_data_length();
    for(auto i = 0u; i < a.get_data_length(); ++i)
        ret &= a.get_data_ptr()[i] == b.get_data_ptr()[i];
    //    if(a.get_byte_enable_ptr() && b.get_byte_enable_ptr()) {
    //        ret &= a.get_byte_enable_length() == b.get_byte_enable_length();
    //        for(auto i=0u; i<a.get_byte_enable_length(); ++i)
    //            ret &= a.get_byte_enable_ptr()[i] == b.get_byte_enable_ptr()[i];
    //    }
    ret &= a.get_command() == b.get_command();
    // if(!ret) SCCWARN()<<"Comparison failed: "<<a<<" and "<<b;
    return ret;
}

template <typename bus_cfg>
tlm::tlm_generic_payload* prepare_trans(uint64_t start_address, unsigned addr_incr, unsigned len, unsigned width, unsigned id) {
    auto trans = tlm::scc::tlm_mm<>::get().allocate<axi::axi4_extension>(len);
    trans->set_address(start_address);
    tlm::scc::setId(*trans, id);
    auto ext = trans->get_extension<axi::axi4_extension>();
    trans->set_data_length(len);
    trans->set_streaming_width(len);
    ext->set_size(scc::ilog2(width));
    sc_assert(len < (bus_cfg::BUSWIDTH / 8) || len % (bus_cfg::BUSWIDTH / 8) == 0);
    auto length = (len * 8 - 1) / (8 * width);
    //    if(width == (bus_cfg::BUSWIDTH / 8) && start_address % (bus_cfg::BUSWIDTH / 8))
    //        length++;
    if(start_address % (bus_cfg::BUSWIDTH / 8) + width > (bus_cfg::BUSWIDTH / 8))
        length++;
    ext->set_length(length);
    // ext->set_burst(len * 8 > bus_cfg::buswidth ? axi::burst_e::INCR : axi::burst_e::FIXED);
    ext->set_burst(axi::burst_e::INCR);
    ext->set_id(id);
    return trans;
}

inline void randomize(tlm::tlm_generic_payload& gp) {
    static uint8_t req_cnt{0};
    for(size_t i = 0; i < gp.get_data_length(); ++i) {
        *(gp.get_data_ptr() + i) = i % 2 ? i : req_cnt;
    }
    req_cnt++;
}

template <typename STATE> unsigned run_scenario(STATE& state) {
    auto& dut = factory::get<testbench>();
    dut.tgt_pe.set_operation_cb([&state](axi::axi_protocol_types::tlm_payload_type& trans) -> unsigned {
        auto id = axi::get_axi_id(trans);
        if(trans.is_read()) {
            for(size_t i = 0; i < trans.get_data_length(); ++i) {
                *(trans.get_data_ptr() + i) = i % 2 ? i : (state.resp_cnt + 128);
            }
            state.read_tx[id].second.emplace_back(&trans);
        }
        if(trans.is_write())
            state.write_tx[id].second.emplace_back(&trans);
        SCCDEBUG(__FUNCTION__) << "RX: " << trans;
        state.resp_cnt++;
        return 0;
    });

    dut.rst.write(false);
    sc_start(state.ResetCycles * dut.clk.period());
    dut.rst.write(true);
    sc_start(dut.clk.period());

    auto run1 = sc_spawn([&dut, &state]() {
        unsigned int StartAddr{0x0};
        for(int i = 0; i < state.NumberOfIterations; ++i) {
            tlm::scc::tlm_gp_shared_ptr trans =
                prepare_trans<testbench::bus_cfg>(StartAddr + (state.unaligned ? 2 : 0), 4, state.BurstLengthByte, state.BurstSizeBytes, 1);
            trans->set_command(tlm::TLM_READ_COMMAND);
            SCCDEBUG("run1") << "iteration " << i << " TX: " << *trans;
            dut.intor_pe.transport(*trans, false);
            state.read_tx[axi::get_axi_id(*trans)].first.emplace_back(trans);
            StartAddr += state.BurstSizeBytes;
        }
        SCCDEBUG("run1") << "finished " << state.NumberOfIterations << " iterations";
    });
    auto run2 = sc_spawn([&dut, &state]() {
        unsigned int StartAddr{0x2000};
        for(int i = 0; i < state.NumberOfIterations; ++i) {
            tlm::scc::tlm_gp_shared_ptr trans =
                prepare_trans<testbench::bus_cfg>(StartAddr + (state.unaligned ? 2 : 0), 4, state.BurstLengthByte, state.BurstSizeBytes, 2);
            trans->set_command(tlm::TLM_WRITE_COMMAND);
            randomize(*trans);
            SCCDEBUG("run2") << "iteration " << i << " TX: " << *trans;
            dut.intor_pe.transport(*trans, false);
            state.write_tx[axi::get_axi_id(*trans)].first.emplace_back(trans);
            StartAddr += state.BurstSizeBytes;
        }
        SCCDEBUG("run2") << "finished " << state.NumberOfIterations << " iterations";
    });
    auto run3 = sc_spawn([&dut, &state]() {
        unsigned int StartAddr{0x1000};
        for(int i = 0; i < state.NumberOfIterations; ++i) {
            tlm::scc::tlm_gp_shared_ptr trans =
                prepare_trans<testbench::bus_cfg>(StartAddr + (state.unaligned ? 2 : 0), 4, state.BurstLengthByte, state.BurstSizeBytes, 3);
            trans->set_command(tlm::TLM_READ_COMMAND);
            SCCDEBUG("run3") << "iteration " << i << " TX: " << *trans;
            dut.intor_pe.transport(*trans, false);
            state.read_tx[axi::get_axi_id(*trans)].first.emplace_back(trans);
            StartAddr += state.BurstSizeBytes;
        }
        SCCDEBUG("run3") << "finished " << state.NumberOfIterations << " iterations";
    });
    auto run4 = sc_spawn([&dut, &state]() {
        unsigned int StartAddr{0x3000};
        for(int i = 0; i < state.NumberOfIterations; ++i) {
            tlm::scc::tlm_gp_shared_ptr trans =
                prepare_trans<testbench::bus_cfg>(StartAddr + (state.unaligned ? 2 : 0), 4, state.BurstLengthByte, state.BurstSizeBytes, 4);
            trans->set_command(tlm::TLM_WRITE_COMMAND);
            randomize(*trans);
            SCCDEBUG("run4") << "iteration " << i << " TX: " << *trans;
            dut.intor_pe.transport(*trans, false);
            state.write_tx[axi::get_axi_id(*trans)].first.emplace_back(trans);
            StartAddr += state.BurstSizeBytes;
        }
        SCCDEBUG("run4") << "finished " << state.NumberOfIterations << " iterations";
    });

    unsigned cycles{0};
    while(cycles < 1000 && !(run1.terminated() && run2.terminated() && run3.terminated() && run4.terminated())) {
        sc_start(10 * dut.clk.period());
        cycles += 10;
    }
    return cycles;
}

void axi4_burst_alignment(bool pipelined_wrreq, bool write_bp, bool unaligned = false) {
    SCCINFO(__FUNCTION__) << "starting with pipelined_wrreq=" << pipelined_wrreq << " and write_bp=" << write_bp;
    struct {
        unsigned int ResetCycles{4};
        unsigned int BurstLengthByte{16};
        unsigned int BurstSizeBytes{8};
        unsigned int NumberOfIterations{8};
        std::unordered_map<unsigned, std::pair<tlm_gp_shared_ptr_vec, tlm_gp_shared_ptr_vec>> read_tx;
        std::unordered_map<unsigned, std::pair<tlm_gp_shared_ptr_vec, tlm_gp_shared_ptr_vec>> write_tx;
        unsigned resp_cnt{0};
        bool unaligned{false};
    } state;

    state.unaligned = unaligned;
    auto& dut = factory::get<testbench>();
    dut.intor_bfm.pipelined_wrreq = pipelined_wrreq;
    dut.tgt_pe.wr_data_accept_delay.set_value(write_bp ? 1 : 0);
    auto cycles = run_scenario(state);

    REQUIRE(cycles < 1000);
    REQUIRE(sc_report_handler::get_count(SC_ERROR) == 0);
    REQUIRE(sc_report_handler::get_count(SC_WARNING) == 0);

    REQUIRE(state.resp_cnt == 4 * state.NumberOfIterations);
    for(auto& e : state.write_tx) {
        auto const& send_tx = e.second.first;
        auto const& recv_tx = e.second.second;
        REQUIRE(send_tx.size() == recv_tx.size());
        for(auto i = 0; i < send_tx.size(); ++i) {
            REQUIRE(send_tx[i]->get_response_status() == tlm::TLM_OK_RESPONSE);
            CHECK(is_equal(*send_tx[i], *recv_tx[i]));
        }
    }
    for(auto& e : state.read_tx) {
        auto const& send_tx = e.second.first;
        auto const& recv_tx = e.second.second;
        REQUIRE(send_tx.size() == recv_tx.size());
        for(auto i = 0; i < send_tx.size(); ++i) {
            auto addr = send_tx[i]->get_address();
            if(addr % (testbench::bus_cfg::ADDRWIDTH / 8)) {
                CHECK(send_tx[i]->get_data_length() <= recv_tx[i]->get_data_length());
                CHECK(send_tx[i]->get_byte_enable_length() <= recv_tx[i]->get_byte_enable_length());
                // adjust the length of the read due to misalignment
                recv_tx[i]->set_data_length(send_tx[i]->get_data_length());
                recv_tx[i]->set_byte_enable_length(send_tx[i]->get_byte_enable_length());
                recv_tx[i]->set_streaming_width(send_tx[i]->get_streaming_width());
            }
            REQUIRE(send_tx[i]->get_response_status() == tlm::TLM_OK_RESPONSE);
            CHECK(is_equal(*send_tx[i], *recv_tx[i]));
        }
    }
}

void axi4_narrow_burst(bool pipelined_wrreq, bool write_bp, bool unaligned = false) {
    SCCINFO(__FUNCTION__) << "starting with pipelined_wrreq=" << pipelined_wrreq << ", write_bp = " << write_bp
                          << " and unaligned=" << unaligned;
    struct {
        unsigned int ResetCycles{4};
        unsigned int BurstLengthByte{16};
        unsigned int BurstSizeBytes{4};
        unsigned int NumberOfIterations{8};
        std::unordered_map<unsigned, std::pair<tlm_gp_shared_ptr_vec, tlm_gp_shared_ptr_vec>> read_tx;
        std::unordered_map<unsigned, std::pair<tlm_gp_shared_ptr_vec, tlm_gp_shared_ptr_vec>> write_tx;
        unsigned resp_cnt{0};
        bool unaligned{false};
    } state;

    state.unaligned = unaligned;
    auto& dut = factory::get<testbench>();
    dut.intor_bfm.pipelined_wrreq = pipelined_wrreq;
    dut.tgt_pe.wr_data_accept_delay.set_value(write_bp ? 1 : 0);
    auto cycles = run_scenario(state);

    REQUIRE(cycles < 1000);
    REQUIRE(sc_report_handler::get_count(SC_ERROR) == 0);
    REQUIRE(sc_report_handler::get_count(SC_WARNING) == 0);

    REQUIRE(state.resp_cnt == 4 * state.NumberOfIterations);
    for(auto& e : state.write_tx) {
        auto const& send_tx = e.second.first;
        auto const& recv_tx = e.second.second;
        REQUIRE(send_tx.size() == recv_tx.size());
        for(auto i = 0; i < send_tx.size(); ++i)
            CHECK(is_equal(*send_tx[i], *recv_tx[i]));
    }
    for(auto& e : state.read_tx) {
        auto const& send_tx = e.second.first;
        auto const& recv_tx = e.second.second;
        REQUIRE(send_tx.size() == recv_tx.size());
        for(auto i = 0; i < send_tx.size(); ++i)
            CHECK(is_equal(*send_tx[i], *recv_tx[i]));
    }
}

TEST_CASE("axi4_burst_alignment", "[AXI][pin-level]") { axi4_burst_alignment(false, false); }

TEST_CASE("axi4_burst_alignment_unaligned_addr", "[AXI][pin-level]") { axi4_burst_alignment(false, false, true); }

TEST_CASE("axi4_narrow_burst", "[AXI][pin-level]") { axi4_narrow_burst(false, false); }

// TEST_CASE("axi4_narrow_burst_unaligned_addr", "[AXI][pin-level]") { axi4_narrow_burst(false, false, true); }

TEST_CASE("axi4_burst_alignment_with_bp", "[AXI][pin-level]") { axi4_burst_alignment(false, true); }

TEST_CASE("axi4_burst_alignment_with_bp_unaligned_addr", "[AXI][pin-level]") { axi4_burst_alignment(false, true, true); }

TEST_CASE("axi4_narrow_burst_with_bp", "[AXI][pin-level]") { axi4_narrow_burst(false, true); }

// TEST_CASE("axi4_narrow_burst_with_bp_unaligned_addr", "[AXI][pin-level]") { axi4_narrow_burst(false, true, true); }

TEST_CASE("axi4_burst_alignment_pipelined_write", "[AXI][pin-level]") { axi4_burst_alignment(true, false); }

TEST_CASE("axi4_burst_alignment_pipelined_write_unaligned_addr", "[AXI][pin-level]") { axi4_burst_alignment(true, false, true); }

TEST_CASE("axi4_narrow_burst_pipelined_write", "[AXI][pin-level]") { axi4_narrow_burst(true, false); }

// TEST_CASE("axi4_narrow_burst_pipelined_write_unaligned_addr", "[AXI][pin-level]") { axi4_narrow_burst(true, false, true); }

TEST_CASE("axi4_burst_alignment_pipelined_write_with_bp", "[AXI][pin-level]") { axi4_burst_alignment(true, true); }

TEST_CASE("axi4_burst_alignment_pipelined_write_with_bp_unaligned_addr", "[AXI][pin-level]") { axi4_burst_alignment(true, true, true); }

TEST_CASE("axi4_narrow_burst_pipelined_write_with_bp", "[AXI][pin-level]") { axi4_narrow_burst(true, true); }

// TEST_CASE("axi4_narrow_burst_pipelined_write_with_bp_unaligned_addr", "[AXI][pin-level]") { axi4_narrow_burst(true, true, true); }
