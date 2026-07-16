#ifndef TOP_SERVER_H_
#define TOP_SERVER_H_

#include <scc/memory.h>
#include <scc/tcp4tlm_bridge.h>
#include <sysc/kernel/sc_module.h>

namespace tcp4tlm_bridge {
struct top_server : public sc_core::sc_module {
    scc::tcp4tlm_bridge bridge{"bridge"};
    scc::memory<16_MB> target{"target"};

    top_server(sc_core::sc_module_name name)
    : sc_module(name) {
        bridge.is_connection_server.set_value(true);
        bridge.write_no_response.set_value(false);
        bridge.no_systemc_sync.set_value(false);
        bridge.isckt.bind(target.target);
        dummy.bind(bridge.tsckt);
        SC_METHOD(shut_down);
        sensitive << bridge.get_shutdown_event();
        dont_initialize();
    }

    void start_of_simulation() { bridge.wait4connection(); }

    tlm_utils::simple_initiator_socket<top_server, scc::LT> dummy;

    void shut_down() { sc_core::sc_stop(); }
};
} // namespace tcp4tlm_bridge
#endif /* TOP_SERVER_H_ */
