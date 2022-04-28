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

#pragma once

#include <obi/obi_tlm.h>
#include <scc/mt19937_rng.h>
#include <scc/peq.h>
#include <scc/report.h>
#include <systemc>
#include <tlm/scc/initiator_mixin.h>
#include <tlm/scc/scv/tlm_rec_initiator_socket.h>
#include <tlm/scc/tlm_gp_shared.h>
#include <tlm/scc/tlm_id.h>
#include <tlm/scc/tlm_mm.h>
#include <tlm>

#include <memory>
#include <queue>
#include <tuple>
#include <unordered_map>

namespace obi {

namespace pin {
template <unsigned int DATA_WIDTH = 32, unsigned int ADDR_WIDTH = 32, unsigned int ID_WIDTH = 0,
          unsigned int USER_WIDTH = 0>
class target : public sc_core::sc_module {
public:
    using payload_type = tlm::tlm_base_protocol_types::tlm_payload_type;
    using phase_type = tlm::tlm_base_protocol_types::tlm_phase_type;

    template <unsigned WIDTH = 0, typename TYPE = sc_dt::sc_uint<WIDTH>, int N = 1>
    using sc_in_opt = sc_core::sc_port<sc_core::sc_signal_in_if<TYPE>, N, sc_core::SC_ZERO_OR_MORE_BOUND>;
    template <unsigned WIDTH = 0, typename TYPE = sc_dt::sc_uint<WIDTH>, int N = 1>
    using sc_out_opt = sc_core::sc_port<sc_core::sc_signal_write_if<TYPE>, N, sc_core::SC_ZERO_OR_MORE_BOUND>;

    SC_HAS_PROCESS(target);

    target(sc_core::sc_module_name nm);

    tlm::scc::initiator_mixin<tlm::scc::scv::tlm_rec_initiator_socket<DATA_WIDTH>> isckt{"isckt"};
    // Global signals
    sc_core::sc_in<bool> clk_i{"clk_i"};
    sc_core::sc_in<bool> resetn_i{"resetn_i"}; // active low reset
    // A Channel signals
    sc_core::sc_in<bool> req_i{"req_i"};
    sc_core::sc_out<bool> gnt_o{"gnt_o"};
    sc_core::sc_in<sc_dt::sc_uint<ADDR_WIDTH>> addr_i{"addr_i"};
    sc_core::sc_in<bool> we_i{"we_i"};
    sc_core::sc_in<sc_dt::sc_uint<DATA_WIDTH / 8>> be_i{"be_i"};
    sc_core::sc_in<sc_dt::sc_uint<DATA_WIDTH>> wdata_i{"wdata_i"};
    sc_in_opt<USER_WIDTH> auser_i{"auser_i"};
    sc_in_opt<USER_WIDTH> wuser_i{"wuser_i"};
    sc_in_opt<ID_WIDTH> aid_i{"aid_i"};
    // R Channel signals
    sc_core::sc_out<bool> rvalid_o{"rvalid_o"};
    sc_core::sc_in<bool> rready_i{"rready_i"};
    sc_core::sc_out<sc_dt::sc_uint<DATA_WIDTH>> rdata_o{"rdata_o"};
    sc_core::sc_out<bool> err_o{"err_o"};
    sc_out_opt<USER_WIDTH> ruser_o{"ruser_o"};
    sc_out_opt<ID_WIDTH> r_id_o{"r_id_o"};

    tlm::tlm_sync_enum nb_transport_bw(payload_type& trans, phase_type& phase, sc_core::sc_time& t);

#ifdef HAS_CCI
    cci::cci_param<sc_core::sc_time> sample_delay{"sample_delay", 1_ps};
    cci::cci_param<int> req2gnt_delay{"req2gnt_delay", 0};
    cci::cci_param<int> addr2data_delay{"addr2data_delay", 0};
#else
    sc_core::sc_time sample_delay{0_ns};
    int req2gnt_delay{0};
    int addr2data_delay{0};
#endif
private:
    void clk_cb();
    void achannel_req_t();
    void rchannel_rsp_t();

