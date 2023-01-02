/*******************************************************************************
 * Copyright 2023 MINRES Technologies GmbH
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *******************************************************************************/
#include "test_initiator.h"
#include <array>
#include <scc/report.h>
#include <scc/utilities.h>

namespace sysc {
using namespace sc_core;

test_initiator::test_initiator(sc_module_name nm)
: sc_module(nm)
{
    SC_THREAD(run);
}

void test_initiator::run() {
    // wait for reset
    if (rst_i.read() == false) wait(rst_i.posedge_event());
    wait(rst_i.negedge_event());
    wait(10_ns);
    write_bus(0, 0xCAFFEE);     // memory
    write_bus(1_kB, 0x00FF1CE); // memory
    write_bus(64_kB, 0xDEADBEEF);
    write_bus(64_kB+4, 0xCAFED00D);
    write_bus(64_kB+0x100, 0xBAADF00D);
    wait(20_ns);
    reg_check(0, 0xCAFFEE);     // in memory
    reg_check(1_kB, 0x00FF1CE); // in memory
    reg_check(64_kB, 0xCAFED00D);
    reg_check(64_kB+4, 0xCAFED00D);
    reg_check(64_kB+0x100, 0xBAADF00D);
    wait(20_ns);
    sc_stop();
}

void test_initiator::write_bus(std::uint32_t adr, std::uint32_t dat) {
    tlm::tlm_generic_payload gp;
    std::array<uint8_t, 4> data;
    data[3] = 0xff & dat >> 24;
    data[2] = 0xff & dat >> 16;
    data[1] = 0xff & dat >> 8;
    data[0] = 0xff & dat;
    SCCDEBUG("test_initiator") << "write_bus(0x" << std::hex << adr << ") : 0x" << dat;
    gp.set_command(tlm::TLM_WRITE_COMMAND);
    gp.set_address(adr);
    gp.set_data_ptr(data.data());
    gp.set_data_length(data.size());
    gp.set_streaming_width(4);
    sc_time delay;
    intor->b_transport(gp, delay);
    if (gp.get_response_status() != tlm::TLM_OK_RESPONSE) {
        SCCERR(SCMOD)<< "write to 0x"<<std::hex<<adr<<" failed with "<<gp.get_response_status();
    }
    if(delay.value()) wait(delay);
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
    if (gp.get_response_status() != tlm::TLM_OK_RESPONSE) {
        SCCERR(SCMOD)<< "read from 0x"<<std::hex<<adr<<" failed with "<<gp.get_response_status();
    }
    if(delay.value()) wait(delay);
    std::uint32_t rdat = data[3] << 24 | data[2] << 16 | data[1] << 8 | data[0];
    SCCDEBUG("test_initiator") << "read_bus(0x" << std::hex << adr << ") -> 0x" << rdat;
    return rdat;
}

void test_initiator::reg_check(std::uint32_t adr, std::uint32_t exp) {
    uint32_t dat = read_bus(adr);
    if (dat != exp) {
        SCCERR("test_initiator") << "register check failed for address 0x" << std::hex << adr << ": 0x" << dat << " !=  0x" << exp;
    } else {
        SCCDEBUG("test_initiator") << "register check passed for address 0x" << std::hex << adr << ": 0x" << dat;
    }
}
} /* namespace sysc */
