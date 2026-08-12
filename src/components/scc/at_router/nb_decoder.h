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

#ifndef _SYSC_NB_ROUTER_NB_DECODER_H_
#define _SYSC_NB_ROUTER_NB_DECODER_H_

#include "types.h"
#include <scc/report.h>
#include <tlm>
#include <util/range_lut.h>

namespace scc {
namespace at_router {
template <typename TYPES = tlm::tlm_base_protocol_types>
struct nb_decoder : public tlm::tlm_fw_nonblocking_transport_if<typename TYPES::tlm_payload_type, typename TYPES::tlm_phase_type>,
                    public tlm::tlm_bw_nonblocking_transport_if<typename TYPES::tlm_payload_type, typename TYPES::tlm_phase_type> {
    using this_class = nb_decoder<TYPES>;
    using tlm_generic_payload = typename TYPES::tlm_payload_type;
    using tlm_phase = typename TYPES::tlm_phase_type;

    t_port<TYPES> tport{"tport"};

    sc_core::sc_vector<i_port<TYPES>> iport;

    nb_decoder(char const* name, util::range_lut<unsigned> const& decoder)
    : tport(name)
    , iport((std::string(name) + "_iport").c_str())
    , decoder(decoder) {
        tport.fw.bind(*this);
    }
    tlm::tlm_sync_enum nb_transport_fw(tlm_generic_payload& trans, tlm_phase& phase, sc_core::sc_time& t) override {
        auto addr = trans.get_address();
        auto idx = decoder.getEntry(addr);
        if(idx == decoder.null_entry) {
            trans.set_response_status(tlm::TLM_ADDRESS_ERROR_RESPONSE);
            if(phase == tlm::BEGIN_REQ)
                phase = tlm::END_RESP;
            return tlm::TLM_COMPLETED;
        }
        return iport[idx].fw->nb_transport_fw(trans, phase, t);
    }

    tlm::tlm_sync_enum nb_transport_bw(tlm_generic_payload& trans, tlm_phase& phase, sc_core::sc_time& t) override {
        return tport.bw->nb_transport_bw(trans, phase, t);
    }

private:
    i_port<TYPES>& add_isckt() {
        auto* sckt = new i_port<TYPES>("iport" + iport.size());
        iport.push_back(sckt);
        sckt->bw.bind(*this);
        return *sckt;
    }

    util::range_lut<unsigned> const& decoder;
};
} // namespace at_router
} // namespace scc
#endif