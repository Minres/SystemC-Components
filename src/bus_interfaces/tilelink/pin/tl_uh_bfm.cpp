/*******************************************************************************
 * Copyright (C) 2018 MINRES Technologies GmbH
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
 *
 *******************************************************************************/

#include "tl_uh_bfm.h"

#include <scc/report.h>
#include <scc/utilities.h>

namespace sysc {

using namespace sc_core;

tl_uh_bfm::tl_uh_bfm(sc_module_name nm, int64_t offset)
: sc_module(nm)
, offset(offset)
, NAMED(socket)
, NAMED(clock)
, NAMED(reset)
, NAMED(a_bits_address)
, NAMED(a_bits_data)
, NAMED(a_ready)
, NAMED(a_valid)
, NAMED(a_bits_opcode)
, NAMED(a_bits_param)
, NAMED(a_bits_size)
, NAMED(a_bits_source)
, NAMED(a_bits_mask)
, NAMED(a_bits_corrupt)
, NAMED(d_bits_data)
, NAMED(d_ready)
, NAMED(d_valid)
, NAMED(d_bits_opcode)
, NAMED(d_bits_size)
, NAMED(d_bits_source)
, NAMED(fw_queue) {
    socket.register_nb_transport_fw(
        [this](tlm::tlm_generic_payload& gp, tlm::tlm_phase& phase, sc_core::sc_time& delay) -> tlm::tlm_sync_enum {
            if(phase == tlm::BEGIN_REQ && gp.get_command() != tlm::TLM_IGNORE_COMMAND) {
                gp.acquire();
                fw_queue.notify(gp, delay);
                return tlm::TLM_ACCEPTED;
            } else if(phase == tlm::END_RESP) {
                gp.release();
                d_ready = true;
            }
        });

    SC_METHOD(tl_response_method);
    sensitive << clock.pos();
    SC_THREAD(fw_thread);
}

tl_uh_bfm::~tl_uh_bfm() = default;

void tl_uh_bfm::fw_thread() {
    d_ready = true;
    while(true) {
        a_valid = false;
        wait(fw_queue.get_event());
        auto gp = fw_queue.get_next_transaction();
        if(gp->get_data_length() == 4) {
            auto addr = gp->get_address() + offset;
            a_bits_address = addr;
            a_valid = true;
            a_bits_param = 0;
            a_bits_size = 2; // 2^2 bytes
            a_bits_source = 0x55;
            a_bits_mask = 0xf;
            a_bits_corrupt = false;
            if(gp->get_command() == tlm::TLM_WRITE_COMMAND) {
                a_bits_opcode = PutFullData;
                a_bits_data = *(uint32_t*)gp->get_data_ptr();
            } else {
                a_bits_opcode = Get;
                a_bits_data = 0;
            }
            tl_in_progress.push_back(gp);
            do {
                wait(clock.posedge_event());
            } while(a_ready == false);
        } else
            SCCERR("tlbfm") << "Got transaction with unequal length";
    }
}

void tl_uh_bfm::tl_response_method() {
    if(d_valid && d_ready) {
        //        if(d_bits_source==0x55){ // this is ours
        auto gp = tl_in_progress.front();
        sc_assert(gp && "Got TL response without a request in queue");
        tl_in_progress.pop_front();
        if(gp->get_command() == tlm::TLM_WRITE_COMMAND) {
            sc_assert(d_bits_opcode == AccessAck && "TL did not respond with AccessAck to write request");
        } else {
            sc_assert(d_bits_opcode == AccessAckData && "TL did not respond with AccessAckData to read request");
            *(uint32_t*)(gp->get_data_ptr()) = d_bits_data;
        }
        gp->set_response_status(tlm::TLM_OK_RESPONSE);
        sc_core::sc_time delay;
        tlm::tlm_phase phase{tlm::BEGIN_RESP};
        auto ret = socket->nb_transport_bw(*gp, phase, delay);
        if(ret == tlm::TLM_COMPLETED || (ret == tlm::TLM_UPDATED && phase == tlm::END_RESP)) {
            d_ready = true;
            gp->release();
        } else
            d_ready = false;
        //        }
    }
}

} /* namespace sysc */
