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
 * router.h
 *
 *  Created on: Nov 5, 2016
 *      Author: eyck
 */

#ifndef _SYSC_ROUTER_H_
#define _SYSC_ROUTER_H_

#include <util/range_lut.h>
#include "utilities.h"
// pragmas to disable the deprecated warnings for SystemC headers
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#include <tlm.h>
#include <scv4tlm/tlm_rec_initiator_socket.h>
#include <scv4tlm/tlm_rec_target_socket.h>
#include <sysc/initiator_mixin.h>
#include <sysc/target_mixin.h>
#include <sysc/utils/sc_vector.h>
#pragma GCC diagnostic pop
#include <limits>

namespace sysc {

template<unsigned BUSWIDTH = 32>
struct router: sc_core::sc_module {
    using intor_sckt  = sysc::initiator_mixin<scv4tlm::tlm_rec_initiator_socket<BUSWIDTH>>;
    using target_sckt = sysc::target_mixin<scv4tlm::tlm_rec_target_socket<BUSWIDTH>>;

    sc_core::sc_vector<target_sckt> target;
    sc_core::sc_vector<intor_sckt>  initiator;

    router(const sc_core::sc_module_name& nm, unsigned slave_cnt=1, unsigned master_cnt=1);

    ~router() {}

    void set_initiator_base(size_t idx, uint64_t base){ibases[idx]=base;}

