/*
 * blocks.h
 *
 *  Created on: 04.05.2020
 *      Author: eyck
 */

#ifndef _SIM_PERFORMANCE_PKT_SENDER_H_
#define _SIM_PERFORMANCE_PKT_SENDER_H_

#include "packet.h"
#include <systemc>
#include <tlm/scc/initiator_mixin.h>
#include <tlm/scc/target_mixin.h>

class pkt_sender : sc_core::sc_module {
public:
    sc_core::sc_in<bool> clk_i{"clk_i"};
    tlm::scc::initiator_mixin<tlm::tlm_initiator_socket<32>> isck;
    tlm::scc::target_mixin<tlm::tlm_target_socket<32>> tsck;
    pkt_sender(sc_core::sc_module_name const&, unsigned dim, unsigned pos_x, unsigned pos_y, unsigned count);
    virtual ~pkt_sender() = default;
    sc_core::sc_event const& get_finish_event() { return finish_evt; }

private:
    void run();
    void gen_routing(std::vector<uint8_t>& route_vec);
    void received();
    tlm_utils::peq_with_get<tlm::tlm_generic_payload> bw_peq, fw_peq;
    tlm::tlm_sync_enum nb_fw(tlm::tlm_generic_payload&, tlm::tlm_phase&, sc_core::sc_time&);
    tlm::tlm_sync_enum nb_bw(tlm::tlm_generic_payload&, tlm::tlm_phase&, sc_core::sc_time&);
    sc_core::sc_event finish_evt;
    std::pair<unsigned, unsigned> my_pos;
    const unsigned dim, count;
};

#endif /* _SIM_PERFORMANCE_PKT_SENDER_H_ */
