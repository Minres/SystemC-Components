/*
 * blocks.cpp
 *
 *  Created on: 04.05.2020
 *      Author: eyck
 */

#ifndef SC_INCLUDE_DYNAMIC_PROCESSES // needed for sc_spawn
#define SC_INCLUDE_DYNAMIC_PROCESSES
#include <tlm_core/tlm_2/tlm_generic_payload/tlm_gp.h>
#endif
#include "pkt_switch.h"
#include "types.h"
#include <scc/report.h>

using namespace sc_core;

pkt_switch::pkt_switch(const sc_core::sc_module_name& nm)
: sc_module(nm) {
    SC_HAS_PROCESS(pkt_switch);
    auto index = 0U;
    for(auto& s : isck) {
        s.register_nb_transport_bw([this](unsigned id, tlm::tlm_generic_payload& gp, tlm::tlm_phase& phase,
                                          sc_core::sc_time& delay) -> tlm::tlm_sync_enum { return this->nb_bw(id, gp, phase, delay); },
                                   index++);
    }
    index = 0U;
    for(auto& s : tsck) {
        s.register_nb_transport_fw([this](unsigned id, tlm::tlm_generic_payload& gp, tlm::tlm_phase& phase,
                                          sc_core::sc_time& delay) -> tlm::tlm_sync_enum { return this->nb_fw(id, gp, phase, delay); },
                                   index++);
    }
    SC_METHOD(clock_cb);
    sensitive << clk_i.pos();
    dont_initialize();
    for(auto i = 0U; i < SIDES; ++i) {
        sc_core::sc_spawn_options opts;
        opts.spawn_method();
        opts.set_sensitivity(&out_fifo[i].data_written_event());
        sc_core::sc_spawn([this, i]() -> void { this->output_cb(i); }, sc_core::sc_gen_unique_name("out_peq"), &opts);
    }
}

tlm::tlm_sync_enum pkt_switch::nb_fw(unsigned id, tlm::tlm_generic_payload& gp, tlm::tlm_phase& phase, sc_core::sc_time& delay) {
    in_tx[id].notify(&gp);
    if(phase == tlm::BEGIN_REQ)
        phase = tlm::END_REQ;
    else
        SCCERR(SCMOD) << "WTF!?!";
    return tlm::TLM_UPDATED;
}

void pkt_switch::clock_cb() {
    std::array<std::vector<std::tuple<unsigned, tlm::tlm_generic_payload*>>, SIDES> routing{};
    bool nothing_todo = true;
    for(auto i = 0U; i < SIDES; ++i) {
        if(in_tx[i].has_next()) {
            auto gp = in_tx[i].get();
            auto ext = gp->get_extension<packet_ext>();
            sc_assert(ext);
            routing[ext->routing.back()].emplace_back(i, gp);
            nothing_todo = false;
        }
    }
    if(nothing_todo)
        return;
    for(auto i = 0U; i < SIDES; ++i) {
        if(routing[i].size()) {
            auto e = routing[i].front();
            auto selected_input = std::get<0>(e);
            auto gp = std::get<1>(e);
            if(out_fifo[i].nb_write(gp)) {
                auto ext = gp->get_extension<packet_ext>();
                ext->routing.pop_back();
                gp->acquire();
                tlm::tlm_phase phase{tlm::BEGIN_RESP};
                sc_core::sc_time delay;
                auto res = tsck[selected_input]->nb_transport_bw(*gp, phase, delay);
                if(res != tlm::TLM_COMPLETED && !(res == tlm::TLM_UPDATED && phase == tlm::END_RESP))
                    SCCERR(SCMOD) << "WTF!?!";
                in_tx[selected_input].clear();
            }
        }
    }
}

void pkt_switch::output_cb(unsigned id) {

    if(out_fifo[id].num_available()) {
        auto* gp = out_fifo[id].read();
        tlm::tlm_phase phase{tlm::BEGIN_REQ};
        sc_time delay;
        auto sync = isck[id]->nb_transport_fw(*gp, phase, delay);
        sc_assert(sync == tlm::TLM_UPDATED && phase == tlm::END_REQ);
    }
}

tlm::tlm_sync_enum pkt_switch::nb_bw(unsigned id, tlm::tlm_generic_payload& gp, tlm::tlm_phase& phase, sc_core::sc_time& delay) {
    gp.release();
    sc_assert(phase == tlm::BEGIN_RESP);
    phase = tlm::END_RESP;
    return tlm::TLM_COMPLETED;
}
