#include "signal_opt_ports.h"
template class sc_core::sc_port<sc_core::sc_signal_in_if<bool>, 1, sc_core::SC_ZERO_OR_MORE_BOUND>;
template class sc_core::sc_port<sc_core::sc_signal_in_if<sc_dt::sc_logic>, 1, sc_core::SC_ZERO_OR_MORE_BOUND>;
template class sc_core::sc_port<sc_core::sc_signal_inout_if<bool>, 1, sc_core::SC_ZERO_OR_MORE_BOUND>;
template class sc_core::sc_port<sc_core::sc_signal_inout_if<sc_dt::sc_logic>, 1, sc_core::SC_ZERO_OR_MORE_BOUND>;
