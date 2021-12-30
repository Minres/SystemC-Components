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

#ifndef _AXI_SIGNAL_IF_H_
#define _AXI_SIGNAL_IF_H_

#include <systemc>

namespace axi {

template<unsigned int BUSWDTH = 32, unsigned int ADDRWDTH = 32, unsigned int IDWDTH = 32, unsigned int USERWDTH = 1>
struct axi_cfg {
    static_assert(BUSWDTH>0);
    static_assert(ADDRWDTH>0);
    static_assert(IDWDTH>0);
    static_assert(USERWDTH>0);
    constexpr static unsigned int BUSWIDTH = BUSWDTH;
    constexpr static unsigned int ADDRWIDTH = ADDRWDTH;
    constexpr static unsigned int IDWIDTH = IDWDTH;
    constexpr static unsigned int USERWIDTH = USERWDTH;
};

template <unsigned WIDTH = 0, typename TYPE = sc_dt::sc_uint<WIDTH>, int N = 1>
using sc_in_opt = sc_core::sc_port<sc_core::sc_signal_in_if<TYPE>, N, sc_core::SC_ZERO_OR_MORE_BOUND>;
template <unsigned WIDTH = 0, typename TYPE = sc_dt::sc_uint<WIDTH>, int N = 1>
using sc_out_opt = sc_core::sc_port<sc_core::sc_signal_write_if<TYPE>, N, sc_core::SC_ZERO_OR_MORE_BOUND>;

struct master_types {
    template <class T>
    using m2s_t = sc_core::sc_out<T>;
    template <class T>
    using s2m_t = sc_core::sc_in<T>;
    template <class T>
    using m2s_opt_t = sc_core::sc_port<sc_core::sc_signal_write_if<T>, 1, sc_core::SC_ZERO_OR_MORE_BOUND>;
    template <class T>
    using s2m_opt_t = sc_core::sc_port<sc_core::sc_signal_in_if<T>, 1, sc_core::SC_ZERO_OR_MORE_BOUND>;
};

struct slave_types {
    template <class T>
    using m2s_t = sc_core::sc_in<T>;
    template <class T>
    using s2m_t = sc_core::sc_out<T>;
    template <class T>
    using m2s_opt_t = sc_core::sc_port<sc_core::sc_signal_in_if<T>, 1, sc_core::SC_ZERO_OR_MORE_BOUND>;
    template <class T>
    using s2m_opt_t = sc_core::sc_port<sc_core::sc_signal_write_if<T>, 1, sc_core::SC_ZERO_OR_MORE_BOUND>;
};

struct signal_types {
    template <class T>
    using m2s_t = sc_core::sc_signal<T>;
    template <class T>
    using s2m_t = sc_core::sc_signal<T>;
    template <class T>
    using m2s_opt_t = sc_core::sc_signal<T>;
    template <class T>
    using s2m_opt_t = sc_core::sc_signal<T>;
};

inline std::string concatenate(const char* prefix, const char* name){
    if(prefix) return std::string(prefix)+name; else return std::string(name);
}
//! Write address channel signals
template<typename CFG, typename TYPES=master_types, const char* PREFIX=nullptr>
struct aw_ch {
    typename TYPES::template m2s_t<sc_dt::sc_uint<CFG::IDWIDTH>>       aw_id{concatenate(PREFIX, "aw_id").c_str()};
    typename TYPES::template m2s_t<sc_dt::sc_uint<CFG::ADDRWIDTH>>     aw_addr{concatenate(PREFIX, "aw_addr").c_str()};
    typename TYPES::template s2m_t<bool>                               aw_ready{concatenate(PREFIX, "aw_ready").c_str()};
    typename TYPES::template m2s_opt_t<sc_dt::sc_uint<2>>              aw_lock{concatenate(PREFIX, "aw_lock").c_str()}; // only AXI3
    typename TYPES::template m2s_t<bool>                               aw_valid{concatenate(PREFIX, "aw_valid").c_str()};
    typename TYPES::template m2s_t<sc_dt::sc_uint<3>>                  aw_prot{concatenate(PREFIX, "aw_prot").c_str()};
    typename TYPES::template m2s_t<sc_dt::sc_uint<3>>                  aw_size{concatenate(PREFIX, "aw_size").c_str()};
    typename TYPES::template m2s_t<sc_dt::sc_uint<4>>                  aw_cache{concatenate(PREFIX, "aw_cache").c_str()};
    typename TYPES::template m2s_t<sc_dt::sc_uint<2>>                  aw_burst{concatenate(PREFIX, "aw_burst").c_str()};
    typename TYPES::template m2s_t<sc_dt::sc_uint<4>>                  aw_qos{concatenate(PREFIX, "aw_qos").c_str()};
    typename TYPES::template m2s_t<sc_dt::sc_uint<4>>                  aw_region{concatenate(PREFIX, "aw_region").c_str()};
    typename TYPES::template m2s_t<sc_dt::sc_uint<8>>                  aw_len{concatenate(PREFIX, "aw_len").c_str()};
    typename TYPES::template m2s_opt_t<sc_dt::sc_uint<CFG::USERWIDTH>> aw_user{concatenate(PREFIX, "aw_user").c_str()};

