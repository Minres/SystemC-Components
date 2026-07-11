/*
 * test_generator.cpp
 *
 *  Created on: 08.07.2011
 *      Author: eyck
 */

#include "test_generator.h"
#include <boost/asio.hpp>
#include <tlm_core/tlm_2/tlm_generic_payload/tlm_gp.h>

#define PIPELINE_LEN 5
#define NUMBER_OF_RANDOM_ROUNDS 1000
test_generator::test_generator(sc_core::sc_module_name nm, scc::tcp4tlm_bridge& bridge)
: sc_module(nm)
, bridge(bridge)
, gotInterrupt(false)
, gotFinalAccess(false) {
    SC_THREAD(run);
}

test_generator::~test_generator() {}

void test_generator::prepare_trans(tlm::tlm_generic_payload& req, bool read, unsigned long addr, unsigned numberOfBytes) {
    req.set_command(read ? tlm::TLM_READ_COMMAND : tlm::TLM_WRITE_COMMAND);
    req.set_address(addr);
    req.set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);

    if(req.get_data_ptr() && numberOfBytes > req.get_data_length()) {
        delete req.get_data_ptr();
        req.set_data_ptr(nullptr);
    }

    if(req.get_data_ptr() == NULL) {
        req.set_data_ptr(new unsigned char(numberOfBytes));
    }

    req.set_data_length(numberOfBytes);

    if(read) {
        memset(req.get_data_ptr(), req.get_data_length(), 1);
    } else {
        for(unsigned j = 0; j < req.get_data_length(); j++) {
            req.get_data_ptr()[j] = j;
        }
    }
}

void test_generator::run() {
    wait(10_ns);
    bridge.wait4connection();
    test();
    SCCDEBUG(SCMOD) << "===== Finished all tests =====";
    // bridge.endConnection();
    sc_core::sc_stop();
}

void test_generator::test() {
    SCCDEBUG(SCMOD) << "===== Running serial_test =====";
    tlm::tlm_generic_payload req;
    sc_core::sc_time delay;
    uint32_t* data = new uint32_t[4];
    data[0] = 0x1;  // command: read
    data[1] = 0x1;  // rep count
    data[2] = 0x10; // address
    data[3] = 0x4;  // length
    writeToHss(data);
    data[0] = 0x2;     // command: write
    data[1] = 0x1;     // rep count
    data[2] = 0xffffc; // address
    data[3] = 0x4;     // length
    writeToHss(data);
    data[0] = 0x11; // command: Raise interrupt
    writeToHss(data);
    data[0] = 0x10; // command: Clear interrupt
    writeToHss(data);

    while(!(gotFinalAccess && gotInterrupt)) {
        SCCDEBUG(SCMOD) << "Waiting for accesses to finish";
        boost::this_thread::sleep(boost::posix_time::milliseconds(10));
        wait(200_ns);
    }

    delete data;
}

void test_generator::writeToHss(uint32_t* data) {
    tlm::tlm_generic_payload req;
    sc_core::sc_time delay;
    req.set_address(0x10000000);
    req.set_data_ptr((unsigned char*)(data));
    req.set_data_length(16);
    req.set_command(tlm::TLM_WRITE_COMMAND);
    (tm[0])->isckt->b_transport(req, delay); // Blocking
    // Poll the status
    while(data[0]) {
        wait(10_ns);
        req.set_address(0x10000000);
        req.set_data_ptr((unsigned char*)(data));
        req.set_data_length(4);
        req.set_command(tlm::TLM_READ_COMMAND);
        delay = sc_core::SC_ZERO_TIME;
        (tm[0])->isckt->b_transport(req, delay); // Blocking
    }
}

void test_generator::execute(int idx, tlm::tlm_generic_payload& req, unsigned delay, bool blocking) {
    sc_core::sc_time d(delay, sc_core::SC_NS);
    (tm[idx])->isckt->b_transport(req, d);
}

tlm::tlm_sync_enum test_generator::adapter_cb(tlm::tlm_generic_payload& p) {
    if(p.get_address() >= 0xffffc && p.get_address() <= 0xfffff && p.get_command() == tlm::TLM_WRITE_COMMAND) {
        gotFinalAccess = true;
    }

    return tlm::TLM_COMPLETED;
}
