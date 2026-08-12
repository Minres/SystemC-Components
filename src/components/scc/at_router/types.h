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

#include <sysc/communication/sc_export.h>
#include <tlm_core/tlm_2/tlm_2_interfaces/tlm_fw_bw_ifs.h>

namespace scc {
namespace at_router {
template <typename TYPES = tlm::tlm_base_protocol_types> struct t_port {
    using fw_if = tlm::tlm_fw_nonblocking_transport_if<typename TYPES::tlm_payload_type, typename TYPES::tlm_phase_type>;
    using bw_if = tlm::tlm_bw_nonblocking_transport_if<typename TYPES::tlm_payload_type, typename TYPES::tlm_phase_type>;

    sc_core::sc_export<fw_if> fw;
    sc_core::sc_port<bw_if> bw;
    t_port(char const* name)
    : fw((std::string(name) + "_fw").c_str())
    , bw((std::string(name) + "_bw").c_str()) {}

    void bind(t_port<TYPES>& o) {
        this->fw.bind(o.fw);
        this->bw.bind(o.bw);
    }
};
template <typename TYPES = tlm::tlm_base_protocol_types> struct i_port {
    using fw_if = tlm::tlm_fw_nonblocking_transport_if<typename TYPES::tlm_payload_type, typename TYPES::tlm_phase_type>;
    using bw_if = tlm::tlm_bw_nonblocking_transport_if<typename TYPES::tlm_payload_type, typename TYPES::tlm_phase_type>;

    sc_core::sc_port<fw_if> fw;
    sc_core::sc_export<bw_if> bw;
    i_port(char const* name)
    : fw((std::string(name) + "_fw").c_str())
    , bw((std::string(name) + "_bw").c_str()) {}

    void bind(i_port<TYPES>& o) {
        this->fw.bind(o.fw);
        this->bw.bind(o.bw);
    }

    void bind(t_port<TYPES>& o) {
        this->fw.bind(o.fw);
        this->bw.bind(o.bw);
    }
};
template <typename TYPES> struct i_port_bw_adapter : public at_router::i_port<TYPES>::bw_if {
    sc_core::sc_port_b<tlm::tlm_bw_transport_if<TYPES>>& p;
    i_port_bw_adapter(sc_core::sc_port_b<tlm::tlm_bw_transport_if<TYPES>>& p)
    : p(p) {}
    i_port_bw_adapter(i_port_bw_adapter const&) = default;
    i_port_bw_adapter(i_port_bw_adapter&&) = default;
    i_port_bw_adapter& operator=(i_port_bw_adapter const&) = default;
    i_port_bw_adapter& operator=(i_port_bw_adapter&&) = default;

    tlm::tlm_sync_enum nb_transport_bw(typename TYPES::tlm_payload_type& trans, typename TYPES::tlm_phase_type& phase,
                                       sc_core::sc_time& t) override {
        return p->nb_transport_bw(trans, phase, t);
    }
};

} // namespace at_router
} // namespace scc
#endif