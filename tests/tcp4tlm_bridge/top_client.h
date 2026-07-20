#ifndef TOP_CLIENT_H_
#define TOP_CLIENT_H_

#include "initiator.h"
#include <scc/tcp4tlm_bridge.h>

namespace tcp4tlm_bridge {
struct top_client : public sc_core::sc_module {
    scc::tcp4tlm_bridge bridge{"bridge"};
    initiator intor{"intor"};

    top_client(sc_core::sc_module_name name)
    : sc_module(name) {
        bridge.is_connection_server.set_value(false);
        bridge.write_no_response.set_value(false);
        bridge.no_systemc_sync.set_value(false);
        bridge.isckt.bind(dummy);
        intor.isckt.bind(bridge.tsckt);
    };

    tlm_utils::simple_target_socket<top_client, scc::LT> dummy;

    void end_of_simulation() override { bridge.end_connection(); }
};
} // namespace tcp4tlm_bridge
#endif /* TOP_CLIENT_H_ */
