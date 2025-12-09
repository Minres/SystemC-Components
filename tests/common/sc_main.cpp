/*
 * sc_main.cpp
 *
 *  Created on:
 *      Author:
 */

#include "factory.h"
#include <catch2/catch_session.hpp>
#include <csetjmp>
#include <csignal>
#include <cstdlib>
#include <scc/configurer.h>
#include <scc/report.h>
#include <scc/trace.h>
#include <scc/tracer.h>
#include <util/ities.h>

using namespace scc;
using namespace sc_core;

jmp_buf abrt;
void ABRThandler(int sig) { longjmp(abrt, 1); }

int sc_main(int argc, char* argv[]) {
    signal(SIGABRT, ABRThandler);
    auto my_name = util::split(argv[0], '/').back();
    auto level = getenv("SCC_TEST_VERBOSE");
    auto log_lvl = level ? static_cast<scc::log>(std::min(strtoul(level, nullptr, 10) + 4, 7UL)) : log::FATAL;
    scc::init_logging(LogConfig().logLevel(log_lvl).logAsync(false).msgTypeFieldWidth(35));
    scc::configurer cfg("");
    // create tracer if environment variable SCC_TEST_TRACE is defined
    std::unique_ptr<scc::tracer> tracer;
    if(auto* test_trace = getenv("SCC_TEST_TRACE")) {
        tracer = std::make_unique<scc::tracer>(my_name, scc::tracer::ENABLE, scc::tracer::ENABLE);
        cfg.set_value("scc_tracer.default_trace_enable", true);
    }
    int result = -1;
    if(setjmp(abrt) == 0) {
        // instantiate design(s)
        factory::get_instance().create();
        // run tests
        result = Catch::Session().run(argc, argv);
        // destroy design(s)
        sc_stop();
        SCCTRACE() << "Test sequence finished";
        factory::get_instance().destroy();
    }
    return result;
}
