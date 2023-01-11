/*******************************************************************************
 * Copyright 2016-2022 MINRES Technologies GmbH
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

#include <tlm/scc/lwtr/lwtr4tlm2_extension_registry.h>
#include "tlm2_lwtr.h"
#include <tlm/scc/tlm_id.h>

namespace lwtr { template <class Archive>
void record(Archive &ar, tlm::scc::tlm_id_extension const& e) {ar & field("uid", e.id);}}

namespace tlm {
namespace scc {
namespace lwtr {
namespace {
const std::array<std::string, 3> cmd2char{{"READ", "WRITE", "IGNORE"}};
const std::array<std::string, 7> resp2char{
    {"OK", "INCOMPLETE", "GENERIC_ERROR", "ADDRESS_ERROR", "COMMAND_ERROR", "BURST_ERROR", "BYTE_ENABLE_ERROR"}};
const std::array<std::string, 3> gp_option2char{{"MIN_PAYLOAD", "FULL_PAYLOAD", "FULL_PAYLOAD_ACCEPTED"}};
const std::array<std::string, 5> phase2char{{"UNINITIALIZED_PHASE", "BEGIN_REQ", "END_REQ", "BEGIN_RESP", "END_RESP"}};
const std::array<std::string, 4> dmi2char{
    {"DMI_ACCESS_NONE", "DMI_ACCESS_READ", "DMI_ACCESS_WRITE", "DMI_ACCESS_READ_WRITE"}};
const std::array<std::string, 3> sync2char{{"ACCEPTED", "UPDATED", "COMPLETED"}};

} // namespace

class tlm_id_ext_recording : public tlm::scc::lwtr::lwtr4tlm2_extension_registry_if<tlm::tlm_base_protocol_types> {
    void recordBeginTx(::lwtr::tx_handle& handle, tlm::tlm_base_protocol_types::tlm_payload_type& trans) override {
        if(auto* ext = trans.get_extension<tlm::scc::tlm_id_extension>())
            handle.record_attribute("trans", *ext);
    }
    void recordEndTx(::lwtr::tx_handle& handle, tlm::tlm_base_protocol_types::tlm_payload_type& trans) override {
    }
};

//using namespace tlm::scc::scv;
#if defined(__GNUG__)
__attribute__((constructor))
#endif
bool register_extensions() {
    tlm::scc::tlm_id_extension ext(nullptr); // NOLINT
    lwtr4tlm2_extension_registry<tlm::tlm_base_protocol_types>::inst().register_ext_rec(
        ext.ID, new tlm_id_ext_recording()) ; // NOLINT
    return true;                                  // NOLINT
}
bool registered = register_extensions();

} // namespace lwtr
} // namespace scc
} // namespace tlm
