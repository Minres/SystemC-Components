/*
 * blocks.cpp
 *
 *  Created on: 04.05.2020
 *      Author: eyck
 */

#include "pkt_sender.h"
#include "types.h"
#include <scc/report.h>
#include <tlm/scc/tlm_mm.h>

using namespace sc_core;

pkt_sender::pkt_sender(const sc_core::sc_module_name& nm, unsigned dim, unsigned pos_x, unsigned pos_y, unsigned count)
: sc_module(nm)
, bw_peq("bw_peq")
, fw_peq("fw_peq")
, my_pos{pos_x, pos_y}
, dim{dim}
, count{count} {
    SCCDEBUG(SCMOD) << "instantiating sender " << pos_x << "/" << pos_y;
    SC_HAS_PROCESS(pkt_sender);
    isck.register_nb_transport_bw([this](tlm::tlm_generic_payload& gp, tlm::tlm_phase& phase,
                                         sc_core::sc_time& delay) -> tlm::tlm_sync_enum { return this->nb_bw(gp, phase, delay); });
    tsck.register_nb_transport_fw([this](tlm::tlm_generic_payload& gp, tlm::tlm_phase& phase,
                                         sc_core::sc_time& delay) -> tlm::tlm_sync_enum { return this->nb_fw(gp, phase, delay); });
    SC_METHOD(received);
    sensitive << fw_peq.get_event();
    dont_initialize();
    SC_THREAD(run);
}

void pkt_sender::gen_routing(std::vector<uint8_t>& route_vec) {
    if(std::get<0>(my_pos) == 0) {
        for(auto i = 0; i < dim; ++i)
            route_vec.push_back(RIGHT);
    } else if(std::get<0>(my_pos) == dim + 1) {
        for(auto i = 0; i < dim; ++i)
            route_vec.push_back(LEFT);
    } else if(std::get<1>(my_pos) == 0) {
        for(auto i = 0; i < dim; ++i)
            route_vec.push_back(BOTTOM);
    } else if(std::get<1>(my_pos) == dim + 1) {
        for(auto i = 0; i < dim; ++i)
            route_vec.push_back(TOP);
    } else
        SCCERR(SCMOD) << "WTF!?!";
}

void pkt_sender::run() {
    wait(clk_i.posedge_event());
    for(auto i = 0U; i < count; i++) {
        tlm::tlm_generic_payload* gp = tlm::scc::tlm_mm<>::get().allocate<packet_ext>();
        gen_routing(gp->get_extension<packet_ext>()->routing);
        tlm::tlm_phase phase{tlm::BEGIN_REQ};
        sc_time delay;
        gp->acquire();
        auto sync = isck->nb_transport_fw(*gp, phase, delay);
        sc_assert(sync == tlm::TLM_UPDATED && phase == tlm::END_REQ);
        tlm::tlm_generic_payload* ret{nullptr};
        while(!(ret = bw_peq.get_next_transaction())) {
            wait(bw_peq.get_event());
        }
        sc_assert(gp == ret);
        ret->release();
    }
    finish_evt.notify(SC_ZERO_TIME);
}

tlm::tlm_sync_enum pkt_sender::nb_bw(tlm::tlm_generic_payload& gp, tlm::tlm_phase& phase, sc_core::sc_time& delay) {
    sc_assert(phase == tlm::BEGIN_RESP);
    bw_peq.notify(gp, delay);
    phase = tlm::END_RESP;
    return tlm::TLM_COMPLETED;
}

tlm::tlm_sync_enum pkt_sender::nb_fw(tlm::tlm_generic_payload& gp, tlm::tlm_phase& phase, sc_core::sc_time& delay) {
    sc_assert(phase == tlm::BEGIN_REQ);
    auto ext = gp.get_extension<packet_ext>();
    sc_assert(ext->routing.size() == 0);
    gp.acquire();
    fw_peq.notify(gp, delay);
    phase = tlm::END_REQ;
    return tlm::TLM_UPDATED;
}

void pkt_sender::received() {
    if(auto gp = fw_peq.get_next_transaction()) {
        tlm::tlm_phase phase{tlm::BEGIN_RESP};
        sc_time delay;
        auto sync = tsck->nb_transport_bw(*gp, phase, delay);
        sc_assert(sync == tlm::TLM_COMPLETED && phase == tlm::END_RESP);
        gp->release();
    }
}
