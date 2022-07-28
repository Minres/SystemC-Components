/*******************************************************************************
 * Copyright 2020 MINRES Technologies GmbH
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

#ifndef _TLM_TLM2_PV_AV_H_
#define _TLM_TLM2_PV_AV_H_

#include <memory>
#include <tlm>
#include <util/ities.h>

//! @brief SystemC TLM
namespace tlm {
//! @brief SCC TLM utilities
namespace scc {

template <unsigned int BUSWIDTH = 32, typename TYPES = tlm_base_protocol_types, int N = 1,
          sc_core::sc_port_policy POL = sc_core::SC_ONE_OR_MORE_BOUND,
          typename TSOCKET_TYPE = tlm::tlm_target_socket<BUSWIDTH, TYPES, N, POL>,
          typename ISOCKET_TYPE = tlm::tlm_initiator_socket<BUSWIDTH, TYPES, N, POL>>
class tlm2_pv_av_target_adapter : public sc_core::sc_module,
                                  public tlm::tlm_fw_transport_if<TYPES>,
                                  public tlm::tlm_bw_transport_if<TYPES> {
public:
    using tlm_payload_type = typename TYPES::tlm_payload_type;
    using tlm_phase_type = typename TYPES::tlm_phase_type;
    using target_socket_type = TSOCKET_TYPE;
    using initiator_socket_type = ISOCKET_TYPE;

    target_socket_type tsck{"tsck"};

    tlm2_pv_av_target_adapter()
    : tlm2_pv_av_target_adapter(sc_core::sc_gen_unique_name("tlm_pv_av_split")) {}

    tlm2_pv_av_target_adapter(sc_core::sc_module_name const& nm)
    : sc_core::sc_module(nm) {
        tsck.bind(*this);
    }

    void bind_pv(target_socket_type& tsck) {
        pv_isck = util::make_unique<initiator_socket_type>(sc_core::sc_gen_unique_name("pv_isck"));
        pv_isck->bind(*this);
        pv_isck->bind(tsck);
    }

    void bind_av(TSOCKET_TYPE& tsck) {
        av_isck = util::make_unique<initiator_socket_type>(sc_core::sc_gen_unique_name("av_isck"));
        av_isck->bind(*this);
        av_isck->bind(tsck);
    }

    virtual ~tlm2_pv_av_target_adapter() = default;

private:
    tlm::tlm_sync_enum nb_transport_fw(tlm_payload_type& trans, tlm_phase_type& phase, sc_core::sc_time& t) {
        if(av_isck)
            return (*av_isck)->nb_transport_fw(trans, phase, t);
        trans.set_response_status(tlm::TLM_GENERIC_ERROR_RESPONSE);
        return tlm::TLM_COMPLETED;
    };

    void b_transport(tlm_payload_type& trans, sc_core::sc_time& t) {
        if(pv_isck)
            (*pv_isck)->b_transport(trans, t);
        else
            trans.set_response_status(tlm::TLM_GENERIC_ERROR_RESPONSE);
    }

    bool get_direct_mem_ptr(tlm_payload_type& trans, tlm_dmi& dmi_data) {
        if(pv_isck)
            return (*pv_isck)->get_direct_mem_ptr(trans, dmi_data);
        trans.set_response_status(tlm::TLM_GENERIC_ERROR_RESPONSE);
        trans.set_dmi_allowed(false);
        return false;
    }

    unsigned int transport_dbg(tlm_payload_type& trans) {
        if(pv_isck)
            return (*pv_isck)->transport_dbg(trans);
        trans.set_response_status(tlm::TLM_GENERIC_ERROR_RESPONSE);
        return 0;
    }

    tlm::tlm_sync_enum nb_transport_bw(tlm_payload_type& trans, tlm_phase_type& phase, sc_core::sc_time& t) {
        return tsck->nb_transport_bw(trans, phase, t);
    }

    void invalidate_direct_mem_ptr(sc_dt::uint64 start_range, sc_dt::uint64 end_range) {
        tsck->invalidate_direct_mem_ptr(start_range, end_range);
    }

    std::unique_ptr<initiator_socket_type> pv_isck;
    std::unique_ptr<initiator_socket_type> av_isck;
};

template <unsigned int BUSWIDTH = 32, typename TYPES = tlm_base_protocol_types, int N = 1,
          sc_core::sc_port_policy POL = sc_core::SC_ONE_OR_MORE_BOUND,
          typename TSOCKET_TYPE = tlm::tlm_target_socket<BUSWIDTH, TYPES, N, POL>,
          typename ISOCKET_TYPE = tlm::tlm_initiator_socket<BUSWIDTH, TYPES, N, POL>>
class tlm2_pv_av_initiator_adapter : public sc_core::sc_module,
                                     public tlm::tlm_fw_transport_if<TYPES>,
                                     public tlm::tlm_bw_transport_if<TYPES> {
public:
    using tlm_payload_type = typename TYPES::tlm_payload_type;
    using tlm_phase_type = typename TYPES::tlm_phase_type;
    using target_socket_type = TSOCKET_TYPE;
    using initiator_socket_type = ISOCKET_TYPE;

    initiator_socket_type isck{"isck"};

    tlm2_pv_av_initiator_adapter()
    : tlm2_pv_av_initiator_adapter(sc_core::sc_gen_unique_name("tlm_pv_av_split")) {}

    tlm2_pv_av_initiator_adapter(sc_core::sc_module_name const& nm)
    : sc_core::sc_module(nm) {
        isck.bind(*this);
    }

    void bind_pv(initiator_socket_type& isck) {
        pv_tsck = util::make_unique<target_socket_type>(sc_core::sc_gen_unique_name("pv_tsck"));
        pv_tsck->bind(*this);
        isck(*pv_tsck);
    }

    void bind_av(initiator_socket_type& isck) {
        av_tsck = util::make_unique<target_socket_type>(sc_core::sc_gen_unique_name("av_tsck"));
        av_tsck->bind(*this);
        isck(*av_tsck);
    }

    virtual ~tlm2_pv_av_initiator_adapter() = default;

private:
    tlm::tlm_sync_enum nb_transport_fw(tlm_payload_type& trans, tlm_phase_type& phase, sc_core::sc_time& t) {
        return isck->nb_transport_fw(trans, phase, t);
    };

    void b_transport(tlm_payload_type& trans, sc_core::sc_time& t) { isck->b_transport(trans, t); }

    bool get_direct_mem_ptr(tlm_payload_type& trans, tlm_dmi& dmi_data) {
        return isck->get_direct_mem_ptr(trans, dmi_data);
    }

    unsigned int transport_dbg(tlm_payload_type& trans) { return isck->transport_dbg(trans); }

    tlm::tlm_sync_enum nb_transport_bw(tlm_payload_type& trans, tlm_phase_type& phase, sc_core::sc_time& t) {
        if(av_tsck)
            return (*av_tsck)->nb_transport_bw(trans, phase, t);
        trans.set_response_status(tlm::TLM_GENERIC_ERROR_RESPONSE);
        return tlm::TLM_COMPLETED;
    }

    void invalidate_direct_mem_ptr(sc_dt::uint64 start_range, sc_dt::uint64 end_range) {
        if(pv_tsck)
            (*pv_tsck)->invalidate_direct_mem_ptr(start_range, end_range);
    }

    std::unique_ptr<target_socket_type> pv_tsck{};
    std::unique_ptr<target_socket_type> av_tsck{};
};

} // namespace scc
} /* namespace tlm */

#endif /* OSCI_LIB_SCC_INCL_TLM_TLM2_PV_AV_TARGET_ADAPTER_H_ */
