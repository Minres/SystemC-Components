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

#ifndef _SYSC_TLM_TARGET_H_
#define _SYSC_TLM_TARGET_H_

#include "resource_access_if.h"
#include "tlm/scc/scv4tlm/tlm_rec_target_socket.h"
#include "tlm/scc/target_mixin.h"
#include "util/range_lut.h"
#include "scc/utilities.h"
#include <array>

namespace scc {
/**
 * @struct addr_range
 * @brief struct representing address range
 *
 */
struct addr_range {
    uint64_t base, size;
};
/**
 * @class tlm_target
 * @brief a simple access-width based bus interface (no DMI support)
 *
 * @tparam BUSWIDTH
 */
template <unsigned int BUSWIDTH = 32> class tlm_target {
public:
    using this_type = tlm_target<BUSWIDTH>;
    /**
     * @fn  tlm_target(sc_core::sc_time&)
     * @brief the constructor
     *
     * @param clock the clock period of the component
     */
    tlm_target(sc_core::sc_time& clock);
    //! the target socket
    tlm::scc::target_mixin<scv4tlm::tlm_rec_target_socket<BUSWIDTH>> socket;
    /**
     * @fn void b_tranport_cb(tlm::tlm_generic_payload&, sc_core::sc_time&)
     * @brief the blocking transport callback
     *
     * @param gp the generic payload
     * @param d the delay in the local time domain
     */
    void b_tranport_cb(tlm::tlm_generic_payload& gp, sc_core::sc_time& d);
    /**
     * @fn unsigned int tranport_dbg_cb(tlm::tlm_generic_payload&)
     * @brief the debug transport callback
     *
     * @param gp the generic payload
     * @return number of transferred bytes
     */
    unsigned int tranport_dbg_cb(tlm::tlm_generic_payload& gp);
    /**
     * @fn void addResource(resource_access_if&, uint64_t)
     * @brief add a resource to this target at a certain address within the socket address range
     *
     * @param rai the resource to add
     * @param base_addr the offset of the resource (from address 0)
     */
    void addResource(resource_access_if& rai, uint64_t base_addr) {
        socket_map.addEntry(std::make_pair(&rai, base_addr), base_addr, rai.size());
    }
    /**
     * @fn void addResource(indexed_resource_access_if&, uint64_t)
     * @brief add an indexed resource to this target at a certain address within the socket address range
     * @param irai the resource to add
     * @param base_addr the offset of the resource (from address 0)
     */
    void addResource(indexed_resource_access_if& irai, uint64_t base_addr) {
        for(size_t idx = 0; idx < irai.size(); ++idx) {
            socket_map.addEntry(std::make_pair(&irai[idx], base_addr), base_addr, irai[idx].size());
            base_addr += irai[idx].size();
        }
    }

private:
    sc_core::sc_time& clk;

protected:
    util::range_lut<std::pair<resource_access_if*, uint64_t>> socket_map;
};
/**
 * helper structure to define a address range for a socket
 */
template <unsigned BUSWIDTH = 32> struct target_memory_map_entry {
    tlm::tlm_target_socket<BUSWIDTH>& target;
    sc_dt::uint64 start;
    sc_dt::uint64 size;
};
/**
 * helper structure to define a named address range
 */
template <unsigned BUSWIDTH = 32> struct target_name_map_entry {
    std::string name;
    sc_dt::uint64 start;
    sc_dt::uint64 size;
};

} /* namespace scc */

template <unsigned int BUSWIDTH>
inline scc::tlm_target<BUSWIDTH>::tlm_target(sc_core::sc_time& clock)
: socket("socket")
, clk(clock)
, socket_map(std::make_pair(nullptr, 0)) {
    socket.register_b_transport(
            [=](tlm::tlm_generic_payload& gp, sc_core::sc_time& delay) -> void { this->b_tranport_cb(gp, delay); });
    socket.register_transport_dbg([=](tlm::tlm_generic_payload& gp) -> unsigned { return this->tranport_dbg_cb(gp); });
}

template <unsigned int BUSWIDTH>
void scc::tlm_target<BUSWIDTH>::b_tranport_cb(tlm::tlm_generic_payload& gp, sc_core::sc_time& delay) {
    resource_access_if* ra = nullptr;
    uint64_t base = 0;
    std::tie(ra, base) = socket_map.getEntry(gp.get_address());
    if(ra) {
        gp.set_response_status(tlm::TLM_BURST_ERROR_RESPONSE);
        if(gp.get_data_length() <= ra->size()) {
            gp.set_response_status(tlm::TLM_BYTE_ENABLE_ERROR_RESPONSE);
            if(gp.get_byte_enable_ptr() == nullptr) {
                gp.set_response_status(tlm::TLM_GENERIC_ERROR_RESPONSE);
                if(gp.get_data_length() == gp.get_streaming_width()) {
                    if(gp.get_command() == tlm::TLM_READ_COMMAND) {
                        if(ra->read(gp.get_data_ptr(), gp.get_data_length(), (gp.get_address() - base), delay))
                            gp.set_response_status(tlm::TLM_OK_RESPONSE);
                    } else if(gp.get_command() == tlm::TLM_WRITE_COMMAND) {
                        if(ra->write(gp.get_data_ptr(), gp.get_data_length(), (gp.get_address() - base), delay))
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

template <unsigned int BUSWIDTH> unsigned int scc::tlm_target<BUSWIDTH>::tranport_dbg_cb(tlm::tlm_generic_payload& gp) {
    resource_access_if* ra = nullptr;
    uint64_t base = 0;
    std::tie(ra, base) = socket_map.getEntry(gp.get_address());
    if(ra) {
        if(gp.get_data_length() == ra->size() && gp.get_byte_enable_ptr() == nullptr &&
                gp.get_data_length() == gp.get_streaming_width()) {
            if(gp.get_command() == tlm::TLM_READ_COMMAND) {
                if(ra->read_dbg(gp.get_data_ptr(), gp.get_data_length(), (gp.get_address() - base) / ra->size()))
                    return gp.get_data_length();
            } else if(gp.get_command() == tlm::TLM_READ_COMMAND) {
                if(ra->write_dbg(gp.get_data_ptr(), gp.get_data_length(), (gp.get_address() - base) / ra->size()))
                    return gp.get_data_length();
            }
        }
    }
    return 0;
}

#endif /* _SYSC_TLM_TARGET_H_ */
