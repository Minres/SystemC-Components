/*******************************************************************************
 * Copyright 2016, 2017 MINRES Technologies GmbH
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

#ifndef TLM_GP_DATA_EXT_H_
#define TLM_GP_DATA_EXT_H_

#ifdef WITH_SCV
#include <scv.h>
#define SCVNS()
#else
#include <scv-tr.h>
#define SCVNS() scv_tr::
#endif
#include <tlm/scc/scv/tlm_gp_data.h>

template <> class SCVNS()scv_extensions<tlm::tlm_command> : public SCVNS()scv_enum_base<tlm::tlm_command> {
public:
    SCV_ENUM_CTOR(tlm::tlm_command) {      // NOLINT
        SCV_ENUM(tlm::TLM_READ_COMMAND);   // NOLINT
        SCV_ENUM(tlm::TLM_WRITE_COMMAND);  // NOLINT
        SCV_ENUM(tlm::TLM_IGNORE_COMMAND); // NOLINT
    }
};

template <> class SCVNS()scv_extensions<tlm::tlm_response_status> : public SCVNS()scv_enum_base<tlm::tlm_response_status> {
public:
    SCV_ENUM_CTOR(tlm::tlm_response_status) {          // NOLINT
        SCV_ENUM(tlm::TLM_OK_RESPONSE);                // NOLINT
        SCV_ENUM(tlm::TLM_INCOMPLETE_RESPONSE);        // NOLINT
        SCV_ENUM(tlm::TLM_GENERIC_ERROR_RESPONSE);     // NOLINT
        SCV_ENUM(tlm::TLM_ADDRESS_ERROR_RESPONSE);     // NOLINT
        SCV_ENUM(tlm::TLM_COMMAND_ERROR_RESPONSE);     // NOLINT
        SCV_ENUM(tlm::TLM_BURST_ERROR_RESPONSE);       // NOLINT
        SCV_ENUM(tlm::TLM_BYTE_ENABLE_ERROR_RESPONSE); // NOLINT
    }
};

template <> class SCVNS()scv_extensions<tlm::tlm_gp_option> : public SCVNS()scv_enum_base<tlm::tlm_gp_option> {
public:
    SCV_ENUM_CTOR(tlm::tlm_gp_option) {           // NOLINT
        SCV_ENUM(tlm::TLM_MIN_PAYLOAD);           // NOLINT
        SCV_ENUM(tlm::TLM_FULL_PAYLOAD);          // NOLINT
        SCV_ENUM(tlm::TLM_FULL_PAYLOAD_ACCEPTED); // NOLINT
    }
};

template <> class SCVNS()scv_extensions<tlm::scc::scv::tlm_phase_enum> : public SCVNS()scv_enum_base<tlm::scc::scv::tlm_phase_enum> {
public:
    SCV_ENUM_CTOR(tlm::scc::scv::tlm_phase_enum) {    // NOLINT
        SCV_ENUM(tlm::scc::scv::UNINITIALIZED_PHASE); // NOLINT
        SCV_ENUM(tlm::scc::scv::BEGIN_REQ);           // NOLINT
        SCV_ENUM(tlm::scc::scv::END_REQ);             // NOLINT
        SCV_ENUM(tlm::scc::scv::BEGIN_RESP);          // NOLINT
        SCV_ENUM(tlm::scc::scv::END_RESP);            // NOLINT
        SCV_ENUM(tlm::scc::scv::CUSTOM1);             // NOLINT
        SCV_ENUM(tlm::scc::scv::CUSTOM2);             // NOLINT
        SCV_ENUM(tlm::scc::scv::CUSTOM3);             // NOLINT
        SCV_ENUM(tlm::scc::scv::CUSTOM4);             // NOLINT
        SCV_ENUM(tlm::scc::scv::CUSTOM5);             // NOLINT
        SCV_ENUM(tlm::scc::scv::CUSTOM6);             // NOLINT
    }
};

template <> class SCVNS()scv_extensions<tlm::tlm_sync_enum> : public SCVNS()scv_enum_base<tlm::tlm_sync_enum> {
public:
    SCV_ENUM_CTOR(tlm::tlm_sync_enum) { // NOLINT
        SCV_ENUM(tlm::TLM_ACCEPTED);    // NOLINT
        SCV_ENUM(tlm::TLM_UPDATED);     // NOLINT
        SCV_ENUM(tlm::TLM_COMPLETED);   // NOLINT
    }
};

template <> class SCVNS()scv_extensions<tlm::scc::scv::tlm_gp_data> : public SCVNS()scv_extensions_base<tlm::scc::scv::tlm_gp_data> {
public:
    SCVNS()scv_extensions<sc_dt::uint64> address;
    SCVNS()scv_extensions<tlm::tlm_command> command;
    SCVNS()scv_extensions<unsigned char*> data;
    SCVNS()scv_extensions<unsigned int> data_length;
    SCVNS()scv_extensions<tlm::tlm_response_status> response_status;
    SCVNS()scv_extensions<bool> dmi_allowed;
    SCVNS()scv_extensions<unsigned char*> byte_enable;
    SCVNS()scv_extensions<unsigned int> byte_enable_length;
    SCVNS()scv_extensions<unsigned int> streaming_width;
    SCVNS()scv_extensions<tlm::tlm_gp_option> gp_option;
    SCVNS()scv_extensions<uintptr_t> uid;

    SCV_EXTENSIONS_CTOR(tlm::scc::scv::tlm_gp_data) {
        // must be in order
        SCV_FIELD(address);
        SCV_FIELD(command);
        SCV_FIELD(data);
        SCV_FIELD(data_length);
        SCV_FIELD(response_status);
        SCV_FIELD(dmi_allowed);
        SCV_FIELD(byte_enable);
        SCV_FIELD(byte_enable_length);
        SCV_FIELD(streaming_width);
        SCV_FIELD(gp_option);
        SCV_FIELD(uid);
    }
};

template <> class SCVNS()scv_extensions<tlm::tlm_dmi::dmi_access_e> : public SCVNS()scv_enum_base<tlm::tlm_dmi::dmi_access_e> {
public:
    SCV_ENUM_CTOR(tlm::tlm_dmi::dmi_access_e) {        // NOLINT
        SCV_ENUM(tlm::tlm_dmi::DMI_ACCESS_NONE);       // NOLINT
        SCV_ENUM(tlm::tlm_dmi::DMI_ACCESS_READ);       // NOLINT
        SCV_ENUM(tlm::tlm_dmi::DMI_ACCESS_WRITE);      // NOLINT
        SCV_ENUM(tlm::tlm_dmi::DMI_ACCESS_READ_WRITE); // NOLINT
    }
};

template <>
class SCVNS()scv_extensions<tlm::scc::scv::tlm_dmi_data> : public SCVNS()scv_extensions_base<tlm::scc::scv::tlm_dmi_data> {
public:
    SCVNS()scv_extensions<unsigned char*> dmi_ptr;
    SCVNS()scv_extensions<sc_dt::uint64> dmi_start_address;
    SCVNS()scv_extensions<sc_dt::uint64> dmi_end_address;
    SCVNS()scv_extensions<tlm::tlm_dmi::dmi_access_e> dmi_access;
    SCVNS()scv_extensions<sc_dt::uint64> dmi_read_latency;
    SCVNS()scv_extensions<sc_dt::uint64> dmi_write_latency;
    SCV_EXTENSIONS_CTOR(tlm::scc::scv::tlm_dmi_data) {
        // must be in order
        SCV_FIELD(dmi_ptr);
        SCV_FIELD(dmi_start_address);
        SCV_FIELD(dmi_end_address);
        SCV_FIELD(dmi_access);
        SCV_FIELD(dmi_read_latency);
        SCV_FIELD(dmi_write_latency);
    }
};

#endif /* TLM_GP_DATA_EXT_H_ */
