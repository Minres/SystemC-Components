/*******************************************************************************
 * Copyright 2019-2023 MINRES Technologies GmbH
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
                ", " << "PROT:0x" << std::hex << static_cast<unsigned>(e->get_protection());
    }
    os << " [ptr:" << &t << "]";
    return os;
}

}

