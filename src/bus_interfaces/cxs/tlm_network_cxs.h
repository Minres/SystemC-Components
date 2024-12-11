/*******************************************************************************
 * Copyright 2024 MINRES Technologies GmbH
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

#ifndef _CXS_TLM_NETWROK_CXS_H_
#define _CXS_TLM_NETWROK_CXS_H_

#include <tlm/nw/tlm_network_gp.h>
#include <tlm/nw/tlm_network_sockets.h>
#include <cci_configuration>
#include <tlm/scc/tlm_gp_shared.h>
#include <tlm/scc/tlm_mm.h>
#include <scc/peq.h>

//! @brief CXS TLM utilities
namespace cxs {
enum class CXS_CMD {
    DATA, CREDIT
};

using tlm_network_cxs_payload = tlm::nw::tlm_network_payload<CXS_CMD>;
struct tlm_network_cxs_types {
    using tlm_payload_type = tlm_network_cxs_payload;
    using tlm_phase_type = ::tlm::tlm_phase;
};
}

namespace tlm { namespace scc {
template<> struct tlm_mm_traits<cxs::tlm_network_cxs_types> {
    using mm_if_type =  tlm::nw::tlm_base_mm_interface;
    using payload_base = tlm::nw::tlm_network_payload_base;
};
}}

namespace cxs {
template <unsigned PHITWIDTH=64, int N = 1>
using tlm_network_cxs_initiator_socket = tlm::nw::tlm_network_initiator_socket<PHITWIDTH, CXS_CMD, tlm_network_cxs_types, N>;
template <unsigned PHITWIDTH=64, int N = 1>
using tlm_network_cxs_target_socket = tlm::nw::tlm_network_target_socket<PHITWIDTH, CXS_CMD, tlm_network_cxs_types, N>;
using tlm_cxs_shared_ptr = tlm::scc::tlm_payload_shared_ptr<tlm_network_cxs_payload>;
using tlm_mm_cxs = tlm::scc::tlm_mm<tlm_network_cxs_types, false>;

template <unsigned PHITWIDTH=64>
struct cxs_channel:
        public sc_core::sc_module,
        public tlm::nw::tlm_network_fw_transport_if<tlm_network_cxs_types>,
        public tlm::nw::tlm_network_bw_transport_if<tlm_network_cxs_types> {

    using transaction_type = tlm_network_cxs_types::tlm_payload_type;
    using phase_type = tlm_network_cxs_types::tlm_phase_type;

    sc_core::sc_in<bool> rcv_clk_i{"clk_i"};

    tlm_network_cxs_target_socket<PHITWIDTH> tsck{"tsck"};

    tlm_network_cxs_initiator_socket<PHITWIDTH> isck{"isck"};

    cci::cci_param<sc_core::sc_time> channel_delay{"channel_delay", sc_core::SC_ZERO_TIME, "delay of the CXS channel"};

    cci::cci_param<sc_core::sc_time> clock_period{"clock_period", sc_core::SC_ZERO_TIME, "clock period of the CXS channel"};

    cxs_channel(sc_core::sc_module_name const& nm ): sc_core::sc_module(nm){
        isck(*this);
        tsck(*this);
        SC_HAS_PROCESS(cxs_channel);
        SC_METHOD(clock);
        sensitive<<rcv_clk_i.pos();
        dont_initialize();
    }

    void start_of_simulation() override {
        if(clock_period.get_value() == sc_core::SC_ZERO_TIME)
            if(auto clk_if = dynamic_cast<sc_core::sc_clock*>(rcv_clk_i.get_interface()))
                    clock_period.set_value(clk_if->period());
    }

    void b_transport(transaction_type& trans, sc_core::sc_time& t) override {
        t+=channel_delay;
        isck->b_transport(trans, t);
    }

    tlm::tlm_sync_enum nb_transport_fw(transaction_type& trans, phase_type& phase, sc_core::sc_time& t) override {
        if(phase==tlm::nw::REQUEST) {
            fw_peq.notify(tlm_cxs_shared_ptr(&trans), channel_delay.get_value());
            if(clock_period.get_value() > sc_core::SC_ZERO_TIME)
                t+=clock_period-1_ps;
            phase = tlm::nw::CONFIRM;
            return tlm::TLM_UPDATED;
        }
        throw std::runtime_error("illegal request in forward path");
    }

    tlm::tlm_sync_enum nb_transport_bw(transaction_type& trans, phase_type& phase, sc_core::sc_time& t) override {
        if(phase==tlm::nw::RESPONSE) {
            return tlm::TLM_ACCEPTED;
        }
        throw std::runtime_error("illegal response in backward path");
    }

    unsigned int transport_dbg(transaction_type& trans) override {
        return isck->transport_dbg(trans);
    }
private:
    void clock() {
        if(fw_peq.has_next()) {
            auto phase = tlm::nw::INDICATION;
            auto t = sc_core::SC_ZERO_TIME;
            auto trans = fw_peq.get();
            isck->nb_transport_fw(*trans, phase, t);
        }
    }
    scc::peq<tlm_cxs_shared_ptr> fw_peq;
};
}
#endif // _CXS_TLM_NETWROK_CXS_H_
