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

#ifndef _BUS_APB_PE_APB_INITIATOR_H_
#define _BUS_APB_PE_APB_INITIATOR_H_

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

    tlm::tlm_sync_enum nb_transport_bw(payload_type& trans, phase_type& phase, sc_core::sc_time& t) override;

    void invalidate_direct_mem_ptr(sc_dt::uint64 start_range, sc_dt::uint64 end_range) override;

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

#endif /* _BUS_APB_PE_APB_INITIATOR_H_ */
