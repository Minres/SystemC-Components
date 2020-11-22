#include "AXIreport.h"

namespace report {

//=====================================================================
//
///  @brief helper function for printing AXI extension
//
//=====================================================================
std::string printAXI(const axi::axi_protocol_types::tlm_phase_type& phase) {
    std::string os;
    if(phase == tlm::BEGIN_REQ) {
        os = "BEGIN_REQ";
    }
    if(phase == axi::BEGIN_PARTIAL_REQ) {
        os = "BEGIN_PARTIAL_REQ";
    }
    if(phase == tlm::END_REQ) {
        os = "END_REQ";
    }
    if(phase == axi::END_PARTIAL_REQ) {
        os = "END_PARTIAL_REQ";
    }
    if(phase == tlm::BEGIN_RESP) {
        os = "BEGIN_RESP";
    }
    if(phase == axi::BEGIN_PARTIAL_RESP) {
        os = "BEGIN_PARTIAL_RESP";
    }
    if(phase == tlm::END_RESP) {
        os = "END_RESP";
    }
    if(phase == axi::END_PARTIAL_RESP) {
        os = "END_PARTIAL_RESP";
    }
    return os;
};

std::string pretty_print(axi::axi4_extension& gp, tlm::tlm_phase_enum phase, std::string StartString, std::string timeString) {
    std::ostringstream msg;
    msg.str("");

    //unsigned int id = gp.get_id();
    // unsigned int user = gp.get_user();
    /*  bool exclusive    = gp.is_exclusive();
    bool bufferable   = gp.is_bufferable();
    bool modifiable   = gp.is_modifiable();
    bool read_other_allocate  = gp.is_read_other_allocate();
    bool write_other_allocate = gp.is_write_other_allocate();
    uint8_t length    = gp.get_length();
    uint8_t size      = gp.get_size();
    //burst_e burst     = gp.get_burst();
    uint8_t prot      = gp.get_prot();
    bool  privileged  = gp.is_privileged();
    bool  non_secure  = gp.is_non_secure();
    bool  instruction = gp.is_instruction();
    uint8_t   qos     = gp.get_qos();
    uint8_t   region  = gp.get_region();
    //domain_e  domain  = gp.get_domain();
    //snoop_e   snoop   = gp.get_snoop();
    uint8_t barrier   = gp.get_barrier();
    uint8_t lock      = gp.get_lock();
    uint8_t cache     = gp.get_cache();
    uint8_t unique    = gp.get_unique();
    //resp_e resp       = gp.get_resp();
    bool okay         = gp.is_okay();
    bool exokay       = gp.is_exokay();
    bool slverr       = gp.is_slverr();
    bool decerr       = gp.is_decerr();
    bool pass_dirty          = gp.is_pass_dirty();
    bool shared              = gp.is_shared();
    bool snoop_data_transfer = gp.is_snoop_data_transfer();
    bool snoop_error         = gp.is_snoop_error();
    bool snoop_was_unique    = gp.is_snoop_was_unique();
    */
    //msg << "ID = " << id;

    return msg.str();
}

std::string print(axi::axi4_extension& gp) {
    std::ostringstream msg;
    // msg.str("");
    //unsigned int id = gp.get_id();
    // unsigned int user = gp.get_user();
    /* bool exclusive    = gp.is_exclusive();
    bool bufferable   = gp.is_bufferable();
    bool modifiable   = gp.is_modifiable();
    bool read_other_allocate  = gp.is_read_other_allocate();
    bool write_other_allocate = gp.is_write_other_allocate();
    uint8_t length    = gp.get_length();
    uint8_t size      = gp.get_size();
    //burst_e burst     = gp.get_burst();
    uint8_t prot      = gp.get_prot();
    bool  privileged  = gp.is_privileged();
    bool  non_secure  = gp.is_non_secure();
    bool  instruction = gp.is_instruction();
    uint8_t   qos     = gp.get_qos();
    uint8_t   region  = gp.get_region();
    //domain_e  domain  = gp.get_domain();
    //snoop_e   snoop   = gp.get_snoop();
    uint8_t barrier   = gp.get_barrier();
    uint8_t lock      = gp.get_lock();
    uint8_t cache     = gp.get_cache();
    uint8_t unique    = gp.get_unique();
    //resp_e resp       = gp.get_resp();
    bool okay         = gp.is_okay();
    bool exokay       = gp.is_exokay();
    bool slverr       = gp.is_slverr();
    bool decerr       = gp.is_decerr();
    bool pass_dirty          = gp.is_pass_dirty();
    bool shared              = gp.is_shared();
    bool snoop_data_transfer = gp.is_snoop_data_transfer();
    bool snoop_error         = gp.is_snoop_error();
    bool snoop_was_unique    = gp.is_snoop_was_unique(); */
    //msg << " ID: " << id;
    // msg << " USER: " << user;

    return msg.str();
}

bool compare_gpaxi(tlm::tlm_generic_payload& org, tlm::tlm_generic_payload& ref, unsigned long long org_time, unsigned long long ref_time) {
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
