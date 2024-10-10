/*******************************************************************************
 * Copyright 2019-2024 MINRES Technologies GmbH
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

#include <array>
#include <tilelink/tl_tlm.h>

namespace tilelink {
namespace {
const std::array<std::string, 3> cmd_str{"R", "W", "I"};
}

template <> const char* to_char<opcode_e>(opcode_e v) {
    switch(v) {
    case opcode_e::Get:
        return "Get";
    case opcode_e::AccessAckData:
        return "AccessAckData";
    case opcode_e::PutFullData:
        return "PutFullData";
    case opcode_e::PutPartialData:
        return "PutPartialData";
    case opcode_e::AccessAck:
        return "AccessAck";
    case opcode_e::ArithmeticData:
        return "ArithmeticData";
    case opcode_e::LogicalData:
        return "LogicalData";
    case opcode_e::Intent:
        return "Intent";
    case opcode_e::HintAck:
        return "HintAck";
    case opcode_e::AcquireBlock:
        return "AcquireBlock";
    case opcode_e::AcquirePerm:
        return "AcquirePerm";
    case opcode_e::Grant:
        return "Grant";
    case opcode_e::GrantData:
        return "GrantData";
    case opcode_e::GrantAck:
        return "GrantAck";
    case opcode_e::ProbeBlock:
        return "ProbeBlock";
    case opcode_e::ProbePerm:
        return "ProbePerm";
    case opcode_e::ProbeAck:
        return "ProbeAck";
    case opcode_e::ProbeAckData:
        return "ProbeAckData";
    case opcode_e::Release:
        return "Release";
    case opcode_e::ReleaseData:
        return "ReleaseData";
    case opcode_e::ReleaseAck:
        return "ReleaseAck";
    default:
        return "UNKNOWN";
    }
}

std::ostream& operator<<(std::ostream& os, const tlm::tlm_generic_payload& t) {
    os << "CMD:" << cmd_str[t.get_command()] << ", "
       << "ADDR:0x" << std::hex << t.get_address() << ", TXLEN:0x" << t.get_data_length();
    if(auto e = t.get_extension<tilelink::tilelink_extension>()) {
        os << ", "
           << "PROT:0x" << std::hex << static_cast<unsigned>(e->get_protection()) << "NSE:" << (e->is_nse() ? "True" : "False");
    }
    os << " [ptr:" << &t << "]";
    return os;
}

} // namespace tilelink
