#pragma once
#include <tlm>
#include <lwtr/lwtr.h>

namespace lwtr {

template<> struct value_converter<tlm::tlm_phase> {
	static value to_value(tlm::tlm_phase const& v) {
	    if(v==tlm::UNINITIALIZED_PHASE) return "tlm::UNINITIALIZED_PHASE";
	    if(v==tlm::BEGIN_REQ) return "tlm::BEGIN_REQ";
        if(v==tlm::END_REQ) return "tlm::END_REQ";
        if(v==tlm::BEGIN_RESP) return "tlm::BEGIN_RESP";
        if(v==tlm::END_RESP) return "tlm::END_RESP";
		return value("ILLEGAL");
	}
};
template<> struct value_converter<tlm::tlm_sync_enum> {
	static value to_value(tlm::tlm_sync_enum v) {
		switch(v){
		case tlm::TLM_ACCEPTED:return value("tlm::TLM_ACCEPTED");
		case tlm::TLM_UPDATED:return value("tlm::TLM_UPDATED");
		case tlm::TLM_COMPLETED:return value("tlm::TLM_COMPLETED");
		default:return value("ILLEGAL");
		}
	}
};
template<> struct value_converter<tlm::tlm_command> {
	static value to_value(tlm::tlm_command v) {
		switch(v){
		case tlm::TLM_READ_COMMAND:return value("tlm::TLM_READ_COMMAND");
		case tlm::TLM_WRITE_COMMAND:return value("tlm::TLM_WRITE_COMMAND");
		case tlm::TLM_IGNORE_COMMAND:return value("tlm::TLM_IGNORE_COMMAND");
		default:return value("ILLEGAL");
		}
	}
};
template<> struct value_converter<tlm::tlm_response_status> {
	static value to_value(tlm::tlm_response_status v) {
		switch(v){
		case tlm::TLM_OK_RESPONSE:return value("tlm::TLM_OK_RESPONSE");
		case tlm::TLM_INCOMPLETE_RESPONSE:return value("tlm::TLM_INCOMPLETE_RESPONSE");
		case tlm::TLM_GENERIC_ERROR_RESPONSE:return value("tlm::TLM_GENERIC_ERROR_RESPONSE");
		case tlm::TLM_ADDRESS_ERROR_RESPONSE:return value("tlm::TLM_ADDRESS_ERROR_RESPONSE");
		case tlm::TLM_COMMAND_ERROR_RESPONSE:return value("tlm::TLM_COMMAND_ERROR_RESPONSE");
		case tlm::TLM_BURST_ERROR_RESPONSE:return value("tlm::TLM_BURST_ERROR_RESPONSE");
		case tlm::TLM_BYTE_ENABLE_ERROR_RESPONSE:return value("tlm::TLM_BYTE_ENABLE_ERROR_RESPONSE");
		default:return value("ILLEGAL");
		}
	}
};
template<> struct value_converter<tlm::tlm_gp_option> {
	static value to_value(tlm::tlm_gp_option v) {
		switch(v){
		case tlm::TLM_MIN_PAYLOAD:return value("tlm::TLM_MIN_PAYLOAD");
		case tlm::TLM_FULL_PAYLOAD:return value("tlm::TLM_FULL_PAYLOAD");
		case tlm::TLM_FULL_PAYLOAD_ACCEPTED:return value("tlm::TLM_FULL_PAYLOAD_ACCEPTED");
		default:return value("ILLEGAL");
		}
	}
};
template<> struct value_converter<tlm::tlm_dmi::dmi_access_e> {
	static value to_value(tlm::tlm_dmi::dmi_access_e v) {
		switch(v){
		case tlm::tlm_dmi::DMI_ACCESS_NONE:return value("tlm::DMI_ACCESS_NONE");
		case tlm::tlm_dmi::DMI_ACCESS_READ:return value("tlm::DMI_ACCESS_READ");
		case tlm::tlm_dmi::DMI_ACCESS_WRITE:return value("tlm::DMI_ACCESS_WRITE");
		case tlm::tlm_dmi::DMI_ACCESS_READ_WRITE:return value("tlm::DMI_ACCESS_READ_WRITE");
		default:return value("ILLEGAL");
		}
	}
};

template <class Archive>
void record(Archive &ar, tlm::tlm_dmi const& u) {
	ar & field("start_address", u.get_start_address()) & field("end_address", u.get_end_address()) & field("access", u.get_granted_access());
}
template <class Archive>
void record(Archive &ar, tlm::tlm_generic_payload const& u) {
	ar & field("command", u.get_command());
	ar & field("address", u.get_address());
	//ar & field("data_ptr", u.get_data_ptr());
	ar & field("data_length", u.get_data_length());
	if(u.get_data_length() && u.get_data_ptr()) {
		uint64_t data{0};
		std::copy(u.get_data_ptr(), u.get_data_ptr()+std::min(u.get_data_length(), 8U),
				reinterpret_cast<uint8_t*>(&data));
		ar & field("data", data);
	}
	ar & field("response_status", u.get_response_status());
    ar & field("streaming_width", u.get_streaming_width());
    //ar & field("byte_enable_ptr", u.get_byte_enable_ptr());
    ar & field("byte_enable_length", u.get_byte_enable_length());
    ar & field("dmi_allowed", u.is_dmi_allowed());
    ar & field("gp_option", u.get_gp_option());
}
}
