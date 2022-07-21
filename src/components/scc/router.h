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

#ifndef _SYSC_ROUTER_H_
#define _SYSC_ROUTER_H_

#include "scc/utilities.h"
#include "tlm/scc/initiator_mixin.h"
#include "tlm/scc/target_mixin.h"
#include "util/range_lut.h"
#include <limits>
#include <sysc/utils/sc_vector.h>
#include <tlm.h>
#include <tlm/scc/scv/tlm_rec_initiator_socket.h>
#include <tlm/scc/scv/tlm_rec_target_socket.h>
#include <unordered_map>

namespace scc {
/**
 * @class router
 * @brief a TLM2.0 router for loosly-timed (LT) models
 *
 * It uses the tlm::scc::scv::tlm_rec_initiator_socket so that incoming and outgoing accesses can be traced using SCV
 *
 * @tparam BUSWIDTH the width of the bus
 */
template <unsigned BUSWIDTH = 32> class router : sc_core::sc_module {
public:
    using intor_sckt = tlm::scc::initiator_mixin<tlm::tlm_initiator_socket<BUSWIDTH>>;
    using target_sckt = tlm::scc::target_mixin<tlm::scc::scv::tlm_rec_target_socket<BUSWIDTH>>;
    //! \brief the array of target sockets
    sc_core::sc_vector<target_sckt> target;
    //! \brief  the array of initiator sockets
    sc_core::sc_vector<intor_sckt> initiator;
    /**
     * @fn  router(const sc_core::sc_module_name&, unsigned=1, unsigned=1)
     * @brief constructs a router
     *
     * @param nm the component name
     * @param slave_cnt number of slaves to be connected
     * @param master_cnt number of masters to be connected
     */
    router(const sc_core::sc_module_name& nm, unsigned slave_cnt = 1, unsigned master_cnt = 1);

    ~router() = default;
    /**
     * @fn void bind_target(TYPE&, size_t, uint64_t, uint64_t, bool=true)
     * @brief bind the initiator socket of the router to some target giving a base and size
     *
     * @tparam TYPE the socket type to bind
     * @param socket the target socket to bind
     * @param idx number of the target
     * @param base base address of the target
     * @param size size of the address range occupied by the target
     * @param remap if true address will be rewritten in accesses to be 0-based at the target
     */
    template <typename TYPE>
    void bind_target(TYPE& socket, size_t idx, uint64_t base, uint64_t size, bool remap = true) {
        set_target_range(idx, base, size, remap);
        initiator[idx].bind(socket);
    }
    /**
     * @fn void bind_target(TYPE&, size_t, std::string)
     * @brief bind the initiator socket of the router to some target and name it
     *
     * @tparam TYPE the socket type to bind
     * @param socket the target socket to bind
     * @param idx number of the target
     * @param name of the binding
     */
    template <typename TYPE> void bind_target(TYPE& socket, size_t idx, std::string name) {
        set_target_name(idx, name);
        initiator[idx].bind(socket);
    }
    /**
     * @fn void set_initiator_base(size_t, uint64_t)
     * @brief define a base address of a socket
     *
     * This will be added to the address of each access coming thru this socket
     *
     * @param idx
     * @param base
     */
    void set_initiator_base(size_t idx, uint64_t base) { ibases[idx] = base; }
    /**
     * @fn void set_default_target(size_t)
     * @brief define the default target socket
     *
     * If no target address range is hit the access is routed to this socket.
     * If this is not defined a address error response is generated
     *
     * @param idx the default target
     */
    void set_default_target(size_t idx) { default_idx = idx; }
    /**
     * @fn void set_target_name(size_t, std::string)
     * @brief establish a mapping between socket name and socket index
     *
     * @param idx the index of the socket
     * @param name the name of the connection
     */
    void set_target_name(size_t idx, std::string name) { target_name_lut.insert(std::make_pair(name, idx)); }
    /**
     * @fn void add_target_range(std::string, uint64_t, uint64_t, bool=true)
     * @brief establish a mapping between a named socket and a target address range
     *
     * @param name
     * @param base base address of the target
     * @param size size of the address range occupied by the target
     * @param remap if true address will be rewritten in accesses to be 0-based at the target
     */
    void add_target_range(std::string name, uint64_t base, uint64_t size, bool remap = true);
    /**
     * @fn void set_target_range(size_t, uint64_t, uint64_t, bool=true)
     * @brief establish a mapping between a socket and a target address range
     *
     * @param idx
     * @param base base address of the target
     * @param size size of the address range occupied by the target
     * @param remap if true address will be rewritten in accesses to be 0-based at the target
     */
    void set_target_range(size_t idx, uint64_t base, uint64_t size, bool remap = true);
    /**
     * @fn void b_transport(int, tlm::tlm_generic_payload&, sc_core::sc_time&)
     * @brief tagged blocking transport method
     *
     * @param i the tag
     * @param trans the incoming transaction
     * @param delay the annotated delay
     */
    void b_transport(int i, tlm::tlm_generic_payload& trans, sc_core::sc_time& delay);
    /**
     * @fn bool get_direct_mem_ptr(int, tlm::tlm_generic_payload&, tlm::tlm_dmi&)
     * @brief tagged forward DMI method
     *
     * @param i the tag
     * @param trans the incoming transaction
     * @param dmi_data
     * @return
     */
    bool get_direct_mem_ptr(int i, tlm::tlm_generic_payload& trans, tlm::tlm_dmi& dmi_data);
    /**
     * @fn unsigned transport_dbg(int, tlm::tlm_generic_payload&)
     * @brief tagged debug transaction method
     *
     * @param i the tag
     * @param trans the incoming transaction
     */
    unsigned transport_dbg(int i, tlm::tlm_generic_payload& trans);
    /**
     * @fn void invalidate_direct_mem_ptr(int, ::sc_dt::uint64, ::sc_dt::uint64)
     * @brief tagged backward DMI method
     *
     * @param id the tag
     * @param start_range address range start address
     * @param end_range address range end address
     */
    void invalidate_direct_mem_ptr(int id, ::sc_dt::uint64 start_range, ::sc_dt::uint64 end_range);

protected:
    struct range_entry {
        uint64_t base, size;
        bool remap;
    };
    size_t default_idx = std::numeric_limits<size_t>::max();
    std::vector<uint64_t> ibases;
    std::vector<range_entry> tranges;
    std::vector<sc_core::sc_mutex> mutexes;
    util::range_lut<unsigned> addr_decoder;
    std::unordered_map<std::string, size_t> target_name_lut;
};

template <unsigned BUSWIDTH>
router<BUSWIDTH>::router(const sc_core::sc_module_name& nm, unsigned slave_cnt, unsigned master_cnt)
: sc_module(nm)
, target("target", master_cnt)
, initiator("intor", slave_cnt)
, ibases(master_cnt)
, tranges(slave_cnt)
, mutexes(slave_cnt)
, addr_decoder(std::numeric_limits<unsigned>::max()) {
    for(size_t i = 0; i < target.size(); ++i) {
        target[i].register_b_transport([=](tlm::tlm_generic_payload& trans, sc_core::sc_time& delay) -> void {
            this->b_transport(i, trans, delay);
        });
        target[i].register_get_direct_mem_ptr([=](tlm::tlm_generic_payload& trans, tlm::tlm_dmi& dmi_data) -> bool {
            return this->get_direct_mem_ptr(i, trans, dmi_data);
        });
        target[i].register_transport_dbg(
            [=](tlm::tlm_generic_payload& trans) -> unsigned { return this->transport_dbg(i, trans); });
        ibases[i] = 0ULL;
    }
    for(size_t i = 0; i < initiator.size(); ++i) {
        initiator[i].register_invalidate_direct_mem_ptr(
            [=](::sc_dt::uint64 start_range, ::sc_dt::uint64 end_range) -> void {
                this->invalidate_direct_mem_ptr(i, start_range, end_range);
            });
        tranges[i].base = 0ULL;
        tranges[i].size = 0ULL;
        tranges[i].remap = false;
    }
}

template <unsigned BUSWIDTH>
void router<BUSWIDTH>::set_target_range(size_t idx, uint64_t base, uint64_t size, bool remap) {
    tranges[idx].base = base;
    tranges[idx].size = size;
    tranges[idx].remap = remap;
    addr_decoder.addEntry(idx, base, size);
}

template <unsigned BUSWIDTH>
void router<BUSWIDTH>::add_target_range(std::string name, uint64_t base, uint64_t size, bool remap) {
    auto it = target_name_lut.find(name);
#ifndef NDEBUG
#if(SYSTEMC_VERSION >= 20171012)
    if(it == target_name_lut.end()) {
        std::stringstream ss;
        ss << "No target index entry for '" << name << "' found ";
        ::sc_core::sc_assertion_failed(ss.str().c_str(), __FILE__, __LINE__);
    }
#else
    sc_assert(it != target_name_lut.end());
#endif
#endif
    auto idx = it->second;
    tranges[idx].base = base;
    tranges[idx].size = size;
    tranges[idx].remap = remap;
    addr_decoder.addEntry(idx, base, size);
}

template <unsigned BUSWIDTH>
void router<BUSWIDTH>::b_transport(int i, tlm::tlm_generic_payload& trans, sc_core::sc_time& delay) {
    ::sc_dt::uint64 address = trans.get_address();
    if(ibases[i]) {
        address += ibases[i];
        trans.set_address(address);
    }
    size_t idx = addr_decoder.getEntry(address);
    if(idx == addr_decoder.null_entry) {
        if(default_idx == std::numeric_limits<size_t>::max()) {
            trans.set_response_status(tlm::TLM_ADDRESS_ERROR_RESPONSE);
            return;
        }
        idx = default_idx;
    } else {
        // Modify address within transaction
        trans.set_address(address - (tranges[idx].remap ? tranges[idx].base : 0));
    }
    // Forward transaction to appropriate target
    mutexes[idx].lock();
    initiator[idx]->b_transport(trans, delay);
    mutexes[idx].unlock();
}
template <unsigned BUSWIDTH>
bool router<BUSWIDTH>::get_direct_mem_ptr(int i, tlm::tlm_generic_payload& trans, tlm::tlm_dmi& dmi_data) {
    ::sc_dt::uint64 address = trans.get_address();
    if(ibases[i]) {
        address += ibases[i];
        trans.set_address(address);
    }
    size_t idx = addr_decoder.getEntry(address);
    if(idx == addr_decoder.null_entry) {
        if(default_idx == std::numeric_limits<size_t>::max()) {
            trans.set_response_status(tlm::TLM_ADDRESS_ERROR_RESPONSE);
            return false;
        }
        idx = default_idx;
    } else {
        // Modify address within transaction
        trans.set_address(address - (tranges[idx].remap ? tranges[idx].base : 0));
    }
    bool status = initiator[idx]->get_direct_mem_ptr(trans, dmi_data);
    // Calculate DMI address of target in system address space
    auto offset = tranges[idx].remap ? tranges[idx].base : 0;
    dmi_data.set_start_address(dmi_data.get_start_address() - ibases[i] + offset);
    dmi_data.set_end_address(dmi_data.get_end_address() - ibases[i] + offset);
    return status;
}
template <unsigned BUSWIDTH> unsigned router<BUSWIDTH>::transport_dbg(int i, tlm::tlm_generic_payload& trans) {
    ::sc_dt::uint64 address = trans.get_address();
    if(ibases[i]) {
        address += ibases[i];
        trans.set_address(address);
    }
    size_t idx = addr_decoder.getEntry(address);
    if(idx == addr_decoder.null_entry) {
        if(default_idx == std::numeric_limits<size_t>::max()) {
            trans.set_response_status(tlm::TLM_ADDRESS_ERROR_RESPONSE);
            return 0;
        }
        idx = default_idx;
    } else {
        // Modify address within transaction
        trans.set_address(address - (tranges[idx].remap ? tranges[idx].base : 0));
    }
    // Forward debug transaction to appropriate target
    return initiator[idx]->transport_dbg(trans);
}
template <unsigned BUSWIDTH>
void router<BUSWIDTH>::invalidate_direct_mem_ptr(int id, ::sc_dt::uint64 start_range, ::sc_dt::uint64 end_range) {
    // Reconstruct address range in system memory map
    ::sc_dt::uint64 bw_start_range = start_range;
    if(tranges[id].remap)
        bw_start_range += tranges[id].base;
    ::sc_dt::uint64 bw_end_range = end_range;
    if(tranges[id].remap)
        bw_end_range += tranges[id].base;
    for(size_t i = 0; i < target.size(); ++i) {
        target[i]->invalidate_direct_mem_ptr(bw_start_range - ibases[i], bw_end_range - ibases[i]);
    }
}

} // namespace scc

#endif /* SYSC_AVR_ROUTER_H_ */
