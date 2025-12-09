
#include "testbench.h"
#include <factory.h>
#include <tlm/scc/tlm_gp_shared.h>
#undef CHECK
#include <catch2/catch_all.hpp>
#include <unordered_map>

using namespace sc_core;

factory::add<testbench> tb;

int snoop_id = 0;

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
tlm::tlm_generic_payload* prepare_trans_ace(uint64_t start_address, unsigned addr_incr, unsigned len, unsigned width, unsigned id) {
    auto trans = tlm::scc::tlm_mm<>::get().allocate<axi::ace_extension>(len);
    trans->set_address(start_address);
    tlm::scc::setId(*trans, id);
    auto ext = trans->get_extension<axi::ace_extension>();
    trans->set_data_length(len);
    trans->set_streaming_width(len);
    ext->set_size(scc::ilog2(width));
    sc_assert(len < (bus_cfg::BUSWIDTH / 8) || len % (bus_cfg::BUSWIDTH / 8) == 0);
    auto length = (len * 8 - 1) / (8 * width);
    if(width == (bus_cfg::BUSWIDTH / 8) && start_address % (bus_cfg::BUSWIDTH / 8))
        length++;
    ext->set_length(length);
    // ext->set_burst(len * 8 > bus_cfg::buswidth ? axi::burst_e::INCR : axi::burst_e::FIXED);
    // here len is CachelineSizeBytes
    // here burtst for read/write_trans and snoop_trans are different
    ext->set_burst(axi::burst_e::INCR); // TBD???
    // ext->set_burst(len*8 > bus_cfg::BUSWIDTH ? axi::burst_e::WRAP : axi::burst_e::INCR);
    ext->set_id(id);
    ext->set_snoop(axi::snoop_e::READ_SHARED); // set it so that is_data_less return true???
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

    dut.axi_tgt_pe.set_operation_cb([&state](axi::axi_protocol_types::tlm_payload_type& trans) -> unsigned {
        auto id = axi::get_axi_id(trans);
        if(trans.is_read()) {
            for(size_t i = 0; i < trans.get_data_length(); ++i) {
                *(trans.get_data_ptr() + i) = i % 2 ? 123 : (state.resp_cnt + 128);
            }
            state.read_tx[id].second.emplace_back(&trans);
        }
        if(trans.is_write())
            state.write_tx[id].second.emplace_back(&trans);
        SCCDEBUG(__FUNCTION__) << "RX: " << trans;
        state.resp_cnt++;
        return 0;
    });
    dut.transport_cb = [&state](tlm::tlm_generic_payload& trans) -> unsigned {
        SCCDEBUG(__FUNCTION__) << " update snoop trans, with snoop_id = " << snoop_id;
        // extracting address and snoop_e from ac_trans and pack them into cache data trans
        auto ac_address = trans.get_address();
        auto ext = trans.get_extension<ace_extension>();
        auto ac_snoop = ext->get_snoop();
        for(size_t i = 0; i < trans.get_data_length(); ++i) {
            *(trans.get_data_ptr() + i) = i % 2 ? i : 128;
        }
        state.snoop_tx[snoop_id].second.emplace_back(&trans);
        return 1;
    };

    dut.rst.write(false);
    sc_start(state.ResetCycles * dut.clk.period());
    dut.rst.write(true);
    sc_start(dut.clk.period());

    auto run1 = sc_spawn([&dut, &state]() {
        unsigned int StartAddr{0x0};
        for(int i = 0; i < state.NumberOfIterations; ++i) {
            // tlm::scc::tlm_gp_shared_ptr trans = prepare_trans<testbench::bus_cfg>(StartAddr, 4,
            // state.BurstLengthByte, state.BurstSizeBytes, 1);
            tlm::scc::tlm_gp_shared_ptr trans =
                prepare_trans_ace<testbench::bus_cfg>(StartAddr, 4, state.BurstLengthByte, state.BurstSizeBytes, 1);
            trans->set_command(tlm::TLM_READ_COMMAND);
            SCCDEBUG(__FUNCTION__) << "run1, iteration " << i << " TX: " << *trans;
            dut.intor_pe.transport(*trans, false);
            state.read_tx[axi::get_axi_id(*trans)].first.emplace_back(trans);
            StartAddr += state.BurstSizeBytes;
        }
    });

    auto run2 = sc_spawn([&dut, &state]() {
        unsigned int StartAddr{0x2000};
        for(int i = 0; i < state.NumberOfIterations; ++i) {
            tlm::scc::tlm_gp_shared_ptr trans =
                prepare_trans_ace<testbench::bus_cfg>(StartAddr, 4, state.BurstLengthByte, state.BurstSizeBytes, 2);
            trans->set_command(tlm::TLM_WRITE_COMMAND);
            randomize(*trans);
            SCCDEBUG(__FUNCTION__) << "run2, iteration " << i << " TX: " << *trans;
            dut.intor_pe.transport(*trans, false);
            state.write_tx[axi::get_axi_id(*trans)].first.emplace_back(trans);
            StartAddr += state.BurstSizeBytes;
        }
    });
    auto run3 = sc_spawn([&dut, &state]() {
        unsigned int StartAddr{0x1000};
        for(int i = 0; i < state.NumberOfIterations; ++i) {
            tlm::scc::tlm_gp_shared_ptr trans =
                prepare_trans_ace<testbench::bus_cfg>(StartAddr, 4, state.BurstLengthByte, state.BurstSizeBytes, 3);
            trans->set_command(tlm::TLM_READ_COMMAND);
            SCCDEBUG(__FUNCTION__) << "run3, iteration " << i << " TX: " << *trans;
            dut.intor_pe.transport(*trans, false);
            state.read_tx[axi::get_axi_id(*trans)].first.emplace_back(trans);
            StartAddr += state.BurstSizeBytes;
        }
    });
    auto run4 = sc_spawn([&dut, &state]() {
        unsigned int StartAddr{0x3000};
        for(int i = 0; i < state.NumberOfIterations; ++i) {
            tlm::scc::tlm_gp_shared_ptr trans =
                prepare_trans_ace<testbench::bus_cfg>(StartAddr, 4, state.BurstLengthByte, state.BurstSizeBytes, 4);
            trans->set_command(tlm::TLM_WRITE_COMMAND);
            randomize(*trans);
            SCCDEBUG(__FUNCTION__) << "run4, iteration " << i << " TX: " << *trans;
            dut.intor_pe.transport(*trans, false);
            state.write_tx[axi::get_axi_id(*trans)].first.emplace_back(trans);
            StartAddr += state.BurstSizeBytes;
        }
    });

    auto run5 = sc_spawn([&dut, &state]() {
        unsigned int StartAddr{0x0};
        for(int i = 0; i < state.NumberOfIterations; ++i) {
            tlm::scc::tlm_gp_shared_ptr trans =
                prepare_trans_ace<testbench::bus_cfg>(StartAddr, 4, state.CachelineSizeBytes, state.BurstSizeBytes, 5);
            trans->set_command(tlm::TLM_READ_COMMAND);
            SCCDEBUG(__FUNCTION__) << "run1, iteration " << i << "snoop_id = " << snoop_id << "  TX: " << *trans;
            dut.ace_tgt_pe.snoop(*trans);
            SCCDEBUG(__FUNCTION__) << "run1, after iteration " << i;
            state.snoop_tx[snoop_id].first.emplace_back(trans);
            snoop_id++;
            StartAddr += state.BurstSizeBytes;
        }
    });

    unsigned cycles{0};
    while(cycles < 1000 && !(run1.terminated() && run2.terminated() && run3.terminated() && run4.terminated())) {
        // while(cycles<1000 && !(run5.terminated())){
        sc_start(10 * dut.clk.period());
        cycles += 10;
    }
    return cycles;
}

