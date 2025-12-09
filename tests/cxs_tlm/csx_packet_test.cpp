
#include "testbench.h"
#include <factory.h>
#include <tlm/scc/tlm_gp_shared.h>
#undef CHECK
#include <catch2/catch_all.hpp>
#include <deque>

using namespace sc_core;
namespace cxs {
factory::add<testbench<256>> tb8;
factory::add<testbench<512>> tb9;
factory::add<testbench<1024>> tb10;

template <unsigned WIDTH, typename STATE> unsigned run_scenario(STATE& state) {
    auto& dut = factory::get<testbench<WIDTH>>();
    dut.tx.burst_len.set_value(state.burst_factor);
    dut.rx.max_credit.set_value(state.credits);
    dut.rst.write(true);
    sc_start(state.reset_cycles * dut.clk.period());
    dut.rst.write(false);
    sc_start(dut.clk.period());

    auto run1 = sc_spawn([&dut, &state]() {
        auto burst_cnt{0};
        std::deque<cxs_pkt_shared_ptr> expected_pkt;
        for(auto size : state.packet_sizes) {
            cxs_pkt_shared_ptr tx_pkt = cxs_pkt_mm::get().allocate();
            tx_pkt->get_data().resize(size);
            SCCDEBUG("run_scenario") << "Transmitting packet with size " << size;
            auto phase{tlm::nw::REQUEST};
            sc_core::sc_time t = sc_core::SC_ZERO_TIME;
            auto status = dut.isck->nb_transport_fw(*tx_pkt, phase, t);
            expected_pkt.emplace_back(tx_pkt);
            tx_pkt = nullptr;
            REQUIRE(status == tlm::TLM_UPDATED);
            REQUIRE(phase == tlm::nw::CONFIRM);
            if(++burst_cnt == state.granularity) {
                auto rec_cnt = 0u;
                while(rec_cnt < burst_cnt) {
                    ::sc_core::wait(dut.recv.data_written_event());
                    while(!dut.recv.empty()) {
                        auto recv_pkt = dut.recv.front();
                        dut.recv.pop_front();
                        tx_pkt = expected_pkt.front();
                        expected_pkt.pop_front();
                        REQUIRE(tx_pkt == recv_pkt);
                        REQUIRE(recv_pkt->get_data().size() == state.packet_sizes[state.resp_cnt]);
                        state.resp_cnt++;
                        rec_cnt++;
                        SCCDEBUG("run_scenario") << "Received packet with size " << recv_pkt->get_data().size()
                                                 << ", total number of packets is " << state.resp_cnt;
                    }
                }
                burst_cnt = 0;
            }
        }
    });

    unsigned cycles{0};
    while(cycles < state.max_cycles && !(run1.terminated())) {
        // while(cycles<1000 && !(run5.terminated())){
        sc_start(10 * dut.clk.period());
        cycles += 10;
    }
    return cycles;
}

template <typename STATE> unsigned run_scenario(int width, STATE& state) {
    switch(width) {
    case 8:
    case 256:
        return run_scenario<256>(state);
    case 9:
    case 512:
        return run_scenario<512>(state);
    case 10:
    case 1024:
        return run_scenario<1024>(state);
    }
    return 0;
}

TEST_CASE("single-packet", "[CXS][tlm-level]") {
    struct {
        unsigned int reset_cycles{4};
        unsigned int max_cycles = 20000;
        std::vector<unsigned int> packet_sizes;
        unsigned granularity{1};
        unsigned resp_cnt{0};
        unsigned burst_factor{1};
        unsigned credits{1};
    } state;

    state.packet_sizes.assign({4, 8, 16, 32, 64, 128, 256, 1024});
    std::array<unsigned, 3> credits{1, 4, 15};
    for(auto width = 8; width < 11; ++width) {
        for(auto credit : credits) {
            state.resp_cnt = 0;
            state.credits = credit;
            auto cycles = run_scenario(width, state);

            REQUIRE(cycles < state.max_cycles);
            REQUIRE(sc_report_handler::get_count(SC_ERROR) == 0);
            REQUIRE(sc_report_handler::get_count(SC_WARNING) == 0);

            REQUIRE(state.resp_cnt == state.packet_sizes.size());
        }
    }
}

TEST_CASE("multi-packet", "[CXS][tlm-level]") {
    struct {
        unsigned int reset_cycles{4};
        unsigned int max_cycles = 20000;
        std::vector<unsigned int> packet_sizes;
        unsigned granularity{2};
        unsigned resp_cnt{0};
        unsigned burst_factor{1};
        unsigned credits{15};
    } state;

    state.packet_sizes.assign({4, 8, 16, 32, 16, 64, 16, 128, 16, 256, 16, 1024});
    for(auto width = 8; width < 11; ++width) {
        state.resp_cnt = 0;
        auto cycles = run_scenario(width, state);

        REQUIRE(cycles < state.max_cycles);
        REQUIRE(sc_report_handler::get_count(SC_ERROR) == 0);
        REQUIRE(sc_report_handler::get_count(SC_WARNING) == 0);

        REQUIRE(state.resp_cnt == state.packet_sizes.size());
    }
}
TEST_CASE("single-packet-burst2", "[CXS][tlm-level]") {
    struct {
        unsigned int reset_cycles{4};
        unsigned int max_cycles = 20000;
        std::vector<unsigned int> packet_sizes;
        unsigned granularity{1};
        unsigned resp_cnt{0};
        unsigned burst_factor{2};
        unsigned credits{2};
    } state;

    state.packet_sizes.assign({4, 8, 16, 32, 64, 128, 256, 1024});
    for(auto width = 8; width < 11; ++width) {
        state.resp_cnt = 0;
        auto cycles = run_scenario(width, state);

        REQUIRE(cycles < state.max_cycles);
        REQUIRE(sc_report_handler::get_count(SC_ERROR) == 0);
        REQUIRE(sc_report_handler::get_count(SC_WARNING) == 0);

        REQUIRE(state.resp_cnt == state.packet_sizes.size());
    }
}

TEST_CASE("multi-packet-burst2", "[CXS][tlm-level]") {
    struct {
        unsigned int reset_cycles{4};
        unsigned int max_cycles = 20000;
        std::vector<unsigned int> packet_sizes;
        unsigned granularity{2};
        unsigned resp_cnt{0};
        unsigned burst_factor{2};
        unsigned credits{2};
    } state;

    state.packet_sizes.assign({4, 8, 16, 32, 16, 64, 16, 128, 16, 256, 16, 1024});
    for(auto width = 8; width < 11; ++width) {
        state.resp_cnt = 0;
        auto cycles = run_scenario(width, state);

        REQUIRE(cycles < state.max_cycles);
        REQUIRE(sc_report_handler::get_count(SC_ERROR) == 0);
        REQUIRE(sc_report_handler::get_count(SC_WARNING) == 0);

        REQUIRE(state.resp_cnt == state.packet_sizes.size());
    }
}
} // namespace cxs
