/*******************************************************************************
 * Copyright 2020-2022 MINRES Technologies GmbH
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

#include "apb_initiator.h"
#include <scc/report.h>

using namespace sc_core;
using namespace apb;
using namespace apb::pe;

namespace {
uint8_t log2n(uint8_t siz) { return ((siz > 1) ? 1 + log2n(siz >> 1) : 0); }

} // anonymous namespace

apb_initiator_b::apb_initiator_b(sc_core::sc_module_name nm,
                                 sc_core::sc_port_b<tlm::tlm_fw_transport_if<tlm::tlm_base_protocol_types>>& port,
                                 size_t transfer_width, bool coherent)
: sc_module(nm)
, socket_fw(port)
, transfer_width_in_bytes(transfer_width / 8) {}

apb_initiator_b::~apb_initiator_b() = default;

tlm::tlm_sync_enum apb_initiator_b::nb_transport_bw(payload_type& trans, phase_type& phase, sc_core::sc_time& t) {

    peq.notify(std::make_tuple(&trans, phase), t);
    return tlm::TLM_ACCEPTED;
}

void apb_initiator_b::invalidate_direct_mem_ptr(sc_dt::uint64 start_range, sc_dt::uint64 end_range) {}

void apb_initiator_b::transport(payload_type& trans, bool blocking) {
    SCCTRACE(SCMOD) << "got transport req for id=" << &trans;
    if(blocking) {
        sc_time t;
        socket_fw->b_transport(trans, t);
    } else {
        scc::ordered_semaphore::lock lock(chnl);
        SCCTRACE(SCMOD) << "start transport req for id=" << &trans;
        trans.free_all_extensions();
        tlm::tlm_phase phase{tlm::BEGIN_REQ};
        sc_time t;
        auto res = socket_fw->nb_transport_fw(trans, phase, t);
        if(res == tlm::TLM_COMPLETED || (res == tlm::TLM_UPDATED && phase != tlm::END_REQ && phase != tlm::BEGIN_RESP))
            SCCFATAL(SCMOD) << "target did not respsond with END_REQ or BEGIN_RESP to a BEGIN_REQ";
        if(res != tlm::TLM_UPDATED || phase != tlm::BEGIN_RESP) {
            payload_type* gp{nullptr};
            while(phase != tlm::BEGIN_RESP) {
                std::tie(gp, phase) = peq.get();
                if(gp != &trans)
                    SCCFATAL(SCMOD) << "target did send the wrong transaction";
                if(phase != tlm::END_REQ && phase != tlm::BEGIN_RESP)
                    SCCFATAL(SCMOD) << "target did not respsond with END_REQ or BEGIN_RESP to a BEGIN_REQ";
                if(phase == tlm::END_REQ)
                    wait(clk_i.posedge_event());
            }
        }
        phase = tlm::END_RESP;
        t = SC_ZERO_TIME;
        socket_fw->nb_transport_fw(trans, phase, t);
        SCCTRACE(SCMOD) << "finished non-blocking protocol";
        any_tx_finished.notify(SC_ZERO_TIME);
    }
    SCCTRACE(SCMOD) << "finished transport req for id=" << &trans;
}
