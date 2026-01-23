#include "eth_tlm.h"
namespace eth {
ethernet_frame ethernet_frame::parse_ethernet(nonstd::span<const std::uint8_t> frame) {
    ethernet_frame eth_frame{};

    if(frame.size() < sizeof(ethernet_header))
        throw std::runtime_error("frame too small for Ethernet header");

    // Copy header safely (no unaligned access)
    std::memcpy(&eth_frame.l2, frame.data(), sizeof(ethernet_header));
    std::uint16_t t = bswap16(eth_frame.l2.type_or_len);

    std::size_t offset = sizeof(ethernet_header);

    // VLAN TPIDs commonly: 0x8100 (802.1Q), 0x88A8 (802.1ad/QinQ)
    auto is_vlan_tpid = [](std::uint16_t x) { return x == 0x8100 || x == 0x88A8; };

    if(is_vlan_tpid(t)) {
        if(frame.size() < offset + sizeof(vlan_tag_8021Q) + 2)
            throw std::runtime_error("frame too small for VLAN tag");

        vlan_tag_8021Q tag{};
        std::memcpy(&tag, frame.data() + offset, sizeof(tag));
        eth_frame.has_vlan = true;
        eth_frame.vlan_tpid = t;

        std::uint16_t tci = bswap16(tag.tci);
        eth_frame.vlan_pcp = (tci >> 13) & 0x7;
        eth_frame.vlan_dei = ((tci >> 12) & 0x1) != 0;
        eth_frame.vlan_id = tci & 0x0FFF;

        offset += sizeof(vlan_tag_8021Q);

        // Next 2 bytes after VLAN tag is the *real* EtherType/Len
        std::uint16_t inner{};
        std::memcpy(&inner, frame.data() + offset, sizeof(inner));
        t = bswap16(inner);
        offset += sizeof(inner);
    }

    // Distinguish Ethernet II EtherType vs 802.3 length:
    // <=1500 => length, >=1536 (0x0600) => EtherType
    if(t <= 1500) {
        eth_frame.kind = ether_kind::LENGTH_8023;
        eth_frame.etherType_or_len = t;
    } else {
        eth_frame.kind = ether_kind::ETHER_TYPE;
        eth_frame.etherType_or_len = t;
    }

    if(frame.size() < offset)
        throw std::runtime_error("internal parse error (offset beyond frame)");

    eth_frame.payload = frame.subspan(offset);
    return eth_frame;
}
} // namespace eth