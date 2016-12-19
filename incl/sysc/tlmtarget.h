/*
 * tlmtarget.h
 *
 *  Created on: Nov 16, 2016
 *      Author: developer
 */

#ifndef SYSC_TLMTARGET_H_
#define SYSC_TLMTARGET_H_

#include <util/range_lut.h>
#include "resource_access_if.h"
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#include <tlm_utils/simple_target_socket.h>
#pragma GCC diagnostic pop
#include <array>

namespace sysc {
/**
 * a simple access-width based bus interface (no DMI support)
 */
struct addr_range {
    uint64_t base, size;
};

template<unsigned int BUSWIDTH=32, unsigned RANGES=1>
struct tlm_target {
    typedef tlm_target<BUSWIDTH, RANGES> this_type;

    tlm_target(sc_core::sc_time& clock, std::array<addr_range, RANGES> addr_rngs);

    virtual ~tlm_target(){};

    tlm_utils::simple_target_socket<this_type, BUSWIDTH> socket;

    void b_tranport_cb(tlm::tlm_generic_payload&, sc_core::sc_time&);

    unsigned int tranport_dbg_cb(tlm::tlm_generic_payload&);

    const std::array<addr_range, RANGES> addr_ranges;

protected:
    sc_core::sc_time& clk;
    util::range_lut<resource_access_if*> socket_map;
};

} /* namespace sysc */

template<unsigned int BUSWIDTH, unsigned RANGES>
inline sysc::tlm_target<BUSWIDTH,RANGES>::tlm_target(sc_core::sc_time& clock, std::array<addr_range, RANGES> addr_rngs)
:socket("socket")
, addr_ranges(addr_rngs)
, clk(clock)
, socket_map(nullptr)
{
    socket.register_b_transport(this, &this_type::b_tranport_cb);
    socket.register_transport_dbg(this, &this_type::tranport_dbg_cb);
}

template<unsigned int BUSWIDTH, unsigned RANGES>
void sysc::tlm_target<BUSWIDTH,RANGES>::b_tranport_cb(tlm::tlm_generic_payload& gp, sc_core::sc_time& delay) {
    resource_access_if* ra = socket_map.getEntry(gp.get_address());
    if(ra){
        if(gp.get_data_length()==ra->size() && gp.get_byte_enable_ptr()==0 && gp.get_data_length()==gp.get_streaming_width()){
            if(gp.get_command()== tlm::TLM_READ_COMMAND){
                if(ra->read(gp.get_data_ptr(), gp.get_data_length())) gp.set_response_status(tlm::TLM_OK_RESPONSE);
            } else if(gp.get_command()== tlm::TLM_WRITE_COMMAND){
                if(ra->write(gp.get_data_ptr(), gp.get_data_length())) gp.set_response_status(tlm::TLM_OK_RESPONSE);
            } else {
                gp.set_response_status(tlm::TLM_COMMAND_ERROR_RESPONSE);
            }
        } else {
            gp.set_response_status(tlm::TLM_GENERIC_ERROR_RESPONSE);
        }
    } else {
        gp.set_response_status(tlm::TLM_ADDRESS_ERROR_RESPONSE);
    }
    delay+=clk;
}

template<unsigned int BUSWIDTH, unsigned RANGES>
unsigned int sysc::tlm_target<BUSWIDTH,RANGES>::tranport_dbg_cb(tlm::tlm_generic_payload& gp) {
    resource_access_if* ra = socket_map.getEntry(gp.get_address());
    if(ra){
        if(gp.get_data_length()==ra->size() && gp.get_byte_enable_ptr()==0 && gp.get_data_length()==gp.get_streaming_width()){
            if(gp.get_command()== tlm::TLM_READ_COMMAND){
                if(ra->read_dbg(gp.get_data_ptr(), gp.get_data_length()))
                    return gp.get_data_length();
            } else if(gp.get_command()== tlm::TLM_READ_COMMAND){
                if(ra->write_dbg(gp.get_data_ptr(), gp.get_data_length()))
                    return gp.get_data_length();
            }
        }
    }
    return 0;
}

#endif /* SYSC_TLMTARGET_H_ */
