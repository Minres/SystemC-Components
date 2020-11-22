#include "TLM2report.h"

namespace report {
std::string print(const tlm::tlm_phase phase) {
    std::string os;
    switch(phase) {
    case tlm::BEGIN_REQ: {
        os = "BEGIN_REQ";
        break;
    }
    case tlm::END_REQ: {
        os = "END_REQ";
        break;
    }
    case tlm::BEGIN_RESP: {
        os = "BEGIN_RESP";
        break;
    }
    case tlm::END_RESP: {
        os = "END_RESP";
        break;
    }
    default: {
        os += "UNKNOWN: " + phase;
        break;
    }
    }
    return os;
}

std::string print(const tlm::tlm_sync_enum status) {
    std::string os("TLM_");
    switch(status) {
    case tlm::TLM_COMPLETED: {
        os = "COMPLETED";
        break;
    }
    case tlm::TLM_UPDATED: {
        os = "UPDATED";
        break;
    }
    case tlm::TLM_ACCEPTED: {
        os = "ACCEPTED";
        break;
    }
    default: {
        os += "UNKNOWN: " + os;
        break;
    }
    }
    return os;
}

//=====================================================================
//
///  @brief helper function for printing memory transactions
//
//=====================================================================
std::string pretty_print(tlm::tlm_generic_payload& gp, tlm::tlm_phase_enum phase, std::string StartString, std::string timeString) {
    std::ostringstream msg;
    msg.str("");

    sc_dt::uint64 print_address = gp.get_address();        // memory address
    unsigned char* print_data = gp.get_data_ptr();         // data pointer
    unsigned int data_length = gp.get_data_length();       // data length
    unsigned int word_length = gp.get_streaming_width();   // data length
    unsigned int print_length = data_length / word_length; // data length
    tlm::tlm_command print_command = gp.get_command();     // memory command
    std::string comandString;

    if(phase == tlm::BEGIN_REQ) {
        if(print_command == tlm::TLM_WRITE_COMMAND) {
            comandString = "WReq ";
        } else {
            comandString = "RReq ";
            print_length = 1;
        }
    } else {
        if(print_command == tlm::TLM_WRITE_COMMAND) {
            comandString = "WResp";
            print_length = 1;
        } else {
            comandString = "RResp";
        }
    };
    for(unsigned int i = 0; i < print_length; ++i) {
        msg << timeString << " ";
        for(unsigned j = (timeString.length() + 1) % 8; j < 8; ++j)
            msg << " ";
        msg << StartString << " ";
        for(unsigned j = (StartString.length() + 1) % 8; j < 8; ++j)
            msg << " ";
        msg << comandString;
        for(unsigned j = 5; j < 8; ++j)
            msg << " ";
        msg << dec << (i + 1) << "/" << print_length;
        for(unsigned j = 5; j < 8; ++j)
            msg << " ";
        msg << "0x" << std::internal << std::setw(sizeof(print_address) * 2) << std::setfill('0') << hex << print_address;
        msg << "    0x";
        for(unsigned int j = 0; j < (2 * word_length); j += 2) {
            msg << hex << print_data[i * word_length + j] << print_data[i * word_length + j + 1];
        };
        for(unsigned j = 4; j < 8; ++j)
            msg << " ";
        msg << gp.get_response_string() << std::endl;
    };

    return msg.str();
}

std::string print(tlm::tlm_generic_payload& gp) {
    std::ostringstream msg;
    msg.str("");

    sc_dt::uint64 print_address = gp.get_address();             // memory address
    unsigned char* print_data = gp.get_data_ptr();              // data pointer
    unsigned char* print_be = gp.get_byte_enable_ptr();         // byte enable pointer
    unsigned int print_data_length = gp.get_data_length();      // data length
    unsigned int print_be_length = gp.get_byte_enable_length(); // byte enable length
    tlm::tlm_command print_command = gp.get_command();          // command

    msg << " COMMAND: " << print_command << " LENGTH: " << dec << print_data_length << " ADDR: 0x" << std::internal
        << std::setw(sizeof(print_address) * 2) << std::setfill('0') << hex << print_address;
    if(print_data) {
        for(unsigned int i = 0; i < print_data_length; ++i) {
            msg << " Data[" << i << "]: 0x" << std::internal << std::setw(2) << std::setfill('0') << hex << (int)print_data[i];
        };
    };
    if(print_be) {
        for(unsigned int i = 0; i < print_be_length; ++i) {
            msg << " BE[" << i << "]: 0x";
            //<< std::internal << std::setw( 2 ) << std::setfill( '0' )
            //	<< hex << (int)print_be[i];
        };
    };
    msg << " RSP_STATUS: " << gp.get_response_status() << " STREAMING_WIDTH: " << dec << gp.get_streaming_width()
        << " DMI_ALLOWED: " << gp.is_dmi_allowed() << " BYTE_ENABLE_LENGTH: " << gp.get_byte_enable_length();

    return msg.str();
}

tlm::tlm_generic_payload* parse_command(std::string line) {
    tlm::tlm_generic_payload* gp = NULL;
    return gp;
}

/*
std::string
print ( axi::axi_protocol_types::tlm_payload_type& trans )
{
tlm::tlm_generic_payload my_trans(trans);
print(my_trans);
};

std::string
print ( axi::axi_protocol_types::tlm_phase& phase)
{
std::string os ( "TLM_" );
switch (phase)
  {
  case tlm::BEGIN_REQ:
    {
    os = "BEGIN_REQ";
      break;
    }
  default:
    {
      os += "UNKNOWN: " + os;
      break;
    }
}
return os;

};
*/
std::string print(tlm::tlm_dmi& dmi_properties) {
    std::ostringstream msg;
    msg.str("");
    msg << " &dmi_properties = " << &dmi_properties << std::endl
        << "      "
        << "start addr     = " << dmi_properties.get_start_address() << std::endl
        << "      "
        << "end addr       = " << dmi_properties.get_end_address() << std::endl
        << "      "
        << "read latency   = " << dmi_properties.get_read_latency() << std::endl
        << "      "
        << "write latency  = " << dmi_properties.get_write_latency() << std::endl
        << "      "
        << "granted access = " << dmi_properties.get_granted_access();
    return msg.str();
}

/*
void
sc_trace_trans (
      sc_trace_file* 		tf,
      std::string 		prefix,
      tlm_trace_payload& 	my)
{
sc_trace(tf, my.address,            prefix + ".address");
sc_trace(tf, my.command,            prefix + ".command");
sc_trace(tf, my.data_length,        prefix + ".data_length");
for (unsigned int i=0; i<64; ++i) {
    std::ostringstream buffer_tmp;
    buffer_tmp << ".data_ptr_.value" << i;
    sc_trace(tf, my.data_ptr[i],    prefix + buffer_tmp.str());
}
sc_trace(tf, my.response_status,    prefix + ".response_status");
sc_trace(tf, my.streaming_width,    prefix + ".streaming_width");
sc_trace(tf, my.byte_enable_length, prefix + ".byte_enable_length");
for (unsigned int i=0; i<64; ++i) {
    std::ostringstream buffer_tmp;
    buffer_tmp << ".byte_enable_ptr_.value" << i;
    sc_trace(tf, my.byte_enable_ptr[i], prefix + buffer_tmp.str());
};
sc_trace(tf, my.dmi_allowed,        prefix + ".dmi_allowed");
sc_trace(tf, my.phase_id,        	  prefix + ".phase_id");
sc_trace(tf, my.status,        	  prefix + ".status");
sc_trace(tf, my.is_valid,        	  prefix + ".is_valid");
};
*/
bool compare_gp(tlm::tlm_generic_payload& org, tlm::tlm_generic_payload& ref, unsigned long long org_time, unsigned long long ref_time) {
    if(org_time != ref_time) {
        std::cout << "Warning: time mismatch org/ref" << org_time << "/" << ref_time << std::endl;
    };

    if(org.get_address() != ref.get_address()) {
        std::cout << "address mismatch org/ref " << hex << org.get_address() << "/" << ref.get_address() << dec << std::endl;
        return false;
    };
    if(org.get_data_length() != ref.get_data_length()) {
        std::cout << "data length mismatch" << std::endl;
        return false;
    }; // data length

    if(org.get_command() != ref.get_command()) {
        std::cout << "command mismatch: " << org.get_command() << "/" << ref.get_command() << std::endl;
        return false;
    }; // memory command

    unsigned char *data_org, *data_ref;
    data_org = org.get_data_ptr();
    data_ref = ref.get_data_ptr();
    for(unsigned int i = 0; i < org.get_data_length(); ++i) {
        if(data_org[i] != data_ref[i]) {
            if(org.get_command() != tlm::TLM_READ_COMMAND) {
                std::cout << "Warning: data mismatch org/ref" << data_org[i] << "/" << data_ref[i] << " will be rectified" << std::endl;
            };
            data_org[i] = data_ref[i];
        };
    };
    if(org.get_response_status() != ref.get_response_status()) {
        org.set_response_status(ref.get_response_status());
    };
    if(org.get_streaming_width() != ref.get_streaming_width()) {
        std::cout << "Warning: streaming width mismatch" << std::endl;
        return false;
    };
    if(org.is_dmi_allowed() != ref.is_dmi_allowed()) {
        std::cout << "Warning: DMI flag mismatch" << std::endl;
        return false;
    };
    if(org.get_byte_enable_length() != ref.get_byte_enable_length()) {
        std::cout << "Warning: byte enable width mismatch" << std::endl;
        return false;
    };
    return true;
};
} // end namespace report
