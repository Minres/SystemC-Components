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

#define SC_INCLUDE_DYNAMIC_PROCESSES

#include "ahb_target.h"
#include <scc/report.h>
#include <systemc>
#include <tuple>

using namespace sc_core;
using namespace tlm;
using namespace ahb;
using namespace ahb::pe;

/******************************************************************************
 * target
 ******************************************************************************/

ahb_target_b::ahb_target_b(const sc_core::sc_module_name& nm,
                           sc_core::sc_port_b<tlm::tlm_bw_transport_if<tlm_base_protocol_types>>& port,
                           size_t transfer_width)
: sc_module(nm)
, socket_bw(port) {
    add_attribute(wr_data_accept_delay);
    add_attribute(rd_addr_accept_delay);
    add_attribute(rd_data_beat_delay);
    add_attribute(rd_resp_delay);
    add_attribute(wr_resp_delay);
    dont_initialize();
    sensitive << clk_i.pos();
}

void ahb_target_b::end_of_elaboration() { clk_if = dynamic_cast<sc_core::sc_clock*>(clk_i.get_interface()); }

void ahb_target_b::b_transport(payload_type& trans, sc_time& t) {
    auto latency = operation_cb ? operation_cb(trans) : trans.is_read() ? rd_resp_delay.value : wr_resp_delay.value;
    trans.set_dmi_allowed(false);
    trans.set_response_status(tlm::TLM_OK_RESPONSE);
    clk_if = dynamic_cast<sc_core::sc_clock*>(clk_i.get_interface());
    if(clk_if) {
        t += clk_if->period() * latency;
    }
}

tlm_sync_enum ahb_target_b::nb_transport_fw(payload_type& trans, phase_type& phase, sc_time& t) {
    // TODO:
    return tlm::TLM_COMPLETED;
}

bool ahb_target_b::get_direct_mem_ptr(payload_type& trans, tlm_dmi& dmi_data) {
    trans.set_dmi_allowed(false);
    return false;
}

unsigned int ahb_target_b::transport_dbg(payload_type& trans) { return 0; }

void ahb_target_b::operation_resp(payload_type& trans, bool sync) {
    // TODO
}

void ahb_target_b::send_resp_thread() {
    while(true) {
        // waiting for responses to send
        // TODO
    }
}
