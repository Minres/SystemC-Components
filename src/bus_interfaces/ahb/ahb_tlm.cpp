/*
 * ahb_tlm.cpp
 *
 *  Created on: Dec 21, 2023
 *      Author: eyck
 */

#include "ahb_tlm.h"

namespace ahb {
namespace {
const std::array<std::string, 3> cmd_str{"R", "W", "I"};
}
template <> const char* to_char<burst_e>(burst_e v) {
    switch(v) {
    case burst_e::SINGLE:
        return "SINGLE";
    case burst_e::INCR:
        return "INCR";
    case burst_e::INCR4:
        return "INCR4";
    case burst_e::INCR8:
        return "INCR8";
    case burst_e::INCR16:
        return "INCR16";
    case burst_e::WRAP4:
        return "WRAP4";
    case burst_e::WRAP8:
        return "WRAP8";
    case burst_e::WRAP16:
        return "WRAP16";
    default:
        return "UNKNOWN";
    }
}

template <> const char* to_char<resp_e>(resp_e v) {
    switch(v) {
    case resp_e::OKAY:
        return "OKAY";
    case resp_e::EXOKAY:
        return "EXOKAY";
    case resp_e::DECERR:
        return "DECERR";
    case resp_e::SLVERR:
        return "SLVERR";
    default:
        return "UNKNOWN";
    }
}

std::ostream& operator<<(std::ostream& os, const tlm::tlm_generic_payload& t) {
    os << "CMD:" << cmd_str[t.get_command()] << ", " << "ADDR:0x" << std::hex << t.get_address() << ", TXLEN:0x"
       << t.get_data_length();
    if(auto e = t.get_extension<ahb::ahb_extension>()) {
        os << ", " <<  "BURST:" << to_char(e->get_burst()) <<
                ", " << (e->is_seq()?"SEQ":"NONSEQ") <<
                ", " << "MSTLOCK:"<<e->is_locked() <<
                ", " << "PROT:0x" << std::hex << e->get_protection();
    }
    os << " [ptr:" << &t << "]";
    return os;
}

}

