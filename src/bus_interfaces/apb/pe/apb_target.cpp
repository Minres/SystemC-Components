/*******************************************************************************
 * Copyright 2019-2022 MINRES Technologies GmbH
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

#include "apb_target.h"
#include <scc/report.h>
#include <systemc>
#include <tuple>

using namespace sc_core;
using namespace tlm;
using namespace apb::pe;

/******************************************************************************
 * target
 ******************************************************************************/

apb_target_b::apb_target_b(const sc_core::sc_module_name& nm,
                           sc_core::sc_port_b<tlm::tlm_bw_transport_if<tlm_base_protocol_types>>& port,
                           size_t transfer_width)
: sc_module(nm)
, socket_bw(port) {
    SC_METHOD(response);
    dont_initialize();
    sensitive << clk_i.pos();
}

void apb_target_b::end_of_elaboration() { clk_if = dynamic_cast<sc_core::sc_clock*>(clk_i.get_interface()); }

void apb_target_b::b_transport(payload_type& trans, sc_time& t) {
    if(operation_cb)
        operation_cb(trans);
    trans.set_dmi_allowed(false);
    trans.set_response_status(tlm::TLM_OK_RESPONSE);
    clk_if = dynamic_cast<sc_core::sc_clock*>(clk_i.get_interface());
    if(clk_if) {
        t += 1 * clk_if->period();
    }
}

tlm_sync_enum apb_target_b::nb_transport_fw(payload_type& trans, phase_type& phase, sc_time& t) {
    if(phase == tlm::BEGIN_REQ) {
        sc_assert(active_tx == nullptr);
        if(operation_cb)
            operation_cb(trans);
        active_tx = &trans;
        if(trans.has_mm())
            trans.acquire();
        if(mhndl.valid())
            mhndl.enable();
        phase = tlm::END_REQ;
        t += sc_time(clk_if ? clk_if->period() - 1_ps : SC_ZERO_TIME);
        return tlm::TLM_UPDATED;
    } else if(phase == tlm::END_REQ) {
        sc_assert(active_tx == &trans);
        if(active_tx->has_mm())
            active_tx->release();
        active_tx = nullptr;
        return tlm::TLM_COMPLETED;
    }
    return tlm::TLM_ACCEPTED;
}

bool apb_target_b::get_direct_mem_ptr(payload_type& trans, tlm_dmi& dmi_data) {
    trans.set_dmi_allowed(false);
    return false;
}

unsigned int apb_target_b::transport_dbg(payload_type& trans) { return 0; }

void apb_target_b::response() {
    if(!mhndl.valid())
        mhndl = sc_get_current_process_handle();
    if(active_tx) {
        tlm_phase phase{tlm::BEGIN_RESP};
        sc_time delay;
        auto ret = socket_bw->nb_transport_bw(*active_tx, phase, delay);
        if((ret == tlm::TLM_UPDATED && phase == tlm::END_RESP) || ret == tlm::TLM_COMPLETED) {
            if(active_tx->has_mm())
                active_tx->release();
            active_tx = nullptr;
        }
        if(mhndl.valid())
            mhndl.disable();
    }
}
