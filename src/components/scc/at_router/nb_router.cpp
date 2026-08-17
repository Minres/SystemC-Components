#include "nb_router.h"
#include <memory>

namespace scc {
namespace at_router {

namespace crossbar {
template <>
std::unique_ptr<nb_router<tlm::tlm_base_protocol_types>>
create<tlm::tlm_base_protocol_types>(unsigned bus_width, unsigned igress_cnt, unsigned egress_cnt, util::range_lut<unsigned> const& decoder,
                                     std::vector<range_entry> const& tranges, sc_core::sc_time const& clk_period) {
    auto rt = std::make_unique<nb_router<tlm::tlm_base_protocol_types>>(clk_period);
    rt->igress.init(igress_cnt);
    rt->egress.init(egress_cnt);
    // create the decoders and size the iport vector
    rt->decoder.init(igress_cnt, [&decoder, &tranges, egress_cnt](char const* name, size_t) {
        return new nb_decoder<tlm::tlm_base_protocol_types>(name, egress_cnt, decoder, tranges);
    });
    for(auto i = 0u; i < igress_cnt; ++i) {
        rt->igress[i].fw(rt->decoder[i].tport.fw);
        rt->decoder[i].tport.bw(rt->igress[i].bw);
    }
    // create the arbiter and size the tport vector
    rt->arbiter.init(egress_cnt, [bus_width, igress_cnt](char const* name, size_t) {
        return new nb_arbiter<tlm::tlm_base_protocol_types>(name, bus_width, igress_cnt);
    });
    for(auto i = 0u; i < egress_cnt; ++i) {
        rt->arbiter[i].iport.fw(rt->egress[i].fw);
        rt->egress[i].bw(rt->arbiter[i].iport.bw);
    }
    // connect the decoder iports to the arbiter tport
    for(auto i = 0u; i < igress_cnt; ++i) {
        auto& d = rt->decoder[i];
        for(auto j = 0u; j < egress_cnt; ++j) {
            auto& a = rt->arbiter[j];
            d.iport[j].fw(a.tport[i].fw);
            a.tport[i].bw(d.iport[j].bw);
        }
    }
    return rt;
}
} // namespace crossbar
namespace hub {
template <>
std::unique_ptr<nb_router<tlm::tlm_base_protocol_types>>
create<tlm::tlm_base_protocol_types>(unsigned bus_width, unsigned igress_cnt, unsigned egress_cnt, util::range_lut<unsigned> const& decoder,
                                     std::vector<range_entry> const& tranges, sc_core::sc_time const& clk_period) {
    auto rt = std::make_unique<nb_router<tlm::tlm_base_protocol_types>>(clk_period);
    rt->igress.init(igress_cnt);
    rt->egress.init(egress_cnt);
    // create the decoders and size the iport vector
    rt->decoder.init(1, [&decoder, &tranges, egress_cnt](char const* name, size_t) {
        return new nb_decoder<tlm::tlm_base_protocol_types>(name, egress_cnt, decoder, tranges);
    });
    for(auto i = 0u; i < egress_cnt; ++i) {
        rt->decoder[0].iport[i].fw(rt->egress[i].fw);
        rt->egress[i].bw(rt->decoder[0].iport[i].bw);
    }
    // create the arbiter and size the tport vector
    rt->arbiter.init(1, [bus_width, igress_cnt](char const* name, size_t) {
        return new nb_arbiter<tlm::tlm_base_protocol_types>(name, bus_width, igress_cnt);
    });
    for(auto i = 0u; i < igress_cnt; ++i) {
        rt->igress[i].fw(rt->arbiter[0].tport[i].fw);
        rt->arbiter[0].tport[i].bw(rt->igress[i].bw);
    }
    // connect the decoder iports to the arbiter tport
    rt->arbiter[0].iport.bind(rt->decoder[0].tport);
    return rt;
}
} // namespace hub
} // namespace at_router
} // namespace scc