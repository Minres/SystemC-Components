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
#include <scc/utilities.h>
#include <tlm/scc/target_mixin.h>
#include <tlm/scc/scv/tlm_rec_target_socket.h>
#include <util/range_lut.h>
#include <array>
#include <numeric>

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
 * @tparam ADDR_UNIT_WIDTH
 */
template <unsigned int BUSWIDTH = LT, unsigned int ADDR_UNIT_WIDTH = 8> class tlm_target {
public:
    using this_type = tlm_target<BUSWIDTH, ADDR_UNIT_WIDTH>;
    /**
     * @fn  tlm_target(sc_core::sc_time&)
     * @brief the constructor
     *
     * @param clock the clock period of the component
     */
    tlm_target(sc_core::sc_time& clock);
    //! the target socket
    tlm::scc::target_mixin<tlm::scc::scv::tlm_rec_target_socket<BUSWIDTH>> socket;
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
        socket_map.addEntry(std::make_pair(&rai, base_addr), base_addr,
                            std::max<size_t>(1, rai.size() / (ADDR_UNIT_WIDTH / 8)));
    }
    /**
     * @fn void addResource(indexed_resource_access_if&, uint64_t)
     * @brief add an indexed resource to this target at a certain address within the socket address range
     * @param irai the resource to add
     * @param base_addr the offset of the resource (from address 0)
     */
    void addResource(indexed_resource_access_if& irai, uint64_t base_addr) {
        for(size_t idx = 0; idx < irai.size(); ++idx) {
            auto irai_size = std::max<size_t>(1, irai[idx].size() / (ADDR_UNIT_WIDTH / 8));
            socket_map.addEntry(std::make_pair(&irai[idx], base_addr), base_addr, irai_size);
            base_addr += irai_size;
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
template <unsigned BUSWIDTH = LT> struct target_memory_map_entry {
    tlm::tlm_target_socket<BUSWIDTH>& target;
    ::sc_dt::uint64 start;
    ::sc_dt::uint64 size;
};
/**
 * helper structure to define a named address range
 */
template <unsigned BUSWIDTH = LT> struct target_name_map_entry {
    std::string name;
    ::sc_dt::uint64 start;
    ::sc_dt::uint64 size;
};
template <unsigned int BUSWIDTH = LT, unsigned int ADDR_UNIT_WIDTH = 8> struct tlm_target_mod : sc_core::sc_module, public tlm_target<BUSWIDTH, ADDR_UNIT_WIDTH> {
    tlm_target_mod(sc_core::sc_module_name nm, sc_core::sc_time& clk_period) : sc_module(nm), tlm_target<BUSWIDTH, ADDR_UNIT_WIDTH>(clk_period){

    }
};

} /* namespace scc */

template <unsigned int BUSWIDTH, unsigned int ADDR_UNIT_WIDTH>
inline scc::tlm_target<BUSWIDTH, ADDR_UNIT_WIDTH>::tlm_target(sc_core::sc_time& clock)
: socket("socket")
, clk(clock)
, socket_map(std::make_pair(nullptr, 0)) {
    socket.register_b_transport(
        [=](tlm::tlm_generic_payload& gp, sc_core::sc_time& delay) -> void { this->b_tranport_cb(gp, delay); });
    socket.register_transport_dbg([=](tlm::tlm_generic_payload& gp) -> unsigned { return this->tranport_dbg_cb(gp); });
}

template <unsigned int BUSWIDTH, unsigned int ADDR_UNIT_WIDTH>
void scc::tlm_target<BUSWIDTH, ADDR_UNIT_WIDTH>::b_tranport_cb(tlm::tlm_generic_payload& gp, sc_core::sc_time& delay) {
    resource_access_if* ra = nullptr;
    uint64_t base = 0;
    std::tie(ra, base) = socket_map.getEntry(gp.get_address());
    if(ra) {
        auto offset = 0;
        auto len = gp.get_data_length();
        auto contigous = true;
        if(gp.get_byte_enable_ptr()) {
            auto lower = std::numeric_limits<unsigned>::max();
            auto upper = std::numeric_limits<unsigned>::max();
            auto en = false;
            auto p = gp.get_byte_enable_ptr();
            auto i = 0u;
            for(; i < gp.get_byte_enable_length(); ++i, ++p){
                if(*p && !en) {
                    if(lower != std::numeric_limits<unsigned>::max()) {
                        contigous = false;
                        break;
                    } else {
                        lower = i;
                        en = true;
                    }
                }
                if(!*p && en) {
                    if(upper != std::numeric_limits<unsigned>::max()) {
                        contigous = false;
                        break;
                    } else {
                        upper = i;
                        en = false;
                    }
                }
            }
            if(i== gp.get_byte_enable_length() && upper == std::numeric_limits<unsigned>::max())
                upper = i;
            if(contigous) {
                offset = lower;
                len = upper-lower;
            }
        }
        if(gp.get_data_length() > ra->size()) {
            gp.set_response_status(tlm::TLM_BURST_ERROR_RESPONSE);
        } else if(gp.get_data_length() != gp.get_streaming_width()){
            gp.set_response_status(tlm::TLM_GENERIC_ERROR_RESPONSE);
        } else if(gp.get_byte_enable_ptr() != nullptr && !(contigous && gp.get_byte_enable_length()==gp.get_data_length())) {
            gp.set_response_status(tlm::TLM_BYTE_ENABLE_ERROR_RESPONSE);
        } else {
            gp.set_response_status(tlm::TLM_COMMAND_ERROR_RESPONSE);
            switch(gp.get_command()) {
            case tlm::TLM_READ_COMMAND:
                if(ra->read(gp.get_data_ptr()+offset, len, (gp.get_address() - base + offset), delay))
                    gp.set_response_status(tlm::TLM_OK_RESPONSE);
                break;
            case tlm::TLM_WRITE_COMMAND:
                if(ra->write(gp.get_data_ptr()+offset, len, (gp.get_address() - base + offset), delay))
                    gp.set_response_status(tlm::TLM_OK_RESPONSE);
                break;
            }
        }
    } else {
        gp.set_response_status(tlm::TLM_ADDRESS_ERROR_RESPONSE);
    }
    delay += clk;
}

template <unsigned int BUSWIDTH, unsigned int ADDR_UNIT_WIDTH>
unsigned int scc::tlm_target<BUSWIDTH, ADDR_UNIT_WIDTH>::tranport_dbg_cb(tlm::tlm_generic_payload& gp) {
    resource_access_if* ra = nullptr;
    uint64_t base = 0;
    std::tie(ra, base) = socket_map.getEntry(gp.get_address());
    if(ra) {
        if(gp.get_data_length() == ra->size() && gp.get_byte_enable_ptr() == nullptr &&
           gp.get_data_length() == gp.get_streaming_width()) {
            if(gp.get_command() == tlm::TLM_READ_COMMAND) {
                if(ra->read_dbg(gp.get_data_ptr(), gp.get_data_length(), (gp.get_address() - base) / ra->size()))
                    return gp.get_data_length();
            } else if(gp.get_command() == tlm::TLM_WRITE_COMMAND) {
                if(ra->write_dbg(gp.get_data_ptr(), gp.get_data_length(), (gp.get_address() - base) / ra->size()))
                    return gp.get_data_length();
            }
        }
    }
    return 0;
}


#endif /* _SYSC_TLM_TARGET_H_ */
