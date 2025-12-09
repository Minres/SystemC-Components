/*
 * blocks.h
 *
 *  Created on: 04.05.2020
 *      Author: eyck
 */

#ifndef _SIM_PERFORMANCE_PKT_SWITCH_H_
#define _SIM_PERFORMANCE_PKT_SWITCH_H_

#include "packet.h"
#include <array>
#include <scc/sc_owning_signal.h>
#include <systemc>
#include <tlm/scc/tagged_initiator_mixin.h>
#include <tlm/scc/tagged_target_mixin.h>

class pkt_switch : sc_core::sc_module {
public:
    enum { NONE = std::numeric_limits<unsigned>::max() };
    sc_core::sc_in<bool> clk_i{"clk_i"};
    sc_core::sc_vector<tlm::scc::tagged_target_mixin<tlm::tlm_target_socket<32>>> tsck{"tsck", 4};
    sc_core::sc_vector<tlm::scc::tagged_initiator_mixin<tlm::tlm_initiator_socket<32>>> isck{"isck", 4};
    pkt_switch(sc_core::sc_module_name const&);
    virtual ~pkt_switch() = default;

private:
    void clock_cb();
    void output_cb(unsigned);
    tlm::tlm_sync_enum nb_fw(unsigned, tlm::tlm_generic_payload&, tlm::tlm_phase&, sc_core::sc_time&);
    tlm::tlm_sync_enum nb_bw(unsigned, tlm::tlm_generic_payload&, tlm::tlm_phase&, sc_core::sc_time&);
    sc_core::sc_vector<scc::sc_owning_signal<tlm::tlm_generic_payload>> in_tx{"in_sig", 4};
    sc_core::sc_vector<sc_core::sc_fifo<tlm::tlm_generic_payload*>> out_fifo{"out_peq", 4};
    std::array<unsigned, 4> last_sel_inp{NONE, NONE, NONE, NONE};
};

#endif /* _SIM_PERFORMANCE_PKT_SWITCH_H_ */
