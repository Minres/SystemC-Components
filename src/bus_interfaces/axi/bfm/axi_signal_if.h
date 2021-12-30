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

#ifndef _AXI_BFM_AXI_IF_H_
#define _AXI_BFM_AXI_IF_H_

#include <systemc>

namespace axi {
namespace bfm {

template<unsigned int BUSWDTH = 32, unsigned int ADDRWDTH = 32, unsigned int IDWDTH = 32, unsigned int USERWDTH = 0>
struct axi_cfg {
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

//! Write address channel signals
template<typename CFG, typename MST=master_types>
struct aw_ch {
    typename MST::template m2s_t<sc_dt::sc_uint<CFG::IDWIDTH>>       aw_id{"aw_id"};
    typename MST::template m2s_t<sc_dt::sc_uint<CFG::ADDRWIDTH>>     aw_addr{"aw_addr"};
    typename MST::template s2m_t<bool>                               aw_ready{"aw_ready"};
    typename MST::template m2s_opt_t<sc_dt::sc_uint<2>>              aw_lock{"aw_lock"}; // only AXI3
    typename MST::template m2s_t<bool>                               aw_valid{"aw_valid"};
    typename MST::template m2s_t<sc_dt::sc_uint<3>>                  aw_prot{"aw_prot"};
    typename MST::template m2s_t<sc_dt::sc_uint<3>>                  aw_size{"aw_size"};
    typename MST::template m2s_t<sc_dt::sc_uint<4>>                  aw_cache{"aw_cache"};
    typename MST::template m2s_t<sc_dt::sc_uint<2>>                  aw_burst{"aw_burst"};
    typename MST::template m2s_t<sc_dt::sc_uint<4>>                  aw_qos{"aw_qos"};
    typename MST::template m2s_t<sc_dt::sc_uint<4>>                  aw_region{"aw_region"};
    typename MST::template m2s_t<sc_dt::sc_uint<8>>                  aw_len{"aw_len"};
    typename MST::template m2s_opt_t<sc_dt::sc_uint<CFG::USERWIDTH>> aw_user{"aw_user"};
};

//! write data channel signals
template<typename CFG, typename MST=master_types>
struct wdata_ch {
    typename MST::template m2s_t<sc_dt::sc_uint<CFG::IDWIDTH>>       w_id{"w_id"};
    typename MST::template m2s_t<sc_dt::sc_biguint<CFG::BUSWIDTH>>   w_data{"w_data"};
    typename MST::template m2s_t<sc_dt::sc_uint<CFG::BUSWIDTH / 8>>  w_strb{"w_strb"};
    typename MST::template m2s_t<bool>                               w_last{"w_last"};
    typename MST::template m2s_t<bool>                               w_valid{"w_valid"};
    typename MST::template s2m_t<bool>                               w_ready{"w_ready"};
    typename MST::template m2s_opt_t<sc_dt::sc_uint<CFG::USERWIDTH>> w_user{"w_user"};
};

//! write response channel signals
template<typename CFG, typename MST=master_types>
struct b_ch {
    typename MST::template s2m_t<bool>                               b_valid{"b_valid"};
    typename MST::template m2s_t<bool>                               b_ready{"b_ready"};
    typename MST::template s2m_t<sc_dt::sc_uint<CFG::IDWIDTH>>       b_id{"b_id"};
    typename MST::template s2m_t<sc_dt::sc_uint<2>>                  b_resp{"b_resp"};
    typename MST::template s2m_opt_t<sc_dt::sc_uint<CFG::USERWIDTH>> b_user{"b_user"};
};

//! read address channel signals
template<typename CFG, typename MST=master_types>
struct ar_ch {
    typename MST::template m2s_t<sc_dt::sc_uint<CFG::IDWIDTH>>       ar_id{"ar_id"};
    typename MST::template m2s_t<sc_dt::sc_uint<CFG::ADDRWIDTH>>     ar_addr{"ar_addr"};
    typename MST::template m2s_t<sc_dt::sc_uint<8>>                  ar_len{"ar_len"};
    typename MST::template m2s_t<sc_dt::sc_uint<3>>                  ar_size{"ar_size"};
    typename MST::template m2s_t<sc_dt::sc_uint<2>>                  ar_burst{"ar_burst"};
    typename MST::template m2s_opt_t<sc_dt::sc_uint<2>>              ar_lock{"ar_lock"}; // only AXI3
    typename MST::template m2s_t<sc_dt::sc_uint<4>>                  ar_cache{"ar_cache"};
    typename MST::template m2s_t<sc_dt::sc_uint<3>>                  ar_prot{"ar_prot"};
    typename MST::template m2s_t<sc_dt::sc_uint<4>>                  ar_qos{"ar_qos"};
    typename MST::template m2s_t<sc_dt::sc_uint<4>>                  ar_region{"ar_region"};
    typename MST::template m2s_t<bool>                               ar_valid{"ar_valid"};
    typename MST::template s2m_t<bool>                               ar_ready{"ar_ready"};
    typename MST::template m2s_opt_t<sc_dt::sc_uint<CFG::USERWIDTH>> ar_user{"ar_user"};
};

//! Read data channel signals
template<typename CFG, typename MST=master_types>
struct rresp_ch {
    typename MST::template s2m_t<sc_dt::sc_uint<CFG::IDWIDTH>>       r_id{"r_id"};
    typename MST::template s2m_t<sc_dt::sc_biguint<CFG::BUSWIDTH>>   r_data{"r_data"};
    typename MST::template s2m_t<sc_dt::sc_uint<2>>                  r_resp{"r_resp"};
    typename MST::template s2m_t<bool>                               r_last{"r_last"};
    typename MST::template s2m_t<bool>                               r_valid{"r_valid"};
    typename MST::template m2s_t<bool>                               r_ready{"r_ready"};
    typename MST::template s2m_opt_t<sc_dt::sc_uint<CFG::USERWIDTH>> r_user{"r_user"};
};

}
}
#endif /* _AXI_BFM_AXI_IF_H_ */
