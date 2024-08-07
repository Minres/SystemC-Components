/*******************************************************************************
 * Copyright 2021 MINRES Technologies GmbH
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

#ifndef _BUS_TILELINK_SIGNAL_IF_H_
#define _BUS_TILELINK_SIGNAL_IF_H_

#include <scc/signal_opt_ports.h>
#include <systemc>

namespace tilelink {

const sc_core::sc_time CLK_DELAY = 1_ps;

struct master_types {
    template <typename T> using m2s_t = sc_core::sc_out<T>;
    template <typename T> using s2m_t = sc_core::sc_in<T>;
    template <typename T> using m2s_full_t = sc_core::sc_out<T>;
    template <typename T> using s2m_full_t = sc_core::sc_in<T>;
    template <typename T> using m2s_opt_t = scc::sc_out_opt<T>;
    template <typename T> using s2m_opt_t = scc::sc_in_opt<T>;
};

struct slave_types {
    template <typename T> using m2s_t = sc_core::sc_in<T>;
    template <typename T> using s2m_t = sc_core::sc_out<T>;
    template <typename T> using m2s_full_t = sc_core::sc_in<T>;
    template <typename T> using s2m_full_t = sc_core::sc_out<T>;
    template <typename T> using m2s_opt_t = scc::sc_in_opt<T>;
    template <typename T> using s2m_opt_t = scc::sc_out_opt<T>;
};

struct signal_types {
    template <typename T> using m2s_t = sc_core::sc_signal<T>;
    template <typename T> using s2m_t = sc_core::sc_signal<T>;
    template <typename T> using m2s_full_t = sc_core::sc_signal<T>;
    template <typename T> using s2m_full_t = sc_core::sc_signal<T>;
    template <typename T> using m2s_opt_t = sc_core::sc_signal<T>;
    template <typename T> using s2m_opt_t = sc_core::sc_signal<T>;
};

template <bool Cond, class T, class S> struct select_if { typedef S type; };
template <class T, class S> struct select_if<true, T, S> { typedef T type; };

/**
 * @struct tl_cfg
 * @brief
 *
 * @tparam W: Width of the data bus in bytes. Must be a power of two.
 * @tparam A: Width of each address field in bits.
 * @tparam Z: Width of each size field in bits.
 * @tparam O: Number of bits needed to disambiguate per-link master sources.
 * @tparam I: cacheline size in Bytes, defaults value is 64 bytes
 */
template <unsigned int W = 32, unsigned int A = 32, unsigned int Z = 32, unsigned int O = 1,
          unsigned int I = 3>
struct tl_cfg {

    static_assert(W > 0, "W shall be larger than 0");
    static_assert(W <= 4096, "W shall be less than or equal 4096");
    // static_assert(CACHELINE > 0);
    static_assert(A >= 0, "A shall be larger than or equal 0");
    static_assert(A <= 128, "A shall be less than or equal 128");
    static_assert(Z > 0, "Z shall be larger than 0");
    static_assert(Z < 5, "Z shall be less than or equal 4");
    static_assert(O >= 0, "O shall be larger than or equal 0");
    static_assert(O <= 64, "O shall be less than or equal 64");
    static_assert(I >= 0, "I shall be larger than or equal 0");
    static_assert(I <= 64, "I shall be less than or equal 64");
    constexpr static unsigned int BUSWIDTH = 8*W;
    constexpr static unsigned int MASKWIDTH = W;
    constexpr static unsigned int ADDRWIDTH = A;
    constexpr static unsigned int SZWIDTH = Z;
    constexpr static unsigned int MIDWIDTH = O;
    constexpr static unsigned int SIDWIDTH = I;
    using addr_t = typename select_if<A <= 64, sc_dt::sc_uint<ADDRWIDTH>, sc_dt::sc_biguint<ADDRWIDTH>>::type;
    using mask_t = typename select_if<W <= 64, sc_dt::sc_uint<MASKWIDTH>, sc_dt::sc_biguint<MASKWIDTH>>::type;
    using data_t = typename select_if<(8*W) <= 64, sc_dt::sc_uint<BUSWIDTH>, sc_dt::sc_biguint<BUSWIDTH>>::type;
    using slave_types = ::tilelink::slave_types;
    using master_types = ::tilelink::master_types;
};

inline std::string concat(const char* prefix, const char* name) { return std::string(prefix) + name; }

