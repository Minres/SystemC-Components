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

#ifndef _SPI_SPI_TLM_H_
#define _SPI_SPI_TLM_H_

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

//! @brief SPI TLM utilities
namespace spi {
enum class SPI_PKT { DATA };
struct spi_packet_payload : public tlm::nw::tlm_network_payload<SPI_PKT> {
    spi_packet_payload() = default;

    spi_packet_payload(unsigned target_id):target_id(target_id) {}

    explicit spi_packet_payload(tlm::nw::tlm_base_mm_interface* mm)
    : tlm::nw::tlm_network_payload<SPI_PKT>(mm) {}

    explicit spi_packet_payload(tlm::nw::tlm_base_mm_interface* mm, unsigned target_id)
    : tlm::nw::tlm_network_payload<SPI_PKT>(mm),target_id(target_id) {}

    unsigned get_target_id() const {
        return target_id;
    }

private:
    unsigned target_id{0};
    sc_core::sc_time  sender_clk_period{sc_core::SC_ZERO_TIME};
};

struct spi_packet_types {
    using tlm_payload_type = spi_packet_payload;
    using tlm_phase_type = ::tlm::tlm_phase;
};

} // namespace spi

namespace tlm {
namespace scc {
// provide needed info for SCC memory manager
template <> struct tlm_mm_traits<spi::spi_packet_types> {
    using mm_if_type = tlm::nw::tlm_base_mm_interface;
    using payload_base = tlm::nw::tlm_network_payload_base;
};
} // namespace scc
} // namespace tlm

namespace spi {
template <int N = 1> using spi_pkt_initiator_socket = tlm::nw::tlm_network_initiator_socket<1, SPI_PKT, spi_packet_types, N>;
template <int N = 1> using spi_pkt_target_socket = tlm::nw::tlm_network_target_socket<1, SPI_PKT, spi_packet_types, N>;
using spi_pkt_shared_ptr = tlm::scc::tlm_payload_shared_ptr<spi_packet_payload>;
using spi_pkt_mm = tlm::scc::tlm_mm<spi_packet_types, false>;

struct spi_channel : public sc_core::sc_module,
                     public tlm::nw::tlm_network_fw_transport_if<spi_packet_types>,
                     public tlm::nw::tlm_network_bw_transport_if<spi_packet_types> {

    using transaction_type = spi_packet_types::tlm_payload_type;
    using phase_type = spi_packet_types::tlm_phase_type;

    spi_pkt_target_socket<> tsck{"tsck"};

    sc_core::sc_vector<spi_pkt_initiator_socket<>> isck{"isck"};

    cci::cci_param<sc_core::sc_time> channel_delay{"channel_delay", sc_core::SC_ZERO_TIME, "delay of the SPI channel"};

    spi_channel(sc_core::sc_module_name const& nm, size_t slave_count)
    : sc_core::sc_module(nm)
    , isck{"isckt", slave_count}{
        for(auto& is:isck) is(*this);
        tsck(*this);
    }

    spi_pkt_target_socket<>& operator()() {
        return tsck;
    }

    spi_pkt_initiator_socket<>& operator()(size_t idx) {
        return isck.at(idx);
    }

    void b_transport(transaction_type& trans, sc_core::sc_time& t) override {
        t += channel_delay;
        isck.at(trans.get_target_id())->b_transport(trans, t);
    }

    tlm::tlm_sync_enum nb_transport_fw(transaction_type& trans, phase_type& phase, sc_core::sc_time& t) override {
        SCCTRACE(SCMOD) << "Received non-blocking transaction in fw path with phase " << phase.get_name();
        if(phase== tlm::nw::REQUEST) {
            auto idx = trans.get_target_id();
            if(idx<isck.size()) {
                phase_type ph = tlm::nw::INDICATION;
                auto ret = isck.at(idx)->nb_transport_fw(trans, ph, t);
                if(ph==tlm::nw::RESPONSE)
                    phase = tlm::nw::CONFIRM;
                return ret;
            } else {
                trans.set_response_status(tlm::TLM_ADDRESS_ERROR_RESPONSE);
                phase=tlm::nw::CONFIRM;
                return tlm::TLM_COMPLETED;
            }
        }
        return tlm::TLM_ACCEPTED;
    }

    tlm::tlm_sync_enum nb_transport_bw(transaction_type& trans, phase_type& phase, sc_core::sc_time& t) override {
        SCCTRACE(SCMOD) << "Received non-blocking transaction in bw path with phase " << phase.get_name();
        if(phase== tlm::nw::RESPONSE) {
            phase_type ph = tlm::nw::CONFIRM;
            return tsck->nb_transport_bw(trans, ph, t);
        }
        return tlm::TLM_ACCEPTED;
    }

    unsigned int transport_dbg(transaction_type& trans) override { return isck.at(trans.get_target_id())->transport_dbg(trans); }

};
} // namespace spi
#endif // _SPI_SPI_TLM_H_
