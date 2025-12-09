
#include "testbench.h"
#include <factory.h>
#include <tlm/scc/tlm_gp_shared.h>
#undef CHECK
#include <catch2/catch_all.hpp>
#include <unordered_map>

using namespace sc_core;
using namespace ahb;

factory::add<testbench> tb;
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
template <unsigned BUSWIDTH> tlm::tlm_generic_payload* prepare_trans(uint64_t start_address, unsigned len, unsigned width) {
    static unsigned id{0};
    auto trans = tlm::scc::tlm_mm<>::get().allocate<apb::apb_extension>(len);
    trans->set_address(start_address);
    tlm::scc::setId(*trans, ++id);
    auto ext = trans->get_extension<apb::apb_extension>();
    trans->set_data_length(len);
    trans->set_streaming_width(len);
    ext->set_instruction();
    return trans;
}

inline void randomize(tlm::tlm_generic_payload& gp) {
    static uint8_t req_cnt{0};
    for(size_t i = 0; i < gp.get_data_length(); ++i) {
        *(gp.get_data_ptr() + i) = i % 2 ? i : req_cnt;
    }
    req_cnt++;
}

template <typename STATE> unsigned run_scenario(STATE& state, unsigned wait_states = 0) {
    auto& dut = factory::get<testbench>();
    dut.register_b_transport([&state, wait_states](tlm::tlm_base_protocol_types::tlm_payload_type& trans, sc_core::sc_time& d) {
        if(trans.is_read()) {
            for(size_t i = 0; i < trans.get_data_length(); ++i) {
                *(trans.get_data_ptr() + i) = i % 2 ? i : (state.resp_cnt + 128);
            }
            state.read_tx.second.emplace_back(&trans);
        }
        if(trans.is_write())
            state.write_tx.second.emplace_back(&trans);
        SCCDEBUG(__FUNCTION__) << "RX: " << trans;
        for(unsigned i = 0; i < wait_states; ++i)
            sc_core::wait(factory::get<testbench>().clk.posedge_event());
        state.resp_cnt++;
        trans.set_response_status(tlm::TLM_OK_RESPONSE);
    });

    dut.rst_n.write(false);
    sc_start(state.ResetCycles * dut.clk.period());
    dut.rst_n.write(true);
    sc_start(dut.clk.period());
    sc_start(dut.clk.period());

    auto run1 = sc_spawn([&dut, &state]() {
        unsigned int StartAddr{0x0};
        for(int i = 0; i < state.NumberOfIterations; ++i) {
            tlm::scc::tlm_gp_shared_ptr trans =
                prepare_trans<testbench::DATA_WIDTH>(StartAddr, state.BurstLengthByte, state.BurstSizeBytes);
            trans->set_command(tlm::TLM_READ_COMMAND);
            SCCDEBUG(__FUNCTION__) << "task run1, iteration " << i << " TX: " << *trans;
            sc_core::sc_time d;
            dut.isck->b_transport(*trans, d);
            state.read_tx.first.emplace_back(trans);
            StartAddr += state.BurstSizeBytes;
        }
        SCCDEBUG(__FUNCTION__) << "task run1 finished";
    });
    auto run2 = sc_spawn([&dut, &state]() {
        unsigned int StartAddr{0x2000};
        for(int i = 0; i < state.NumberOfIterations; ++i) {
            tlm::scc::tlm_gp_shared_ptr trans =
                prepare_trans<testbench::DATA_WIDTH>(StartAddr, state.BurstLengthByte, state.BurstSizeBytes);
            trans->set_command(tlm::TLM_WRITE_COMMAND);
            randomize(*trans);
            SCCDEBUG(__FUNCTION__) << "task run2, iteration " << i << " TX: " << *trans;
            sc_core::sc_time d;
            dut.isck->b_transport(*trans, d);
            state.write_tx.first.emplace_back(trans);
            StartAddr += state.BurstSizeBytes;
        }
        SCCDEBUG(__FUNCTION__) << "task run2 finished";
    });
    auto run3 = sc_spawn([&dut, &state]() {
        unsigned int StartAddr{0x1000};
        for(int i = 0; i < state.NumberOfIterations; ++i) {
            tlm::scc::tlm_gp_shared_ptr trans =
                prepare_trans<testbench::DATA_WIDTH>(StartAddr, state.BurstLengthByte, state.BurstSizeBytes);
            trans->set_command(tlm::TLM_READ_COMMAND);
            SCCDEBUG(__FUNCTION__) << "task run3, iteration " << i << " TX: " << *trans;
            sc_core::sc_time d;
            dut.isck->b_transport(*trans, d);
            state.read_tx.first.emplace_back(trans);
            StartAddr += state.BurstSizeBytes;
        }
        SCCDEBUG(__FUNCTION__) << "task run3 finished";
    });
    auto run4 = sc_spawn([&dut, &state]() {
        unsigned int StartAddr{0x3000};
        for(int i = 0; i < state.NumberOfIterations; ++i) {
            tlm::scc::tlm_gp_shared_ptr trans =
                prepare_trans<testbench::DATA_WIDTH>(StartAddr, state.BurstLengthByte, state.BurstSizeBytes);
            trans->set_command(tlm::TLM_WRITE_COMMAND);
            randomize(*trans);
            SCCDEBUG(__FUNCTION__) << "task run4, iteration " << i << " TX: " << *trans;
            sc_core::sc_time d;
            dut.isck->b_transport(*trans, d);
            state.write_tx.first.emplace_back(trans);
            StartAddr += state.BurstSizeBytes;
        }
        SCCDEBUG(__FUNCTION__) << "task run4 finished";
    });

    unsigned cycles{0};
    while(cycles < 1000 && !(run1.terminated() && run2.terminated() && run3.terminated() && run4.terminated())) {
        sc_start(10 * dut.clk.period());
        cycles += 10;
    }
    return cycles;
}

