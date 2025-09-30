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

#ifndef _TLM_NW_TLM_NETWORK_SOCKETS_H_
#define _TLM_NW_TLM_NETWORK_SOCKETS_H_

#include "tlm_network_gp.h"
#include <sysc/kernel/sc_object.h>
#include <tlm/scc/tlm_mm.h>
#ifdef CWR_SYSTEMC
#include <tlm_h/tlm_sockets/tlm_initiator_socket.h>
#include <tlm_h/tlm_sockets/tlm_target_socket.h>
#else
#include <tlm>
#endif
/**
 * @file tlm_network_sockets.h
 *
 * @brief The SystemC Transaction Level Model (TLM) Network TLM utilities.
 *
 * @ingroup tlm-nw
 */

/**
 * @brief The SystemC Transaction Level Model (TLM) Network TLM utilities.
 *
 * @namespace tlm::nw
 */
namespace tlm {
namespace nw {

template <typename SIG> class tlm_network_gp;

struct tlm_network_baseprotocol_types {
    using tlm_payload_type = tlm_network_payload_base;
    using tlm_phase_type = ::tlm::tlm_phase;
};

/**
 * @brief Definition of the additional protocol phases.
 */
DECLARE_EXTENDED_PHASE(REQUEST);
DECLARE_EXTENDED_PHASE(CONFIRM);
DECLARE_EXTENDED_PHASE(INDICATION);
DECLARE_EXTENDED_PHASE(RESPONSE);

template <typename TYPES = tlm_network_baseprotocol_types>
struct tlm_network_fw_transport_if : public virtual tlm::tlm_blocking_transport_if<typename TYPES::tlm_payload_type>,
                                     public virtual tlm::tlm_fw_nonblocking_transport_if<typename TYPES::tlm_payload_type>,
                                     public virtual tlm::tlm_transport_dbg_if<typename TYPES::tlm_payload_type> {
    typedef TYPES protocol_types;
};

template <typename TYPES = tlm_network_baseprotocol_types>
struct tlm_network_bw_transport_if : public virtual tlm::tlm_bw_nonblocking_transport_if<typename TYPES::tlm_payload_type> {
    typedef TYPES protocol_types;
};

#if SC_VERSION_MAJOR < 3
using type_index = sc_core::sc_type_index;
#else
using type_index = std::type_index;
#endif

/**
 * @brief Definition of the tlm_network_initiator_socket class.
 *
 * This socket is used to initiate transactions to a target.
 *
 * @ingroup tlm-nw
 */
template <unsigned PHITWIDTH, typename CMDENUM, typename TYPES = tlm_network_baseprotocol_types, int N = 1,
          sc_core::sc_port_policy POL = sc_core::SC_ONE_OR_MORE_BOUND>
struct tlm_network_initiator_socket
: public tlm_base_initiator_socket<0, tlm_network_fw_transport_if<TYPES>, tlm_network_bw_transport_if<TYPES>, N, POL> {

    using protocol_types = TYPES;
    using transaction_type = typename TYPES::tlm_payload_type;
    using phase_type = typename TYPES::tlm_phase_type;

    using base_class = tlm_base_initiator_socket<0, tlm_network_fw_transport_if<TYPES>, tlm_network_bw_transport_if<TYPES>, N, POL>;

    using fw_interface_type = tlm_network_fw_transport_if<TYPES>;
    using bw_interface_type = tlm_network_bw_transport_if<TYPES>;

    using port_type = sc_core::sc_port<fw_interface_type, N, POL>;
    using export_type = sc_core::sc_export<bw_interface_type>;
    /**
     * @brief Constructor with default name.
     *
     * Initializes the tlm_network_initiator_socket object with a default name.
     */
    tlm_network_initiator_socket()
    : tlm_base_initiator_socket<0, tlm_network_fw_transport_if<TYPES>, tlm_network_bw_transport_if<TYPES>, N, POL>() {}
    /**
     * @brief Constructor with specified name.
     *
     * Initializes the tlm_network_initiator_socket object with a specified name.
     *
     * @param name The name of the socket.
     */
    explicit tlm_network_initiator_socket(const char* name)
    : tlm_base_initiator_socket<0, tlm_network_fw_transport_if<TYPES>, tlm_network_bw_transport_if<TYPES>, N, POL>(name) {}
    /**
     * virtual destructor of the tlm_network_initiator_socket.
     */
    virtual ~tlm_network_initiator_socket() = default;
    /**
     * Provides the kind() method to retrieve the socket's kind as string.
     */
    virtual const char* kind() const { return "tlm_network_initiator_socket"; }
    /**
     * Returns the type index of the protocol types associated with this socket.
     */
    virtual type_index get_protocol_types() const { return typeid(TYPES); }
};
/**
 * @brief Definition of the tlm_network_target_socket class.
 *
 * This socket is used to accept transactions from an initiator.
 *
 * @ingroup tlm-nw
 */
template <unsigned PHITWIDTH, typename CMDENUM, typename TYPES = tlm_network_baseprotocol_types, int N = 1,
          sc_core::sc_port_policy POL = sc_core::SC_ONE_OR_MORE_BOUND>
struct tlm_network_target_socket
: public ::tlm::tlm_base_target_socket<0, tlm_network_fw_transport_if<TYPES>, tlm_network_bw_transport_if<TYPES>, N, POL> {

    using protocol_types = TYPES;
    using transaction_type = typename TYPES::tlm_payload_type;
    using phase_type = typename TYPES::tlm_phase_type;
    /**
     * @brief Constructor with default name.
     *
     * Initializes the tlm_network_target_socket object with a default name.
     */
    tlm_network_target_socket()
    : tlm_base_target_socket<0, tlm_network_fw_transport_if<TYPES>, tlm_network_bw_transport_if<TYPES>, N, POL>() {}
    /**
     * @brief Constructor with name.
     *
     * Initializes the tlm_network_target_socket object with a default name.
     */
    explicit tlm_network_target_socket(const char* name)
    : tlm_base_target_socket<0, tlm_network_fw_transport_if<TYPES>, tlm_network_bw_transport_if<TYPES>, N, POL>(name) {}
    /**
     * virtual destructor of the tlm_network_target_socket.
     */
    virtual ~tlm_network_target_socket() = default;
    /**
     * Provides the kind() method to retrieve the socket's kind as string.
     */
    virtual const char* kind() const { return "tlm_network_target_socket"; }
    /**
     * Returns the type index of the protocol types associated with this socket.
     */
    virtual type_index get_protocol_types() const { return typeid(TYPES); }
};
} // namespace nw
namespace scc {
template <> struct tlm_mm_traits<tlm::nw::tlm_network_baseprotocol_types> {
    using mm_if_type = tlm::nw::tlm_base_mm_interface;
    using payload_base = tlm::nw::tlm_network_payload_base;
};

} // namespace scc
} // namespace tlm

#endif /* _TLM_NW_TLM_NETWORK_SOCKETS_H_ */
