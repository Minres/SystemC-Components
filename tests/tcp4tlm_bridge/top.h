/*
 * top.h
 *
 *  Created on: Jul 9, 2014
 *      Author: ejentzsx
 */

#ifndef TOP_H_
#define TOP_H_

#include "initiator.h"
#include "scc/memory.h"
#include "scc/router.h"
#include "test_generator.h"
#include <scc/tcp4tlm_bridge.h>

#define MOD(X) X(#X)
#define MOD1A(X, A) X(#X, A)
#define MOD2A(X, A, B) X(#X, A, B)
#define ATTR(X, DEF) X(#X, DEF, this)
#define SIG(X) X(#X)

struct top : public sc_core::sc_module {
protected:
    std::vector<tlm::scc::tcp::initiator*> tmv;

public:
    SC_HAS_PROCESS(top);

    tlm::scc::tcp::initiator master0{"master0"};
    scc::router<> bus{"bus", 2, 2};
    scc::memory<16_MB> ram0{"ram0"};
    scc::tcp4tlm_bridge bridge{"bridge"};
    test_generator tg{"tg", bridge};
    sc_core::sc_signal<bool> intr{"intr"};

    top(sc_core::sc_module_name name)
    : sc_module(name) {
        tg.add_test_master(&master0);

        master0.isckt(bus.target[0]);
        bridge.isckt(bus.target[1]);

        bus.bind_target(ram0.target, 0, 0x0, ram0.getSize());
        bus.bind_target(bridge.tsckt, 1, 0x10000000, 0x10);
        bridge.signals[0](intr);

        SC_METHOD(intr_cb);
        dont_initialize();
        sensitive << intr;
    }

    virtual ~top() {}

    void intr_cb() {
        SCCINFO(SCMOD) << "Interrupt " << (intr.read() ? "raised" : "cleared");
        if(!intr.read()) {
            tg.gotIrq();
        }
    }
};

#endif /* TOP_H_ */
