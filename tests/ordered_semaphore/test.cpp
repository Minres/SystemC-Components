#ifndef SC_INCLUDE_DYNAMIC_PROCESSES
#define SC_INCLUDE_DYNAMIC_PROCESSES
#endif
#include <catch2/catch_all.hpp>
#include <factory.h>
#include <scc/ordered_semaphore.h>
#include <scc/utilities.h>
#include <systemc>

using namespace sc_core;

struct top : public sc_core::sc_module {
    top()
    : top("top") {}
    top(sc_module_name const& nm)
    : sc_core::sc_module(nm) {}
    scc::ordered_semaphore sem{"sem", 2};
    scc::ordered_semaphore_t<2> sem_t{"sem_t"};
};

factory::add<top> tb;

TEST_CASE("simple ordered_semaphore test", "[SCC][ordered_semaphore]") {

    auto& dut = factory::get<top>();
    auto run1 = sc_spawn([&dut]() {
        dut.sem.wait();
        dut.sem_t.wait();
        dut.sem.set_capacity(4);
        dut.sem_t.set_capacity(4); // should fail
        dut.sem_t.post();
        dut.sem.post();
    });

    sc_start(1_ns);
    REQUIRE(run1.terminated());
    REQUIRE(sc_report_handler::get_count(SC_ERROR) == 0);
    REQUIRE(sc_report_handler::get_count(SC_WARNING) == 1);
}
