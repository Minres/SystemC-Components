/*
 * Copyright 2020 Arteris IP
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
 */

#include "cxs/cxs_tlm.h"
#include "util/ities.h"
#include <fmt/format.h>
#include <tlm/scc/scv/tlm_extension_recording_registry.h>
#include <tlm/scc/tlm_id.h>

namespace cxs {
using namespace tlm::scc::scv;

struct cxs_ext_record : public tlm_extension_record_if {

    cxs_ext_record() { recordBegin = &recordBeginTx; }

    static void recordBeginTx(SCVNS scv_tr_handle& handle, tlm::tlm_extension_base* e, std::string const& prefix) {
        if(auto ext = dynamic_cast<cxs_packet_extension*>(e)) {
            auto idx = 0U;
            for(auto const& pkt : ext->packets) {
                auto prf = fmt::format("{}packet{}.", prefix, idx++);
                handle.record_attribute(fmt::format("{}size", prf).c_str(), pkt->get_data().size());
                if(pkt->get_extension_count()) {
                    auto size = tlm_extension_record_registry::get().size();
                    for(auto i = 0u; i < size; ++i)
                        if(auto orig_ext = pkt->get_extension(i))
                            tlm_extension_record_registry::get().recordBeginTx(i, handle, orig_ext, prf);
                }
            }
        }
    }
};

struct cxs_ext_recording : public tlm_extensions_recording_if<cxs_flit_types> {

    cxs_ext_recording() { recordBegin = &recordBeginTx; }

    static void recordBeginTx(SCVNS scv_tr_handle& h, cxs_flit_types::tlm_payload_type& trans) {
        tlm_extension_record_registry::get().recordBeginTx(cxs_packet_extension::ID, h, trans.get_extension<cxs_packet_extension>(),
                                                           "flit.");
    }
};
#if defined(__GNUG__)
__attribute__((constructor))
#endif
bool register_extensions() {
    cxs::cxs_packet_extension ext; // NOLINT
    if(!tlm_extension_recording_registry<cxs::cxs_flit_types>::get().is_ext_registered(ext.ID))
        tlm_extension_recording_registry<cxs::cxs_flit_types>::get().register_ext_rec(
            ext.ID,
            util::make_unique<cxs::cxs_ext_recording>()); // NOLINT
    if(!tlm_extension_record_registry::get().is_ext_registered(ext.ID))
        tlm_extension_record_registry::get().register_ext_rec(ext.ID, util::make_unique<cxs_ext_record>()); // NOLINT
    return true;                                                                                            // NOLINT
}
bool registered = register_extensions();
} // namespace cxs
