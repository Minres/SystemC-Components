#ifndef SC_INCLUDE_DYNAMIC_PROCESSES
#define SC_INCLUDE_DYNAMIC_PROCESSES
#include <sysc/kernel/sc_simcontext.h>
#endif
#include <array>
#include <catch2/catch_all.hpp>
#include <factory.h>
#include <scc/cci_param_restricted.h>
#include <scc/utilities.h>
#include <systemc>
using namespace sc_core;

struct top : public sc_core::sc_module {
    top()
    : top("top") {}
    top(sc_module_name const& nm)
    : sc_core::sc_module(nm) {}

    scc::cci_param_restricted<int> param1{"param1", 5, scc::min_max_restriction(0, 10), "This is parameter 1"};
    scc::cci_param_restricted<int> param2{"param2", 5, scc::min_max_excl_restriction(0, 10), "This is parameter 3"};
    scc::cci_param_restricted<int> param3{"param3", 10, scc::min_restriction(0), "This is parameter 2"};
    scc::cci_param_restricted<int> param4{"param4", 10, scc::min_excl_restriction(0), "This is parameter 4"};
    scc::cci_param_restricted<int> param5{"param5", 1, scc::max_restriction(10), "This is parameter 5"};
    scc::cci_param_restricted<int> param6{"param6", 1, scc::max_excl_restriction(10), "This is parameter 6"};
    scc::cci_param_restricted<int> param7{"param7", 4, scc::discrete_restriction({1, 2, 4, 8, 16}), "This is parameter 7"};
    std::array<int, 5> values_arr{1, 2, 4, 8, 16};
    scc::cci_param_restricted<int> param8{"param8", 4, scc::discrete_restriction(values_arr), "This is parameter 8"};
    std::vector<int> values_vec{1, 2, 4, 8, 16};
    scc::cci_param_restricted<int> param9{"param9", 4, scc::discrete_restriction(values_vec), "This is parameter 9"};
};

factory::add<top> tb;

TEST_CASE("simple cci_param_restricted min_max test", "[SCC][cci_param_restricted]") {
    sc_report_handler::set_actions(SC_ERROR, SC_LOG | SC_CACHE_REPORT | SC_DISPLAY | SC_DO_NOTHING);
    auto& dut = factory::get<top>();
    auto run1 = sc_spawn([&dut]() {
        sc_core::wait(1_ns);
        for(auto i : {0, 5, 10, -1, 11}) {
            dut.param1.set_value(i);
            sc_core::wait(1_ns);
        }
    });

    sc_start(10_ns);
    REQUIRE(run1.terminated());
    REQUIRE(sc_report_handler::get_count(SC_ERROR) == 2);
    REQUIRE(sc_report_handler::get_count(SC_WARNING) == 0);
    sc_report_handler::initialize();
    auto run2 = sc_spawn([&dut]() {
        sc_core::wait(1_ns);
        for(auto i : {0, 5, 10, -1, 11}) {
            dut.param2.set_value(i);
            sc_core::wait(1_ns);
        }
    });

    sc_start(10_ns);
    REQUIRE(run2.terminated());
    REQUIRE(sc_report_handler::get_count(SC_ERROR) == 4);
    REQUIRE(sc_report_handler::get_count(SC_WARNING) == 0);
    sc_report_handler::initialize();
}

TEST_CASE("simple cci_param_restricted min test", "[SCC][cci_param_restricted]") {
    sc_report_handler::set_actions(SC_ERROR, SC_LOG | SC_CACHE_REPORT | SC_DISPLAY | SC_DO_NOTHING);
    auto& dut = factory::get<top>();
    auto run1 = sc_spawn([&dut]() {
        sc_core::wait(1_ns);
        for(auto i : {0, 5, -1}) {
            dut.param3.set_value(i);
            sc_core::wait(1_ns);
        }
    });

    sc_start(10_ns);
    REQUIRE(run1.terminated());
    REQUIRE(sc_report_handler::get_count(SC_ERROR) == 1);
    REQUIRE(sc_report_handler::get_count(SC_WARNING) == 0);
    sc_report_handler::initialize();
    auto run2 = sc_spawn([&dut]() {
        sc_core::wait(1_ns);
        for(auto i : {0, 5, -1}) {
            dut.param4.set_value(i);
            sc_core::wait(1_ns);
        }
    });

    sc_start(10_ns);
    REQUIRE(run2.terminated());
    REQUIRE(sc_report_handler::get_count(SC_ERROR) == 2);
    REQUIRE(sc_report_handler::get_count(SC_WARNING) == 0);
    sc_report_handler::initialize();
}

TEST_CASE("simple cci_param_restricted max test", "[SCC][cci_param_restricted]") {
    sc_report_handler::set_actions(SC_ERROR, SC_LOG | SC_CACHE_REPORT | SC_DISPLAY | SC_DO_NOTHING);
    auto& dut = factory::get<top>();
    auto run1 = sc_spawn([&dut]() {
        sc_core::wait(1_ns);
        for(auto i : {0, 10, 11}) {
            dut.param5.set_value(i);
            sc_core::wait(1_ns);
        }
    });

    sc_start(10_ns);
    REQUIRE(run1.terminated());
    REQUIRE(sc_report_handler::get_count(SC_ERROR) == 1);
    REQUIRE(sc_report_handler::get_count(SC_WARNING) == 0);
    sc_report_handler::initialize();
    auto run2 = sc_spawn([&dut]() {
        sc_core::wait(1_ns);
        for(auto i : {0, 10, 11}) {
            dut.param6.set_value(i);
            sc_core::wait(1_ns);
        }
    });

    sc_start(10_ns);
    REQUIRE(run2.terminated());
    REQUIRE(sc_report_handler::get_count(SC_ERROR) == 2);
    REQUIRE(sc_report_handler::get_count(SC_WARNING) == 0);
    sc_report_handler::initialize();
}

TEST_CASE("simple cci_param_restricted discrete test", "[SCC][cci_param_restricted]") {
    sc_report_handler::set_actions(SC_ERROR, SC_LOG | SC_CACHE_REPORT | SC_DISPLAY | SC_DO_NOTHING);
    auto& dut = factory::get<top>();
    auto run1 = sc_spawn([&dut]() {
        sc_core::wait(1_ns);
        for(auto i : {4, 10}) {
            dut.param7.set_value(i);
            dut.param8.set_value(i);
            dut.param9.set_value(i);
            sc_core::wait(1_ns);
        }
    });

    sc_start(10_ns);
    REQUIRE(run1.terminated());
    REQUIRE(sc_report_handler::get_count(SC_ERROR) == 3);
    REQUIRE(sc_report_handler::get_count(SC_WARNING) == 0);
    sc_report_handler::initialize();
}
