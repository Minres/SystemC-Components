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

#ifndef TLM_GP_DATA_EXT_H_
#define TLM_GP_DATA_EXT_H_

#include "tlm_gp_data.h"
#include <scv.h>

template <> class scv_extensions<tlm::tlm_command> : public scv_enum_base<tlm::tlm_command> {
public:
    SCV_ENUM_CTOR(tlm::tlm_command) {
        SCV_ENUM(tlm::TLM_READ_COMMAND);
        SCV_ENUM(tlm::TLM_WRITE_COMMAND);
        SCV_ENUM(tlm::TLM_IGNORE_COMMAND);
    }
};

template <> class scv_extensions<tlm::tlm_response_status> : public scv_enum_base<tlm::tlm_response_status> {
public:
    SCV_ENUM_CTOR(tlm::tlm_response_status) {
        SCV_ENUM(tlm::TLM_OK_RESPONSE);
        SCV_ENUM(tlm::TLM_INCOMPLETE_RESPONSE);
        SCV_ENUM(tlm::TLM_GENERIC_ERROR_RESPONSE);
        SCV_ENUM(tlm::TLM_ADDRESS_ERROR_RESPONSE);
        SCV_ENUM(tlm::TLM_COMMAND_ERROR_RESPONSE);
        SCV_ENUM(tlm::TLM_BURST_ERROR_RESPONSE);
        SCV_ENUM(tlm::TLM_BYTE_ENABLE_ERROR_RESPONSE);
    }
};

template <> class scv_extensions<tlm::tlm_gp_option> : public scv_enum_base<tlm::tlm_gp_option> {
public:
    SCV_ENUM_CTOR(tlm::tlm_gp_option) {
        SCV_ENUM(tlm::TLM_MIN_PAYLOAD);
        SCV_ENUM(tlm::TLM_FULL_PAYLOAD);
        SCV_ENUM(tlm::TLM_FULL_PAYLOAD_ACCEPTED);
    }
};

template <> class scv_extensions<tlm::tlm_phase_enum> : public scv_enum_base<tlm::tlm_phase_enum> {
public:
    SCV_ENUM_CTOR(tlm::tlm_phase_enum) {
        SCV_ENUM(tlm::UNINITIALIZED_PHASE);
        SCV_ENUM(tlm::BEGIN_REQ);
        SCV_ENUM(tlm::END_REQ);
        SCV_ENUM(tlm::BEGIN_RESP);
        SCV_ENUM(tlm::END_RESP);
    }
};

template <> class scv_extensions<tlm::tlm_sync_enum> : public scv_enum_base<tlm::tlm_sync_enum> {
public:
    SCV_ENUM_CTOR(tlm::tlm_sync_enum) {
        SCV_ENUM(tlm::TLM_ACCEPTED);
        SCV_ENUM(tlm::TLM_UPDATED);
        SCV_ENUM(tlm::TLM_COMPLETED);
    }
};

template <> class scv_extensions<scv4tlm::tlm_gp_data> : public scv_extensions_base<scv4tlm::tlm_gp_data> {
public:
    scv_extensions<sc_dt::uint64> address;
    scv_extensions<tlm::tlm_command> command;
    scv_extensions<unsigned char *> data;
    scv_extensions<unsigned int> data_length;
    scv_extensions<tlm::tlm_response_status> response_status;
    scv_extensions<bool> dmi_allowed;
    scv_extensions<unsigned char *> byte_enable;
    scv_extensions<unsigned int> byte_enable_length;
    scv_extensions<unsigned int> streaming_width;
    scv_extensions<tlm::tlm_gp_option> gp_option;

    SCV_EXTENSIONS_CTOR(scv4tlm::tlm_gp_data) {
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
    }
};

template <> class scv_extensions<tlm::tlm_dmi::dmi_access_e> : public scv_enum_base<tlm::tlm_dmi::dmi_access_e> {
public:
    SCV_ENUM_CTOR(tlm::tlm_dmi::dmi_access_e) {
        SCV_ENUM(tlm::tlm_dmi::DMI_ACCESS_NONE);
        SCV_ENUM(tlm::tlm_dmi::DMI_ACCESS_READ);
        SCV_ENUM(tlm::tlm_dmi::DMI_ACCESS_WRITE);
        SCV_ENUM(tlm::tlm_dmi::DMI_ACCESS_READ_WRITE);
    }
};

template <> class scv_extensions<scv4tlm::tlm_dmi_data> : public scv_extensions_base<scv4tlm::tlm_dmi_data> {
public:
    scv_extensions<unsigned char *> dmi_ptr;
    scv_extensions<sc_dt::uint64> dmi_start_address;
    scv_extensions<sc_dt::uint64> dmi_end_address;
    scv_extensions<tlm::tlm_dmi::dmi_access_e> dmi_access;
    scv_extensions<sc_dt::uint64> dmi_read_latency;
    scv_extensions<sc_dt::uint64> dmi_write_latency;
    SCV_EXTENSIONS_CTOR(scv4tlm::tlm_dmi_data) {
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
