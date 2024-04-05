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

#include <scc/signal_opt_ports.h>
#include <systemc>

namespace axi {

const sc_core::sc_time CLK_DELAY = 1_ps;

struct master_types {
    template <typename T> using m2s_t = sc_core::sc_out<T>;
    template <typename T> using s2m_t = sc_core::sc_in<T>;
    template <typename T> using m2s_full_t = sc_core::sc_out<T>;
    template <typename T> using s2m_full_t = sc_core::sc_in<T>;
    template <typename T> using m2s_opt_t = scc::sc_out_opt<T>;
    template <typename T> using s2m_opt_t = scc::sc_in_opt<T>;
};

struct lite_master_types {
    template <typename T> using m2s_t = sc_core::sc_out<T>;
    template <typename T> using s2m_t = sc_core::sc_in<T>;
    template <typename T> using m2s_full_t = scc::sc_out_opt<T>;
    template <typename T> using s2m_full_t = scc::sc_in_opt<T>;
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

struct lite_slave_types {
    template <typename T> using m2s_t = sc_core::sc_in<T>;
    template <typename T> using s2m_t = sc_core::sc_out<T>;
    template <typename T> using m2s_full_t = scc::sc_in_opt<T>;
    template <typename T> using s2m_full_t = scc::sc_out_opt<T>;
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

template <unsigned int BUSWDTH = 32, unsigned int ADDRWDTH = 32, unsigned int IDWDTH = 32, unsigned int USERWDTH = 1> struct axi4_cfg {
    static_assert(BUSWDTH > 0, "BUSWIDTH shall be larger than 0");
    static_assert(ADDRWDTH > 0, "ADDRWDTH shall be larger than 0");
    static_assert(IDWDTH > 0, "IDWDTH shall be larger than 0");
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
    static_assert(BUSWDTH > 0, "BUSWIDTH shall be larger than 0");
    static_assert(ADDRWDTH > 0, "ADDRWDTH shall be larger than 0");
    constexpr static bool IS_LITE = true;
    constexpr static unsigned int BUSWIDTH = BUSWDTH;
    constexpr static unsigned int ADDRWIDTH = ADDRWDTH;
    constexpr static unsigned int IDWIDTH = 0;
    constexpr static unsigned int USERWIDTH = 1;
    using data_t = typename select_if<BUSWDTH <= 64, sc_dt::sc_uint<BUSWIDTH>, sc_dt::sc_biguint<BUSWIDTH>>::type;
    using slave_types = ::axi::lite_slave_types;
    using master_types = ::axi::lite_master_types;
};
/**
 * @struct ace_cfg
 * @brief
 *
 * @tparam BUSWDTH
 * @tparam ADDRWDTH
 * @tparam IDWDTH
 * @tparam USERWDTH
 * @tparam CACHELINE: cacheline size in Bytes, defaults value is 64 bytes
 */
template <unsigned int BUSWDTH = 32, unsigned int ADDRWDTH = 32, unsigned int IDWDTH = 32, unsigned int USERWDTH = 1,
          unsigned int AWSNOOPWDTH = 3, unsigned int RESPWDTH = 4>
struct ace_cfg {

