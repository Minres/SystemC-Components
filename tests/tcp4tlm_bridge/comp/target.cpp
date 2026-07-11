/*******************************************************************************
 * Copyright 2026 MINRES Technologies GmbH
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

#include "target.h"
#include <sstream>
namespace tlm {
namespace scc {
namespace tcp {

target::target(sc_core::sc_module_name mn)
: sc_module(mn)
, regs(4) {
    tsckt.register_b_transport(this, &target::btransport_cb);
}

void target::btransport_cb(tlm::tlm_generic_payload& gp, sc_core::sc_time& d) {
    std::stringstream buf;
    unsigned char* reg_raw = ((unsigned char*)&regs[0]) + gp.get_address();

    if(gp.get_command() == tlm::TLM_READ_COMMAND) {
        buf << "Got read request @" << sc_core::sc_time_stamp();
        std::copy(reg_raw, reg_raw + gp.get_data_length(), gp.get_data_ptr());
        gp.set_response_status(tlm::TLM_OK_RESPONSE);

        if(gp.get_address() == 0) {
            *reg_raw = 0;
        }
    } else if(gp.get_command() == tlm::TLM_WRITE_COMMAND) {
        buf << "Got write request @" << sc_core::sc_time_stamp();
        std::copy(gp.get_data_ptr(), gp.get_data_ptr() + gp.get_data_length(), reg_raw);
        gp.set_response_status(tlm::TLM_OK_RESPONSE);
    } else {
        gp.set_response_status(tlm::TLM_GENERIC_ERROR_RESPONSE);
    }

    SC_REPORT_INFO("slave", buf.str().c_str());
};
} // namespace tcp
} // namespace scc
} // namespace tlm
