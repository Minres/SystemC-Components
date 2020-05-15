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

#ifndef _SYSC_MEMORY_H_
#define _SYSC_MEMORY_H_

// Needed for the simple_target_socket
#define SC_INCLUDE_DYNAMIC_PROCESSES

#include "report.h"
#include "target_mixin.h"
#include "utilities.h"
#include <tlm.h>
#include <scc/mt19937_rng.h>
#include <util/sparse_array.h>

namespace scc {

/**
 *  simple memory model
 *
 *  TODO: add some more attributes/parameters to configure access time and type (DMI allowed, read only, etc)
 */
template <unsigned long long SIZE, unsigned BUSWIDTH = 32> class memory : public sc_core::sc_module {
public:
    //! the target socket to connect to TLM
    scc::target_mixin<tlm::tlm_target_socket<BUSWIDTH>> target;
    /**
     * constructor with explicit instance name
     *
     * @param nm
     */
    memory(const sc_core::sc_module_name& nm);

    constexpr unsigned long long getSize() const { return SIZE; }

    void set_operation_callback(std::function<int(memory<SIZE, BUSWIDTH>&, tlm::tlm_generic_payload&)> cb) {
        operation_cb = cb;
    }

    void set_dmi_callback(std::function<int(memory<SIZE, BUSWIDTH>&, tlm::tlm_generic_payload&, tlm::tlm_dmi&)> cb) {
        dmi_cb = cb;
    }

protected:
    //! the real memory structure
    util::sparse_array<uint8_t, SIZE> mem;

public:
    //!! handle the memory operation independent on interface function used
    int handle_operation(tlm::tlm_generic_payload& trans);
    //! handle the dmi functionality
    bool handle_dmi(tlm::tlm_generic_payload& gp, tlm::tlm_dmi& dmi_data);
    std::function<int(memory<SIZE, BUSWIDTH>&, tlm::tlm_generic_payload&)> operation_cb;
    std::function<int(memory<SIZE, BUSWIDTH>&, tlm::tlm_generic_payload&, tlm::tlm_dmi&)> dmi_cb;
};

template <unsigned long long SIZE, unsigned BUSWIDTH>
memory<SIZE, BUSWIDTH>::memory(const sc_core::sc_module_name& nm)
: sc_module(nm)
, NAMED(target) {
    // Register callback for incoming b_transport interface method call
    target.register_b_transport([this](tlm::tlm_generic_payload& gp, sc_core::sc_time& delay) -> void {
        operation_cb ? operation_cb(*this, gp) : handle_operation(gp);
    });
    target.register_transport_dbg([this](tlm::tlm_generic_payload& gp) -> unsigned {
        return operation_cb ? operation_cb(*this, gp) : handle_operation(gp);
    });
    target.register_get_direct_mem_ptr([this](tlm::tlm_generic_payload& gp, tlm::tlm_dmi& dmi_data) -> bool {
        return dmi_cb ? dmi_cb(*this, gp, dmi_data) : handle_dmi(gp, dmi_data);
    });
}

template <unsigned long long SIZE, unsigned BUSWIDTH>
int memory<SIZE, BUSWIDTH>::handle_operation(tlm::tlm_generic_payload& trans) {
    sc_dt::uint64 adr = trans.get_address();
    uint8_t* ptr = trans.get_data_ptr();
    unsigned len = trans.get_data_length();
    uint8_t* byt = trans.get_byte_enable_ptr();
    unsigned wid = trans.get_streaming_width();
    // check address range and check for unsupported features,
    //   i.e. byte enables, streaming, and bursts
    // Can ignore DMI hint and extensions
    if(adr + len > sc_dt::uint64(SIZE)) {
        SC_REPORT_ERROR("TLM-2", "generic payload transaction exceeeds memory size");
        trans.set_response_status(tlm::TLM_ADDRESS_ERROR_RESPONSE);
        return 0;
    }
    if(adr + len > sc_dt::uint64(SIZE) || byt != 0 || wid < len) {
        SC_REPORT_ERROR("TLM-2", "generic payload transaction not supported");
        trans.set_response_status(tlm::TLM_GENERIC_ERROR_RESPONSE);
        return 0;
    }

    tlm::tlm_command cmd = trans.get_command();
    SCCTRACE(SCMOD) << (cmd == tlm::TLM_READ_COMMAND ? "read" : "write") << " access to addr 0x" << std::hex << adr;
    if(cmd == tlm::TLM_READ_COMMAND) {
        if(mem.is_allocated(adr)) {
            const auto& p = mem(adr / mem.page_size);
            auto offs = adr & mem.page_addr_mask;
            std::copy(p.data() + offs, p.data() + offs + len, ptr);
        } else {
            // no allocated page so return randomized data
            for(size_t i = 0; i < len; i++)
                ptr[i] = scc::MT19937::uniform() % 256;
        }
    } else if(cmd == tlm::TLM_WRITE_COMMAND) {
        auto& p = mem(adr / mem.page_size);
        auto offs = adr & mem.page_addr_mask;
        std::copy(ptr, ptr + len, p.data() + offs);
    }
    trans.set_response_status(tlm::TLM_OK_RESPONSE);
    trans.set_dmi_allowed(true);
    return len;
}

template <unsigned long long SIZE, unsigned BUSWIDTH>
inline bool memory<SIZE, BUSWIDTH>::handle_dmi(tlm::tlm_generic_payload& gp, tlm::tlm_dmi& dmi_data) {
    auto& p = mem(gp.get_address() / mem.page_size);
    dmi_data.set_start_address(gp.get_address() & ~mem.page_addr_mask);
    // TODO: fix to provide the correct end address
    dmi_data.set_end_address(dmi_data.get_start_address() + mem.page_size - 1);
    dmi_data.set_dmi_ptr(p.data());
    dmi_data.set_granted_access(tlm::tlm_dmi::DMI_ACCESS_READ_WRITE);
    return true;
}

} // namespace scc

#endif /* _SYSC_MEMORY_H_ */
