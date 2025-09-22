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
#include <fmt/format.h>
#include <tlm/scc/scv/tlm_extension_recording_registry.h>
#include <tlm/scc/tlm_id.h>

namespace cxs {

class cxs_ext_recording : public tlm::scc::scv::tlm_extensions_recording_if<cxs_flit_types> {

    void recordBeginTx(SCVNS scv_tr_handle& h, cxs_flit_types::tlm_payload_type& trans) override {
        if(auto ext = trans.get_extension<orig_pkt_extension>()) { // CTRL, DATA, RESP
            auto idx = 0U;
            for(auto& pkt : ext->orig_ext) {
                h.record_attribute(fmt::format("flit.packet{}.size", idx++).c_str(), pkt->get_data().size());
                if(pkt->get_extension_count())
                    for(auto& extensionRecording : tlm::scc::scv::tlm_extension_recording_registry<cxs_packet_types>::inst().get())
                        if(extensionRecording)
                            extensionRecording->recordBeginTx(h, *pkt);
            }
        }
    }

    void recordEndTx(SCVNS scv_tr_handle& handle, cxs_flit_types::tlm_payload_type& trans) override {}
};
#if defined(__GNUG__)
__attribute__((constructor))
#endif
bool register_extensions() {
    cxs::orig_pkt_extension ext; // NOLINT
    if(!tlm::scc::scv::tlm_extension_recording_registry<cxs::cxs_flit_types>::inst().is_ext_registered(ext.ID))
        tlm::scc::scv::tlm_extension_recording_registry<cxs::cxs_flit_types>::inst().register_ext_rec(
            ext.ID,
            new cxs::cxs_ext_recording()); // NOLINT
    return true;                           // NOLINT
}
bool registered = register_extensions();
} // namespace cxs