    template<typename OTYPES>
    void bind_aw(aw_ch<CFG, OTYPES>& o){
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
template<typename CFG, typename TYPES=master_types, const char* PREFIX=nullptr>
struct wdata_ch {
    typename TYPES::template m2s_t<sc_dt::sc_uint<CFG::IDWIDTH>>       w_id{concatenate(PREFIX, "w_id").c_str()};
    typename TYPES::template m2s_t<sc_dt::sc_biguint<CFG::BUSWIDTH>>   w_data{concatenate(PREFIX, "w_data").c_str()};
    typename TYPES::template m2s_t<sc_dt::sc_uint<CFG::BUSWIDTH / 8>>  w_strb{concatenate(PREFIX, "w_strb").c_str()};
    typename TYPES::template m2s_t<bool>                               w_last{concatenate(PREFIX, "w_last").c_str()};
    typename TYPES::template m2s_t<bool>                               w_valid{concatenate(PREFIX, "w_valid").c_str()};
    typename TYPES::template s2m_t<bool>                               w_ready{concatenate(PREFIX, "w_ready").c_str()};
    typename TYPES::template m2s_opt_t<sc_dt::sc_uint<CFG::USERWIDTH>> w_user{concatenate(PREFIX, "w_user").c_str()};

    template<typename OTYPES>
    void bind_wdata(wdata_ch<CFG, OTYPES>& o){
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
template<typename CFG, typename TYPES=master_types, const char* PREFIX=nullptr>
struct b_ch {
    typename TYPES::template s2m_t<bool>                               b_valid{concatenate(PREFIX, "b_valid").c_str()};
    typename TYPES::template m2s_t<bool>                               b_ready{concatenate(PREFIX, "b_ready").c_str()};
    typename TYPES::template s2m_t<sc_dt::sc_uint<CFG::IDWIDTH>>       b_id{concatenate(PREFIX, "b_id").c_str()};
    typename TYPES::template s2m_t<sc_dt::sc_uint<2>>                  b_resp{concatenate(PREFIX, "b_resp").c_str()};
    typename TYPES::template s2m_opt_t<sc_dt::sc_uint<CFG::USERWIDTH>> b_user{concatenate(PREFIX, "b_user").c_str()};

    template<typename OTYPES>
    void bind_b(b_ch<CFG, OTYPES>& o){
        b_valid.bind(o.b_valid);
        b_ready.bind(o.b_ready);
        b_id.bind(o.b_id);
        b_resp.bind(o.b_resp);
        b_user.bind(o.b_user);
    }
};

//! read address channel signals
template<typename CFG, typename TYPES=master_types, const char* PREFIX=nullptr>
struct ar_ch {
    typename TYPES::template m2s_t<sc_dt::sc_uint<CFG::IDWIDTH>>       ar_id{concatenate(PREFIX, "ar_id").c_str()};
    typename TYPES::template m2s_t<sc_dt::sc_uint<CFG::ADDRWIDTH>>     ar_addr{concatenate(PREFIX, "ar_addr").c_str()};
    typename TYPES::template m2s_t<sc_dt::sc_uint<8>>                  ar_len{concatenate(PREFIX, "ar_len").c_str()};
    typename TYPES::template m2s_t<sc_dt::sc_uint<3>>                  ar_size{concatenate(PREFIX, "ar_size").c_str()};
    typename TYPES::template m2s_t<sc_dt::sc_uint<2>>                  ar_burst{concatenate(PREFIX, "ar_burst").c_str()};
    typename TYPES::template m2s_opt_t<sc_dt::sc_uint<2>>              ar_lock{concatenate(PREFIX, "ar_lock").c_str()}; // only AXI3
    typename TYPES::template m2s_t<sc_dt::sc_uint<4>>                  ar_cache{concatenate(PREFIX, "ar_cache").c_str()};
    typename TYPES::template m2s_t<sc_dt::sc_uint<3>>                  ar_prot{concatenate(PREFIX, "ar_prot").c_str()};
    typename TYPES::template m2s_t<sc_dt::sc_uint<4>>                  ar_qos{concatenate(PREFIX, "ar_qos").c_str()};
    typename TYPES::template m2s_t<sc_dt::sc_uint<4>>                  ar_region{concatenate(PREFIX, "ar_region").c_str()};
    typename TYPES::template m2s_t<bool>                               ar_valid{concatenate(PREFIX, "ar_valid").c_str()};
    typename TYPES::template s2m_t<bool>                               ar_ready{concatenate(PREFIX, "ar_ready").c_str()};
    typename TYPES::template m2s_opt_t<sc_dt::sc_uint<CFG::USERWIDTH>> ar_user{concatenate(PREFIX, "ar_user").c_str()};

    template<typename OTYPES>
    void bind_ar(ar_ch<CFG, OTYPES>& o){
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
template<typename CFG, typename TYPES=master_types, const char* PREFIX=nullptr>
struct rresp_ch {
    typename TYPES::template s2m_t<sc_dt::sc_uint<CFG::IDWIDTH>>       r_id{concatenate(PREFIX, "r_id").c_str()};
    typename TYPES::template s2m_t<sc_dt::sc_biguint<CFG::BUSWIDTH>>   r_data{concatenate(PREFIX, "r_data").c_str()};
    typename TYPES::template s2m_t<sc_dt::sc_uint<2>>                  r_resp{concatenate(PREFIX, "r_resp").c_str()};
    typename TYPES::template s2m_t<bool>                               r_last{concatenate(PREFIX, "r_last").c_str()};
    typename TYPES::template s2m_t<bool>                               r_valid{concatenate(PREFIX, "r_valid").c_str()};
    typename TYPES::template m2s_t<bool>                               r_ready{concatenate(PREFIX, "r_ready").c_str()};
    typename TYPES::template s2m_opt_t<sc_dt::sc_uint<CFG::USERWIDTH>> r_user{concatenate(PREFIX, "r_user").c_str()};

    template<typename OTYPES>
    void bind_rresp(rresp_ch<CFG, OTYPES>& o){
        r_id.bind(o.r_id);
        r_data.bind(o.r_data);
        r_resp.bind(o.r_resp);
        r_last.bind(o.r_last);
        r_valid.bind(o.r_valid);
        r_ready.bind(o.r_ready);
        r_user.bind(o.r_user);
    }
};

}
#endif /* _AXI_SIGNAL_IF_H_ */