TEST_CASE("apb_read_write", "[APB][pin-level]") {
    struct {
        unsigned int ResetCycles{4};
        unsigned int BurstLengthByte{4};
        unsigned int BurstSizeBytes{4};
        unsigned int NumberOfIterations{1};
        std::pair<std::vector<tlm::scc::tlm_gp_shared_ptr>, std::vector<tlm::scc::tlm_gp_shared_ptr>> read_tx;
        std::pair<std::vector<tlm::scc::tlm_gp_shared_ptr>, std::vector<tlm::scc::tlm_gp_shared_ptr>> write_tx;
        unsigned resp_cnt{0};
    } state;

    auto cycles = run_scenario(state);

    REQUIRE(cycles < 1000);
    REQUIRE(sc_report_handler::get_count(SC_ERROR) == 0);
    REQUIRE(sc_report_handler::get_count(SC_WARNING) == 0);

    REQUIRE(state.resp_cnt == 4 * state.NumberOfIterations);
    {
        auto& e = state.write_tx;
        auto const& send_tx = e.first;
        auto const& recv_tx = e.second;
        REQUIRE(send_tx.size() == recv_tx.size());
        for(auto i = 0; i < send_tx.size(); ++i) {
            REQUIRE(send_tx[i]->get_response_status() == tlm::TLM_OK_RESPONSE);
            CHECK(is_equal(*send_tx[i], *recv_tx[i]));
        }
    }
    {
        auto& e = state.read_tx;
        auto const& send_tx = e.first;
        auto const& recv_tx = e.second;
        REQUIRE(send_tx.size() == recv_tx.size());
        for(auto i = 0; i < send_tx.size(); ++i) {
            REQUIRE(send_tx[i]->get_response_status() == tlm::TLM_OK_RESPONSE);
            CHECK(is_equal(*send_tx[i], *recv_tx[i]));
        }
    }
}

TEST_CASE("apb_narrow_read_write", "[APB][pin-level]") {
    struct {
        unsigned int ResetCycles{4};
        unsigned int BurstLengthByte{1};
        unsigned int BurstSizeBytes{1};
        unsigned int NumberOfIterations{8};
        std::pair<std::vector<tlm::scc::tlm_gp_shared_ptr>, std::vector<tlm::scc::tlm_gp_shared_ptr>> read_tx;
        std::pair<std::vector<tlm::scc::tlm_gp_shared_ptr>, std::vector<tlm::scc::tlm_gp_shared_ptr>> write_tx;
        unsigned resp_cnt{0};
    } state;

    auto cycles = run_scenario(state);

    REQUIRE(cycles < 1000);
    REQUIRE(sc_report_handler::get_count(SC_ERROR) == 0);
    REQUIRE(sc_report_handler::get_count(SC_WARNING) == 0);

    REQUIRE(state.resp_cnt == 4 * state.NumberOfIterations);
    {
        auto& e = state.write_tx;
        auto const& send_tx = e.first;
        auto const& recv_tx = e.second;
        REQUIRE(send_tx.size() == recv_tx.size());
        for(auto i = 0; i < send_tx.size(); ++i)
            CHECK(is_equal(*send_tx[i], *recv_tx[i]));
    }
    {
        auto& e = state.read_tx;
        auto const& send_tx = e.first;
        auto const& recv_tx = e.second;
        REQUIRE(send_tx.size() == recv_tx.size());
        // Narrow reads cannot be checked as they arrive a word read at the target
        // for(auto i = 0; i < send_tx.size(); ++i)
        //     CHECK(is_equal(*send_tx[i], *recv_tx[i]));
    }
}

TEST_CASE("apb_delayed_read_write", "[APB][pin-level]") {
    struct {
        unsigned int ResetCycles{4};
        unsigned int BurstLengthByte{4};
        unsigned int BurstSizeBytes{4};
        unsigned int NumberOfIterations{2};
        std::pair<std::vector<tlm::scc::tlm_gp_shared_ptr>, std::vector<tlm::scc::tlm_gp_shared_ptr>> read_tx;
        std::pair<std::vector<tlm::scc::tlm_gp_shared_ptr>, std::vector<tlm::scc::tlm_gp_shared_ptr>> write_tx;
        unsigned resp_cnt{0};
    } state;

    auto cycles = run_scenario(state, 2);

    REQUIRE(cycles < 1000);
    REQUIRE(sc_report_handler::get_count(SC_ERROR) == 0);
    REQUIRE(sc_report_handler::get_count(SC_WARNING) == 0);

    REQUIRE(state.resp_cnt == 4 * state.NumberOfIterations);
    {
        auto& e = state.write_tx;
        auto const& send_tx = e.first;
        auto const& recv_tx = e.second;
        REQUIRE(send_tx.size() == recv_tx.size());
        for(auto i = 0; i < send_tx.size(); ++i) {
            REQUIRE(send_tx[i]->get_response_status() == tlm::TLM_OK_RESPONSE);
            CHECK(is_equal(*send_tx[i], *recv_tx[i]));
        }
    }
    {
        auto& e = state.read_tx;
        auto const& send_tx = e.first;
        auto const& recv_tx = e.second;
        REQUIRE(send_tx.size() == recv_tx.size());
        for(auto i = 0; i < send_tx.size(); ++i) {
            REQUIRE(send_tx[i]->get_response_status() == tlm::TLM_OK_RESPONSE);
            CHECK(is_equal(*send_tx[i], *recv_tx[i]));
        }
    }
}
