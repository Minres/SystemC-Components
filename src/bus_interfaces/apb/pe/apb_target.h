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

#ifndef _BUS_APB_PE_APB_TARGET_H_
#define _BUS_APB_PE_APB_TARGET_H_

#include <functional>
#include <scc/ordered_semaphore.h>
#include <tlm>

//! TLM2.0 components modeling APB
namespace apb {
//! protocol engine implementations
namespace pe {
/**
 * the target protocol engine base class
 */
class apb_target_b : public sc_core::sc_module, public tlm::tlm_fw_transport_if<tlm::tlm_base_protocol_types> {
public:
    SC_HAS_PROCESS(apb_target_b);

    using payload_type = tlm::tlm_base_protocol_types::tlm_payload_type;
    using phase_type = tlm::tlm_base_protocol_types::tlm_phase_type;

    sc_core::sc_in<bool> clk_i{"clk_i"};

    void b_transport(payload_type& trans, sc_core::sc_time& t) override;

    tlm::tlm_sync_enum nb_transport_fw(payload_type& trans, phase_type& phase, sc_core::sc_time& t) override;

    bool get_direct_mem_ptr(payload_type& trans, tlm::tlm_dmi& dmi_data) override;

    unsigned int transport_dbg(payload_type& trans) override;
    /**
     * @brief Set the operation callback function
     *
     * This callback is invoked once a transaction arrives. This function is not allowed to block and returns the
     * latency of the operation i.e. the duration until the reponse phase starts
     * @todo refine API
     *
     * @param cb the callback function
     */

    void set_operation_cb(std::function<unsigned(payload_type& trans)> cb) { operation_cb = cb; }

protected:
    /**
     * the constructor. Protected as it should only be called by derived classes
     *
     * @param nm the module name
     * @param port
     * @param transfer_width
     */
    explicit apb_target_b(const sc_core::sc_module_name& nm,
                          sc_core::sc_port_b<tlm::tlm_bw_transport_if<tlm::tlm_base_protocol_types>>& port,
                          size_t transfer_width);

    apb_target_b() = delete;

    apb_target_b(apb_target_b const&) = delete;

    apb_target_b(apb_target_b&&) = delete;

    apb_target_b& operator=(apb_target_b const&) = delete;

    apb_target_b& operator=(apb_target_b&&) = delete;

    tlm::tlm_generic_payload* active_tx{nullptr};
    void response();
    sc_core::sc_port_b<tlm::tlm_bw_transport_if<tlm::tlm_base_protocol_types>>& socket_bw;
    std::function<unsigned(payload_type& trans)> operation_cb;
    sc_core::sc_clock* clk_if{nullptr};
    void end_of_elaboration() override;
    sc_core::sc_process_handle mhndl;
};

/**
 * the target socket protocol engine adapted to a particular target socket configuration
 */
template <unsigned int BUSWIDTH = 32, typename TYPES = tlm::tlm_base_protocol_types, int N = 1,
          sc_core::sc_port_policy POL = sc_core::SC_ONE_OR_MORE_BOUND>
class apb_target : public apb_target_b {
public:
    using base = apb_target_b;
    using payload_type = base::payload_type;
    using phase_type = base::phase_type;
    /**
     * @brief the constructor
     * @param socket reference to the initiator socket used to send and receive transactions
     */
    apb_target(tlm::tlm_target_socket<BUSWIDTH, TYPES, N, POL>& socket)
    : // @suppress("Class members should be properly initialized")
        apb_target(sc_core::sc_gen_unique_name("simple_target"), socket) {}

    apb_target(const sc_core::sc_module_name& nm, tlm::tlm_target_socket<BUSWIDTH, TYPES, N, POL>& socket)
    : apb_target_b(nm, socket.get_base_port(), BUSWIDTH)
    , socket(socket) {
        socket(*this);
    }

    apb_target() = delete;

    apb_target(apb_target const&) = delete;

    apb_target(apb_target&&) = delete;

    apb_target& operator=(apb_target const&) = delete;

    apb_target& operator=(apb_target&&) = delete;

private:
    tlm::tlm_target_socket<BUSWIDTH, TYPES, N, POL>& socket;
};

} // namespace pe
} // namespace apb

#endif /*_BUS_APB_PE_APB_TARGET_H_*/
