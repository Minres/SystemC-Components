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

#include <tlm>

//! @brief SCC TLM utilities
namespace scc {

template <unsigned int TGT_WIDTH= 32, unsigned int INTOR_BUSWIDTH = 32, typename TYPES = tlm::tlm_base_protocol_types, int N = 1,
          sc_core::sc_port_policy POL = sc_core::SC_ONE_OR_MORE_BOUND>
class socket_width_adapter : public sc_core::sc_module,
                                  public tlm::tlm_fw_transport_if<TYPES>,
                                  public tlm::tlm_bw_transport_if<TYPES> {
public:

    using tlm_payload_type = typename TYPES::tlm_payload_type;
    using tlm_phase_type = typename TYPES::tlm_phase_type;
    using target_socket_type = tlm::tlm_target_socket<TGT_WIDTH, TYPES, N, POL>;
    using initiator_socket_type = tlm::tlm_initiator_socket<INTOR_BUSWIDTH, TYPES, N, POL>;

    target_socket_type tsck{"tsck"};

    initiator_socket_type isck{"isck"};

    socket_width_adapter(sc_core::sc_module_name const& nm)
    : sc_core::sc_module(nm) {
        tsck.bind(*this);
        isck.bind(*this);
    }

    socket_width_adapter() = delete;

    socket_width_adapter(socket_width_adapter const&) = delete;

    socket_width_adapter(socket_width_adapter&&) = delete;

    virtual ~socket_width_adapter() = default;

private:
    tlm::tlm_sync_enum nb_transport_fw(tlm_payload_type& trans, tlm_phase_type& phase, sc_core::sc_time& t) override {
        return isck->nb_transport_fw(trans, phase, t);
    };

    void b_transport(tlm_payload_type& trans, sc_core::sc_time& t) override {
        isck->b_transport(trans, t);
    }

    bool get_direct_mem_ptr(tlm_payload_type& trans, tlm::tlm_dmi& dmi_data) override {
        return isck->get_direct_mem_ptr(trans, dmi_data);
    }

    unsigned int transport_dbg(tlm_payload_type& trans) override {
        return isck->transport_dbg(trans);
    }

    tlm::tlm_sync_enum nb_transport_bw(tlm_payload_type& trans, tlm_phase_type& phase, sc_core::sc_time& t) override {
        return tsck->nb_transport_bw(trans, phase, t);
    }

    void invalidate_direct_mem_ptr(sc_dt::uint64 start_range, sc_dt::uint64 end_range) override {
        tsck->invalidate_direct_mem_ptr(start_range, end_range);
    }
};
}
#endif // _SCC_SOCKET_WIDTH_ADAPTER_H_
