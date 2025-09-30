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

#ifndef SC_INCLUDE_DYNAMIC_PROCESSES
#define SC_INCLUDE_DYNAMIC_PROCESSES
#include "tlm/nw/tlm_network_sockets.h"
#endif
#include "tlm_recorder.h"
#include <tlm/scc/scv/tlm_extension_recording_registry.h>
#include <tlm/scc/tlm_id.h>

namespace tlm {
namespace nw {
namespace scv {
namespace {
const std::array<std::string, 3> cmd2char{{"READ", "WRITE", "IGNORE"}};
const std::array<std::string, 7> resp2char{
    {"OK", "INCOMPLETE", "GENERIC_ERROR", "ADDRESS_ERROR", "COMMAND_ERROR", "BURST_ERROR", "BYTE_ENABLE_ERROR"}};
const std::array<std::string, 3> gp_option2char{{"MIN_PAYLOAD", "FULL_PAYLOAD", "FULL_PAYLOAD_ACCEPTED"}};
const std::array<std::string, 5> phase2char{{"UNINITIALIZED_PHASE", "BEGIN_REQ", "END_REQ", "BEGIN_RESP", "END_RESP"}};
const std::array<std::string, 4> dmi2char{{"DMI_ACCESS_NONE", "DMI_ACCESS_READ", "DMI_ACCESS_WRITE", "DMI_ACCESS_READ_WRITE"}};
const std::array<std::string, 3> sync2char{{"ACCEPTED", "UPDATED", "COMPLETED"}};

} // namespace
void record(SCVNS scv_tr_handle& handle, tlm::nw::tlm_network_payload_base& o) {
    // handle.record_attribute("trans.ptr", reinterpret_cast<uintptr_t>(&o));
    // handle.record_attribute("trans.address", o.get_address());
    // handle.record_attribute("trans.cmd", cmd2char.at(o.get_command()));
    // handle.record_attribute("trans.data_ptr", o.get_data_ptr());
    // handle.record_attribute("trans.data_length", o.get_data_length());
    // handle.record_attribute("trans.response", resp2char.at(1 - o.get_response_status()));
    // handle.record_attribute("trans.dmi_allowed", o.is_dmi_allowed());
    // handle.record_attribute("trans.byte_enable", o.get_byte_enable_ptr());
    // handle.record_attribute("trans.byte_enable_length", o.get_byte_enable_length());
    // handle.record_attribute("trans.streaming_width", o.get_streaming_width());
    // handle.record_attribute("trans.gp_option", gp_option2char.at(o.get_gp_option()));
    // if(o.get_data_length() && o.get_data_length() < 9 && o.get_data_ptr()) {
    //     uint64_t buf = 0;
    //     memcpy(&buf, o.get_data_ptr(), o.get_data_length());
    //     handle.record_attribute("trans.data_value", buf);
    // }
}
void record(SCVNS scv_tr_handle& handle, tlm::tlm_phase& o) {
    unsigned id = o;
    if(id < phase2char.size())
        handle.record_attribute("phase", phase2char[id]);
    else
        handle.record_attribute("phase_id", id);
}
void record(SCVNS scv_tr_handle& handle, tlm::tlm_sync_enum o) { handle.record_attribute("tlm_sync", sync2char.at(o)); }

} // namespace scv
} // namespace nw
} // namespace tlm
