/*
 * router.h
 *
 *  Created on: Nov 5, 2016
 *      Author: eyck
 */

#ifndef SYSC_AVR_ROUTER_H_
#define SYSC_AVR_ROUTER_H_

#include <util/range_lut.h>
#include "utilities.h"
// pragmas to disable the deprecated warnings for SystemC headers
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#include "tlm.h"
#include <scv4tlm/tlm_rec_initiator_socket.h>
#include <scv4tlm/tlm_rec_target_socket.h>
#include <scv4tlm/initiator_mixin.h>
#include <scv4tlm/target_mixin.h>
#pragma GCC diagnostic pop
#include <limits>

namespace sysc {

template<unsigned BUSWIDTH = 32, unsigned SLAVES=1, unsigned MASTERS=1>
struct router: sc_core::sc_module {
    using intor_sckt  = scv4tlm::initiator_mixin<scv4tlm::tlm_rec_initiator_socket<BUSWIDTH>>;
    using target_sckt = scv4tlm::target_mixin<scv4tlm::tlm_rec_target_socket<BUSWIDTH>>;

    target_sckt target_sockets[MASTERS];
    intor_sckt  initiator_sockets[SLAVES];

    router(const sc_core::sc_module_name& nm);

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
    uint64_t ibases[MASTERS];
    range_entry tranges[SLAVES];
    sc_core::sc_mutex mutexes[SLAVES];
    util::range_lut<unsigned> addr_decoder;
};

template<unsigned BUSWIDTH, unsigned SLAVES, unsigned MASTERS>
router<BUSWIDTH, SLAVES, MASTERS>::router(const sc_core::sc_module_name& nm)
: sc_module(nm), addr_decoder(std::numeric_limits<unsigned>::max())
  {
    for (size_t i = 0; i < MASTERS; ++i) {
        target_sockets[i].register_b_transport(
                std::bind(&router::b_transport, this, i, std::placeholders::_1, std::placeholders::_2));
        target_sockets[i].register_get_direct_mem_ptr(
                std::bind(&router::get_direct_mem_ptr, this, i, std::placeholders::_1, std::placeholders::_2));
        target_sockets[i].register_transport_dbg(
                std::bind(&router::transport_dbg, this, i, std::placeholders::_1));
        ibases[i]=0ULL;
    }
    for (size_t i = 0; i < SLAVES; ++i) {
        initiator_sockets[i].register_invalidate_direct_mem_ptr(
                std::bind(&router::invalidate_direct_mem_ptr, this, i, std::placeholders::_1, std::placeholders::_2));
        tranges[i].base=0ULL;
        tranges[i].size=0ULL;
        tranges[i].remap=false;
    }
  }

template<unsigned BUSWIDTH, unsigned SLAVES, unsigned MASTERS>
void router<BUSWIDTH, SLAVES, MASTERS>::add_target_range(size_t idx, uint64_t base, uint64_t size, bool remap) {
    tranges[idx].base=base;
    tranges[idx].size=size;
    tranges[idx].remap=remap;
    addr_decoder.addEntry(idx, base, size);
}
template<unsigned BUSWIDTH, unsigned SLAVES, unsigned MASTERS>
void router<BUSWIDTH, SLAVES, MASTERS>::b_transport(int i, tlm::tlm_generic_payload& trans, sc_core::sc_time& delay) {
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
    initiator_sockets[idx]->b_transport(trans, delay);
    mutexes[idx].unlock();
}
template<unsigned BUSWIDTH, unsigned SLAVES, unsigned MASTERS>
bool router<BUSWIDTH, SLAVES, MASTERS>::get_direct_mem_ptr(int i, tlm::tlm_generic_payload& trans, tlm::tlm_dmi& dmi_data) {
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
    bool status = initiator_sockets[idx]->get_direct_mem_ptr(trans, dmi_data);
    // Calculate DMI address of target in system address space
    dmi_data.set_start_address(dmi_data.get_start_address() - ibases[i] + tranges[idx].remap?tranges[idx].base:0);
    dmi_data.set_end_address(dmi_data.get_end_address()- ibases[i] + tranges[idx].remap?tranges[idx].base:0);
    return status;
}
template<unsigned BUSWIDTH, unsigned SLAVES, unsigned MASTERS>
unsigned router<BUSWIDTH, SLAVES, MASTERS>::transport_dbg(int i, tlm::tlm_generic_payload& trans) {
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
    return initiator_sockets[idx]->transport_dbg(trans);
}
template<unsigned BUSWIDTH, unsigned SLAVES, unsigned MASTERS>
void router<BUSWIDTH, SLAVES, MASTERS>::invalidate_direct_mem_ptr(int id, sc_dt::uint64 start_range, sc_dt::uint64 end_range) {
    // Reconstruct address range in system memory map
    sc_dt::uint64 bw_start_range = start_range;
    if(tranges[id].remap) bw_start_range+= tranges[id].base;
    sc_dt::uint64 bw_end_range = end_range;
    if(tranges[id].remap) bw_end_range+= tranges[id].base;
    for (size_t i = 0; i < MASTERS; ++i) {
        target_sockets[i]->invalidate_direct_mem_ptr(bw_start_range - ibases[i], bw_end_range - ibases[i]);
    }
}

}  // namespace sysc

#endif /* SYSC_AVR_ROUTER_H_ */
