/*
 * packet.h
 *
 *  Created on: 04.05.2020
 *      Author: eyck
 */

#ifndef _SIM_PERFORMANCE_PACKET_H_
#define _SIM_PERFORMANCE_PACKET_H_

#include <tlm>

struct packet {
    std::vector<uint8_t> routing;
};

struct packet_ext : public tlm::tlm_extension<packet_ext>, public packet {

    packet_ext() = default;

    packet_ext& operator=(packet_ext const& o) = default;

    tlm_extension_base* clone() const override { return new packet_ext(*this); }

    void copy_from(tlm_extension_base const& o) override {
        auto* ext = dynamic_cast<packet_ext const*>(&o);
        if(ext)
            this->routing = ext->routing;
    }
};

#endif /* _SIM_PERFORMANCE_PACKET_H_ */
