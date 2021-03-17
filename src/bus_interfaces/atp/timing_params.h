#ifndef _TLM_TIMING_PARAMS_H_
#define _TLM_TIMING_PARAMS_H_

#include <tlm>

namespace atp {

struct timing_params : public tlm::tlm_extension<timing_params> {
    tlm_extension_base* clone() const override {
        timing_params* e = new timing_params(artv, awtv, wbv, rbr, br);
        return e;
    }
    void copy_from(tlm_extension_base const& from) override { assert(false && "copy_from() not supported"); }

    timing_params() = delete;
    timing_params(unsigned artv, unsigned awtv, unsigned wbv, unsigned rbr, unsigned br)
    : artv(artv)
    , awtv(awtv)
    , wbv(wbv)
    , rbr(rbr)
    , br(br) {}

    const unsigned artv;
    const unsigned awtv;
    const unsigned wbv;
    const unsigned rbr;
    const unsigned br;
};

} // namespace atp
#endif /* _TLM_TIMING_PARAMS_H_ */