    scc::peq<tlm::scc::tlm_gp_shared_ptr> achannel_rsp;
    std::deque<std::tuple<tlm::scc::tlm_gp_shared_ptr, unsigned>> rchannel_pending_rsp;
    scc::peq<tlm::scc::tlm_gp_shared_ptr> rchannel_rsp;
    struct tx_state {
        bool addrPhaseFinished;
        tlm::tlm_phase last_phase;
        tlm::scc::tlm_gp_shared_ptr pending_tx;
    };
    std::unordered_map<payload_type*, tx_state> states;
};

/////////////////////////////////////////////////////////////////////////////////////////
// Class definition
/////////////////////////////////////////////////////////////////////////////////////////
template <unsigned int DATA_WIDTH, unsigned int ADDR_WIDTH, unsigned int ID_WIDTH, unsigned int USER_WIDTH>
inline target<DATA_WIDTH, ADDR_WIDTH, ID_WIDTH, USER_WIDTH>::target::target(sc_core::sc_module_name nm)
: sc_module(nm) {
    isckt.register_nb_transport_bw(
        [this](payload_type& trans, phase_type& phase, sc_core::sc_time& t) -> tlm::tlm_sync_enum {
            return nb_transport_bw(trans, phase, t);
        });
    SC_METHOD(clk_cb)
    sensitive << clk_i.pos() << resetn_i.neg();
    SC_THREAD(achannel_req_t)
    SC_THREAD(rchannel_rsp_t);
}

template <unsigned int DATA_WIDTH, unsigned int ADDR_WIDTH, unsigned int ID_WIDTH, unsigned int USER_WIDTH>
inline void target<DATA_WIDTH, ADDR_WIDTH, ID_WIDTH, USER_WIDTH>::target::clk_cb() {
    sc_core::sc_time delay = sc_core::SC_ZERO_TIME;
    if(clk_i.event()) {
        if(rchannel_pending_rsp.size()) {
            auto& head = rchannel_pending_rsp.front();
            if(std::get<1>(head) == 0) {
                rchannel_rsp.notify(std::get<0>(head));
                rchannel_pending_rsp.pop_front();
            }
            for(auto& e : rchannel_pending_rsp) {
                if(std::get<1>(e))
                    --std::get<1>(e);
            }
        }
    }
}

template <unsigned int DATA_WIDTH, unsigned int ADDR_WIDTH, unsigned int ID_WIDTH, unsigned int USER_WIDTH>
inline tlm::tlm_sync_enum
target<DATA_WIDTH, ADDR_WIDTH, ID_WIDTH, USER_WIDTH>::target::nb_transport_bw(payload_type& trans, phase_type& phase,
                                                                              sc_core::sc_time& t) {
    auto id = obi::get_obi_id(trans);
    auto* ext = trans.get_extension<obi::obi_extension>();
    sc_assert(ext && "obi_extension missing");
    switch(phase) {
    case tlm::END_REQ: {
        auto it = states.find(&trans);
        sc_assert(it != states.end());
        it->second.last_phase = tlm::END_REQ;
        achannel_rsp.notify(&trans, t);
        return tlm::TLM_ACCEPTED;
    }
    case tlm::BEGIN_RESP: {
        auto it = states.find(&trans);
        sc_assert(it != states.end());
        auto& state = it->second;
        if(state.pending_tx) {
            unsigned resp_delay = addr2data_delay < 0 ? scc::MT19937::uniform(0, -addr2data_delay) : addr2data_delay;
            if(resp_delay) {
                rchannel_pending_rsp.push_back({state.pending_tx, resp_delay - 1});
            } else
                rchannel_pending_rsp.push_back({state.pending_tx, 0});
        }
        state.last_phase = tlm::BEGIN_RESP;
        return tlm::TLM_ACCEPTED;
    }
    default:
        SCCWARN(SCMOD) << phase << " is unsupported phase transaction combination";
        return tlm::TLM_ACCEPTED;
    }
}

template <unsigned int DATA_WIDTH, unsigned int ADDR_WIDTH, unsigned int ID_WIDTH, unsigned int USER_WIDTH>
inline void target<DATA_WIDTH, ADDR_WIDTH, ID_WIDTH, USER_WIDTH>::achannel_req_t() {
    wait(SC_ZERO_TIME);
    wait(clk_i.posedge_event());
    while(true) {
        while(resetn_i.read() == false)
            wait(clk_i.posedge_event());
        while(resetn_i.read() == true) {
            gnt_o.write(req2gnt_delay == 0);
            do {
                wait(this->req_i.posedge_event() | clk_i.negedge_event());
            } while(this->req_i.read() == false);
            auto data_len = DATA_WIDTH / 8;
            tlm::scc::tlm_gp_shared_ptr gp = tlm::scc::tlm_mm<>::get().allocate<obi::obi_extension>(data_len);
            gp->set_streaming_width(data_len);
            gp->set_address(addr_i.read());
            gp->set_command(we_i.read() ? tlm::TLM_WRITE_COMMAND : tlm::TLM_READ_COMMAND);
            auto be = static_cast<unsigned>(be_i.read());
            auto cnt = util::bit_count(be);
            gp->set_streaming_width(cnt);
            gp->set_data_length(cnt);
            if(we_i.read()) {
                auto bus_data = wdata_i.read();
                auto tx_byte_idx = 0;
                for(auto i = 0U, be_idx = 0U; i < DATA_WIDTH; i += 8, ++be_idx) {
                    if(be & (1U << be_idx)) {
                        *(gp->get_data_ptr() + tx_byte_idx) = bus_data.range(i + 7, i).to_uint();
                        tx_byte_idx++;
                    }
                }
                SCCTRACE(SCMOD) << "Got write request to address 0x" << std::hex << gp->get_address();
            } else
                SCCTRACE(SCMOD) << "Got read request to address 0x" << std::hex << gp->get_address();
            auto ext = gp->get_extension<obi::obi_extension>();
            if(ID_WIDTH && aid_i.get_interface())
                ext->set_id(aid_i->read());
            if(USER_WIDTH) {
                if(auser_i.get_interface())
                    ext->set_auser(auser_i->read());
                if(wuser_i.get_interface() && we_i.read())
                    ext->set_duser(wuser_i->read());
            }
            auto& state = states[gp.get()];
            phase_type phase = tlm::BEGIN_REQ;
            auto delay = sc_core::SC_ZERO_TIME;
            auto ret = isckt->nb_transport_fw(*gp, phase, delay);
            auto startResp = false;
            state.last_phase = phase;
            if(ret == tlm::TLM_ACCEPTED) {
                SCCTRACE(SCMOD) << "waiting for TLM reponse";
                auto resp = achannel_rsp.get();
                sc_assert(resp.get() == gp.get());
            }
            unsigned gnt_delay = req2gnt_delay < 0 ? scc::MT19937::uniform(0, -req2gnt_delay) : req2gnt_delay;
            for(unsigned i = 0U; i < gnt_delay; ++i)
                wait(clk_i.posedge_event());
            gnt_o.write(true);
            wait(clk_i.posedge_event());
            state.addrPhaseFinished = true;
            if(state.last_phase == tlm::BEGIN_RESP) {
                unsigned resp_delay =
                    addr2data_delay < 0 ? scc::MT19937::uniform(0, -addr2data_delay) : addr2data_delay;
                if(resp_delay) {
                    rchannel_pending_rsp.push_back({gp, resp_delay - 1});
                } else
                    rchannel_rsp.notify(gp);
            } else {
                state.pending_tx = gp;
            }
        }
    }
}

template <unsigned int DATA_WIDTH, unsigned int ADDR_WIDTH, unsigned int ID_WIDTH, unsigned int USER_WIDTH>
inline void target<DATA_WIDTH, ADDR_WIDTH, ID_WIDTH, USER_WIDTH>::rchannel_rsp_t() {
    rdata_o.write(0);
    err_o.write(false);
    if(ID_WIDTH && r_id_o.get_interface())
        r_id_o->write(0);
    if(USER_WIDTH && ruser_o.get_interface())
        ruser_o->write(0);
    while(true) {
        rvalid_o.write(false);
        tlm::scc::tlm_gp_shared_ptr tx = rchannel_rsp.get();
        if(tx->get_command() == tlm::TLM_READ_COMMAND) {
            auto offset = tx->get_address() % (DATA_WIDTH / 8);
            auto rx_data{sc_dt::sc_uint<DATA_WIDTH>(0)};
            for(unsigned i = 0U, j = offset * 8; i < tx->get_data_length(); ++i, j += 8) {
                rx_data.range(j + 7, j) = *(tx->get_data_ptr() + i);
            }
            rdata_o.write(rx_data);
            SCCTRACE(SCMOD) << "responding to read request to address 0x" << std::hex << tx->get_address();
        } else
            SCCTRACE(SCMOD) << "responding to write request to address 0x" << std::hex << tx->get_address();
        err_o.write(tx->get_response_status() != tlm::TLM_OK_RESPONSE);
        if(ID_WIDTH || USER_WIDTH) {
            auto ext = tx->get_extension<obi::obi_extension>();
            if(ID_WIDTH && r_id_o.get_interface())
                r_id_o->write(ext->get_id());
            if(USER_WIDTH && ruser_o.get_interface())
                ruser_o->write(ext->get_duser());
        }
        rvalid_o.write(true);
        wait(sample_delay); // let rready settle
        while(!rready_i.read())
            wait(rready_i.value_changed_event());
        phase_type phase = tlm::END_RESP;
        auto delay = sc_core::SC_ZERO_TIME;
        auto ret = isckt->nb_transport_fw(*tx, phase, delay);
        auto it = states.find(tx.get());
        sc_assert(it != states.end());
        states.erase(it);
        wait(clk_i.posedge_event());
    }
}
} // namespace pin
} // namespace obi
