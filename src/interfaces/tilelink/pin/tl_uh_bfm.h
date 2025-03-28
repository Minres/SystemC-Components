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

#ifndef _TLBFM_H_
#define _TLBFM_H_

#include <tlm/scc/scv/tlm_rec_target_socket.h>
#include <tlm/scc/target_mixin.h>
#include <tlm>

namespace sysc {

class tl_uh_bfm : public sc_core::sc_module {
public:
    SC_HAS_PROCESS(tl_uh_bfm); // NOLINT

    enum { Get = 4, AccessAckData = 1, PutFullData = 0, PutPartialData = 1, AccessAck = 0 };

    tlm::scc::target_mixin<tlm::scc::scv::tlm_rec_target_socket<32>> socket;

    sc_core::sc_in<bool> clock;
    sc_core::sc_in<bool> reset;
    sc_core::sc_in<bool> a_ready;
    sc_core::sc_out<bool> a_valid;
    sc_core::sc_out<sc_dt::sc_uint<32>> a_bits_address;
    sc_core::sc_out<sc_dt::sc_uint<32>> a_bits_data;
    sc_core::sc_out<sc_dt::sc_uint<32>> a_bits_opcode;
    sc_core::sc_out<sc_dt::sc_uint<32>> a_bits_param;
    sc_core::sc_out<sc_dt::sc_uint<32>> a_bits_size;
    sc_core::sc_out<sc_dt::sc_uint<32>> a_bits_source;
    sc_core::sc_out<sc_dt::sc_uint<32>> a_bits_mask;
    sc_core::sc_out<bool> a_bits_corrupt;
    sc_core::sc_in<sc_dt::sc_uint<32>> d_bits_data;
    sc_core::sc_out<bool> d_ready;
    sc_core::sc_in<bool> d_valid;
    sc_core::sc_in<sc_dt::sc_uint<32>> d_bits_opcode;
    sc_core::sc_in<sc_dt::sc_uint<32>> d_bits_size;
    sc_core::sc_in<sc_dt::sc_uint<32>> d_bits_source;

    tl_uh_bfm(sc_core::sc_module_name nm, int64_t offset = 0);

    ~tl_uh_bfm() override;

private:
    const int64_t offset;
    tlm_utils::peq_with_get<tlm::tlm_generic_payload> fw_queue;
    std::deque<tlm::tlm_generic_payload*> tl_in_progress;
    void fw_thread();
    void tl_response_method();
};

} /* namespace sysc */

#endif /* _TLBFM_H_ */
