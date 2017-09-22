/*
 * utilities.cpp
 *
 *  Created on: Nov 5, 2016
 *      Author: eyck
 */
#include <sysc/utilities.h>

namespace sc_core {
void sc_trace(sc_trace_file *tf, const sc_time &t, const std::string &name) { sc_trace(tf, t.value(), name); }

void sc_trace(sc_trace_file *tf, const sc_time &t, const char *name) { sc_trace(tf, t.value(), name); }
template <> void sc_trace(sc_trace_file *tf, const sc_in<sc_time> &port, const std::string &name) {
    const sc_signal_in_if<sc_time> *iface = nullptr;
    if (sc_get_curr_simcontext()->elaboration_done()) {
        iface = DCAST<const sc_signal_in_if<sc_time> *>(port.get_interface());
    }

    if (iface)
        sc_trace(tf, iface->read(), name);
    else
        port.add_trace_internal(tf, name);
}

template <> void sc_trace(sc_trace_file *tf, const sc_inout<sc_time> &port, const std::string &name) {
    const sc_signal_in_if<sc_time> *iface = nullptr;
    if (sc_get_curr_simcontext()->elaboration_done()) {
        iface = DCAST<const sc_signal_in_if<sc_time> *>(port.get_interface());
    }

    if (iface)
        sc_trace(tf, iface->read(), name);
    else
        port.add_trace_internal(tf, name);
}
}
