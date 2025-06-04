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

#ifndef _AXIS_AXIS_TLM_H_
#define _AXIS_AXIS_TLM_H_

#include <cci_configuration>
#include <cstdint>
#include <scc/fifo_w_cb.h>
#include <scc/peq.h>
#include <scc/report.h>
#include <scc/sc_variable.h>
#include <tlm/nw/tlm_network_gp.h>
#include <tlm/nw/tlm_network_sockets.h>
#include <tlm/scc/tlm_gp_shared.h>
#include <tlm/scc/tlm_mm.h>

//! @brief AXIS TLM utilities
namespace axis {
enum class AXIS_PKT { DATA };
struct axis_packet_payload : public tlm::nw::tlm_network_payload<AXIS_PKT> {
    axis_packet_payload() = default;

    explicit axis_packet_payload(tlm::nw::tlm_base_mm_interface* mm)
    : tlm::nw::tlm_network_payload<AXIS_PKT>(mm) {}

    const sc_core::sc_time& get_sender_clk_period() const { return sender_clk_period; }

    void set_sender_clk_period(sc_core::sc_time period) { this->sender_clk_period = sender_clk_period; }

private:
    sc_core::sc_time sender_clk_period{sc_core::SC_ZERO_TIME};
};

struct axis_packet_types {
    using tlm_payload_type = axis_packet_payload;
    using tlm_phase_type = ::tlm::tlm_phase;
};

} // namespace axis

namespace tlm {
namespace scc {
// provide needed info for SCC memory manager
template <> struct tlm_mm_traits<axis::axis_packet_types> {
    using mm_if_type = tlm::nw::tlm_base_mm_interface;
    using payload_base = tlm::nw::tlm_network_payload_base;
};
} // namespace scc
} // namespace tlm

namespace axis {
template <int N = 1> using axis_pkt_initiator_socket = tlm::nw::tlm_network_initiator_socket<1, AXIS_PKT, axis_packet_types, N>;
template <int N = 1> using axis_pkt_target_socket = tlm::nw::tlm_network_target_socket<1, AXIS_PKT, axis_packet_types, N>;
using axis_pkt_shared_ptr = tlm::scc::tlm_payload_shared_ptr<axis_packet_payload>;
using axis_pkt_mm = tlm::scc::tlm_mm<axis_packet_types, false>;

struct axis_channel : public sc_core::sc_module,
                     public tlm::nw::tlm_network_fw_transport_if<axis_packet_types>,
                     public tlm::nw::tlm_network_bw_transport_if<axis_packet_types> {

    using transaction_type = axis_packet_types::tlm_payload_type;
    using phase_type = axis_packet_types::tlm_phase_type;

    axis_pkt_target_socket<> tsck{"tsck"};

    axis_pkt_initiator_socket<> isck{"isck"};

    cci::cci_param<sc_core::sc_time> channel_delay{"channel_delay", sc_core::SC_ZERO_TIME, "delay of the AXIS channel"};

    axis_channel(sc_core::sc_module_name const& nm, size_t slave_count)
    : sc_core::sc_module(nm)
    , isck{"isckt", slave_count} {
        for(auto& is : isck)
            is(*this);
        tsck(*this);
    }

    axis_pkt_target_socket<>& operator()() { return tsck; }

    axis_pkt_initiator_socket<>& operator()(size_t idx) { return isck.at(idx); }

    void b_transport(transaction_type& trans, sc_core::sc_time& t) override {
        t += channel_delay;
        isck->b_transport(trans, t);
    }

    tlm::tlm_sync_enum nb_transport_fw(transaction_type& trans, phase_type& phase, sc_core::sc_time& t) override {
        SCCTRACE(SCMOD) << "Received non-blocking transaction in fw path with phase " << phase.get_name();
        if(phase == tlm::nw::REQUEST) {
            phase_type ph = tlm::nw::INDICATION;
            auto ret = isck->nb_transport_fw(trans, ph, t);
            if(ph == tlm::nw::RESPONSE)
                phase = tlm::nw::CONFIRM;
            return ret;
        } else {
            trans.set_response_status(tlm::TLM_ADDRESS_ERROR_RESPONSE);
            phase = tlm::nw::CONFIRM;
            return tlm::TLM_COMPLETED;
        }
        return tlm::TLM_ACCEPTED;
    }

    tlm::tlm_sync_enum nb_transport_bw(transaction_type& trans, phase_type& phase, sc_core::sc_time& t) override {
        SCCTRACE(SCMOD) << "Received non-blocking transaction in bw path with phase " << phase.get_name();
        if(phase == tlm::nw::RESPONSE) {
            phase_type ph = tlm::nw::CONFIRM;
            return tsck->nb_transport_bw(trans, ph, t);
        }
        return tlm::TLM_ACCEPTED;
    }

    unsigned int transport_dbg(transaction_type& trans) override { return isck->transport_dbg(trans); }
};
} // namespace axis
#endif // _AXIS_AXIS_TLM_H_
