/*******************************************************************************
 * Copyright 2018-2022 MINRES Technologies GmbH
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

#ifndef _TLM_TLM_SIGNAL_SOCKETS_H_
#define _TLM_TLM_SIGNAL_SOCKETS_H_

#include "scc/report.h"

#include <sysc/kernel/sc_object.h>
#ifdef CWR_SYSTEMC
#include <tlm_h/tlm_sockets/tlm_initiator_socket.h>
#include <tlm_h/tlm_sockets/tlm_target_socket.h>
#else
#include <tlm_core/tlm_2/tlm_sockets/tlm_initiator_socket.h>
#include <tlm_core/tlm_2/tlm_sockets/tlm_target_socket.h>
#endif

//! @brief SystemC TLM
namespace tlm {
//! @brief SCC TLM utilities
namespace scc {

template <typename SIG> class tlm_signal_gp;

template <typename SIG = bool> struct tlm_signal_baseprotocol_types {
    using tlm_signal_type = SIG;
    using tlm_payload_type = tlm_signal_gp<tlm_signal_type>;
    using tlm_phase_type = ::tlm::tlm_phase;
};

template <typename SIG = bool, typename TYPES = tlm_signal_baseprotocol_types<SIG>>
struct tlm_signal_fw_transport_if : public virtual sc_core::sc_interface {
    typedef TYPES protocol_types;
    // virtual void b_transport(typename TYPES::tlm_payload_type&, sc_core::sc_time&) = 0;
    virtual ::tlm::tlm_sync_enum nb_transport_fw(typename TYPES::tlm_payload_type&, ::tlm::tlm_phase&,
                                                 sc_core::sc_time&) = 0;
};

template <typename SIG = bool, typename TYPES = tlm_signal_baseprotocol_types<SIG>>
struct tlm_signal_bw_transport_if : public virtual sc_core::sc_interface {
    typedef TYPES protocol_types;
    virtual ::tlm::tlm_sync_enum nb_transport_bw(typename TYPES::tlm_payload_type&, ::tlm::tlm_phase&,
                                                 sc_core::sc_time&) = 0;
};

template <typename SIG = bool, typename TYPES = tlm_signal_baseprotocol_types<SIG>, int N = 1,
          sc_core::sc_port_policy POL = sc_core::SC_ONE_OR_MORE_BOUND>
struct tlm_signal_initiator_socket : public tlm_base_initiator_socket<0, tlm_signal_fw_transport_if<SIG, TYPES>,
                                                                      tlm_signal_bw_transport_if<SIG, TYPES>, N, POL> {
    using tlm_signal_type = SIG;
    using protocol_types = TYPES;
    using transaction_type = typename TYPES::tlm_payload_type;
    using phase_type = typename TYPES::tlm_phase_type;

    tlm_signal_initiator_socket()
    : tlm_base_initiator_socket<0, tlm_signal_fw_transport_if<SIG, TYPES>, tlm_signal_bw_transport_if<SIG, TYPES>, N,
                                POL>() {}

    explicit tlm_signal_initiator_socket(const char* name)
    : tlm_base_initiator_socket<0, tlm_signal_fw_transport_if<SIG, TYPES>, tlm_signal_bw_transport_if<SIG, TYPES>, N,
                                POL>(name) {}

    virtual const char* kind() const { return "tlm_signal_initiator_socket"; }

    virtual sc_core::sc_type_index get_protocol_types() const { return typeid(TYPES); }
};

template <typename SIG = bool, typename TYPES = tlm_signal_baseprotocol_types<SIG>, int N = 1>
using tlm_signal_opt_initiator_socket =
    struct tlm_signal_initiator_socket<SIG, TYPES, N, sc_core::SC_ZERO_OR_MORE_BOUND>;

template <typename SIG = bool, typename TYPES = tlm_signal_baseprotocol_types<SIG>, int N = 1,
          sc_core::sc_port_policy POL = sc_core::SC_ONE_OR_MORE_BOUND>
struct tlm_signal_target_socket : public ::tlm::tlm_base_target_socket<0, tlm_signal_fw_transport_if<SIG, TYPES>,
                                                                       tlm_signal_bw_transport_if<SIG, TYPES>, N, POL> {
    using tlm_signal_type = SIG;
    using protocol_types = TYPES;
    using transaction_type = typename TYPES::tlm_payload_type;
    using phase_type = typename TYPES::tlm_phase_type;

    tlm_signal_target_socket()
    : tlm_base_target_socket<0, tlm_signal_fw_transport_if<TYPES>, tlm_signal_bw_transport_if<TYPES>, N, POL>() {}

    explicit tlm_signal_target_socket(const char* name)
    : tlm_base_target_socket<0, tlm_signal_fw_transport_if<SIG, TYPES>, tlm_signal_bw_transport_if<SIG, TYPES>, N, POL>(
          name) {}

    virtual const char* kind() const { return "tlm_signal_target_socket"; }

    virtual sc_core::sc_type_index get_protocol_types() const { return typeid(TYPES); }
};

template <typename SIG = bool, typename TYPES = tlm_signal_baseprotocol_types<SIG>, int N = 1>
using tlm_signal_opt_target_socket = struct tlm_signal_target_socket<SIG, TYPES, N, sc_core::SC_ZERO_OR_MORE_BOUND>;
} // namespace scc
} // namespace tlm

#endif /* _TLM_TLM_SIGNAL_SOCKETS_H_ */
