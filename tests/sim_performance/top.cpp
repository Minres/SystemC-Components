/*
 * top.cpp
 *
 *  Created on: 04.05.2020
 *      Author: eyck
 */

#include "top.h"
#include <fmt/format.h>
#include <scc/report.h>
#include <scc/utilities.h>

using namespace sc_core;
using namespace fmt;

top::top(sc_core::sc_module_name const& nm, uint8_t dimension, unsigned count)
: sc_module(nm) {
    sc_assert(dimension > 0);
    SC_HAS_PROCESS(top);
    for(auto yidx = 0U; yidx < dimension; ++yidx) {
        for(auto xidx = 0U; xidx < dimension; ++xidx) {
            auto name = format("sw_{}_{}", xidx, yidx);
            SCCDEBUG(SCMOD) << "instantiating switch " << xidx << "/" << yidx;
            switches.push_back(scc::make_unique<pkt_switch>(sc_module_name(name.c_str())));
            switches.back()->clk_i(clk);
        }
    }
    for(auto yidx = 0U; yidx < dimension; ++yidx) {
        for(auto xidx = 0U; xidx < dimension; ++xidx) {
            auto& sw = switches[yidx * dimension + xidx];
            if(xidx < dimension - 1) {
                auto& swr = switches[yidx * dimension + (xidx + 1)];
                sw->isck[RIGHT](swr->tsck[LEFT]);
                swr->isck[LEFT](sw->tsck[RIGHT]);
            }
            if(yidx < dimension - 1) {
                auto& swb = switches[(yidx + 1) * dimension + xidx];
                sw->isck[BOTTOM](swb->tsck[TOP]);
                swb->isck[TOP](sw->tsck[BOTTOM]);
            }
        }
    }
    auto yidx = 0U;
    auto xidx = 0U;
    for(xidx = 0U; xidx < dimension; ++xidx) {
        auto name = format("snd_{}_{}", xidx + 1, 0);
        senders[TOP].push_back(scc::make_unique<pkt_sender>(sc_module_name(name.c_str()), dimension, xidx + 1, 0, count));
        auto& snd = senders[TOP].back();
        snd->clk_i(clk);
        auto& sw = switches[yidx * dimension + xidx];
        snd->isck(sw->tsck[TOP]);
        sw->isck[TOP](snd->tsck);
    }
    yidx = dimension - 1;
    for(xidx = 0U; xidx < dimension; ++xidx) {
        auto name = format("snd_{}_{}", xidx + 1, dimension + 1);
        senders[BOTTOM].push_back(scc::make_unique<pkt_sender>(sc_module_name(name.c_str()), dimension, xidx + 1, dimension + 1, count));
        auto& snd = senders[BOTTOM].back();
        snd->clk_i(clk);
        auto& sw = switches[yidx * dimension + xidx];
        snd->isck(sw->tsck[BOTTOM]);
        sw->isck[BOTTOM](snd->tsck);
    }
    xidx = 0U;
    for(yidx = 0U; yidx < dimension; ++yidx) {
        auto name = format("snd_{}_{}", 0, yidx + 1);
        senders[LEFT].push_back(scc::make_unique<pkt_sender>(sc_module_name(name.c_str()), dimension, 0, yidx + 1, count));
        auto& snd = senders[LEFT].back();
        snd->clk_i(clk);
        auto& sw = switches[yidx * dimension + xidx];
        snd->isck(sw->tsck[LEFT]);
        sw->isck[LEFT](snd->tsck);
    }
    xidx = dimension - 1;
    for(yidx = 0U; yidx < dimension; ++yidx) {
        auto name = format("snd_{}_{}", dimension + 1, yidx + 1);
        senders[RIGHT].push_back(scc::make_unique<pkt_sender>(sc_module_name(name.c_str()), dimension, dimension + 1, yidx + 1, count));
        auto& snd = senders[RIGHT].back();
        snd->clk_i(clk);
        auto& sw = switches[yidx * dimension + xidx];
        snd->isck(sw->tsck[RIGHT]);
        sw->isck[RIGHT](snd->tsck);
    }
    SC_THREAD(run);
}

void top::run() {
    sc_event_and_list evt_list;
    for(auto& sides : senders) {
        for(auto& sender : sides) {
            evt_list &= sender->get_finish_event();
        }
    }
    wait(evt_list);
    sc_stop();
}
