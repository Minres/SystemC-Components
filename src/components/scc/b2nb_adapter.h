/*******************************************************************************
 * Copyright 2024 MINRES Technologies GmbH
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

#ifndef _SCC_SOCKET_WIDTH_ADAPTER_H_
#define _SCC_SOCKET_WIDTH_ADAPTER_H_

#include "tlm/scc/initiator_mixin.h"
#include "tlm/scc/target_mixin.h"
#include <tlm>

//! @brief SCC TLM utilities
namespace scc {
/**
 * @brief The socket_width_adapter class is a TLM (Transaction-Level Modeling) socket width adapter.
 *
 * The socket_width_adapter class is a template class that adapts the width of a TLM socket.
 * It allows the connection of modules with different bus widths by converting the data width between the initiator and target sockets.
 *
 * @tparam TGT_WIDTH The width of the target socket.
 * @tparam INTOR_BUSWIDTH The width of the initiator socket.
 * @tparam TYPES The TLM protocol types.
 * @tparam N The number of socket instances.
 * @tparam POL The port binding policy.
 *
 * @note The socket_width_adapter class is a part of the SystemC Component (SCC) library.
 *
 * @author Your Name
 * @date YYYY-MM-DD
 */
template <unsigned int BUSWIDTH, typename TYPES = tlm::tlm_base_protocol_types> class b2nb_adapter : public sc_core::sc_module {
public:
    using tlm_payload_type = typename TYPES::tlm_payload_type;
    using tlm_phase_type = typename TYPES::tlm_phase_type;
    using target_socket_type = tlm::scc::target_mixin<tlm::tlm_target_socket<BUSWIDTH, TYPES>>;
    using initiator_socket_type = tlm::scc::initiator_mixin<tlm::tlm_initiator_socket<BUSWIDTH, TYPES>>;
    /**
     * @brief The target socket for the adapter.
     *
     * This socket is used to connect the target module with the adapter.
     */
    target_socket_type tsck{"tsck"};
    /**
     * @brief The initiator socket for the adapter.
     *
     * This socket is used to connect the initiator module with the adapter.
     */
    initiator_socket_type isck{"isck"};
    /**
     * @brief Constructor for the socket_width_adapter class.
     *
     * @param nm The name of the socket_width_adapter instance.
     */
    b2nb_adapter(sc_core::sc_module_name const& nm)
    : sc_core::sc_module(nm) {
        tsck.register_nb_transport_fw([this](tlm_payload_type& trans, tlm_phase_type& phase, sc_core::sc_time& t) -> tlm::tlm_sync_enum {
            return isck->nb_transport_fw(trans, phase, t);
        });
        isck.register_nb_transport_bw([this](tlm_payload_type& trans, tlm_phase_type& phase, sc_core::sc_time& t) -> tlm::tlm_sync_enum {
            return tsck->nb_transport_bw(trans, phase, t);
        });
        tsck.register_get_direct_mem_ptr([this](tlm_payload_type& trans, tlm::tlm_dmi& dmi) -> bool {
            return isck->get_direct_mem_ptr(trans, dmi);
        });
        isck.register_invalidate_direct_mem_ptr([this](sc_dt::uint64 start, sc_dt::uint64 end) {
            tsck->invalidate_direct_mem_ptr(start, end);
        });
    }

    b2nb_adapter() = delete;

    b2nb_adapter(b2nb_adapter const&) = delete;

    b2nb_adapter(b2nb_adapter&&) = delete;
    /**
     * @brief Virtual destructor for the socket_width_adapter class.
     */
    virtual ~b2nb_adapter() = default;
};
} // namespace scc
#endif // _SCC_SOCKET_WIDTH_ADAPTER_H_