    void set_default_target(size_t idx){default_idx=idx;}
    // bind target socket hierarchically
    void add_target_range(size_t idx, uint64_t base, uint64_t size, bool remap=true);
    // tagged blocking transport method
    void b_transport(int i, tlm::tlm_generic_payload& trans, sc_core::sc_time& delay);
    // tagged forward DMI method
    bool get_direct_mem_ptr(int i, tlm::tlm_generic_payload& trans, tlm::tlm_dmi& dmi_data);
    // tagged debug transaction method
    unsigned transport_dbg(int i, tlm::tlm_generic_payload& trans);
    // Tagged backward DMI method
    void invalidate_direct_mem_ptr(int id, sc_dt::uint64 start_range, sc_dt::uint64 end_range);

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
};

template<unsigned BUSWIDTH>
router<BUSWIDTH>::router(const sc_core::sc_module_name& nm, unsigned slave_cnt, unsigned master_cnt)
: sc_module(nm)
  , target("target", master_cnt)
  , initiator("intor", slave_cnt)
  , ibases(master_cnt)
  , tranges(slave_cnt)
  , mutexes(slave_cnt)
  , addr_decoder(std::numeric_limits<unsigned>::max())
  {
    for (size_t i = 0; i < target.size(); ++i) {
        target[i].register_b_transport(
                [=](tlm::tlm_generic_payload& trans, sc_core::sc_time& delay)->void {this->b_transport(i, trans, delay);
        });
        target[i].register_get_direct_mem_ptr([=](tlm::tlm_generic_payload& trans, tlm::tlm_dmi& dmi_data)->bool {
            return this->get_direct_mem_ptr(i, trans, dmi_data);
        });
        target[i].register_transport_dbg(
                [=](tlm::tlm_generic_payload& trans)->unsigned {return this->transport_dbg(i, trans);
        });
        ibases[i]=0ULL;
    }
    for (size_t i = 0; i < initiator.size(); ++i) {
        initiator[i].register_invalidate_direct_mem_ptr(
                [=](sc_dt::uint64 start_range, sc_dt::uint64 end_range)->void {this->invalidate_direct_mem_ptr(i, start_range, end_range);
        });
        tranges[i].base=0ULL;
        tranges[i].size=0ULL;
        tranges[i].remap=false;
    }
  }

template<unsigned BUSWIDTH>
void router<BUSWIDTH>::add_target_range(size_t idx, uint64_t base, uint64_t size, bool remap) {
    tranges[idx].base=base;
    tranges[idx].size=size;
    tranges[idx].remap=remap;
    addr_decoder.addEntry(idx, base, size);
}
template<unsigned BUSWIDTH>
void router<BUSWIDTH>::b_transport(int i, tlm::tlm_generic_payload& trans, sc_core::sc_time& delay) {
    sc_dt::uint64 address = trans.get_address();
    if(ibases[i]){
        address+=ibases[i];
        trans.set_address(address);
    }
    size_t idx = addr_decoder.getEntry(address);
    if (idx == addr_decoder.null_entry) {
        if(default_idx == std::numeric_limits<size_t>::max()){
            trans.set_response_status(tlm::TLM_ADDRESS_ERROR_RESPONSE);
            return;
        }
        idx=default_idx;
    } else {
        // Modify address within transaction
        trans.set_address(address - (tranges[idx].remap?tranges[idx].base:0));
    }
    // Forward transaction to appropriate target
    mutexes[idx].lock();
    initiator[idx]->b_transport(trans, delay);
    mutexes[idx].unlock();
}
template<unsigned BUSWIDTH>
bool router<BUSWIDTH>::get_direct_mem_ptr(int i, tlm::tlm_generic_payload& trans, tlm::tlm_dmi& dmi_data) {
    sc_dt::uint64 address = trans.get_address();
    if(ibases[i]){
        address+=ibases[i];
        trans.set_address(address);
    }
    size_t idx = addr_decoder.getEntry(address);
    if (idx == addr_decoder.null_entry) {
        if(default_idx == std::numeric_limits<size_t>::max()){
            trans.set_response_status(tlm::TLM_ADDRESS_ERROR_RESPONSE);
            return false;
        }
        idx=default_idx;
    } else {
        // Modify address within transaction
        trans.set_address(address - (tranges[idx].remap?tranges[idx].base:0));
    }
    bool status = initiator[idx]->get_direct_mem_ptr(trans, dmi_data);
    // Calculate DMI address of target in system address space
    dmi_data.set_start_address(dmi_data.get_start_address() - ibases[i] + tranges[idx].remap?tranges[idx].base:0);
    dmi_data.set_end_address(dmi_data.get_end_address()- ibases[i] + tranges[idx].remap?tranges[idx].base:0);
    return status;
}
template<unsigned BUSWIDTH>
unsigned router<BUSWIDTH>::transport_dbg(int i, tlm::tlm_generic_payload& trans) {
    sc_dt::uint64 address = trans.get_address();
    if(ibases[i]){
        address+=ibases[i];
        trans.set_address(address);
    }
    size_t idx = addr_decoder.getEntry(address);
    if (idx == addr_decoder.null_entry) {
        if(default_idx == std::numeric_limits<size_t>::max()){
            trans.set_response_status(tlm::TLM_ADDRESS_ERROR_RESPONSE);
            return 0;
        }
        idx=default_idx;
    } else {
        // Modify address within transaction
        trans.set_address(address - (tranges[idx].remap?tranges[idx].base:0));
    }
    // Forward debug transaction to appropriate target
    return initiator[idx]->transport_dbg(trans);
}
template<unsigned BUSWIDTH>
void router<BUSWIDTH>::invalidate_direct_mem_ptr(int id, sc_dt::uint64 start_range, sc_dt::uint64 end_range) {
    // Reconstruct address range in system memory map
    sc_dt::uint64 bw_start_range = start_range;
    if(tranges[id].remap) bw_start_range+= tranges[id].base;
    sc_dt::uint64 bw_end_range = end_range;
    if(tranges[id].remap) bw_end_range+= tranges[id].base;
    for (size_t i = 0; i < target.size(); ++i) {
        target[i]->invalidate_direct_mem_ptr(bw_start_range - ibases[i], bw_end_range - ibases[i]);
    }
}

}  // namespace sysc

#endif /* SYSC_AVR_ROUTER_H_ */
