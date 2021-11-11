/*
 * tlm_recorder.cpp
 *
 *  Created on: Jul 28, 2021
 *      Author: eyckj
 */

#define SC_INCLUDE_DYNAMIC_PROCESSES
#include "tlm_recorder.h"
#include "tlm_extension_recording_registry.h"
#include <tlm>

namespace tlm {
namespace scc {
namespace scv {
namespace {
const std::array<std::string, 3> cmd2char{{"READ", "WRITE", "IGNORE"}};
const std::array<std::string, 7> resp2char{
    {"OK", "INCOMPLETE", "GENERIC_ERROR", "ADDRESS_ERROR", "COMMAND_ERROR", "BURST_ERROR", "BYTE_ENABLE_ERROR"}};
const std::array<std::string, 3> gp_option2char{{"MIN_PAYLOAD", "FULL_PAYLOAD", "FULL_PAYLOAD_ACCEPTED"}};
const std::array<std::string, 5> phase2char{{"UNINITIALIZED_PHASE", "BEGIN_REQ", "END_REQ", "BEGIN_RESP", "END_RESP"}};
const std::array<std::string, 4> dmi2char{
    {"DMI_ACCESS_NONE", "DMI_ACCESS_READ", "DMI_ACCESS_WRITE", "DMI_ACCESS_READ_WRITE"}};
const std::array<std::string, 3> sync2char{{"ACCEPTED", "UPDATED", "COMPLETED"}};

} // namespace
void record(SCVNS scv_tr_handle& handle, tlm::tlm_generic_payload& o) {
    handle.record_attribute("trans.address", o.get_address());
    handle.record_attribute("trans.cmd", cmd2char.at(o.get_command()));
    handle.record_attribute("trans.data_ptr", o.get_data_ptr());
    handle.record_attribute("trans.data_length", o.get_data_length());
    handle.record_attribute("trans.response", resp2char.at(1-o.get_response_status()));
    handle.record_attribute("trans.dmi_allowed", o.is_dmi_allowed());
    handle.record_attribute("trans.byte_enable", o.get_byte_enable_ptr());
    handle.record_attribute("trans.byte_enable_length", o.get_byte_enable_length());
    handle.record_attribute("trans.streaming_width", o.get_streaming_width());
    handle.record_attribute("trans.gp_option", gp_option2char.at(o.get_gp_option()));
    if(o.get_data_length() < 8 && o.get_data_ptr()) {
        uint64_t buf = 0;
        // FIXME: this is endianess dependent
        for(size_t i = 0; i < o.get_data_length(); i++)
            buf += (*o.get_data_ptr()) << i * 8;
        handle.record_attribute("trans.data_value", buf);
    }
}
void record(SCVNS scv_tr_handle& handle, tlm::tlm_phase& o) {
    unsigned id = o;
    if(id < phase2char.size())
        handle.record_attribute("phase", phase2char[id]);
    else
        handle.record_attribute("phase_id", id);
}
void record(SCVNS scv_tr_handle& handle, tlm::tlm_sync_enum o) { handle.record_attribute("tlm_sync", sync2char.at(o)); }
void record(SCVNS scv_tr_handle& handle, tlm::tlm_dmi& o) {
    handle.record_attribute("trans.dmi_ptr", o.get_dmi_ptr());
    handle.record_attribute("trans.start_address", o.get_start_address());
    handle.record_attribute("trans.end_address", o.get_end_address());
    handle.record_attribute("trans.granted_access", dmi2char.at(o.get_granted_access()));
    handle.record_attribute("trans.read_latency", o.get_read_latency().to_string());
    handle.record_attribute("trans.write_latency", o.get_write_latency().to_string());
}
} // namespace scv
} // namespace scc
} // namespace tlm
