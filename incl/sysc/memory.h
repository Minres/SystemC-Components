/*******************************************************************************
 * Copyright 2016 MINRES Technologies GmbH
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
/*
 * memory.h
 *
 *  Created on: Nov 4, 2016
 *      Author: eyck
 */

#ifndef _SYSC_MEMORY_H_
#define _SYSC_MEMORY_H_

// Needed for the simple_target_socket
#define SC_INCLUDE_DYNAMIC_PROCESSES

#include "utilities.h"
#include <sysc/report.h>
#include <tlm.h>
#include <tlm_utils/simple_target_socket.h>

namespace sysc {

// simple memory model

template <unsigned SIZE, unsigned BUSWIDTH = 32, bool LOG_ACCESS = false> struct memory : sc_core::sc_module {

    tlm_utils::simple_target_socket<memory, BUSWIDTH> socket;

    memory(const sc_core::sc_module_name &nm);

    void b_transport(tlm::tlm_generic_payload &trans, sc_core::sc_time &delay);

    unsigned transport_dbg(tlm::tlm_generic_payload &trans);

protected:
    uint8_t mem[SIZE];

private:
    int handle_operation(tlm::tlm_generic_payload &trans);
};

template <unsigned SIZE, unsigned BUSWIDTH, bool LOG_ACCESS>
memory<SIZE, BUSWIDTH, LOG_ACCESS>::memory(const sc_core::sc_module_name &nm)
: sc_module(nm)
, NAMED(socket) {
    // Register callback for incoming b_transport interface method call
    socket.register_b_transport(this, &memory::b_transport);
    socket.register_transport_dbg(this, &memory::transport_dbg);
    // Initialize memory with random data
    for (size_t i = 0; i < SIZE; i++) mem[i] = rand() % 256;
}

template <unsigned SIZE, unsigned BUSWIDTH, bool LOG_ACCESS>
void memory<SIZE, BUSWIDTH, LOG_ACCESS>::b_transport(tlm::tlm_generic_payload &trans, sc_core::sc_time &delay) {
    handle_operation(trans);
}

template <unsigned SIZE, unsigned BUSWIDTH, bool LOG_ACCESS>
unsigned memory<SIZE, BUSWIDTH, LOG_ACCESS>::transport_dbg(tlm::tlm_generic_payload &trans) {
    return handle_operation(trans);
}

template <unsigned SIZE, unsigned BUSWIDTH, bool LOG_ACCESS>
int memory<SIZE, BUSWIDTH, LOG_ACCESS>::handle_operation(tlm::tlm_generic_payload &trans) {
    sc_dt::uint64 adr = trans.get_address();
    unsigned char *ptr = trans.get_data_ptr();
    unsigned len = trans.get_data_length();
    unsigned char *byt = trans.get_byte_enable_ptr();
    unsigned wid = trans.get_streaming_width();
    // check address range and check for unsupported features,
    //   i.e. byte enables, streaming, and bursts
    // Can ignore DMI hint and extensions
    if (adr + len > sc_dt::uint64(SIZE)) {
        SC_REPORT_ERROR("TLM-2", "generic payload transaction exceeeds memory size");
        trans.set_response_status(tlm::TLM_ADDRESS_ERROR_RESPONSE);
        return 0;
    }
    if (adr + len > sc_dt::uint64(SIZE) || byt != 0 || wid < len) {
        SC_REPORT_ERROR("TLM-2", "generic payload transaction not supported");
        trans.set_response_status(tlm::TLM_GENERIC_ERROR_RESPONSE);
        return 0;
    }

    tlm::tlm_command cmd = trans.get_command();
    if (LOG_ACCESS) {
        if (adr >= 0x20 && adr < 0x60)
            LOG(WARNING) << (cmd == tlm::TLM_READ_COMMAND ? "read" : "write") << " access to addr 0x" << std::hex
                         << adr - 0x20 << "(0x" << (adr) << ")" << std::dec;
        else
            LOG(WARNING) << (cmd == tlm::TLM_READ_COMMAND ? "read" : "write") << " access to addr 0x" << std::hex << adr
                         << std::dec;
    }
    if (cmd == tlm::TLM_READ_COMMAND)
        memcpy(ptr, mem + adr, len);
    else if (cmd == tlm::TLM_WRITE_COMMAND)
        memcpy(mem + adr, ptr, len);
    trans.set_response_status(tlm::TLM_OK_RESPONSE);
    return len;
}

} // namespace sysc

#endif /* _SYSC_MEMORY_H_ */
