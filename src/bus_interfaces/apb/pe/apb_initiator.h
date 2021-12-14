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

#ifndef _TLM_APB_PE_APB_INITIATOR_H_
#define _TLM_APB_PE_APB_INITIATOR_H_

#include <scc/ordered_semaphore.h>
#include <scc/peq.h>
#include <tlm>

//! TLM2.0 components modeling APB
namespace apb {
//! protocol engine implementations
namespace pe {

class apb_initiator_b : public sc_core::sc_module, public tlm::tlm_bw_transport_if<tlm::tlm_base_protocol_types> {
public:
    SC_HAS_PROCESS(apb_initiator_b);

    using payload_type = tlm::tlm_generic_payload;
    using phase_type = tlm::tlm_phase;

    sc_core::sc_in<bool> clk_i{"clk_i"};

    tlm::tlm_sync_enum nb_transport_bw(payload_type& trans, phase_type& phase, sc_core::sc_time& t);

    void invalidate_direct_mem_ptr(sc_dt::uint64 start_range, sc_dt::uint64 end_range);

    size_t get_transferwith_in_bytes() const { return transfer_width_in_bytes; }
    /**
     * @brief The forward transport function. It behaves blocking and is re-entrant.
     *
     * This function initiates the forward transport either using b_transport() if blocking=true
     *  or the nb_transport_* interface.
     *
     * @param trans the transaction to send
     * @param blocking execute in using the blocking interface
     */
    void transport(payload_type& trans, bool blocking);

    apb_initiator_b(sc_core::sc_module_name nm,
                    sc_core::sc_port_b<tlm::tlm_fw_transport_if<tlm::tlm_base_protocol_types>>& port,
                    size_t transfer_width, bool coherent);

    virtual ~apb_initiator_b();

    apb_initiator_b() = delete;

    apb_initiator_b(apb_initiator_b const&) = delete;

    apb_initiator_b(apb_initiator_b&&) = delete;

    apb_initiator_b& operator=(apb_initiator_b const&) = delete;

    apb_initiator_b& operator=(apb_initiator_b&&) = delete;

protected:
    const size_t transfer_width_in_bytes;

    sc_core::sc_port_b<tlm::tlm_fw_transport_if<tlm::tlm_base_protocol_types>>& socket_fw;

    scc::peq<std::tuple<payload_type*, tlm::tlm_phase>> peq;

    scc::ordered_semaphore_t<1> chnl;

    sc_core::sc_event any_tx_finished;

    sc_core::sc_time clk_period{10, sc_core::SC_NS};

private:
    sc_core::sc_clock* clk_if{nullptr};
    void end_of_elaboration() override { clk_if = dynamic_cast<sc_core::sc_clock*>(clk_i.get_interface()); }

    unsigned m_clock_counter{0};
    unsigned m_prev_clk_cnt{0};
};

/**
 * the apb initiator socket protocol engine adapted to a particular initiator socket configuration
 */
template <unsigned int BUSWIDTH = 32, typename TYPES = tlm::tlm_base_protocol_types, int N = 1,
          sc_core::sc_port_policy POL = sc_core::SC_ONE_OR_MORE_BOUND>
class apb_initiator : public apb_initiator_b {
public:
    using base = apb_initiator_b;

    using payload_type = base::payload_type;
    using phase_type = base::phase_type;
    /**
     * @brief the constructor
     *
     * @param nm the module name
     * @param socket reference to the initiator socket used to send and receive transactions
     */
    apb_initiator(const sc_core::sc_module_name& nm, tlm::tlm_initiator_socket<BUSWIDTH, TYPES, N, POL>& socket)
    : apb_initiator_b(nm, socket.get_base_port(), BUSWIDTH, false)
    , socket(socket) {
        socket(*this);
    }

    apb_initiator() = delete;

    apb_initiator(apb_initiator const&) = delete;

    apb_initiator(apb_initiator&&) = delete;

    apb_initiator& operator=(apb_initiator const&) = delete;

    apb_initiator& operator=(apb_initiator&&) = delete;

private:
    tlm::tlm_initiator_socket<BUSWIDTH, TYPES, N, POL>& socket;
};

} /* namespace pe */
} /* namespace apb */

#endif /* _TLM_APB_PE_APB_INITIATOR_H_ */
