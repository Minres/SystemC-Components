////////////////////////////////////////////////////////////////////////////////
// Copyright 2017 eyck@minres.com
//
// Licensed under the Apache License, Version 2.0 (the "License"); you may not
// use this file except in compliance with the License.  You may obtain a copy
// of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
// WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
// License for the specific language governing permissions and limitations under
// the License.
////////////////////////////////////////////////////////////////////////////////
/*
 * test_initiator.cpp
 *
 *  Created on: 17.09.2017
 *      Author: eyck@minres.com
 */

#include "test_initiator.h"
#include <array>
#include <scc/report.h>
#include <scc/utilities.h>

// todo: move into gen folder somewhere (adapt code-generator)
#define PLIC_PRIO1_REG 0x0C000004
#define PLIC_PRIO2_REG 0x0C000008
#define PLIC_PRIO3_REG 0x0C00000C
#define PLIC_PRIO4_REG 0x0C000010
#define PLIC_PENDING_REG 0x0C001000
#define PLIC_ENABLE_REG 0x0C002000
#define PLIC_PRIO_TRESHOLD_REG 0x0C200000
#define PLIC_CLAIM_COMPLETE_REG 0x0C200004

namespace sysc {
using namespace sc_core;

test_initiator::test_initiator(sc_module_name nm)
: sc_module(nm)
, NAMED(intor)
, NAMED(rst_i)
, NAMED(global_interrupts_o, 256)
, NAMED(core_interrupt_i) {
    SC_THREAD(run);

    SC_METHOD(core_irq_handler);
    sensitive << core_interrupt_i;
    dont_initialize();
}

void test_initiator::run() {
    // wait for reset
    if(rst_i.read() == false)
        wait(rst_i.posedge_event());
    wait(rst_i.negedge_event());
    wait(10_ns);

    // apply test-sequences
    test_unique_irq();
    test_frequent_irq();
    test_parallel_irq();
    test_irq_stress();

    // todo: review irq sequences from FW point of view ... expected ???
    wait(100_ns);
    sc_stop();
}

void test_initiator::test_unique_irq() {

    //// enable reg is not set
    // -> irq to be ignored
    // -> no core_interrupt
    // -> no entry in pending reg

    // generate interrupt pulse (note: 1 is lowest usable register)
    global_interrupts_o[2].write(true);
    wait(10_ns);
    global_interrupts_o[2].write(false);
    wait(10_ns);

    reg_check(PLIC_PENDING_REG, 0x0);
    wait(10_ns);
    reg_check(PLIC_CLAIM_COMPLETE_REG, 0x0);
    wait(10_ns);

    //// enable reg is set, then
    // -> pending bit change expected
    // -> core_interrupt expected

    read_bus(PLIC_PRIO1_REG);
    wait(10_ns);

    // enable single interrupt
    write_bus(PLIC_PRIO1_REG, 0x1);
    wait(10_ns);

    write_bus(PLIC_ENABLE_REG, 0x2);
    wait(10_ns);

    // generate interrupt pulse (note: 1 is lowest usable register)
    global_interrupts_o[1].write(true);
    wait(10_ns);
    global_interrupts_o[1].write(false);
    wait(10_ns);

    // read claim_complete register
    reg_check(PLIC_PENDING_REG, 0x2);
    wait(10_ns);
    reg_check(PLIC_CLAIM_COMPLETE_REG, 0x1);
    wait(10_ns);

    //// after writing to claim_complete reg (per fw)
    // -> pending bit expected to be unset
    // -> enable bit expected to be set ... test with / without enable being set
    write_bus(PLIC_CLAIM_COMPLETE_REG, 0x1);
    wait(10_ns);
    reg_check(PLIC_PENDING_REG, 0x0);
    wait(10_ns);
    reg_check(PLIC_CLAIM_COMPLETE_REG, 0x0);
    wait(10_ns);

    // todo: remove wait statements once the tlm_initiator is in place
    // todo: evaluate error messages ... provide correct pass/fail verdict

    wait(100_ns);
}

void test_initiator::test_frequent_irq() {}

void test_initiator::test_parallel_irq() {

    //// create three parallel global_int requests
    // -> read and clear bits one after the other
    // -> different priorities applied (reverse order)
    // -> correct priority handing expected
    // -> three core interrupts expected in total

    // reverse order priority configuration
    write_bus(PLIC_PRIO1_REG, 0x3);
    wait(10_ns);
    write_bus(PLIC_PRIO2_REG, 0x2);
    wait(10_ns);
    write_bus(PLIC_PRIO3_REG, 0x1);
    wait(10_ns);

    // enable all three interrupts
    write_bus(PLIC_ENABLE_REG, 0xE);
    wait(10_ns);

    // generate interrupt pulse (note: 1 is lowest usable register)
    global_interrupts_o[1].write(true);
    wait(10_ns);
    global_interrupts_o[1].write(false);
    wait(10_ns);
    global_interrupts_o[2].write(true);
    wait(10_ns);
    global_interrupts_o[2].write(false);
    wait(10_ns);
    global_interrupts_o[3].write(true);
    wait(10_ns);
    global_interrupts_o[3].write(false);
    wait(10_ns);

    // expect three pending registers
    reg_check(PLIC_PENDING_REG, 0xE);
    wait(10_ns);

    // expect lowest interrupt id to be highest int
    reg_check(PLIC_CLAIM_COMPLETE_REG, 0x1);
    wait(10_ns);

    //// after writing to claim_complete reg (per fw)
    // -> next int to become highest irq
    write_bus(PLIC_CLAIM_COMPLETE_REG, 0x1);
    wait(10_ns);
    reg_check(PLIC_PENDING_REG, 0xC);
    wait(10_ns);
    reg_check(PLIC_CLAIM_COMPLETE_REG, 0x2);
    wait(10_ns);

    //// after writing to claim_complete reg again (per fw)
    // -> next int to become highest irq
    write_bus(PLIC_CLAIM_COMPLETE_REG, 0x2);
    wait(10_ns);
    reg_check(PLIC_PENDING_REG, 0x8);
    wait(10_ns);
    reg_check(PLIC_CLAIM_COMPLETE_REG, 0x3);
    wait(10_ns);

    //// after last writing to claim_complete reg again (per fw)
    // -> no further pending irq expected
    write_bus(PLIC_CLAIM_COMPLETE_REG, 0x3);
    wait(10_ns);
    reg_check(PLIC_PENDING_REG, 0x0);
    wait(10_ns);
    reg_check(PLIC_CLAIM_COMPLETE_REG, 0x0);
    wait(10_ns);

    // todo: advance upon register-write access ... remove above 10_ns waits
    // todo: evaluate error messages ... provide correct pass/fail verdict

    wait(100_ns);
}

void test_initiator::test_irq_stress() {}

void test_initiator::write_bus(std::uint32_t adr, std::uint32_t dat) {
    tlm::tlm_generic_payload gp;
    std::array<uint8_t, 4> data;
    data[3] = 0xff & dat >> 24;
    data[2] = 0xff & dat >> 16;
    data[1] = 0xff & dat >> 8;
    data[0] = 0xff & dat;

    SCCDEBUG("test_initiator") << "write_bus(0x" << std::hex << adr << ") : " << dat;

    gp.set_command(tlm::TLM_WRITE_COMMAND);
    gp.set_address(adr);
    gp.set_data_ptr(data.data());
    gp.set_data_length(data.size());
    gp.set_streaming_width(4);
    sc_time delay;
    intor->b_transport(gp, delay);

    if(gp.get_response_status() != tlm::TLM_OK_RESPONSE) {
        throw std::exception();
    }
}

std::uint32_t test_initiator::read_bus(std::uint32_t adr) {

    tlm::tlm_generic_payload gp;
    std::array<uint8_t, 4> data;

    gp.set_command(tlm::TLM_READ_COMMAND);
    gp.set_address(adr);
    gp.set_data_ptr(data.data());
    gp.set_data_length(data.size());
    gp.set_streaming_width(4);
    sc_time delay;
    intor->b_transport(gp, delay);

    if(gp.get_response_status() != tlm::TLM_OK_RESPONSE) {
        // todo: improve output in case of exception, define own exception class to carry transaction-infos
        // ... i.e. out-of-range report with info about legal mem boundaries
        throw std::exception();
    }

    // todo: use reinterpret_cast instead
    std::uint32_t rdat = data[3] << 24 | data[2] << 16 | data[1] << 8 | data[0];

    SCCDEBUG("test_initiator") << "read_bus(0x" << std::hex << adr << ") -> " << rdat;
    return rdat;
}

void test_initiator::reg_check(std::uint32_t adr, std::uint32_t exp) {
    uint32_t dat = read_bus(adr);
    if(dat != exp) {
        SCCERR("test_initiator") << "register check failed for address 0x" << std::hex << adr << ": " << dat << " !=  " << exp;
    } else {
        SCCDEBUG("test_initiator") << "register check passed for address 0x" << std::hex << adr << ": " << dat;
    }
}

void test_initiator::core_irq_handler() { SCCDEBUG("test_initiator") << "core_interrupt_i edge detected -> " << core_interrupt_i.read(); }

} /* namespace sysc */
