/*******************************************************************************
 * Copyright 2016, 2018 MINRES Technologies GmbH
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

#include "report.h"
#include "target_mixin.h"
#include "utilities.h"
#include <tlm.h>
#include <util/sparse_array.h>

namespace scc {

// simple memory model
// TODO: add attributes/parameters to configure access time and type (DMI allowed, read only, etc)
template <unsigned long long SIZE, unsigned BUSWIDTH = 32, bool LOG_ACCESS = false>
class memory : public sc_core::sc_module {
public:
    scc::target_mixin<tlm::tlm_target_socket<BUSWIDTH>> target;

    memory(const sc_core::sc_module_name &nm);

protected:
    util::sparse_array<uint8_t, SIZE> mem;

private:
    int handle_operation(tlm::tlm_generic_payload &trans);
    bool handle_dmi(tlm::tlm_generic_payload &gp, tlm::tlm_dmi &dmi_data);
};

template <unsigned long long SIZE, unsigned BUSWIDTH, bool LOG_ACCESS>
memory<SIZE, BUSWIDTH, LOG_ACCESS>::memory(const sc_core::sc_module_name &nm)
: sc_module(nm)
, NAMED(target) {
    // Register callback for incoming b_transport interface method call
    target.register_b_transport([=](tlm::tlm_generic_payload &gp, sc_core::sc_time &delay) -> void {
        auto count = this->handle_operation(gp);
    });
    target.register_transport_dbg(
        [this](tlm::tlm_generic_payload &gp) -> unsigned { return this->handle_operation(gp); });
    target.register_get_direct_mem_ptr([this](tlm::tlm_generic_payload &gp, tlm::tlm_dmi &dmi_data) -> bool {
        return this->handle_dmi(gp, dmi_data);
    });
}

template <unsigned long long SIZE, unsigned BUSWIDTH, bool LOG_ACCESS>
int memory<SIZE, BUSWIDTH, LOG_ACCESS>::handle_operation(tlm::tlm_generic_payload &trans) {
    sc_dt::uint64 adr = trans.get_address();
    uint8_t *ptr = trans.get_data_ptr();
    unsigned len = trans.get_data_length();
    uint8_t *byt = trans.get_byte_enable_ptr();
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
    if (cmd == tlm::TLM_READ_COMMAND) {
        if (mem.is_allocated(adr)) {
            const auto &p = mem(adr / mem.page_size);
            auto offs = adr & mem.page_addr_mask;
            std::copy(p.data() + offs, p.data() + offs + len, ptr);
        } else {
            // no allocated page so return randomized data
            for (size_t i = 0; i < len; i++) ptr[i] = rand() % 256;
        }
    } else if (cmd == tlm::TLM_WRITE_COMMAND) {
        auto &p = mem(adr / mem.page_size);
        auto offs = adr & mem.page_addr_mask;
        std::copy(ptr, ptr + len, p.data() + offs);
    }
    trans.set_response_status(tlm::TLM_OK_RESPONSE);
    trans.set_dmi_allowed(true);
    return len;
}

template <unsigned long long SIZE, unsigned BUSWIDTH, bool LOG_ACCESS>
inline bool memory<SIZE, BUSWIDTH, LOG_ACCESS>::handle_dmi(tlm::tlm_generic_payload &gp, tlm::tlm_dmi &dmi_data) {
    auto &p = mem(gp.get_address() / mem.page_size);
    dmi_data.set_start_address(gp.get_address() & ~mem.page_addr_mask);
    // TODO: fix to provide the correct end address
    dmi_data.set_end_address(dmi_data.get_start_address() + mem.page_size - 1);
    dmi_data.set_dmi_ptr(p.data());
    dmi_data.set_granted_access(tlm::tlm_dmi::DMI_ACCESS_READ_WRITE);
    return true;
}

} // namespace scc

#endif /* _SYSC_MEMORY_H_ */
