/*
 * top.h
 *
 *  Created on: 04.05.2020
 *      Author: eyck
 */

#ifndef _SIM_PERFORMANCE_TOP_H_
#define _SIM_PERFORMANCE_TOP_H_

#include "pkt_sender.h"
#include "pkt_switch.h"
#include "types.h"
#include <memory>
#include <systemc>
#include <vector>

class top : public sc_core::sc_module {
public:
    top(sc_core::sc_module_name const&, uint8_t, unsigned);
    virtual ~top() = default;

private:
    void run();
    sc_core::sc_clock clk;
    std::array<std::vector<std::unique_ptr<pkt_sender>>, SIDES> senders;
    std::vector<std::unique_ptr<pkt_switch>> switches;
};

#endif /* TESTS_SIM_PERFORMANCE_TOP_H_ */
