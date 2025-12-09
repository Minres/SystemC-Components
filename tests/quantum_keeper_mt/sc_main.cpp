#include "top_module.h"
#include "util/logging.h"
#include <csetjmp>
#include <csignal>
#include <cstdlib>
#include <scc/configurer.h>
#include <scc/report.h>
#include <scc/trace.h>
#include <scc/tracer.h>
#include <sysc/kernel/sc_simcontext.h>
#include <sysc/utils/sc_report.h>
#include <sysc/utils/sc_report_handler.h>
#include <tlm_utils/tlm_quantumkeeper.h>
#include <util/ities.h>

using namespace scc;
using namespace sc_core;

jmp_buf abrt;
void ABRThandler(int sig) { longjmp(abrt, 1); }

int sc_main(int argc, char* argv[]) {
    signal(SIGABRT, ABRThandler);
    auto my_name = util::split(argv[0], '/').back();
    auto level = getenv("SCC_TEST_VERBOSE");
    auto log_lvl = level ? static_cast<scc::log>(std::min(strtoul(level, nullptr, 10), 7UL)) : log::FATAL;
    scc::init_logging(LogConfig().logLevel(log_lvl).logAsync(false).msgTypeFieldWidth(35).printSysTime());
    scc::configurer cfg("");
    // create tracer if environment variable SCC_TEST_TRACE is defined
    std::unique_ptr<scc::tracer> tracer;
    if(auto* test_trace = getenv("SCC_TEST_TRACE")) {
        tracer = std::make_unique<scc::tracer>(my_name, scc::tracer::ENABLE, scc::tracer::ENABLE);
        tracer->set_default_trace_enable(true);
    }
    int result = -1;
    tlm_utils::tlm_quantumkeeper::set_global_quantum(3_us);
    if(setjmp(abrt) == 0) {
        // instantiate design(s)
        top_module top_mod("top_module_inst");
        // Start the simulation
        sc_core::sc_start(20_us);
        if(sc_core::sc_is_running())
            sc_core::sc_stop();
        SCCINFO() << "Simulation finished";
    } else {
        SCCERR() << "Some error occured";
    }
    return sc_core::sc_report_handler::get_count(sc_core::SC_ERROR) + sc_core::sc_report_handler::get_count(sc_core::SC_WARNING);
}
