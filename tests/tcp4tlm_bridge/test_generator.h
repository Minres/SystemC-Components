/*
 * test_generator.h
 *
 *  Created on: 08.07.2011
 *      Author: eyck
 */

#ifndef TEST_GENERATOR_H_
#define TEST_GENERATOR_H_
#include "initiator.h"
#include <scc/tcp4tlm_bridge.h>
#include <sysc/kernel/sc_module.h>

class test_generator : public sc_core::sc_module {
public:
    SC_HAS_PROCESS(test_generator);
    test_generator(sc_core::sc_module_name nm, scc::tcp4tlm_bridge& bridge);
    virtual ~test_generator();
    void add_test_master(tlm::scc::tcp::initiator* t) { tm.push_back(t); }
    void gotIrq() { gotInterrupt = true; }
    void gotFinalAcc() { gotFinalAccess = true; }

protected:
    scc::tcp4tlm_bridge& bridge;
    void run();
    void test();
    void prepare_trans(tlm::tlm_generic_payload&, bool read, unsigned long, unsigned numberOfBytes = 4);
    void execute(int idx, tlm::tlm_generic_payload& req, unsigned delay = 0, bool blocking = true);
    tlm::tlm_sync_enum adapter_cb(tlm::tlm_generic_payload& p);

private:
    std::vector<tlm::scc::tcp::initiator*> tm;
    bool gotInterrupt;
    bool gotFinalAccess;
    void writeToHss(uint32_t* data);
};

#endif /* TEST_GENERATOR_H_ */
