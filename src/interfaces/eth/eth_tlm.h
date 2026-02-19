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

#ifndef _ETH_ETH_TLM_H_
#define _ETH_ETH_TLM_H_

#include <atomic>
#include <cci_configuration>
#include <cstdint>
#include <mutex>
#include <nonstd/span.h>
#include <scc/fifo_w_cb.h>
#include <scc/peq.h>
#include <scc/report.h>
#include <scc/sc_variable.h>
#include <tlm/nw/tlm_network_gp.h>
#include <tlm/nw/tlm_network_sockets.h>
#include <tlm/scc/tlm_gp_shared.h>
#include <tlm/scc/tlm_mm.h>

//! @brief ETH TLM utilities
namespace eth {

inline std::uint16_t bswap16(std::uint16_t x) { return static_cast<std::uint16_t>((x >> 8) | (x << 8)); }

#pragma pack(push, 1)
struct ethernet_header {
    std::array<std::uint8_t, 6> dst;
    std::array<std::uint8_t, 6> src;
    std::uint16_t type_or_len; // big-endian on the wire (e.g., 0x0800 IPv4, 0x86DD IPv6)
};

struct vlan_tag_8021Q {
    std::uint16_t tpid; // 0x8100 (big-endian)
    std::uint16_t tci;  // PCP(3) | DEI(1) | VID(12) (big-endian)
};
#pragma pack(pop)

static_assert(sizeof(ethernet_header) == 14);
static_assert(sizeof(vlan_tag_8021Q) == 4);

enum class ether_kind { ETHER_TYPE, LENGTH_8023 };

struct ethernet_frame {
    ethernet_header l2; // original 14-byte header
    bool has_vlan = false;
    std::uint16_t vlan_tpid = 0; // host order (e.g., 0x8100)
    std::uint16_t vlan_id = 0;   // 0..4095
    std::uint16_t vlan_pcp = 0;  // 0..7
    bool vlan_dei = false;

    ether_kind kind = ether_kind::ETHER_TYPE;
    std::uint16_t etherType_or_len = 0; // host order: EtherType or 802.3 length
    nonstd::span<const std::uint8_t> payload;
    static ethernet_frame parse_ethernet(nonstd::span<const std::uint8_t> frame);
};

//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
enum class ETH { FRAME };
struct eth_packet_payload : public tlm::nw::tlm_network_payload<ETH> {
    eth_packet_payload() { m_data.resize(1536); };

    explicit eth_packet_payload(tlm::nw::tlm_base_mm_interface* mm)
    : tlm::nw::tlm_network_payload<ETH>(mm) {
        m_data.resize(1536);
    }

    const sc_core::sc_time& get_sender_clk_period() const { return sender_clk_period; }

    void set_sender_clk_period(sc_core::sc_time period) { this->sender_clk_period = period; }

    const uint64_t unique_id{get_id()};

private:
    sc_core::sc_time sender_clk_period{sc_core::SC_ZERO_TIME};

    static uint64_t get_id() {
        static std::atomic<uint64_t> id{0};
        return id.fetch_add(1);
    }
};

struct eth_packet_types {
    using tlm_payload_type = eth_packet_payload;
    using tlm_phase_type = ::tlm::tlm_phase;
};

} // namespace eth

namespace tlm {
namespace scc {
// provide needed info for SCC memory manager
template <> struct tlm_mm_traits<eth::eth_packet_types> {
    using mm_if_type = tlm::nw::tlm_base_mm_interface;
    using payload_base = tlm::nw::tlm_network_payload_base;
};
} // namespace scc
} // namespace tlm

namespace eth {
template <int N = 1> using eth_pkt_initiator_socket = tlm::nw::tlm_network_initiator_socket<1, ETH, eth_packet_types, N>;
template <int N = 1> using eth_pkt_target_socket = tlm::nw::tlm_network_target_socket<1, ETH, eth_packet_types, N>;
using eth_pkt_shared_ptr = tlm::scc::tlm_payload_shared_ptr<eth_packet_payload>;
using eth_pkt_mm = tlm::scc::tlm_mm<eth_packet_types, false>;

struct eth_channel : public sc_core::sc_module,
                     public tlm::nw::tlm_network_fw_transport_if<eth_packet_types>,
                     public tlm::nw::tlm_network_bw_transport_if<eth_packet_types> {

    using transaction_type = eth_packet_types::tlm_payload_type;
    using phase_type = eth_packet_types::tlm_phase_type;

    eth_pkt_target_socket<> tsck{"tsck"};

    eth_pkt_initiator_socket<> isck{"isck"};

    cci::cci_param<sc_core::sc_time> channel_delay{"channel_delay", sc_core::SC_ZERO_TIME, "delay of the ETH channel"};

    eth_channel(sc_core::sc_module_name const& nm)
    : sc_core::sc_module(nm)
    , isck{"isckt"} {
        isck(*this);
        tsck(*this);
    }

    void b_transport(transaction_type& trans, sc_core::sc_time& t) override {
        t += trans.get_data().size() * 8 * trans.get_sender_clk_period() + channel_delay.get_value();
        isck->b_transport(trans, t);
    }
    // TODO: fix non-blocking channel timing
    tlm::tlm_sync_enum nb_transport_fw(transaction_type& trans, phase_type& phase, sc_core::sc_time& t) override {
        SCCTRACE(SCMOD) << "Received non-blocking transaction in fw path with phase " << phase.get_name();
        if(phase == tlm::nw::REQUEST) {
            phase_type ph = tlm::nw::INDICATION;
            auto ret = isck->nb_transport_fw(trans, ph, t);
            if(ph == tlm::nw::RESPONSE)
                phase = tlm::nw::CONFIRM;
            return ret;
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
} // namespace eth
#endif // _ETH_ETH_TLM_H_
