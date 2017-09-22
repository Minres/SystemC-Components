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
 * tlmtarget.h
 *
 *  Created on: Nov 16, 2016
 *      Author: developer
 */

#ifndef _SYSC_TLM_TARGET_H_
#define _SYSC_TLM_TARGET_H_

#include "resource_access_if.h"
#include <array>
#include <scv4tlm/tlm_rec_target_socket.h>
#include <sysc/target_mixin.h>
#include <util/range_lut.h>

namespace sysc {
/**
 * a simple access-width based bus interface (no DMI support)
 */
struct addr_range {
    uint64_t base, size;
};

template <unsigned int BUSWIDTH = 32> struct tlm_target {
    using this_type = tlm_target<BUSWIDTH>;
    /**
     *
     * @param clock
     */
    tlm_target(sc_core::sc_time &clock);
    /**
     *
     */
    sysc::target_mixin<scv4tlm::tlm_rec_target_socket<BUSWIDTH>> socket;
    /**
     *
     * @param
     * @param
     */
    void b_tranport_cb(tlm::tlm_generic_payload &, sc_core::sc_time &);
    /**
     *
     * @param
     * @return
     */
    unsigned int tranport_dbg_cb(tlm::tlm_generic_payload &);
    /**
     *
     * @param i
     * @param base_addr
     */
    void addResource(resource_access_if &i, uint64_t base_addr) {
        socket_map.addEntry(std::make_pair(&i, base_addr), base_addr, i.size());
    }
    /**
     *
     * @param ii
     * @param base_addr
     */
    void addResource(indexed_resource_access_if &ii, uint64_t base_addr) {
        for (size_t idx = 0; idx < ii.size(); ++idx) {
            socket_map.addEntry(std::make_pair(&ii[idx], base_addr), base_addr, ii[idx].size());
            base_addr += ii[idx].size();
        }
    }

protected:
    sc_core::sc_time &clk;
    util::range_lut<std::pair<resource_access_if *, uint64_t>> socket_map;
};

template <unsigned BUSWIDTH = 32> struct target_memory_map_entry {
    sysc::tlm_target<BUSWIDTH> *target;
    sc_dt::uint64 start;
    sc_dt::uint64 size;
};

template <unsigned int BUSWIDTH = 32, unsigned RANGES = 1> struct tlm_multi_rangetarget : public tlm_target<BUSWIDTH> {
    typedef tlm_multi_rangetarget<BUSWIDTH, RANGES> this_type;

    tlm_multi_rangetarget(sc_core::sc_time &clock, std::array<addr_range, RANGES> addr_rngs)
        : tlm_target<BUSWIDTH>(clock), addr_ranges(addr_rngs) {}

    tlm_multi_rangetarget(sc_core::sc_time &clock) : tlm_target<BUSWIDTH>(clock), addr_ranges({}) {}

    const std::array<addr_range, RANGES> addr_ranges;

protected:
    util::range_lut<resource_access_if *> socket_map;
};

} /* namespace sysc */

template <unsigned int BUSWIDTH>
inline sysc::tlm_target<BUSWIDTH>::tlm_target(sc_core::sc_time &clock)
    : socket("socket"), clk(clock), socket_map(std::make_pair(nullptr, 0)) {
    socket.register_b_transport(
        [=](tlm::tlm_generic_payload &gp, sc_core::sc_time &delay) -> void { this->b_tranport_cb(gp, delay); });
    socket.register_transport_dbg([=](tlm::tlm_generic_payload &gp) -> unsigned { return this->tranport_dbg_cb(gp); });
}

template <unsigned int BUSWIDTH>
void sysc::tlm_target<BUSWIDTH>::b_tranport_cb(tlm::tlm_generic_payload &gp, sc_core::sc_time &delay) {
    resource_access_if *ra = nullptr;
    uint64_t base = 0;
    std::tie(ra, base) = socket_map.getEntry(gp.get_address());
    if (ra) {
        gp.set_response_status(tlm::TLM_BURST_ERROR_RESPONSE);
        if (gp.get_data_length() == ra->size()) {
            gp.set_response_status(tlm::TLM_BYTE_ENABLE_ERROR_RESPONSE);
            if (gp.get_byte_enable_ptr() == 0) {
                gp.set_response_status(tlm::TLM_GENERIC_ERROR_RESPONSE);
                if (gp.get_data_length() == gp.get_streaming_width()) {
                    if (gp.get_command() == tlm::TLM_READ_COMMAND) {
                        if (ra->read(gp.get_data_ptr(), gp.get_data_length(), (gp.get_address() - base) / ra->size()))
                            gp.set_response_status(tlm::TLM_OK_RESPONSE);
                    } else if (gp.get_command() == tlm::TLM_WRITE_COMMAND) {
                        if (ra->write(gp.get_data_ptr(), gp.get_data_length(), (gp.get_address() - base) / ra->size()))
                            gp.set_response_status(tlm::TLM_OK_RESPONSE);
                    } else {
                        gp.set_response_status(tlm::TLM_COMMAND_ERROR_RESPONSE);
                    }
                }
            }
        }
    } else {
        gp.set_response_status(tlm::TLM_ADDRESS_ERROR_RESPONSE);
    }
    delay += clk;
}

template <unsigned int BUSWIDTH>
unsigned int sysc::tlm_target<BUSWIDTH>::tranport_dbg_cb(tlm::tlm_generic_payload &gp) {
    resource_access_if *ra = nullptr;
    uint64_t base = 0;
    std::tie(ra, base) = socket_map.getEntry(gp.get_address());
    if (ra) {
        if (gp.get_data_length() == ra->size() && gp.get_byte_enable_ptr() == 0 &&
            gp.get_data_length() == gp.get_streaming_width()) {
            if (gp.get_command() == tlm::TLM_READ_COMMAND) {
                if (ra->read_dbg(gp.get_data_ptr(), gp.get_data_length(), (gp.get_address() - base) / ra->size()))
                    return gp.get_data_length();
            } else if (gp.get_command() == tlm::TLM_READ_COMMAND) {
                if (ra->write_dbg(gp.get_data_ptr(), gp.get_data_length(), (gp.get_address() - base) / ra->size()))
                    return gp.get_data_length();
            }
        }
    }
    return 0;
}

#endif /* _SYSC_TLM_TARGET_H_ */
