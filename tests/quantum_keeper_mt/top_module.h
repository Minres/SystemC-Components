#ifndef _TOP_MODULE_H_
#define _TOP_MODULE_H_

#include "scc/mt19937_rng.h"
#include <atomic>
#include <chrono>
#include <scc/async_queue.h>
#include <scc/async_thread.h>
#include <scc/report.h>
#include <scc/router.h>
#include <sysc/kernel/sc_initializer_function.h>
#include <sysc/kernel/sc_module.h>
#include <sysc/kernel/sc_simcontext.h>
#include <sysc/kernel/sc_time.h>
#include <thread>
#include <tlm/scc/quantum_keeper.h>
#include <tlm>
#include <tlm_core/tlm_2/tlm_generic_payload/tlm_gp.h>
#include <tlm_utils/simple_initiator_socket.h>
#include <tlm_utils/simple_target_socket.h>
#include <util/logging.h>

enum { CLIENT_DELAY = 100, SC_DELAY = 10 };

struct initiator : ::sc_core ::sc_module {
    tlm_utils::simple_initiator_socket<initiator, scc::LT> isckt{"isckt"};

    initiator(sc_core::sc_module_name nm, sc_core::sc_time period = 1_us)
    : sc_core::sc_module(nm)
    , period(period) {
        SC_THREAD(run);
    }

    ~initiator() = default;

private:
    void run() {
        wait(sc_core::SC_ZERO_TIME); // guard elaboration phase
        quantum_keeper.reset();
        core_executor.start([this]() { return thread_exec(); });
        wait(core_executor.thread_finish_event());
        sc_core::sc_stop();
    }

    sc_core::sc_time thread_exec() {
        SCCDEBUG(SCMOD) << "starting thread_exec";
        for(auto i = 0u; i < 16; ++i) {
            std::this_thread::sleep_for(std::chrono::milliseconds(CLIENT_DELAY));
            if(i && (i % 3) == 0) {
                tlm::tlm_generic_payload gp;
                sc_core::sc_time t;
                SCCDEBUG(SCMOD) << "initiating b_transport at local time " << quantum_keeper.get_local_absolute_time();
                quantum_keeper.execute_on_sysc([this, &gp, &t]() {
                    SCCDEBUG(SCMOD) << "executing b_transport";
                    this->isckt->b_transport(gp, t);
                });
                if(t.value()) {
                    SCCDEBUG(SCMOD) << "incrementing local time by b_transport delay of " << t;
                    quantum_keeper.inc(t);
                }
            } else {
                quantum_keeper.check_and_sync(period);
            }
            SCCDEBUG(SCMOD) << "local time now " << quantum_keeper.get_local_absolute_time();
        }
        SCCDEBUG(SCMOD) << "finished thread_exec at local time " << quantum_keeper.get_local_absolute_time();
        return quantum_keeper.get_local_absolute_time();
    }
    tlm::scc::quantumkeeper_mt quantum_keeper;
    scc::async_thread core_executor;
    const sc_core::sc_time period{1_us};
};

// top_module
struct top_module : ::sc_core ::sc_module {
    initiator core0{"core0"};
    initiator core1{"core1", 1500_ns};
    scc::router<scc::LT> router{"router", 1, 2};

    tlm_utils::simple_target_socket<top_module, scc::LT> tsckt{"tsckt"};

    top_module(sc_core::sc_module_name nm)
    : sc_core::sc_module(nm) {
        core0.isckt(router.target[0]);
        core1.isckt(router.target[1]);
        router.initiator[0](tsckt);
        router.set_default_target(0);
        tsckt.register_b_transport(this, &top_module::b_transport);
        SC_THREAD(run);
    }

    ~top_module() = default;

    void b_transport(tlm::tlm_generic_payload& gp, sc_core::sc_time& t) {
        SCCDEBUG(SCMOD) << "Received b_transport call at local time " << t;
        auto cycles = rng.uniform(0, 5);
        t += sc_core::sc_time(cycles, sc_core::SC_US);
        gp.set_response_status(tlm::TLM_OK_RESPONSE);
    }

    void run() {
        wait(sc_core::SC_ZERO_TIME);
        while(true) {
            std::this_thread::sleep_for(std::chrono::milliseconds(SC_DELAY));
            wait(500_ns);
        }
    }

    scc::MT19937 rng;
};
#endif // _TOP_MODULE_H_
