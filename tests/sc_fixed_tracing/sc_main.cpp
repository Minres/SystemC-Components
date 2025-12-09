#define SC_INCLUDE_FX
#include <scc/cci_util.h>
#include <scc/configurer.h>
#include <scc/observer.h>
#include <scc/sc_variable.h>
#include <scc/tracer.h>
#include <string>
#include <systemc>

using namespace sc_dt;
using namespace std;

struct testbench : public sc_core::sc_module {
    scc::sc_variable<sc_dt::sc_fixed<6, 4>> a{"a", sc_dt::sc_fixed<6, 4>()};
    scc::sc_variable<sc_fixed<4, 2, SC_RND, SC_SAT>> b_sc_sat{"b_sc_sat", 0};
    scc::sc_variable<sc_fixed<8, 3>> a_qant{"a_qant", 0};
    scc::sc_variable<sc_fixed<5, 3, SC_RND>> b_sc_rnd{"b_sc_rnd", 0};
    scc::sc_variable<sc_fixed<5, 3, SC_TRN>> b_sc_trn{"b_sc_trn", 0};
    sc_fixed<5, 3> a_fixed;
    sc_dt::sc_fix a_fix{5, 3};
    sc_fxtype_params params{5, 4};
    sc_fxtype_context context{params};
    // becase we do not specify in b_fix constructor anything
    // the parameters are taken form the latest created context
    sc_fix b_fix;
    sc_fix c_fix{5, 3};

    testbench(sc_core::sc_module_name const& nm)
    : sc_module(nm) {
        SC_HAS_PROCESS(testbench);
        SC_THREAD(run);
    }

    void trace(sc_core::sc_trace_file* trf) const override {
        a.trace(trf);
        scc::sc_trace(trf, a_fixed, std::string(name()) + ".a_fixed");
        scc::sc_trace(trf, a_fix, std::string(name()) + ".a_fix");
        scc::sc_trace(trf, b_fix, std::string(name()) + ".b_fix");
        scc::sc_trace(trf, c_fix, std::string(name()) + ".c_fix");
    }

    void run() {
        wait(1_ns);
        init();
        wait(1_ns);
        test_overflow_modes();
        wait(1_ns);
        test_quantization_modes();
        wait(1_ns);
        this->a = 2;
        wait(1_ps);
        sc_core::sc_stop();
    }

    void init() {
        a_fixed = 1.75;
        a_fix = 1.75;
        b_fix = 1.75;
        c_fix = 1.75;
    }
    void test_overflow_modes() {
        a = -7;
        b_sc_sat = *a;
    }

    void test_quantization_modes() {
        a_qant = -2.3125;
        b_sc_rnd = *a_qant;
        b_sc_trn = *a_qant;
    }
};

int sc_main(int sc_argc, char* sc_argv[]) {
    scc::init_logging(scc::log::INFO);
    scc::configurer cfg("");
    scc::tracer trc("sc_fixed_tracing");
    testbench tb("tb");
    sc_core::sc_start();
    SCCINFO("sc_main") << "End Simulation.";

    return sc_core::sc_report_handler::get_count(sc_core::SC_ERROR) + sc_core::sc_report_handler::get_count(sc_core::SC_WARNING);
} // End of 'sc_main'
