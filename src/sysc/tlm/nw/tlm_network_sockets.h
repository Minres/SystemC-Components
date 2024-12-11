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

#include "scc/report.h"

#include <sysc/kernel/sc_object.h>
#ifdef CWR_SYSTEMC
#include <tlm_h/tlm_sockets/tlm_initiator_socket.h>
#include <tlm_h/tlm_sockets/tlm_target_socket.h>
#else
#include <tlm>
#endif

//! @brief SystemC TLM
namespace tlm {
//! @brief SCC TLM utilities
namespace nw {

template <typename SIG> class tlm_network_gp;

struct tlm_network_baseprotocol_types {
    using tlm_payload_type = tlm_network_payload_base;
    using tlm_phase_type = ::tlm::tlm_phase;
};

/**
 * definition of the additional protocol phases
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

template <unsigned PHITWIDTH, typename CMDENUM, typename TYPES = tlm_network_baseprotocol_types, int N = 1,
          sc_core::sc_port_policy POL = sc_core::SC_ONE_OR_MORE_BOUND>
struct tlm_network_initiator_socket
: public tlm_base_initiator_socket<0, tlm_network_fw_transport_if<TYPES>, tlm_network_bw_transport_if<TYPES>, N, POL> {

    using protocol_types = TYPES;
    using transaction_type = typename TYPES::tlm_payload_type;
    using phase_type = typename TYPES::tlm_phase_type;

    tlm_network_initiator_socket()
    : tlm_base_initiator_socket<0, tlm_network_fw_transport_if<TYPES>, tlm_network_bw_transport_if<TYPES>, N, POL>() {}

    explicit tlm_network_initiator_socket(const char* name)
    : tlm_base_initiator_socket<0, tlm_network_fw_transport_if<TYPES>, tlm_network_bw_transport_if<TYPES>, N, POL>(name) {}

    virtual const char* kind() const { return "tlm_network_initiator_socket"; }

    virtual type_index get_protocol_types() const { return typeid(TYPES); }
};

template <unsigned PHITWIDTH, typename CMDENUM, typename TYPES = tlm_network_baseprotocol_types, int N = 1,
          sc_core::sc_port_policy POL = sc_core::SC_ONE_OR_MORE_BOUND>
struct tlm_network_target_socket
: public ::tlm::tlm_base_target_socket<0, tlm_network_fw_transport_if<TYPES>, tlm_network_bw_transport_if<TYPES>, N, POL> {

    using protocol_types = TYPES;
    using transaction_type = typename TYPES::tlm_payload_type;
    using phase_type = typename TYPES::tlm_phase_type;

    tlm_network_target_socket()
    : tlm_base_target_socket<0, tlm_network_fw_transport_if<TYPES>, tlm_network_bw_transport_if<TYPES>, N, POL>() {}

    explicit tlm_network_target_socket(const char* name)
    : tlm_base_target_socket<0, tlm_network_fw_transport_if<TYPES>, tlm_network_bw_transport_if<TYPES>, N, POL>(name) {}

    virtual const char* kind() const { return "tlm_network_target_socket"; }

    virtual type_index get_protocol_types() const { return typeid(TYPES); }
};
} // namespace nw
} // namespace tlm

#endif /* _TLM_NW_TLM_NETWORK_SOCKETS_H_ */
