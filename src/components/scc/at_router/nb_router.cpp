#include "nb_router.h"
#include <memory>

namespace scc {
namespace at_router {

namespace crossbar {
template <>
std::unique_ptr<nb_router<tlm::tlm_base_protocol_types>> create<tlm::tlm_base_protocol_types>(unsigned bus_width, unsigned igress_cnt,
                                                                                              unsigned egress_cnt,
                                                                                              util::range_lut<unsigned> const& decoder) {
    auto rt = std::make_unique<nb_router<tlm::tlm_base_protocol_types>>();
    rt->igress.init(igress_cnt);
    rt->egress.init(egress_cnt);
    // create the decoders and size the iport vector
    rt->decoder.init(igress_cnt,
                     [&decoder](char const* name, size_t) { return new nb_decoder<tlm::tlm_base_protocol_types>(name, decoder); });
    for(auto i = 0u; i < igress_cnt; ++i) {
        rt->decoder[i].iport.init(egress_cnt);
        rt->igress[i].fw(rt->decoder[i].tport.fw);
        rt->decoder[i].tport.bw(rt->igress[i].bw);
    }
    // create the arbiter and size the tport vector
    rt->arbiter.init(egress_cnt,
                     [bus_width](char const* name, size_t) { return new nb_arbiter<tlm::tlm_base_protocol_types>(name, bus_width); });
    for(auto i = 0u; i < egress_cnt; ++i) {
        rt->arbiter[i].tport.init(igress_cnt);
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
} // namespace at_router
} // namespace scc