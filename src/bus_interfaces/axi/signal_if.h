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

#ifndef _BUS_AXI_SIGNAL_IF_H_
#define _BUS_AXI_SIGNAL_IF_H_

#include <systemc>

namespace axi {

template <unsigned WIDTH = 0, typename TYPE = sc_dt::sc_uint<WIDTH>, int N = 1>
using sc_in_opt = sc_core::sc_port<sc_core::sc_signal_in_if<TYPE>, N, sc_core::SC_ZERO_OR_MORE_BOUND>;
template <unsigned WIDTH = 0, typename TYPE = sc_dt::sc_uint<WIDTH>, int N = 1>
using sc_out_opt = sc_core::sc_port<sc_core::sc_signal_write_if<TYPE>, N, sc_core::SC_ZERO_OR_MORE_BOUND>;

struct master_types {
    template <typename T> using m2s_t = sc_core::sc_out<T>;
    template <typename T> using s2m_t = sc_core::sc_in<T>;
    template <typename T> using m2s_full_t = sc_core::sc_out<T>;
    template <typename T> using s2m_full_t = sc_core::sc_in<T>;
    template <typename T>
    using m2s_opt_t = sc_core::sc_port<sc_core::sc_signal_write_if<T>, 1, sc_core::SC_ZERO_OR_MORE_BOUND>;
    template <typename T>
    using s2m_opt_t = sc_core::sc_port<sc_core::sc_signal_in_if<T>, 1, sc_core::SC_ZERO_OR_MORE_BOUND>;
};

struct lite_master_types {
    template <typename T> using m2s_t = sc_core::sc_out<T>;
    template <typename T> using s2m_t = sc_core::sc_in<T>;
    template <typename T>
    using m2s_full_t = sc_core::sc_port<sc_core::sc_signal_write_if<T>, 1, sc_core::SC_ZERO_OR_MORE_BOUND>;
    template <typename T>
    using s2m_full_t = sc_core::sc_port<sc_core::sc_signal_in_if<T>, 1, sc_core::SC_ZERO_OR_MORE_BOUND>;
    template <typename T>
    using m2s_opt_t = sc_core::sc_port<sc_core::sc_signal_write_if<T>, 1, sc_core::SC_ZERO_OR_MORE_BOUND>;
    template <typename T>
    using s2m_opt_t = sc_core::sc_port<sc_core::sc_signal_in_if<T>, 1, sc_core::SC_ZERO_OR_MORE_BOUND>;
};

struct slave_types {
    template <typename T> using m2s_t = sc_core::sc_in<T>;
    template <typename T> using s2m_t = sc_core::sc_out<T>;
    template <typename T> using m2s_full_t = sc_core::sc_in<T>;
    template <typename T> using s2m_full_t = sc_core::sc_out<T>;
    template <typename T>
    using m2s_opt_t = sc_core::sc_port<sc_core::sc_signal_in_if<T>, 1, sc_core::SC_ZERO_OR_MORE_BOUND>;
    template <typename T>
    using s2m_opt_t = sc_core::sc_port<sc_core::sc_signal_write_if<T>, 1, sc_core::SC_ZERO_OR_MORE_BOUND>;
};

struct lite_slave_types {
    template <typename T> using m2s_t = sc_core::sc_in<T>;
    template <typename T> using s2m_t = sc_core::sc_out<T>;
    template <typename T>
    using m2s_full_t = sc_core::sc_port<sc_core::sc_signal_in_if<T>, 1, sc_core::SC_ZERO_OR_MORE_BOUND>;
    template <typename T>
    using s2m_full_t = sc_core::sc_port<sc_core::sc_signal_write_if<T>, 1, sc_core::SC_ZERO_OR_MORE_BOUND>;
    template <typename T>
    using m2s_opt_t = sc_core::sc_port<sc_core::sc_signal_in_if<T>, 1, sc_core::SC_ZERO_OR_MORE_BOUND>;
    template <typename T>
    using s2m_opt_t = sc_core::sc_port<sc_core::sc_signal_write_if<T>, 1, sc_core::SC_ZERO_OR_MORE_BOUND>;
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

template <unsigned int BUSWDTH = 32, unsigned int ADDRWDTH = 32, unsigned int IDWDTH = 32, unsigned int USERWDTH = 1>
struct axi4_cfg {
    static_assert(BUSWDTH > 0);
    static_assert(ADDRWDTH > 0);
    static_assert(IDWDTH > 0);
    constexpr static bool IS_LITE = false;
    constexpr static unsigned int BUSWIDTH = BUSWDTH;
    constexpr static unsigned int ADDRWIDTH = ADDRWDTH;
    constexpr static unsigned int IDWIDTH = IDWDTH;
    constexpr static unsigned int USERWIDTH = USERWDTH;
    using data_t = typename select_if<BUSWDTH <= 64, sc_dt::sc_uint<BUSWIDTH>, sc_dt::sc_biguint<BUSWIDTH>>::type;
    using slave_types = ::axi::slave_types;
    using master_types = ::axi::master_types;
};

template <unsigned int BUSWDTH = 32, unsigned int ADDRWDTH = 32> struct axi4_lite_cfg {
    static_assert(BUSWDTH > 0);
    static_assert(ADDRWDTH > 0);
    constexpr static bool IS_LITE = true;
    constexpr static unsigned int BUSWIDTH = BUSWDTH;
    constexpr static unsigned int ADDRWIDTH = ADDRWDTH;
    constexpr static unsigned int IDWIDTH = 0;
    constexpr static unsigned int USERWIDTH = 0;
    using data_t = typename select_if<BUSWDTH <= 64, sc_dt::sc_uint<BUSWIDTH>, sc_dt::sc_biguint<BUSWIDTH>>::type;
    using slave_types = ::axi::lite_slave_types;
    using master_types = ::axi::lite_master_types;
};

inline std::string concat(const char* prefix, const char* name) { return std::string(prefix) + name; }

//! Write address channel signals
template <typename CFG, typename TYPES = master_types> struct aw_ch {
    typename TYPES::template m2s_full_t<sc_dt::sc_uint<CFG::IDWIDTH>> aw_id{"aw_id"};
    typename TYPES::template m2s_t<sc_dt::sc_uint<CFG::ADDRWIDTH>> aw_addr{"aw_addr"};
    typename TYPES::template s2m_t<bool> aw_ready{"aw_ready"};
    typename TYPES::template m2s_opt_t<sc_dt::sc_uint<2>> aw_lock{"aw_lock"}; // only AXI3
    typename TYPES::template m2s_t<bool> aw_valid{"aw_valid"};
    typename TYPES::template m2s_t<sc_dt::sc_uint<3>> aw_prot{"aw_prot"};
    typename TYPES::template m2s_full_t<sc_dt::sc_uint<3>> aw_size{"aw_size"};
    typename TYPES::template m2s_full_t<sc_dt::sc_uint<4>> aw_cache{"aw_cache"};
    typename TYPES::template m2s_full_t<sc_dt::sc_uint<2>> aw_burst{"aw_burst"};
    typename TYPES::template m2s_full_t<sc_dt::sc_uint<4>> aw_qos{"aw_qos"};
    typename TYPES::template m2s_full_t<sc_dt::sc_uint<4>> aw_region{"aw_region"};
    typename TYPES::template m2s_full_t<sc_dt::sc_uint<8>> aw_len{"aw_len"};
    typename TYPES::template m2s_opt_t<sc_dt::sc_uint<CFG::USERWIDTH>> aw_user{"aw_user"};

    aw_ch() = default;
    aw_ch(const char* prefix)
    : aw_id{concat(prefix, "aw_id").c_str()}
    , aw_addr{concat(prefix, "aw_addr").c_str()}
    , aw_ready{concat(prefix, "aw_ready").c_str()}
    , aw_lock{concat(prefix, "aw_lock").c_str()}
    , aw_valid{concat(prefix, "aw_valid").c_str()}
    , aw_prot{concat(prefix, "aw_prot").c_str()}
    , aw_size{concat(prefix, "aw_size").c_str()}
    , aw_cache{concat(prefix, "aw_cache").c_str()}
    , aw_burst{concat(prefix, "aw_burst").c_str()}
    , aw_qos{concat(prefix, "aw_qos").c_str()}
    , aw_region{concat(prefix, "aw_region").c_str()}
    , aw_len{concat(prefix, "aw_len").c_str()}
    , aw_user{concat(prefix, "aw_user").c_str()} {}

    template <typename OTYPES> void bind_aw(aw_ch<CFG, OTYPES>& o) {
        aw_id.bind(o.aw_id);
        aw_addr.bind(o.aw_addr);
        aw_ready.bind(o.aw_ready);
        aw_lock.bind(o.aw_lock); // only AXI3
        aw_valid.bind(o.aw_valid);
        aw_prot.bind(o.aw_prot);
        aw_size.bind(o.aw_size);
        aw_cache.bind(o.aw_cache);
        aw_burst.bind(o.aw_burst);
        aw_qos.bind(o.aw_qos);
        aw_region.bind(o.aw_region);
        aw_len.bind(o.aw_len);
        aw_user.bind(o.aw_user);
    }
};

//! write data channel signals
template <typename CFG, typename TYPES = master_types> struct wdata_ch {
    typename TYPES::template m2s_opt_t<sc_dt::sc_uint<CFG::IDWIDTH>> w_id{"w_id"};
    typename TYPES::template m2s_t<typename CFG::data_t> w_data{"w_data"};
    typename TYPES::template m2s_t<sc_dt::sc_uint<CFG::BUSWIDTH / 8>> w_strb{"w_strb"};
    typename TYPES::template m2s_full_t<bool> w_last{"w_last"};
    typename TYPES::template m2s_t<bool> w_valid{"w_valid"};
    typename TYPES::template s2m_t<bool> w_ready{"w_ready"};
    typename TYPES::template m2s_opt_t<sc_dt::sc_uint<CFG::USERWIDTH>> w_user{"w_user"};

    wdata_ch() = default;
    wdata_ch(const char* prefix)
    : w_id{concat(prefix, "w_id").c_str()}
    , w_data{concat(prefix, "w_data").c_str()}
    , w_strb{concat(prefix, "w_strb").c_str()}
    , w_last{concat(prefix, "w_last").c_str()}
    , w_valid{concat(prefix, "w_valid").c_str()}
    , w_ready{concat(prefix, "w_ready").c_str()}
    , w_user{concat(prefix, "w_user").c_str()} {}

    template <typename OTYPES> void bind_w(wdata_ch<CFG, OTYPES>& o) {
        w_id.bind(o.w_id);
        w_data.bind(o.w_data);
        w_strb.bind(o.w_strb);
        w_last.bind(o.w_last);
        w_valid.bind(o.w_valid);
        w_ready.bind(o.w_ready);
        w_user.bind(o.w_user);
    }
};

//! write response channel signals
template <typename CFG, typename TYPES = master_types> struct b_ch {
    typename TYPES::template s2m_t<bool> b_valid{"b_valid"};
    typename TYPES::template m2s_t<bool> b_ready{"b_ready"};
    typename TYPES::template s2m_full_t<sc_dt::sc_uint<CFG::IDWIDTH>> b_id{"b_id"};
    typename TYPES::template s2m_t<sc_dt::sc_uint<2>> b_resp{"b_resp"};
    typename TYPES::template s2m_opt_t<sc_dt::sc_uint<CFG::USERWIDTH>> b_user{"b_user"};

    b_ch() = default;
    b_ch(const char* prefix)
    : b_valid{concat(prefix, "b_valid").c_str()}
    , b_ready{concat(prefix, "b_ready").c_str()}
    , b_id{concat(prefix, "b_id").c_str()}
    , b_resp{concat(prefix, "b_resp").c_str()}
    , b_user{concat(prefix, "b_user").c_str()} {}

    template <typename OTYPES> void bind_b(b_ch<CFG, OTYPES>& o) {
        b_valid.bind(o.b_valid);
        b_ready.bind(o.b_ready);
        b_id.bind(o.b_id);
        b_resp.bind(o.b_resp);
        b_user.bind(o.b_user);
    }
};

//! read address channel signals
template <typename CFG, typename TYPES = master_types> struct ar_ch {
    typename TYPES::template m2s_full_t<sc_dt::sc_uint<CFG::IDWIDTH>> ar_id{"ar_id"};
    typename TYPES::template m2s_t<sc_dt::sc_uint<CFG::ADDRWIDTH>> ar_addr{"ar_addr"};
    typename TYPES::template m2s_full_t<sc_dt::sc_uint<8>> ar_len{"ar_len"};
    typename TYPES::template m2s_full_t<sc_dt::sc_uint<3>> ar_size{"ar_size"};
    typename TYPES::template m2s_full_t<sc_dt::sc_uint<2>> ar_burst{"ar_burst"};
    typename TYPES::template m2s_opt_t<sc_dt::sc_uint<2>> ar_lock{"ar_lock"}; // only AXI3
    typename TYPES::template m2s_full_t<sc_dt::sc_uint<4>> ar_cache{"ar_cache"};
    typename TYPES::template m2s_t<sc_dt::sc_uint<3>> ar_prot{"ar_prot"};
    typename TYPES::template m2s_full_t<sc_dt::sc_uint<4>> ar_qos{"ar_qos"};
    typename TYPES::template m2s_full_t<sc_dt::sc_uint<4>> ar_region{"ar_region"};
    typename TYPES::template m2s_t<bool> ar_valid{"ar_valid"};
    typename TYPES::template s2m_t<bool> ar_ready{"ar_ready"};
    typename TYPES::template m2s_opt_t<sc_dt::sc_uint<CFG::USERWIDTH>> ar_user{"ar_user"};

    ar_ch() = default;
    ar_ch(const char* prefix)
    : ar_id{concat(prefix, "ar_id").c_str()}
    , ar_addr{concat(prefix, "ar_addr").c_str()}
    , ar_len{concat(prefix, "ar_len").c_str()}
    , ar_size{concat(prefix, "ar_size").c_str()}
    , ar_burst{concat(prefix, "ar_burst").c_str()}
    , ar_lock{concat(prefix, "ar_lock").c_str()} // only AXI3
    , ar_cache{concat(prefix, "ar_cache").c_str()}
    , ar_prot{concat(prefix, "ar_prot").c_str()}
    , ar_qos{concat(prefix, "ar_qos").c_str()}
    , ar_region{concat(prefix, "ar_region").c_str()}
    , ar_valid{concat(prefix, "ar_valid").c_str()}
    , ar_ready{concat(prefix, "ar_ready").c_str()}
    , ar_user{concat(prefix, "ar_user").c_str()} {}

    template <typename OTYPES> void bind_ar(ar_ch<CFG, OTYPES>& o) {
        ar_id.bind(o.ar_id);
        ar_addr.bind(o.ar_addr);
        ar_len.bind(o.ar_len);
        ar_size.bind(o.ar_size);
        ar_burst.bind(o.ar_burst);
        ar_lock.bind(o.ar_lock); // only AXI3
        ar_cache.bind(o.ar_cache);
        ar_prot.bind(o.ar_prot);
        ar_qos.bind(o.ar_qos);
        ar_region.bind(o.ar_region);
        ar_valid.bind(o.ar_valid);
        ar_ready.bind(o.ar_ready);
        ar_user.bind(o.ar_user);
    }
};

//! Read data channel signals
template <typename CFG, typename TYPES = master_types> struct rresp_ch {
    typename TYPES::template s2m_full_t<sc_dt::sc_uint<CFG::IDWIDTH>> r_id{"r_id"};
    typename TYPES::template s2m_t<typename CFG::data_t> r_data{"r_data"};
    typename TYPES::template s2m_t<sc_dt::sc_uint<2>> r_resp{"r_resp"};
    typename TYPES::template s2m_full_t<bool> r_last{"r_last"};
    typename TYPES::template s2m_t<bool> r_valid{"r_valid"};
    typename TYPES::template m2s_t<bool> r_ready{"r_ready"};
    typename TYPES::template s2m_opt_t<sc_dt::sc_uint<CFG::USERWIDTH>> r_user{"r_user"};

    rresp_ch() = default;
    rresp_ch(const char* prefix)
    : r_id{concat(prefix, "r_id").c_str()}
    , r_data{concat(prefix, "r_data").c_str()}
    , r_resp{concat(prefix, "r_resp").c_str()}
    , r_last{concat(prefix, "r_last").c_str()}
    , r_valid{concat(prefix, "r_valid").c_str()}
    , r_ready{concat(prefix, "r_ready").c_str()}
    , r_user{concat(prefix, "r_user").c_str()} {}

    template <typename OTYPES> void bind_r(rresp_ch<CFG, OTYPES>& o) {
        r_id.bind(o.r_id);
        r_data.bind(o.r_data);
        r_resp.bind(o.r_resp);
        r_last.bind(o.r_last);
        r_valid.bind(o.r_valid);
        r_ready.bind(o.r_ready);
        r_user.bind(o.r_user);
    }
};

} // namespace axi
#endif /* _BUS_AXI_SIGNAL_IF_H_ */
