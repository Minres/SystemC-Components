/*******************************************************************************
 * Copyright (C) 2020, MINRES Technologies GmbH
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 * Contributors:
 *       eyck@minres.com - initial API and implementation
 ******************************************************************************/

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
