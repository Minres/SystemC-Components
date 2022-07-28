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

#ifndef _BUS_AHB_PE_INITIATOR_H_
#define _BUS_AHB_PE_INITIATOR_H_

#include <ahb/ahb_tlm.h>
#include <scc/ordered_semaphore.h>
#include <scc/peq.h>
#include <systemc>
#include <tlm_utils/peq_with_get.h>
#include <tuple>
#include <unordered_map>

//! TLM2.0 components modeling AHB
namespace ahb {
//! protocol engine implementations
namespace pe {

class ahb_initiator_b : public sc_core::sc_module, public tlm::tlm_bw_transport_if<tlm::tlm_base_protocol_types> {
public:
    SC_HAS_PROCESS(ahb_initiator_b);

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

    ahb_initiator_b(sc_core::sc_module_name nm,
                    sc_core::sc_port_b<tlm::tlm_fw_transport_if<tlm::tlm_base_protocol_types>>& port,
                    size_t transfer_width, bool coherent);

    virtual ~ahb_initiator_b();

    ahb_initiator_b() = delete;

    ahb_initiator_b(ahb_initiator_b const&) = delete;

    ahb_initiator_b(ahb_initiator_b&&) = delete;

    ahb_initiator_b& operator=(ahb_initiator_b const&) = delete;

    ahb_initiator_b& operator=(ahb_initiator_b&&) = delete;

    void snoop_resp(payload_type& trans, bool sync = false) {}

    //! Read address valid to next read address valid
    sc_core::sc_attribute<unsigned> artv{"artv", 0};
    //! Write address valid to next write address valid
    sc_core::sc_attribute<unsigned> awtv{"awtv", 0};
    //! Write data handshake to next beat valid
    sc_core::sc_attribute<unsigned> wbv{"wbv", 0};
    //! Read data valid to same beat ready
    sc_core::sc_attribute<unsigned> rbr{"rbr", 0};
    //! Write response valid to ready
    sc_core::sc_attribute<unsigned> br{"br", 0};

protected:
    unsigned calculate_beats(payload_type& p) {
        sc_assert(p.get_data_length() > 0);
        return p.get_data_length() < transfer_width_in_bytes ? 1 : p.get_data_length() / transfer_width_in_bytes;
    }

    const size_t transfer_width_in_bytes;

    const bool coherent;

    sc_core::sc_port_b<tlm::tlm_fw_transport_if<tlm::tlm_base_protocol_types>>& socket_fw;

    std::function<unsigned(payload_type& trans)>* snoop_cb{nullptr};

    struct tx_state {
        payload_type* active_tx{nullptr};
        scc::peq<std::tuple<payload_type*, tlm::tlm_phase>> peq;
        // scc::ordered_semaphore mtx{1};
    };
    std::unordered_map<payload_type*, tx_state*> tx_state_by_id;

    scc::ordered_semaphore_t<1> addr_chnl;

    scc::ordered_semaphore_t<1> data_chnl;

    sc_core::sc_event any_tx_finished;

    sc_core::sc_time clk_period{10, sc_core::SC_NS};

private:
    sc_core::sc_clock* clk_if{nullptr};
    void end_of_elaboration() override { clk_if = dynamic_cast<sc_core::sc_clock*>(clk_i.get_interface()); }

    tlm::tlm_phase send(payload_type& trans, ahb_initiator_b::tx_state* txs, tlm::tlm_phase phase);

    unsigned m_clock_counter{0};
    unsigned m_prev_clk_cnt{0};
};

/**
 * the ahb initiator socket protocol engine adapted to a particular initiator socket configuration
 */
template <unsigned int BUSWIDTH = 32, typename TYPES = tlm::tlm_base_protocol_types, int N = 1,
          sc_core::sc_port_policy POL = sc_core::SC_ONE_OR_MORE_BOUND>
class ahb3_initiator : public ahb_initiator_b {
public:
    using base = ahb_initiator_b;

    using payload_type = base::payload_type;
    using phase_type = base::phase_type;
    /**
     * @brief the constructor
     *
     * @param nm the module name
     * @param socket reference to the initiator socket used to send and receive transactions
     */
    ahb3_initiator(const sc_core::sc_module_name& nm, tlm::tlm_initiator_socket<BUSWIDTH, TYPES, N, POL>& socket)
    : ahb_initiator_b(nm, socket.get_base_port(), BUSWIDTH, false)
    , socket(socket) {
        socket(*this);
    }

    ahb3_initiator() = delete;

    ahb3_initiator(ahb3_initiator const&) = delete;

    ahb3_initiator(ahb3_initiator&&) = delete;

    ahb3_initiator& operator=(ahb3_initiator const&) = delete;

    ahb3_initiator& operator=(ahb3_initiator&&) = delete;

private:
    tlm::tlm_initiator_socket<BUSWIDTH, TYPES, N, POL>& socket;
};

} /* namespace pe */
} /* namespace ahb */

#endif /* _BUS_AHB_PE_INITIATOR_H_ */