TEST_CASE("ace_burst_alignment", "[AXI][pin-level]") {
    struct {
        unsigned int ResetCycles{4};
        unsigned int BurstLengthByte{16};
        unsigned int BurstSizeBytes{8};
        unsigned int NumberOfIterations{2};
        unsigned int CachelineSizeBytes = {64}; //
        std::unordered_map<unsigned, std::pair<std::vector<tlm::scc::tlm_gp_shared_ptr>, std::vector<tlm::scc::tlm_gp_shared_ptr>>> read_tx;
        std::unordered_map<unsigned, std::pair<std::vector<tlm::scc::tlm_gp_shared_ptr>, std::vector<tlm::scc::tlm_gp_shared_ptr>>>
            write_tx;
        std::unordered_map<unsigned, std::pair<std::vector<tlm::scc::tlm_gp_shared_ptr>, std::vector<tlm::scc::tlm_gp_shared_ptr>>>
            snoop_tx;
        unsigned resp_cnt{0};
    } state;

    state.resp_cnt = 0;
    auto cycles = run_scenario(state);

    REQUIRE(cycles < 1000);
    REQUIRE(sc_report_handler::get_count(SC_ERROR) == 0);
    REQUIRE(sc_report_handler::get_count(SC_WARNING) == 0);

    REQUIRE(state.resp_cnt == 4 * state.NumberOfIterations);
    // REQUIRE(state.resp_cnt==1*state.NumberOfIterations);
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
            REQUIRE(send_tx[i]->get_response_status() == tlm::TLM_OK_RESPONSE);
            SCCDEBUG(__FUNCTION__) << " index = " << i;
            //           SCCDEBUG(__FUNCTION__) <<" send     value = "<<*send_tx[i];
            //           SCCDEBUG(__FUNCTION__) <<" received value = "<<*recv_tx[i];
            CHECK(is_equal(*send_tx[i], *recv_tx[i]));
        }
    }

    for(auto& e : state.snoop_tx) {
        auto const& snoop_tx = e.second.first;
        auto const& recv_tx = e.second.second;
        REQUIRE(snoop_tx.size() == recv_tx.size());
        for(auto i = 0; i < snoop_tx.size(); ++i) {
            REQUIRE(snoop_tx[i]->get_response_status() == tlm::TLM_OK_RESPONSE);
            SCCDEBUG(__FUNCTION__) << " index = " << i;
            SCCDEBUG(__FUNCTION__) << " send snoop value = " << *snoop_tx[i];
            SCCDEBUG(__FUNCTION__) << " received   value = " << *recv_tx[i];
            //          CHECK(*snoop_tx[i] == *recv_tx[i]);
        }
    }
}

TEST_CASE("ace_narrow_burst", "[AXI][pin-level]") {
    struct {
        unsigned int ResetCycles{4};
        unsigned int BurstLengthByte{16};
        unsigned int BurstSizeBytes{8};
        unsigned int NumberOfIterations{2};
        unsigned int CachelineSizeBytes = {64}; //
        std::unordered_map<unsigned, std::pair<std::vector<tlm::scc::tlm_gp_shared_ptr>, std::vector<tlm::scc::tlm_gp_shared_ptr>>> read_tx;
        std::unordered_map<unsigned, std::pair<std::vector<tlm::scc::tlm_gp_shared_ptr>, std::vector<tlm::scc::tlm_gp_shared_ptr>>>
            write_tx;
        std::unordered_map<unsigned, std::pair<std::vector<tlm::scc::tlm_gp_shared_ptr>, std::vector<tlm::scc::tlm_gp_shared_ptr>>>
            snoop_tx;
        unsigned resp_cnt{0};
    } state;

    state.resp_cnt = 0;
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