//! A channel signals
template <typename CFG, typename TYPES = master_types> struct ch_a {
    typename TYPES::template m2s_t<sc_dt::sc_uint<3>> code{"code"};
    typename TYPES::template m2s_t<sc_dt::sc_uint<3>> param{"param"};
    typename TYPES::template m2s_t<sc_dt::sc_uint<CFG::SZWIDTH>> size{"size"};
    typename TYPES::template m2s_t<sc_dt::sc_uint<CFG::MIDWIDTH>> source{"source"};
    typename TYPES::template m2s_t<CFG::addr_t> address{"address"};
    typename TYPES::template m2s_t<CFG::mask_t> mask{"mask"};
    typename TYPES::template m2s_t<CFG::data_t> data{"data"};
    typename TYPES::template s2m_t<bool> corrupt{"corrupt"};
    typename TYPES::template m2s_t<bool> valid{"valid"};
    typename TYPES::template s2m_t<bool> ready{"ready"};

    ch_a() = default;
    ch_a(const char* prefix)
    : code{concat(prefix, "_code").c_str()}
    , param{concat(prefix, "_param").c_str()}
    , size{concat(prefix, "_size").c_str()}
    , source{concat(prefix, "_source").c_str()}
    , address{concat(prefix, "_address").c_str()}
    , mask{concat(prefix, "_mask").c_str()}
    , data{concat(prefix, "_data").c_str()}
    , corrupt{concat(prefix, "_corrupt").c_str()}
    , valid{concat(prefix, "_valid").c_str()}
    , ready{concat(prefix, "_ready").c_str()} {}

    template <typename OTYPES> void bind_a(ch_a<CFG, OTYPES>& o) {
        code.bind(o.code);
        param.bind(o.param);
        size.bind(o.size);
        source.bind(o.source);
        address.bind(o.address);
        mask.bind(o.mask);
        data.bind(o.data);
        corrupt.bind(o.corrupt);
        valid.bind(o.valid);
        ready.bind(o.ready);
    }
};

//! B channel signals
template <typename CFG, typename TYPES = master_types> struct ch_b {
    typename TYPES::template s2m_t<sc_dt::sc_uint<3>> code{"code"};
    typename TYPES::template s2m_t<sc_dt::sc_uint<3>> param{"param"};
    typename TYPES::template s2m_t<sc_dt::sc_uint<CFG::SZWIDTH>> size{"size"};
    typename TYPES::template s2m_t<sc_dt::sc_uint<CFG::MIDWIDTH>> source{"source"};
    typename TYPES::template s2m_t<CFG::addr_t> address{"address"};
    typename TYPES::template s2m_t<CFG::mask_t> mask{"mask"};
    typename TYPES::template s2m_t<CFG::data_t> data{"data"};
    typename TYPES::template m2s_t<bool> corrupt{"corrupt"};
    typename TYPES::template s2m_t<bool> valid{"valid"};
    typename TYPES::template m2s_t<bool> ready{"ready"};

    ch_b() = default;
    ch_b(const char* prefix)
    : code{concat(prefix, "_code").c_str()}
    , param{concat(prefix, "_param").c_str()}
    , size{concat(prefix, "_size").c_str()}
    , source{concat(prefix, "_source").c_str()}
    , address{concat(prefix, "_address").c_str()}
    , mask{concat(prefix, "_mask").c_str()}
    , data{concat(prefix, "_data").c_str()}
    , corrupt{concat(prefix, "_corrupt").c_str()}
    , valid{concat(prefix, "_valid").c_str()}
    , ready{concat(prefix, "_ready").c_str()} {}

    template <typename OTYPES> void bind_b(ch_b<CFG, OTYPES>& o) {
        code.bind(o.code);
        param.bind(o.param);
        size.bind(o.size);
        source.bind(o.source);
        address.bind(o.address);
        mask.bind(o.mask);
        data.bind(o.data);
        corrupt.bind(o.corrupt);
        valid.bind(o.valid);
        ready.bind(o.ready);
    }
};

//! C channel signals
template <typename CFG, typename TYPES = master_types> struct ch_c {
    typename TYPES::template m2s_t<sc_dt::sc_uint<3>> code{"code"};
    typename TYPES::template m2s_t<sc_dt::sc_uint<3>> param{"param"};
    typename TYPES::template m2s_t<sc_dt::sc_uint<CFG::SZWIDTH>> size{"size"};
    typename TYPES::template m2s_t<sc_dt::sc_uint<CFG::MIDWIDTH>> source{"source"};
    typename TYPES::template m2s_t<CFG::addr_t> address{"address"};
    typename TYPES::template m2s_t<CFG::data_t> data{"data"};
    typename TYPES::template s2m_t<bool> corrupt{"corrupt"};
    typename TYPES::template m2s_t<bool> valid{"valid"};
    typename TYPES::template s2m_t<bool> ready{"ready"};

    ch_c() = default;
    ch_c(const char* prefix)
    : code{concat(prefix, "_code").c_str()}
    , param{concat(prefix, "_param").c_str()}
    , size{concat(prefix, "_size").c_str()}
    , source{concat(prefix, "_source").c_str()}
    , address{concat(prefix, "_address").c_str()}
    , data{concat(prefix, "_data").c_str()}
    , corrupt{concat(prefix, "_corrupt").c_str()}
    , valid{concat(prefix, "_valid").c_str()}
    , ready{concat(prefix, "_ready").c_str()} {}

    template <typename OTYPES> void bind_c(ch_b<CFG, OTYPES>& o) {
        code.bind(o.code);
        param.bind(o.param);
        size.bind(o.size);
        source.bind(o.source);
        address.bind(o.address);
        data.bind(o.data);
        corrupt.bind(o.corrupt);
        valid.bind(o.valid);
        ready.bind(o.ready);
    }
};

