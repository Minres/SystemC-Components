#ifndef TOP_SERVER_H_
#define TOP_SERVER_H_

#include "scc/router.h"
#include <scc/memory.h>
#include <scc/tcp4tlm_bridge.h>
#include <sysc/kernel/sc_event.h>
#include <sysc/kernel/sc_module.h>
#include <sysc/kernel/sc_time.h>
#include <sysc/utils/sc_vector.h>

namespace tcp4tlm_bridge {
struct top_server : public sc_core::sc_module {
    sc_core::sc_vector<scc::tcp4tlm_bridge> bridge{"bridge"};
    scc::router<> bus{"bus"};
    scc::memory<16_MB> target{"target"};
    SC_HAS_PROCESS(top_server);

    top_server(sc_core::sc_module_name name, unsigned num_of_clients, unsigned short base_port_num)
    : sc_module(name)
    , bridge("bridge", num_of_clients)
    , bus("bus", 1, num_of_clients)
    , dummy("dummy", num_of_clients) {

        for(size_t idx = 0; idx < num_of_clients; idx++) {
            auto& b = bridge[idx];
            b.is_connection_server.set_value(true);
            b.this_host_port.set_value(base_port_num + idx);
            b.write_no_response.set_value(false);
            b.no_systemc_sync.set_value(false);
            b.isckt.bind(bus.target[idx]);
            dummy[idx].bind(b.tsckt);
        }
        bus.initiator[0](target.target);
        bus.set_default_target(0);
        SC_THREAD(shut_down);
    }

    void start_of_simulation() {
        for(auto& b : bridge) // start all servers
            b.start_server();
        for(auto& b : bridge) // wait for all servers
            b.wait4connection();
    }

    sc_core::sc_vector<tlm_utils::simple_initiator_socket<top_server, scc::LT>> dummy;

    void shut_down() {
        wait(sc_core::SC_ZERO_TIME);
        sc_core::sc_event_and_list finish_list;
        for(auto& b : bridge) {
            finish_list &= b.get_shutdown_event();
        }
        wait(finish_list);
        sc_core::sc_stop();
    }
};
} // namespace tcp4tlm_bridge
#endif /* TOP_SERVER_H_ */