    static_assert(BUSWDTH > 0, "BUSWIDTH shall be larger than 0");
    // static_assert(CACHELINE > 0);
    static_assert(ADDRWDTH > 0, "ADDRWDTH shall be larger than 0");
    static_assert(IDWDTH > 0, "IDWDTH shall be larger than 0");
    constexpr static bool IS_LITE = false;
    constexpr static unsigned int BUSWIDTH = BUSWDTH;
    constexpr static unsigned int ADDRWIDTH = ADDRWDTH;
    constexpr static unsigned int IDWIDTH = IDWDTH;
    constexpr static unsigned int USERWIDTH = USERWDTH;
    constexpr static unsigned int AWSNOOPWIDTH = AWSNOOPWDTH;
    constexpr static unsigned int RESPWIDTH = RESPWDTH;
    using data_t = typename select_if<BUSWDTH <= 64, sc_dt::sc_uint<BUSWIDTH>, sc_dt::sc_biguint<BUSWIDTH>>::type;
    using slave_types = ::axi::slave_types;
    using master_types = ::axi::master_types;
};

inline std::string concat(const char* prefix, const char* name) { return std::string(prefix) + name; }

//! Write address channel signals
template <typename CFG, typename TYPES = master_types> struct aw_axi_lite;
template <typename CFG, typename TYPES = master_types> struct aw_axi {
    typename TYPES::template m2s_full_t<sc_dt::sc_uint<CFG::IDWIDTH>> aw_id{"aw_id"};
    typename TYPES::template m2s_t<sc_dt::sc_uint<CFG::ADDRWIDTH>> aw_addr{"aw_addr"};
    typename TYPES::template s2m_t<bool> aw_ready{"aw_ready"};
    typename TYPES::template m2s_full_t<bool> aw_lock{"aw_lock"}; // AXI3 has 2 bits while AXI4 has 1 bit
    typename TYPES::template m2s_t<bool> aw_valid{"aw_valid"};
    typename TYPES::template m2s_t<sc_dt::sc_uint<3>> aw_prot{"aw_prot"};
    typename TYPES::template m2s_full_t<sc_dt::sc_uint<3>> aw_size{"aw_size"};
    typename TYPES::template m2s_full_t<sc_dt::sc_uint<4>> aw_cache{"aw_cache"};
    typename TYPES::template m2s_full_t<sc_dt::sc_uint<2>> aw_burst{"aw_burst"};
    typename TYPES::template m2s_full_t<sc_dt::sc_uint<4>> aw_qos{"aw_qos"};
    typename TYPES::template m2s_full_t<sc_dt::sc_uint<4>> aw_region{"aw_region"};
    typename TYPES::template m2s_full_t<sc_dt::sc_uint<8>> aw_len{"aw_len"};
    typename TYPES::template m2s_opt_t<sc_dt::sc_uint<CFG::USERWIDTH>> aw_user{"aw_user"};

    aw_axi() = default;
    aw_axi(const char* prefix)
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

    template <typename OTYPES> void bind_aw(aw_axi<CFG, OTYPES>& o) {
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
    template <typename OTYPES> void bind_aw(aw_axi_lite<CFG, OTYPES>& o);
};

//! write data channel signals
template <typename CFG, typename TYPES = master_types> struct wdata_axi_lite;
template <typename CFG, typename TYPES = master_types> struct wdata_axi {
    typename TYPES::template m2s_opt_t<sc_dt::sc_uint<CFG::IDWIDTH>> w_id{"w_id"};
    typename TYPES::template m2s_t<typename CFG::data_t> w_data{"w_data"};
    typename TYPES::template m2s_t<sc_dt::sc_uint<CFG::BUSWIDTH / 8>> w_strb{"w_strb"};
    typename TYPES::template m2s_full_t<bool> w_last{"w_last"};
    typename TYPES::template m2s_t<bool> w_valid{"w_valid"};
    typename TYPES::template s2m_t<bool> w_ready{"w_ready"};
    typename TYPES::template m2s_opt_t<sc_dt::sc_uint<CFG::USERWIDTH>> w_user{"w_user"};
    typename TYPES::template m2s_full_t<bool> w_ack{"w_ack"}; // only ACE
    typename TYPES::template m2s_opt_t<bool> w_trace{"w_trace"};

    wdata_axi() = default;
    wdata_axi(const char* prefix)
    : w_id{concat(prefix, "w_id").c_str()}
    , w_data{concat(prefix, "w_data").c_str()}
    , w_strb{concat(prefix, "w_strb").c_str()}
    , w_last{concat(prefix, "w_last").c_str()}
    , w_valid{concat(prefix, "w_valid").c_str()}
    , w_ready{concat(prefix, "w_ready").c_str()}
    , w_user{concat(prefix, "w_user").c_str()}
    , w_ack{concat(prefix, "w_ack").c_str()} // only ACE
    , w_trace{concat(prefix, "w_trace").c_str()} {}

    template <typename OTYPES> void bind_w(wdata_axi<CFG, OTYPES>& o) {
        w_id.bind(o.w_id);
        w_data.bind(o.w_data);
        w_strb.bind(o.w_strb);
        w_last.bind(o.w_last);
        w_valid.bind(o.w_valid);
        w_ready.bind(o.w_ready);
        w_user.bind(o.w_user);
        w_ack.bind(o.w_ack); // only ACE
        w_trace.bind(o.w_trace);
    }
    template <typename OTYPES> void bind_wdata(wdata_axi<CFG, OTYPES>& o) { bind_w(o); }
    template <typename OTYPES> void bind_w(wdata_axi_lite<CFG, OTYPES>& o);
    template <typename OTYPES> void bind_wdata(wdata_axi_lite<CFG, OTYPES>& o) { bind_w(o); }
};

//! write response channel signals
template <typename CFG, typename TYPES = master_types> struct b_axi_lite;
template <typename CFG, typename TYPES = master_types> struct b_axi {
    typename TYPES::template s2m_t<bool> b_valid{"b_valid"};
    typename TYPES::template m2s_t<bool> b_ready{"b_ready"};
    typename TYPES::template s2m_full_t<sc_dt::sc_uint<CFG::IDWIDTH>> b_id{"b_id"};
    typename TYPES::template s2m_t<sc_dt::sc_uint<2>> b_resp{"b_resp"};
    typename TYPES::template s2m_opt_t<sc_dt::sc_uint<CFG::USERWIDTH>> b_user{"b_user"};
    typename TYPES::template s2m_opt_t<bool> b_trace{"b_trace"};

    b_axi() = default;
    b_axi(const char* prefix)
    : b_valid{concat(prefix, "b_valid").c_str()}
    , b_ready{concat(prefix, "b_ready").c_str()}
    , b_id{concat(prefix, "b_id").c_str()}
    , b_resp{concat(prefix, "b_resp").c_str()}
    , b_user{concat(prefix, "b_user").c_str()}
    , b_trace{concat(prefix, "b_trace").c_str()} {}

    template <typename OTYPES> void bind_b(b_axi<CFG, OTYPES>& o) {
        b_valid.bind(o.b_valid);
        b_ready.bind(o.b_ready);
        b_id.bind(o.b_id);
        b_resp.bind(o.b_resp);
        b_user.bind(o.b_user);
        b_trace.bind(o.b_trace);
    }
    template <typename OTYPES> void bind_b(b_axi_lite<CFG, OTYPES>& o);
};

//! read address channel signals
template <typename CFG, typename TYPES = master_types> struct ar_axi_lite;
template <typename CFG, typename TYPES = master_types> struct ar_axi {
    typename TYPES::template m2s_full_t<sc_dt::sc_uint<CFG::IDWIDTH>> ar_id{"ar_id"};
    typename TYPES::template m2s_t<sc_dt::sc_uint<CFG::ADDRWIDTH>> ar_addr{"ar_addr"};
    typename TYPES::template m2s_full_t<sc_dt::sc_uint<8>> ar_len{"ar_len"};
    typename TYPES::template m2s_full_t<sc_dt::sc_uint<3>> ar_size{"ar_size"};
    typename TYPES::template m2s_full_t<sc_dt::sc_uint<2>> ar_burst{"ar_burst"};
    typename TYPES::template m2s_full_t<bool> ar_lock{"ar_lock"}; // AXI3 has 2 bits while AXI4 has i bit
    typename TYPES::template m2s_full_t<sc_dt::sc_uint<4>> ar_cache{"ar_cache"};
    typename TYPES::template m2s_t<sc_dt::sc_uint<3>> ar_prot{"ar_prot"};
    typename TYPES::template m2s_full_t<sc_dt::sc_uint<4>> ar_qos{"ar_qos"};
    typename TYPES::template m2s_full_t<sc_dt::sc_uint<4>> ar_region{"ar_region"};
    typename TYPES::template m2s_t<bool> ar_valid{"ar_valid"};
    typename TYPES::template s2m_t<bool> ar_ready{"ar_ready"};
    typename TYPES::template m2s_opt_t<sc_dt::sc_uint<CFG::USERWIDTH>> ar_user{"ar_user"};

    ar_axi() = default;
    ar_axi(const char* prefix)
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

    template <typename OTYPES> void bind_ar(ar_axi<CFG, OTYPES>& o) {
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
    template <typename OTYPES> void bind_ar(ar_axi_lite<CFG, OTYPES>& o);
};

//! Read data channel signals
template <typename CFG, typename TYPES = master_types> struct rresp_axi_lite;
template <typename CFG, typename TYPES = master_types> struct rresp_axi {
    typename TYPES::template s2m_full_t<sc_dt::sc_uint<CFG::IDWIDTH>> r_id{"r_id"};
    typename TYPES::template s2m_t<typename CFG::data_t> r_data{"r_data"};
    typename TYPES::template s2m_t<sc_dt::sc_uint<2>> r_resp{"r_resp"};
    typename TYPES::template s2m_full_t<bool> r_last{"r_last"};
    typename TYPES::template s2m_t<bool> r_valid{"r_valid"};
    typename TYPES::template m2s_t<bool> r_ready{"r_ready"};
    typename TYPES::template s2m_opt_t<sc_dt::sc_uint<CFG::USERWIDTH>> r_user{"r_user"};
    typename TYPES::template s2m_opt_t<bool> r_trace{"r_trace"};

    rresp_axi() = default;
    rresp_axi(const char* prefix)
    : r_id{concat(prefix, "r_id").c_str()}
    , r_data{concat(prefix, "r_data").c_str()}
    , r_resp{concat(prefix, "r_resp").c_str()}
    , r_last{concat(prefix, "r_last").c_str()}
    , r_valid{concat(prefix, "r_valid").c_str()}
    , r_ready{concat(prefix, "r_ready").c_str()}
    , r_user{concat(prefix, "r_user").c_str()}
    , r_trace{concat(prefix, "r_trace").c_str()} {}

    template <typename OTYPES> void bind_r(rresp_axi<CFG, OTYPES>& o) {
        r_id.bind(o.r_id);
        r_data.bind(o.r_data);
        r_resp.bind(o.r_resp);
        r_last.bind(o.r_last);
        r_valid.bind(o.r_valid);
        r_ready.bind(o.r_ready);
        r_user.bind(o.r_user);
        r_trace.bind(o.r_trace);
    }
    template <typename OTYPES> void bind_rresp(rresp_axi<CFG, OTYPES>& o) { bind_r(o); }
    template <typename OTYPES> void bind_r(rresp_axi_lite<CFG, OTYPES>& o);
    template <typename OTYPES> void bind_rresp(rresp_axi_lite<CFG, OTYPES>& o) { bind_r(o); }
};

template <typename CFG, typename TYPES = master_types> struct rresp_ace {
    typename TYPES::template s2m_full_t<sc_dt::sc_uint<CFG::IDWIDTH>> r_id{"r_id"};
    typename TYPES::template s2m_t<typename CFG::data_t> r_data{"r_data"};
    typename TYPES::template s2m_t<sc_dt::sc_uint<CFG::RESPWIDTH>> r_resp{"r_resp"}; // ACE5-Lite has only 2
    typename TYPES::template s2m_full_t<bool> r_last{"r_last"};
    typename TYPES::template s2m_t<bool> r_valid{"r_valid"};
    typename TYPES::template m2s_t<bool> r_ready{"r_ready"};
    typename TYPES::template s2m_opt_t<sc_dt::sc_uint<CFG::USERWIDTH>> r_user{"r_user"};
    typename TYPES::template s2m_opt_t<bool> r_trace{"r_trace"}; // ACE5
    typename TYPES::template m2s_t<bool> r_ack{"r_ack"};         // only ACE

    rresp_ace() = default;
    rresp_ace(const char* prefix)
    : r_id{concat(prefix, "r_id").c_str()}
    , r_data{concat(prefix, "r_data").c_str()}
    , r_resp{concat(prefix, "r_resp").c_str()}
    , r_last{concat(prefix, "r_last").c_str()}
    , r_valid{concat(prefix, "r_valid").c_str()}
    , r_ready{concat(prefix, "r_ready").c_str()}
    , r_user{concat(prefix, "r_user").c_str()}
    , r_trace{concat(prefix, "r_trace").c_str()}
    , r_ack{concat(prefix, "r_ack").c_str()} {}

    template <typename OTYPES> void bind_r(rresp_ace<CFG, OTYPES>& o) {
        r_id.bind(o.r_id);
        r_data.bind(o.r_data);
        r_resp.bind(o.r_resp);
        r_last.bind(o.r_last);
        r_valid.bind(o.r_valid);
        r_ready.bind(o.r_ready);
        r_user.bind(o.r_user);
        r_ack.bind(o.r_ack); // only ACE,not for Lite
        r_trace.bind(o.r_trace);
    }
    template <typename OTYPES> void bind_rresp(rresp_ace<CFG, OTYPES>& o) { bind_r(o); }
};

//! Write address channel signals
template <typename CFG, typename TYPES> struct aw_axi_lite {
    typename TYPES::template m2s_t<sc_dt::sc_uint<CFG::ADDRWIDTH>> aw_addr{"aw_addr"};
    typename TYPES::template s2m_t<bool> aw_ready{"aw_ready"};
    typename TYPES::template m2s_t<bool> aw_valid{"aw_valid"};
    typename TYPES::template m2s_t<sc_dt::sc_uint<3>> aw_prot{"aw_prot"};

    aw_axi_lite() = default;
    aw_axi_lite(const char* prefix)
    : aw_addr{concat(prefix, "aw_addr").c_str()}
    , aw_ready{concat(prefix, "aw_ready").c_str()}
    , aw_valid{concat(prefix, "aw_valid").c_str()}
    , aw_prot{concat(prefix, "aw_prot").c_str()} {}

    template <typename OTYPES> void bind_aw(aw_axi<CFG, OTYPES>& o) {
        aw_addr.bind(o.aw_addr);
        aw_ready.bind(o.aw_ready);
        aw_valid.bind(o.aw_valid);
        aw_prot.bind(o.aw_prot);
    }
};

//! write data channel signals
template <typename CFG, typename TYPES> struct wdata_axi_lite {
    typename TYPES::template m2s_t<typename CFG::data_t> w_data{"w_data"};
    typename TYPES::template m2s_t<sc_dt::sc_uint<CFG::BUSWIDTH / 8>> w_strb{"w_strb"};
    typename TYPES::template m2s_t<bool> w_valid{"w_valid"};
    typename TYPES::template s2m_t<bool> w_ready{"w_ready"};

    wdata_axi_lite() = default;
    wdata_axi_lite(const char* prefix)
    : w_data{concat(prefix, "w_data").c_str()}
    , w_strb{concat(prefix, "w_strb").c_str()}
    , w_valid{concat(prefix, "w_valid").c_str()}
    , w_ready{concat(prefix, "w_ready").c_str()} {}

    template <typename OTYPES> void bind_w(wdata_axi<CFG, OTYPES>& o) {
        w_data.bind(o.w_data);
        w_strb.bind(o.w_strb);
        w_valid.bind(o.w_valid);
        w_ready.bind(o.w_ready);
    }
};

//! write response channel signals
template <typename CFG, typename TYPES> struct b_axi_lite {
    typename TYPES::template s2m_t<bool> b_valid{"b_valid"};
    typename TYPES::template m2s_t<bool> b_ready{"b_ready"};
    typename TYPES::template s2m_t<sc_dt::sc_uint<2>> b_resp{"b_resp"};

    b_axi_lite() = default;
    b_axi_lite(const char* prefix)
    : b_valid{concat(prefix, "b_valid").c_str()}
    , b_ready{concat(prefix, "b_ready").c_str()}
    , b_resp{concat(prefix, "b_resp").c_str()} {}

    template <typename OTYPES> void bind_b(b_axi<CFG, OTYPES>& o) {
        b_valid.bind(o.b_valid);
        b_ready.bind(o.b_ready);
        b_resp.bind(o.b_resp);
    }
};

//! read address channel signals
template <typename CFG, typename TYPES> struct ar_axi_lite {
    typename TYPES::template m2s_t<sc_dt::sc_uint<CFG::ADDRWIDTH>> ar_addr{"ar_addr"};
    typename TYPES::template m2s_t<sc_dt::sc_uint<3>> ar_prot{"ar_prot"};
    typename TYPES::template m2s_t<bool> ar_valid{"ar_valid"};
    typename TYPES::template s2m_t<bool> ar_ready{"ar_ready"};

    ar_axi_lite() = default;
    ar_axi_lite(const char* prefix)
    : ar_addr{concat(prefix, "ar_addr").c_str()}
    , ar_prot{concat(prefix, "ar_prot").c_str()}
    , ar_valid{concat(prefix, "ar_valid").c_str()}
    , ar_ready{concat(prefix, "ar_ready").c_str()} {}

    template <typename OTYPES> void bind_ar(ar_axi<CFG, OTYPES>& o) {
        ar_addr.bind(o.ar_addr);
        ar_prot.bind(o.ar_prot);
        ar_valid.bind(o.ar_valid);
        ar_ready.bind(o.ar_ready);
    }
};

//! Read data channel signals
template <typename CFG, typename TYPES> struct rresp_axi_lite {
    typename TYPES::template s2m_t<typename CFG::data_t> r_data{"r_data"};
    typename TYPES::template s2m_t<sc_dt::sc_uint<2>> r_resp{"r_resp"};
    typename TYPES::template s2m_t<bool> r_valid{"r_valid"};
    typename TYPES::template m2s_t<bool> r_ready{"r_ready"};

    rresp_axi_lite() = default;
    rresp_axi_lite(const char* prefix)
    : r_data{concat(prefix, "r_data").c_str()}
    , r_resp{concat(prefix, "r_resp").c_str()}
    , r_valid{concat(prefix, "r_valid").c_str()}
    , r_ready{concat(prefix, "r_ready").c_str()} {}

    template <typename OTYPES> void bind_r(rresp_axi<CFG, OTYPES>& o) {
        r_data.bind(o.r_data);
        r_resp.bind(o.r_resp);
        r_valid.bind(o.r_valid);
        r_ready.bind(o.r_ready);
    }
};

template <typename CFG, typename TYPES = master_types> struct ar_ace {
    typename TYPES::template m2s_full_t<sc_dt::sc_uint<CFG::IDWIDTH>> ar_id{"ar_id"};
    typename TYPES::template m2s_t<sc_dt::sc_uint<CFG::ADDRWIDTH>> ar_addr{"ar_addr"};
    typename TYPES::template m2s_full_t<sc_dt::sc_uint<8>> ar_len{"ar_len"};
    typename TYPES::template m2s_full_t<sc_dt::sc_uint<3>> ar_size{"ar_size"};
    typename TYPES::template m2s_full_t<sc_dt::sc_uint<2>> ar_burst{"ar_burst"};
    typename TYPES::template m2s_t<bool> ar_lock{"ar_lock"};
    typename TYPES::template m2s_full_t<sc_dt::sc_uint<4>> ar_cache{"ar_cache"};
    typename TYPES::template m2s_t<sc_dt::sc_uint<3>> ar_prot{"ar_prot"};
    typename TYPES::template m2s_full_t<sc_dt::sc_uint<4>> ar_qos{"ar_qos"};
    typename TYPES::template m2s_full_t<sc_dt::sc_uint<4>> ar_region{"ar_region"};
    typename TYPES::template m2s_t<bool> ar_valid{"ar_valid"};
    typename TYPES::template s2m_t<bool> ar_ready{"ar_ready"};
    typename TYPES::template m2s_full_t<sc_dt::sc_uint<2>> ar_domain{"ar_domain"};
    typename TYPES::template m2s_full_t<sc_dt::sc_uint<4>> ar_snoop{"ar_snoop"};
    typename TYPES::template m2s_full_t<sc_dt::sc_uint<2>> ar_bar{"ar_bar"};
    typename TYPES::template m2s_opt_t<sc_dt::sc_uint<CFG::USERWIDTH>> ar_user{"ar_user"};
    typename TYPES::template m2s_opt_t<bool> ar_trace{"ar_trace"};
    typename TYPES::template m2s_opt_t<sc_dt::sc_uint<4>> ar_vmidext{"ar_vmidext"}; // ACE5

    ar_ace() = default;
    ar_ace(const char* prefix)
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
    , ar_domain{concat(prefix, "ar_domain").c_str()}
    , ar_snoop{concat(prefix, "ar_snoop").c_str()}
    , ar_bar{concat(prefix, "ar_bar").c_str()}
    , ar_user{concat(prefix, "ar_user").c_str()}
    , ar_trace{concat(prefix, "ar_trace").c_str()}
    , ar_vmidext{concat(prefix, "ar_vmidext").c_str()} {}

    template <typename OTYPES> void bind_ar(ar_ace<CFG, OTYPES>& o) {
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
        ar_domain.bind(o.ar_domain);
        ar_snoop.bind(o.ar_snoop);
        ar_bar.bind(o.ar_bar); //
        ar_user.bind(o.ar_user);
        ar_trace.bind(o.ar_trace);
    }
};

template <typename CFG, typename TYPES = master_types> struct aw_ace {
    typename TYPES::template m2s_full_t<sc_dt::sc_uint<CFG::IDWIDTH>> aw_id{"aw_id"};
    typename TYPES::template m2s_t<sc_dt::sc_uint<CFG::ADDRWIDTH>> aw_addr{"aw_addr"};
    typename TYPES::template s2m_t<bool> aw_ready{"aw_ready"};
    typename TYPES::template m2s_t<bool> aw_lock{"aw_lock"};
    typename TYPES::template m2s_t<bool> aw_valid{"aw_valid"};
    typename TYPES::template m2s_t<sc_dt::sc_uint<3>> aw_prot{"aw_prot"};
    typename TYPES::template m2s_full_t<sc_dt::sc_uint<3>> aw_size{"aw_size"};
    typename TYPES::template m2s_full_t<sc_dt::sc_uint<4>> aw_cache{"aw_cache"};
    typename TYPES::template m2s_full_t<sc_dt::sc_uint<2>> aw_burst{"aw_burst"};
    typename TYPES::template m2s_full_t<sc_dt::sc_uint<4>> aw_qos{"aw_qos"};
    typename TYPES::template m2s_full_t<sc_dt::sc_uint<4>> aw_region{"aw_region"};
    typename TYPES::template m2s_full_t<sc_dt::sc_uint<8>> aw_len{"aw_len"};
    typename TYPES::template m2s_full_t<sc_dt::sc_uint<2>> aw_domain{"aw_domain"};               // ace
    typename TYPES::template m2s_full_t<sc_dt::sc_uint<CFG::AWSNOOPWIDTH>> aw_snoop{"aw_snoop"}; // ace5-lite has 4 bits
    typename TYPES::template m2s_full_t<sc_dt::sc_uint<2>> aw_bar{"aw_bar"};                     // ace
    typename TYPES::template m2s_t<bool> aw_unique{"aw_unique"};                                 // ace, not for lite
    typename TYPES::template m2s_full_t<sc_dt::sc_uint<CFG::USERWIDTH>> aw_user{"aw_user"};
    typename TYPES::template m2s_t<bool> aw_stashniden{"aw_stashniden"}; // stash only for ACE5L
    typename TYPES::template m2s_full_t<sc_dt::sc_uint<11>> aw_stashnid{"aw_stashnid"};
    typename TYPES::template m2s_t<bool> aw_stashlpiden{"aw_stashlpiden"};
    typename TYPES::template m2s_full_t<sc_dt::sc_uint<5>> aw_stashlpid{"aw_stashlpid"};
    typename TYPES::template m2s_opt_t<sc_dt::sc_uint<6>> aw_atop{"aw_atop"}; // only for ace lite
    typename TYPES::template m2s_opt_t<bool> aw_trace{"aw_trace"};

    aw_ace() = default;
    aw_ace(const char* prefix)
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
    , aw_domain{concat(prefix, "aw_domain").c_str()}
    , aw_snoop{concat(prefix, "aw_snoop").c_str()}
    , aw_bar{concat(prefix, "aw_bar").c_str()}
    , aw_unique{concat(prefix, "aw_unique").c_str()}
    , aw_user{concat(prefix, "aw_user").c_str()}
    , aw_stashniden{concat(prefix, "aw_stashniden").c_str()}
    , aw_stashnid{concat(prefix, "aw_stashnid").c_str()}
    , aw_stashlpiden{concat(prefix, "aw_stashlpiden").c_str()}
    , aw_stashlpid{concat(prefix, "aw_stashlpid").c_str()}
    , aw_atop{concat(prefix, "aw_atop").c_str()}
    , aw_trace{concat(prefix, "aw_trace").c_str()} {}

    template <typename OTYPES> void bind_aw(aw_ace<CFG, OTYPES>& o) {
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
        aw_domain.bind(o.aw_domain);
        aw_snoop.bind(o.aw_snoop);
        aw_bar.bind(o.aw_bar);
        aw_unique.bind(o.aw_unique);
        aw_user.bind(o.aw_user);
        aw_stashniden.bind(o.aw_stashniden);
        aw_stashnid.bind(o.aw_stashnid);
        aw_stashlpiden.bind(o.aw_stashlpiden);
        aw_stashlpid.bind(o.aw_stashlpid);
        aw_atop.bind(o.aw_atop);
        aw_trace.bind(o.aw_trace);
    }
};

//! snoop address(AC) channel signals
template <typename CFG, typename TYPES = master_types> struct ac_ace {
    typename TYPES::template s2m_t<bool> ac_valid{"ac_valid"};
    typename TYPES::template m2s_t<bool> ac_ready{"ac_ready"};
    typename TYPES::template s2m_t<sc_dt::sc_uint<CFG::ADDRWIDTH>> ac_addr{"ac_addr"};
    typename TYPES::template s2m_full_t<sc_dt::sc_uint<4>> ac_snoop{"ac_snoop"};
    typename TYPES::template s2m_full_t<sc_dt::sc_uint<3>> ac_prot{"ac_prot"};
    typename TYPES::template s2m_opt_t<sc_dt::sc_uint<4>> ac_vmidext{"ac_vmidext"}; // ACE5
    typename TYPES::template s2m_opt_t<bool> ac_trace{"ac_trace"};                  // ACE5

    ac_ace() = default;
    ac_ace(const char* prefix)
    : ac_valid{concat(prefix, "ac_valid").c_str()}
    , ac_ready{concat(prefix, "ac_ready").c_str()}
    , ac_addr{concat(prefix, "ac_addr").c_str()}
    , ac_snoop{concat(prefix, "ac_snoop").c_str()}
    , ac_prot{concat(prefix, "ac_prot").c_str()}
    , ac_vmidext{concat(prefix, "ac_vmidext").c_str()}
    , ac_trace{concat(prefix, "ac_trace").c_str()} {}

    template <typename OTYPES> void bind_ac(ac_ace<CFG, OTYPES>& o) {
        ac_valid.bind(o.ac_valid);
        ac_ready.bind(o.ac_ready);
        ac_addr.bind(o.ac_addr);
        ac_snoop.bind(o.ac_snoop);
        ac_prot.bind(o.ac_prot);
    }
};

//! snoop data(cd) channel signals
template <typename CFG, typename TYPES = master_types> struct cd_ace {
    typename TYPES::template m2s_t<bool> cd_valid{"cd_valid"};
    typename TYPES::template s2m_t<bool> cd_ready{"cd_ready"};
    typename TYPES::template m2s_t<typename CFG::data_t> cd_data{"cd_data"};
    typename TYPES::template m2s_full_t<bool> cd_last{"cd_last"};

    cd_ace() = default;
    cd_ace(const char* prefix)
    : cd_valid{concat(prefix, "cd_valid").c_str()}
    , cd_ready{concat(prefix, "cd_ready").c_str()}
    , cd_data{concat(prefix, "cd_data").c_str()}
    , cd_last{concat(prefix, "cd_last").c_str()} {}

    template <typename OTYPES> void bind_cd(cd_ace<CFG, OTYPES>& o) {
        cd_valid.bind(o.cd_valid);
        cd_ready.bind(o.cd_ready);
        cd_data.bind(o.cd_data);
        cd_last.bind(o.cd_last);
    }
};

//! snoop response(cr) channel signals
template <typename CFG, typename TYPES = master_types> struct cr_ace {
    typename TYPES::template m2s_t<bool> cr_valid{"cr_valid"};
    typename TYPES::template s2m_t<bool> cr_ready{"cr_ready"};
    typename TYPES::template m2s_t<sc_dt::sc_uint<5>> cr_resp{"cr_resp"};
    typename TYPES::template m2s_opt_t<bool> cr_trace{"cr_trace"}; // ACE5

    cr_ace() = default;
    cr_ace(const char* prefix)
    : cr_valid{concat(prefix, "cr_valid").c_str()}
    , cr_ready{concat(prefix, "cr_ready").c_str()}
    , cr_resp{concat(prefix, "cr_resp").c_str()}
    , cr_trace{concat(prefix, "cr_trace").c_str()} {}

    template <typename OTYPES> void bind_cr(cr_ace<CFG, OTYPES>& o) {
        cr_valid.bind(o.cr_valid);
        cr_ready.bind(o.cr_ready);
        cr_resp.bind(o.cr_resp);
    }
};
//! alias declaration for rresp_ch_ace, wdata_ch_ace, b_ch_ace???
template <typename CFG, typename TYPES = master_types> using wdata_ace = wdata_axi<CFG, TYPES>;
template <typename CFG, typename TYPES = master_types> using b_ace = b_axi<CFG, TYPES>;
template <typename CFG, typename TYPES = master_types> using wdata_ace_lite = wdata_axi<CFG, TYPES>;
template <typename CFG, typename TYPES = master_types> using b_ace_lite = b_axi<CFG, TYPES>;
template <typename CFG, typename TYPES = master_types> using rresp_ace_lite = rresp_axi<CFG, TYPES>;
template <typename CFG, typename TYPES = master_types> using aw_ace_lite = aw_ace<CFG, TYPES>;
template <typename CFG, typename TYPES = master_types> using ar_ace_lite = ar_ace<CFG, TYPES>;

template <typename CFG, typename TYPES> template <typename OTYPES> inline void ar_axi<CFG, TYPES>::bind_ar(ar_axi_lite<CFG, OTYPES>& o) {
    ar_addr.bind(o.ar_addr);
    ar_prot.bind(o.ar_prot);
    ar_valid.bind(o.ar_valid);
    ar_ready.bind(o.ar_ready);
}

template <typename CFG, typename TYPES>
template <typename OTYPES>
inline void rresp_axi<CFG, TYPES>::bind_r(rresp_axi_lite<CFG, OTYPES>& o) {
    r_data.bind(o.r_data);
    r_resp.bind(o.r_resp);
    r_valid.bind(o.r_valid);
    r_ready.bind(o.r_ready);
}

template <typename CFG, typename TYPES> template <typename OTYPES> inline void aw_axi<CFG, TYPES>::bind_aw(aw_axi_lite<CFG, OTYPES>& o) {
    aw_addr.bind(o.aw_addr);
    aw_ready.bind(o.aw_ready);
    aw_valid.bind(o.aw_valid);
    aw_prot.bind(o.aw_prot);
}

template <typename CFG, typename TYPES>
template <typename OTYPES>
inline void wdata_axi<CFG, TYPES>::bind_w(wdata_axi_lite<CFG, OTYPES>& o) {
    w_data.bind(o.w_data);
    w_strb.bind(o.w_strb);
    w_valid.bind(o.w_valid);
    w_ready.bind(o.w_ready);
}

template <typename CFG, typename TYPES> template <typename OTYPES> inline void b_axi<CFG, TYPES>::bind_b(b_axi_lite<CFG, OTYPES>& o) {
    b_valid.bind(o.b_valid);
    b_ready.bind(o.b_ready);
    b_resp.bind(o.b_resp);
}

} // namespace axi
#endif /* _BUS_AXI_SIGNAL_IF_H_ */
