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

#ifndef TLM_GP_DATA_H_
#define TLM_GP_DATA_H_

#include <assert.h>
#include <tlm>

namespace scv4tlm {

class tlm_gp_data {
public:
    //---------------
    // Constructors
    //---------------

    // Default constructor
    tlm_gp_data() = default;

    void reset() {
        // should the other members be reset too?
        gp_option = tlm::TLM_MIN_PAYLOAD;
    };

    // disabled copy ctor
    tlm_gp_data(const tlm_gp_data &x) = default;

    tlm_gp_data(const tlm::tlm_generic_payload &x)
    : address(x.get_address())
    , command(x.get_command())
    , data(x.get_data_ptr())
    , data_length(x.get_data_length())
    , response_status(x.get_response_status())
    , dmi_allowed(x.is_dmi_allowed())
    , byte_enable(x.get_byte_enable_ptr())
    , byte_enable_length(x.get_byte_enable_length())
    , streaming_width(x.get_streaming_width())
    , gp_option(x.get_gp_option()) {}

    // Assignment operator needed for SCV introspection
    tlm_gp_data &operator=(const tlm_gp_data &x) {
        command = x.command;
        address = x.address;
        data = x.data;
        data_length = x.data_length;
        response_status = x.response_status;
        byte_enable = x.byte_enable;
        byte_enable_length = x.byte_enable_length;
        streaming_width = x.streaming_width;
        gp_option = x.gp_option;
        dmi_allowed = x.dmi_allowed;
        return (*this);
    }

    /**
     * update the payload from the tlm_gp_data. Esp. usefull when used in randomization.
     * @param other the tlm_generic_payload to update
     * @param replace_pointers if set to true the ownership of the data and byte_enable pointers is transferred
     */
    void update_generic_payload(tlm::tlm_generic_payload &other, bool transfer_ownership = false) {
        other.set_command(command);
        other.set_address(address);
        other.set_data_length(data_length);
        other.set_byte_enable_length(byte_enable_length);
        other.set_streaming_width (streaming_width);
        other.set_gp_option(gp_option);
        other.set_dmi_allowed(dmi_allowed);
        other.set_response_status(response_status);
        if(transfer_ownership){
        	other.set_byte_enable_ptr(byte_enable);
        	other.set_data_ptr(data);
        	byte_enable=nullptr;
        	data=nullptr;
        }
    }

    //--------------
    // Destructor
    //--------------
    virtual ~tlm_gp_data() {
    }

    std::string get_response_string() const {
        switch (response_status) {
        case tlm::TLM_OK_RESPONSE:
            return "TLM_OK_RESPONSE";
        case tlm::TLM_INCOMPLETE_RESPONSE:
            return "TLM_INCOMPLETE_RESPONSE";
        case tlm::TLM_GENERIC_ERROR_RESPONSE:
            return "TLM_GENERIC_ERROR_RESPONSE";
        case tlm::TLM_ADDRESS_ERROR_RESPONSE:
            return "TLM_ADDRESS_ERROR_RESPONSE";
        case tlm::TLM_COMMAND_ERROR_RESPONSE:
            return "TLM_COMMAND_ERROR_RESPONSE";
        case tlm::TLM_BURST_ERROR_RESPONSE:
            return "TLM_BURST_ERROR_RESPONSE";
        case tlm::TLM_BYTE_ENABLE_ERROR_RESPONSE:
            return "TLM_BYTE_ENABLE_ERROR_RESPONSE";
        }
        return "TLM_UNKNOWN_RESPONSE";
    }

    uint64_t get_data_value(){
    	uint64_t buf=0;
    	//FIXME: this is endianess dependent
    	for(size_t i = 0; i<data_length; i++) buf+=(*(data+i))<<i*8;
    	return buf;
    }
    // attributes are public so that scv_extension mechanism works
    sc_dt::uint64 address{0};
    tlm::tlm_command command{tlm::TLM_IGNORE_COMMAND};
    unsigned char *data{nullptr};
    unsigned int data_length{0};
    tlm::tlm_response_status response_status{tlm::TLM_INCOMPLETE_RESPONSE};
    bool dmi_allowed{false};
    unsigned char *byte_enable{nullptr};
    unsigned int byte_enable_length{0};
    unsigned int streaming_width{0};
    tlm::tlm_gp_option gp_option{tlm::TLM_MIN_PAYLOAD};
};

class tlm_dmi_data {
public:
    tlm_dmi_data() = default;

    tlm_dmi_data(tlm::tlm_dmi &dmi_data)
    : dmi_ptr(dmi_data.get_dmi_ptr())
    , dmi_start_address(dmi_data.get_start_address())
    , dmi_end_address(dmi_data.get_end_address())
    , dmi_access(dmi_data.get_granted_access())
    , dmi_read_latency(dmi_data.get_read_latency().value())
    , dmi_write_latency(dmi_data.get_write_latency().value()) {}
    //--------------
    // Destructor
    //--------------
    virtual ~tlm_dmi_data() {}

    unsigned char *dmi_ptr{nullptr};
    sc_dt::uint64 dmi_start_address{0};
    sc_dt::uint64 dmi_end_address{0};
    tlm::tlm_dmi::dmi_access_e dmi_access{tlm::tlm_dmi::DMI_ACCESS_NONE};
    sc_dt::uint64 dmi_read_latency{0};
    sc_dt::uint64 dmi_write_latency{0};
};
}
#endif /* TLM_GP_DATA_H_ */