//! D channel signals
template <typename CFG, typename TYPES = master_types> struct ch_d {
    typename TYPES::template s2m_t<sc_dt::sc_uint<3>> code{"code"};
    typename TYPES::template s2m_t<sc_dt::sc_uint<2>> param{"param"};
    typename TYPES::template s2m_t<sc_dt::sc_uint<CFG::SZWIDTH>> size{"size"};
    typename TYPES::template s2m_t<sc_dt::sc_uint<CFG::MIDWIDTH>> source{"source"};
    typename TYPES::template s2m_t<sc_dt::sc_uint<CFG::SIDWIDTH>> sink{"sink"};
    typename TYPES::template m2s_t<bool> denied{"denied"};
    typename TYPES::template s2m_t<CFG::data_t> data{"data"};
    typename TYPES::template m2s_t<bool> corrupt{"corrupt"};
    typename TYPES::template s2m_t<bool> valid{"valid"};
    typename TYPES::template m2s_t<bool> ready{"ready"};

    ch_d() = default;
    ch_d(const char* prefix)
    : code{concat(prefix, "_code").c_str()}
    , param{concat(prefix, "_param").c_str()}
    , size{concat(prefix, "_size").c_str()}
    , source{concat(prefix, "_source").c_str()}
    , sink{concat(prefix, "_sink").c_str()}
    , denied{concat(prefix, "_denied").c_str()}
    , data{concat(prefix, "_data").c_str()}
    , corrupt{concat(prefix, "_corrupt").c_str()}
    , valid{concat(prefix, "_valid").c_str()}
    , ready{concat(prefix, "_ready").c_str()} {}

    template <typename OTYPES> void bind_d(ch_d<CFG, OTYPES>& o) {
        code.bind(o.code);
        param.bind(o.param);
        size.bind(o.size);
        source.bind(o.source);
        sink.bind(o.sink);
        denied.bind(o.denied);
        data.bind(o.data);
        corrupt.bind(o.corrupt);
        valid.bind(o.valid);
        ready.bind(o.ready);
    }
};

//! E channel signals
template <typename CFG, typename TYPES = master_types> struct ch_e {
    typename TYPES::template m2s_t<sc_dt::sc_uint<CFG::SIDWIDTH>> sink{"sink"};
    typename TYPES::template m2s_t<bool> valid{"valid"};
    typename TYPES::template s2m_t<bool> ready{"ready"};

    ch_e() = default;
    ch_e(const char* prefix)
    : sink{concat(prefix, "_sink").c_str()}
    , valid{concat(prefix, "_valid").c_str()}
    , ready{concat(prefix, "_ready").c_str()} {}

    template <typename OTYPES> void bind_e(ch_e<CFG, OTYPES>& o) {
        sink.bind(o.sink);
        valid.bind(o.valid);
        ready.bind(o.ready);
    }
};

template <typename CFG, typename TYPES = master_types> struct tl_ul: public ch_a<CFG, TYPES>, public ch_d<CFG, TYPES> {
    tl_ul(): ch_a<CFG, TYPES>("a_"), ch_d<CFG, TYPES>("d_"){}
    tl_ul(const char* prefix)
    : ch_a<CFG, TYPES>(concat(prefix, "_a_").c_str()), ch_d<CFG, TYPES>(concat(prefix, "_d_").c_str()) {}
    template <typename OTYPES> void bind(tl_ul<CFG, OTYPES>& o) {
        bind_a(o);
        bind_d(o);
    }
};

template <typename CFG, typename TYPES = master_types>
using tl_uh = tl_ul<CFG, TYPES>;

template <typename CFG, typename TYPES = master_types> struct tl_c: public ch_a<CFG, TYPES>, public ch_b<CFG, TYPES>, public ch_c<CFG, TYPES>, public ch_d<CFG, TYPES>, public ch_e<CFG, TYPES> {
    tl_c(): ch_a<CFG, TYPES>("a_"), ch_b<CFG, TYPES>("b_"), ch_c<CFG, TYPES>("c_"), ch_d<CFG, TYPES>("d_"), ch_e<CFG, TYPES>("e_"){}
    tl_c(const char* prefix)
    : ch_a<CFG, TYPES>(concat(prefix, "_a_").c_str())
    , ch_b<CFG, TYPES>(concat(prefix, "_b_").c_str())
    , ch_c<CFG, TYPES>(concat(prefix, "_c_").c_str())
    , ch_d<CFG, TYPES>(concat(prefix, "_d_").c_str())
    , ch_e<CFG, TYPES>(concat(prefix, "_e_").c_str())
      {}
    template <typename OTYPES> void bind(tl_c<CFG, OTYPES>& o) {
        bind_a(o);
        bind_b(o);
        bind_c(o);
        bind_d(o);
        bind_r(o);
    }
};

} // namespace tl
#endif /* _BUS_TILELINK_SIGNAL_IF_H_ */
