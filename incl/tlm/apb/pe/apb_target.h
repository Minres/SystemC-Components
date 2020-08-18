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

#ifndef _TLM_APB_PE_APB_TARGET_H_

#include <tlm>
#include <functional>
#include <scc/ordered_semaphore.h>


namespace apb {
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

    /** @defgroup fw_if Initiator foreward interface
     *  @{
     */
    void b_transport(payload_type& trans, sc_core::sc_time& t) override;

    tlm::tlm_sync_enum nb_transport_fw(payload_type& trans, phase_type& phase, sc_core::sc_time& t) override;

    bool get_direct_mem_ptr(payload_type& trans, tlm::tlm_dmi& dmi_data) override;

    unsigned int transport_dbg(payload_type& trans) override;
    /** @} */ // end of fw_if
    /** @defgroup config Initiator configuration interface
     *  @{
     */

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
     * @param port
     * @param transfer_width
     */
    explicit apb_target_b(const sc_core::sc_module_name& nm, sc_core::sc_port_b<tlm::tlm_bw_transport_if<tlm::tlm_base_protocol_types>>& port,
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
} // namespace ahb

#endif /*_TLM_APB_PE_APB_TARGET_H_*/
