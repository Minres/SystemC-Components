/*******************************************************************************
 * Copyright 2016-2022 MINRES Technologies GmbH
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

#define SC_INCLUDE_DYNAMIC_PROCESSES
#include "tlm_recorder.h"
#include "tlm_extension_recording_registry.h"
#include <tlm/scc/tlm_id.h>

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
    handle.record_attribute("trans.ptr", reinterpret_cast<uintptr_t>(&o));
    handle.record_attribute("trans.address", o.get_address());
    handle.record_attribute("trans.cmd", cmd2char.at(o.get_command()));
    handle.record_attribute("trans.data_ptr", o.get_data_ptr());
    handle.record_attribute("trans.data_length", o.get_data_length());
    handle.record_attribute("trans.response", resp2char.at(1 - o.get_response_status()));
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

class tlm_id_ext_recording : public tlm_extensions_recording_if<tlm::tlm_base_protocol_types> {

    void recordBeginTx(SCVNS scv_tr_handle& handle, tlm::tlm_base_protocol_types::tlm_payload_type& trans) override {
        if(auto ext = trans.get_extension<tlm_id_extension>()) {
            handle.record_attribute("trans.uid", ext->id);
        }
    }

    void recordEndTx(SCVNS scv_tr_handle& handle, tlm::tlm_base_protocol_types::tlm_payload_type& trans) override {
    }
};
using namespace tlm::scc::scv;
#if defined(__GNUG__)
__attribute__((constructor))
#endif
bool register_extensions() {
    tlm::scc::tlm_id_extension ext(nullptr); // NOLINT
    tlm_extension_recording_registry<tlm::tlm_base_protocol_types>::inst().register_ext_rec(
        ext.ID, new tlm_id_ext_recording()); // NOLINT
    return true;                             // NOLINT
}
bool registered = register_extensions();

} // namespace scv
} // namespace scc
} // namespace tlm
