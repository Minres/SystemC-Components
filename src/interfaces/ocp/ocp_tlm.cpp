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

#include <apb/apb_tlm.h>
#include <array>

namespace ocp {
namespace {
const std::array<std::string, 3> cmd_str{"R", "W", "I"};
}
std::ostream& operator<<(std::ostream& os, const tlm::tlm_generic_payload& t) {
    os << "CMD:" << cmd_str[t.get_command()] << ", "
       << "ADDR:0x" << std::hex << t.get_address() << ", TXLEN:0x" << t.get_data_length();
    if(auto e = t.get_extension<apb::apb_extension>()) {
        os << ", "
           << "PROT:0x" << std::hex << static_cast<unsigned>(e->get_protection()) << "NSE:" << (e->is_nse() ? "True" : "False");
    }
    os << " [ptr:" << &t << "]";
    return os;
}

} // namespace ocp
