/*******************************************************************************
 * Copyright 2026 MINRES Technologies GmbH
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

#ifndef _SYSC_NB_ROUTER_TYPES_H_
#define _SYSC_NB_ROUTER_TYPES_H_

#include <scc/signal_opt_ports.h>
#include <tlm>

namespace scc {
namespace at_router {
template <typename TYPES = tlm::tlm_base_protocol_types>
using fw_if = tlm::tlm_fw_nonblocking_transport_if<typename TYPES::tlm_payload_type, typename TYPES::tlm_phase_type>;
template <typename TYPES = tlm::tlm_base_protocol_types>
using bw_if = tlm::tlm_bw_nonblocking_transport_if<typename TYPES::tlm_payload_type, typename TYPES::tlm_phase_type>;

template <typename TYPES = tlm::tlm_base_protocol_types> struct t_port {

    sc_core::sc_export<fw_if<TYPES>> fw;
    sc_core::sc_port<bw_if<TYPES>> bw;
    scc::sc_in_opt<sc_core::sc_time> clk;

    std::function<void(unsigned)> set_width;

    t_port(char const* name, unsigned width = 0)
    : fw((std::string(name) + "_fw").c_str())
    , bw((std::string(name) + "_bw").c_str())
    , clk((std::string(name) + "_clk").c_str()) {}
};

template <typename TARGET_SOCKET_TYPE> struct t_port_bw_adapter : public at_router::bw_if<tlm::tlm_base_protocol_types> {
    TARGET_SOCKET_TYPE& p;
    t_port_bw_adapter(TARGET_SOCKET_TYPE& p)
    : p(p) {}
    t_port_bw_adapter(t_port_bw_adapter const&) = default;
    t_port_bw_adapter(t_port_bw_adapter&&) = default;
    t_port_bw_adapter& operator=(t_port_bw_adapter const&) = default;
    t_port_bw_adapter& operator=(t_port_bw_adapter&&) = default;

    tlm::tlm_sync_enum nb_transport_bw(typename tlm::tlm_base_protocol_types::tlm_payload_type& trans,
                                       typename tlm::tlm_base_protocol_types::tlm_phase_type& phase, sc_core::sc_time& t) override {
        return p->nb_transport_bw(trans, phase, t);
    }
};

template <typename TYPES = tlm::tlm_base_protocol_types> struct i_port {
    using fw_if = tlm::tlm_fw_nonblocking_transport_if<typename TYPES::tlm_payload_type, typename TYPES::tlm_phase_type>;
    using bw_if = tlm::tlm_bw_nonblocking_transport_if<typename TYPES::tlm_payload_type, typename TYPES::tlm_phase_type>;

    sc_core::sc_port<fw_if> fw;
    sc_core::sc_export<bw_if> bw;
    scc::sc_in_opt<sc_core::sc_time> clk;

    std::function<void(unsigned)> set_width;

    i_port(char const* name, unsigned width = 0)
    : fw((std::string(name) + "_fw").c_str())
    , bw((std::string(name) + "_bw").c_str())
    , clk((std::string(name) + "_clk").c_str()) {}

    void bind(i_port<TYPES>& o) {
        this->fw.bind(o.fw);
        o.bw(this->bw.bind);
    }

    void bind(t_port<TYPES>& o) {
        this->fw.bind(o.fw);
        o.bw.bind(this->bw);
    }
};

template <typename TYPES> struct i_port_fw_adapter : public at_router::i_port<TYPES>::fw_if {
    sc_core::sc_port_b<tlm::tlm_fw_transport_if<TYPES>>& p;
    i_port_fw_adapter(sc_core::sc_port_b<tlm::tlm_fw_transport_if<TYPES>>& p)
    : p(p) {}
    i_port_fw_adapter(i_port_fw_adapter const&) = default;
    i_port_fw_adapter(i_port_fw_adapter&&) = default;
    i_port_fw_adapter& operator=(i_port_fw_adapter const&) = default;
    i_port_fw_adapter& operator=(i_port_fw_adapter&&) = default;

    tlm::tlm_sync_enum nb_transport_fw(typename TYPES::tlm_payload_type& trans, typename TYPES::tlm_phase_type& phase,
                                       sc_core::sc_time& t) override {
        return p->nb_transport_fw(trans, phase, t);
    }
};

} // namespace at_router
} // namespace scc
#endif
