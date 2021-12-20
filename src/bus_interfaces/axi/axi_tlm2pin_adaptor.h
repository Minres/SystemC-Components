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

#include <axi/axi_tlm.h>
#include <tlm/scc/scv/tlm_rec_target_socket.h>
#include <tlm/scc/target_mixin.h>

#include <systemc>
#include <tlm>

#include <memory>
#include <queue>
#include <unordered_map>

namespace axi {

template <unsigned int BUSWIDTH = 32, unsigned int ADDRWIDTH = 32, unsigned int IDWIDTH = 32,
          unsigned int USERWIDTH = 0>
class axi_tlm2pin_adaptor : public sc_core::sc_module {
public:
    SC_HAS_PROCESS(axi_tlm2pin_adaptor);

    using payload_type = axi::axi_protocol_types::tlm_payload_type;
    using phase_type = axi::axi_protocol_types::tlm_phase_type;

    template <unsigned WIDTH = 0, typename TYPE = sc_dt::sc_uint<WIDTH>, int N = 1>
    using sc_in_opt = sc_core::sc_port<sc_core::sc_signal_in_if<TYPE>, N, sc_core::SC_ZERO_OR_MORE_BOUND>;
    template <unsigned WIDTH = 0, typename TYPE = sc_dt::sc_uint<WIDTH>, int N = 1>
    using sc_out_opt = sc_core::sc_port<sc_core::sc_signal_write_if<TYPE>, N, sc_core::SC_ZERO_OR_MORE_BOUND>;

    axi_tlm2pin_adaptor(sc_core::sc_module_name nm);

    tlm::scc::target_mixin<tlm::scc::scv::tlm_rec_target_socket<BUSWIDTH, axi::axi_protocol_types>,
                           axi::axi_protocol_types>
        input_socket{"input_socket"};

    sc_core::sc_in<bool> clk_i{"clk_i"};
    sc_core::sc_in<bool> resetn_i{"resetn_i"};

    // Write address channel signals
    sc_core::sc_out<sc_dt::sc_uint<IDWIDTH>> aw_id_o{"aw_id_o"};
    sc_core::sc_out<sc_dt::sc_uint<ADDRWIDTH>> aw_addr_o{"aw_addr_o"};
    sc_core::sc_in<bool> aw_ready_i{"aw_ready_i"};
    sc_core::sc_out<bool> aw_lock_o{"aw_lock_o"};
    sc_core::sc_out<bool> aw_valid_o{"aw_valid_o"};
    sc_core::sc_out<sc_dt::sc_uint<3>> aw_prot_o{"aw_prot_o"};
    sc_core::sc_out<sc_dt::sc_uint<3>> aw_size_o{"aw_size_o"};
    sc_core::sc_out<sc_dt::sc_uint<4>> aw_cache_o{"aw_cache_o"};
    sc_core::sc_out<sc_dt::sc_uint<2>> aw_burst_o{"aw_burst_o"};
    sc_core::sc_out<sc_dt::sc_uint<4>> aw_qos_o{"aw_qos_o"};
    sc_core::sc_out<sc_dt::sc_uint<4>> aw_region_o{"aw_region_o"};
    sc_core::sc_out<sc_dt::sc_uint<8>> aw_len_o{"aw_len_o"};
    sc_out_opt<USERWIDTH> aw_user_o{"aw_user_o"};

    // write data channel signals
    sc_core::sc_out<sc_dt::sc_biguint<BUSWIDTH>> w_data_o{"w_data_o"};
    sc_core::sc_out<sc_dt::sc_uint<BUSWIDTH / 8>> w_strb_o{"w_strb_o"};
    sc_core::sc_out<bool> w_last_o{"w_last_o"};
    sc_core::sc_out<bool> w_valid_o{"w_valid_o"};
    sc_core::sc_in<bool> w_ready_i{"w_ready_i"};
    sc_out_opt<USERWIDTH> w_user_o{"w_user_o"};

    // write response channel signals
    sc_core::sc_in<bool> b_valid_i{"b_valid_i"};
    sc_core::sc_out<bool> b_ready_o{"b_ready_o"};
    sc_core::sc_in<sc_dt::sc_uint<IDWIDTH>> b_id_i{"b_id_i"};
    sc_core::sc_in<sc_dt::sc_uint<2>> b_resp_i{"b_resp_i"};
    sc_in_opt<USERWIDTH> b_user_i{"b_user_i"};

    // read address channel signals
    sc_core::sc_out<sc_dt::sc_uint<IDWIDTH>> ar_id_o{"ar_id_o"};
    sc_core::sc_out<sc_dt::sc_uint<ADDRWIDTH>> ar_addr_o{"ar_addr_o"};
    sc_core::sc_out<sc_dt::sc_uint<8>> ar_len_o{"ar_len_o"};
    sc_core::sc_out<sc_dt::sc_uint<3>> ar_size_o{"ar_size_o"};
    sc_core::sc_out<sc_dt::sc_uint<2>> ar_burst_o{"ar_burst_o"};
    sc_core::sc_out<bool> ar_lock_o{"ar_lock_o"};
    sc_core::sc_out<sc_dt::sc_uint<4>> ar_cache_o{"ar_cache_o"};
    sc_core::sc_out<sc_dt::sc_uint<3>> ar_prot_o{"ar_prot_o"};
    sc_core::sc_out<sc_dt::sc_uint<4>> ar_qos_o{"ar_qos_o"};
    sc_core::sc_out<sc_dt::sc_uint<4>> ar_region_o{"ar_region_o"};
    sc_core::sc_out<bool> ar_valid_o{"ar_valid_o"};
    sc_core::sc_in<bool> ar_ready_i{"ar_ready_i"};
    sc_out_opt<USERWIDTH> ar_user_o{"ar_user_o"};

    // Read data channel signals
    sc_core::sc_in<sc_dt::sc_uint<IDWIDTH>> r_id_i{"r_id_i"};
    sc_core::sc_in<sc_dt::sc_biguint<BUSWIDTH>> r_data_i{"r_data_i"};
    sc_core::sc_in<sc_dt::sc_uint<2>> r_resp_i{"r_resp_i"};
    sc_core::sc_in<bool> r_last_i{"r_last_i"};
    sc_core::sc_in<bool> r_valid_i{"r_valid_i"};
    sc_core::sc_out<bool> r_ready_o{"r_ready_o"};
    sc_in_opt<USERWIDTH> r_user_i{"r_user_i"};

    tlm::tlm_sync_enum nb_transport_fw(payload_type& trans, phase_type& phase, sc_core::sc_time& t);

private:
    /**
     * a handle class holding the pointer to a transaction payload and associated phase
     */
    struct trans_handle {
        //! pointer to the associated AXITLM payload
        payload_type* payload = nullptr;
        //! current protocol phase
        phase_type phase = tlm::UNINITIALIZED_PHASE;
        //! beat counter
        unsigned beat_cnt = 0;
    };

    std::unordered_map<uint8_t, std::queue<std::shared_ptr<trans_handle>>> active_w_transactions;
    std::unordered_map<uint8_t, std::queue<std::shared_ptr<trans_handle>>> active_r_transactions;
    std::deque<std::shared_ptr<trans_handle>> active_ar_end_transactions;
    std::deque<std::shared_ptr<trans_handle>> active_aw_end_transactions;
    void remove_trans(const unsigned id, const bool is_read);
    void register_trans(unsigned int axi_id, payload_type& trans, phase_type& phase);
    void end_ar_req();
    void end_aw_req();
 
    void bus_thread();
};

/////////////////////////////////////////////////////////////////////////////////////////
// Class definition
/////////////////////////////////////////////////////////////////////////////////////////
template <unsigned int BUSWIDTH, unsigned int ADDRWIDTH, unsigned int IDWIDTH, unsigned int USERWIDTH>
inline axi_tlm2pin_adaptor<BUSWIDTH, ADDRWIDTH, IDWIDTH, USERWIDTH>::axi_tlm2pin_adaptor(sc_core::sc_module_name nm)
: sc_module(nm) {
    input_socket.register_nb_transport_fw(
        [this](payload_type& trans, phase_type& phase, sc_core::sc_time& t) -> tlm::tlm_sync_enum {
            return nb_transport_fw(trans, phase, t);
        });

    SC_METHOD(bus_thread)
    sensitive << clk_i.pos() << resetn_i.neg();
}

template <unsigned int BUSWIDTH, unsigned int ADDRWIDTH, unsigned int IDWIDTH, unsigned int USERWIDTH>
inline tlm::tlm_sync_enum
axi_tlm2pin_adaptor<BUSWIDTH, ADDRWIDTH, IDWIDTH, USERWIDTH>::nb_transport_fw(payload_type& trans, phase_type& phase,
                                                                              sc_core::sc_time& t) {
    if(trans.has_mm())
        trans.acquire();

    auto axi_id = axi::get_axi_id(trans);
    SCCTRACE(SCMOD) << phase << " FW " << (trans.is_read() ? "RD" : "WR") << " forward trans " << std::hex << &trans
                    << std::dec << " (axi_id:" << axi_id << ")";
    tlm::tlm_sync_enum status{tlm::TLM_ACCEPTED};

    if(phase == axi::BEGIN_PARTIAL_REQ || phase == tlm::BEGIN_REQ) {
        auto ext = trans.get_extension<axi::axi4_extension>();
        sc_assert(ext && "axi4_extension missing");

        if(trans.is_read()) {
            register_trans(axi_id, trans, phase);
	    auto th = std::make_shared<trans_handle>();
	    th->payload = &trans;
	    th->phase = tlm::END_REQ;
	    active_ar_end_transactions.push_back(th);
	    sc_core::sc_spawn([this]()->void {end_ar_req();});
        } else { // Write transaction
            auto it = active_w_transactions.find(axi_id);
            if(it == active_w_transactions.end())
                register_trans(axi_id, trans, phase);
            else {
                auto act_trans = it->second.front();
                if(act_trans->payload == &trans) // if transaction is ongoing update the phase
                    act_trans->phase = phase;
                else // otherwise push transaction in the queue
                    register_trans(axi_id, trans, phase);
            }
	    auto th = std::make_shared<trans_handle>();
	    th->payload = &trans;
	    if(phase == axi::BEGIN_PARTIAL_REQ) th->phase = axi::END_PARTIAL_REQ;
	    else th->phase = tlm::END_REQ;
	    active_aw_end_transactions.push_back(th);
	    sc_core::sc_spawn([this]()->void {end_aw_req();});
        }
	return status;
    };
    if(phase == axi::END_PARTIAL_RESP) {
      return status;
    };
    if(phase == tlm::END_RESP) {
        trans.set_response_status(tlm::TLM_OK_RESPONSE);
        remove_trans(axi_id, trans.is_read());
        if(trans.has_mm())
            trans.release();
        status = tlm::TLM_ACCEPTED;
	return status;
    }
    SCCWARN(SCMOD) << phase << " unkown phase transaction combination for trans";
    return status;
}

// todo: need to add backpressure if the end phase shouldn't be asserted right away 
template <unsigned int BUSWIDTH, unsigned int ADDRWIDTH, unsigned int IDWIDTH, unsigned int USERWIDTH>
void axi_tlm2pin_adaptor<BUSWIDTH, ADDRWIDTH, IDWIDTH, USERWIDTH>::end_ar_req() {
  sc_core::sc_time delay = sc_core::SC_ZERO_TIME;
  auto th = active_ar_end_transactions.front();
  SCCINFO(SCMOD) << th->phase << " BW AR sending transaction req end";
  wait(delay);
  input_socket->nb_transport_bw(*th->payload, th->phase, delay);
  active_ar_end_transactions.pop_front();
};
 
// todo: need to add backpressure if the end phase shouldn't be asserted right away 
template <unsigned int BUSWIDTH, unsigned int ADDRWIDTH, unsigned int IDWIDTH, unsigned int USERWIDTH>
void
axi_tlm2pin_adaptor<BUSWIDTH, ADDRWIDTH, IDWIDTH, USERWIDTH>::end_aw_req() {
  sc_core::sc_time delay = sc_core::SC_ZERO_TIME;
  auto th = active_aw_end_transactions.front();
  SCCINFO(SCMOD) << th->phase << " BW AW sending transaction req end";
  wait(delay);
  input_socket->nb_transport_bw(*th->payload, th->phase, delay);
  active_aw_end_transactions.pop_front();
};
 
template <unsigned int BUSWIDTH, unsigned int ADDRWIDTH, unsigned int IDWIDTH, unsigned int USERWIDTH>
inline void axi_tlm2pin_adaptor<BUSWIDTH, ADDRWIDTH, IDWIDTH, USERWIDTH>::bus_thread() {
    auto delay = sc_core::SC_ZERO_TIME;

    if(!resetn_i.read()) { // active-low reset
        ar_valid_o.write(false);
        aw_valid_o.write(false);
        w_valid_o.write(false);
        w_last_o.write(false);
        b_ready_o.write(false);
        r_ready_o.write(false);
    } else {
        if(aw_ready_i.read())
            aw_valid_o.write(false);
        if(w_ready_i.read()) {
            w_valid_o.write(false);
            w_last_o.write(false);
        }
        if(ar_ready_i.read()) {
            ar_valid_o.write(false);
        }
        r_ready_o.write(true);
        b_ready_o.write(true);

        // AR channel
        for(auto& it : active_r_transactions) {
            auto read_trans = it.second.front();
            if(read_trans->phase == tlm::BEGIN_REQ) {
                payload_type* p = read_trans->payload;
                auto ext = p->get_extension<axi::axi4_extension>();
                sc_assert(ext && "axi4_extension missing");

                ar_addr_o.write(p->get_address());
                ar_id_o.write(ext->get_id());
                ar_len_o.write(ext->get_length());
                ar_size_o.write(ext->get_size());
                ar_burst_o.write(axi::to_int(ext->get_burst()));
                ar_lock_o.write(ext->is_exclusive());
                ar_cache_o.write(ext->get_cache());
                ar_prot_o.write(ext->get_prot());
                ar_qos_o.write(ext->get_qos());
                ar_region_o.write(ext->get_region());
                if(ar_user_o.get_interface()) // optional user interface
                    ar_user_o->write(ext->get_user(common::id_type::CTRL));
                ar_valid_o.write(true);

                read_trans->phase = tlm::END_REQ;
                SCCTRACE(SCMOD) << read_trans->phase << " AR bit assignment trans: " << p << " addr: " << p->get_address()
                                << " axi_id: " << ext->get_id();
                break;
            }
        }

        // AW+W channel
        for(auto& it : active_w_transactions) {
            auto write_trans = it.second.front();
            if(write_trans->phase == axi::BEGIN_PARTIAL_REQ || write_trans->phase == tlm::BEGIN_REQ) {
                payload_type* p = write_trans->payload;
                auto ext = p->get_extension<axi::axi4_extension>();
                sc_assert(ext && "axi4_extension missing");

                if(write_trans->beat_cnt == 0) {
                    aw_id_o.write(ext->get_id());
                    aw_addr_o.write(p->get_address());
                    aw_len_o.write(ext->get_length());
                    aw_size_o.write(ext->get_size());
                    aw_burst_o.write(axi::to_int(ext->get_burst()));
                    aw_lock_o.write(ext->is_exclusive());
                    aw_cache_o.write(ext->get_cache());
                    aw_prot_o.write(ext->get_prot());
                    aw_qos_o.write(ext->get_qos());
                    aw_region_o.write(ext->get_region());
                    if(aw_user_o.get_interface())
                        aw_user_o->write(ext->get_user(common::id_type::CTRL));
                    aw_valid_o.write(true);
                }
                write_trans->beat_cnt++;

                // write data
                sc_dt::sc_biguint<BUSWIDTH> data{0};
                for(size_t j = 0, k = 0, offset = 0; k < 32 / 8; j += 8, ++k, ++offset)
                    data.range(j + 7, j) = *(uint8_t*)(p->get_data_ptr() + offset);

                w_strb_o.write(p->get_byte_enable_length());
                w_data_o.write(data);
                w_valid_o.write(true);
                if(w_user_o.get_interface()) // optional user interface
                    w_user_o->write(ext->get_user(common::id_type::DATA));

                if(write_trans->phase == tlm::BEGIN_REQ)
                    w_last_o.write(true);

                write_trans->phase =
		  (write_trans->phase == axi::BEGIN_PARTIAL_REQ) ? axi::END_PARTIAL_REQ : tlm::END_REQ;
                SCCTRACE(SCMOD) << write_trans->phase << " AW bit assignment trans: " << p << " addr: " << p->get_address()
                                << " axi_id: " << ext->get_id();
                break;
            }
        }

        // The Slave puts the requested data on the Read Data channel and asserts RVALID,
        // indicating the data in the channel is valid.
        if(r_valid_i.read()) {
            unsigned id = r_id_i.read();
            auto it = active_r_transactions.find(id);
            if(it == active_r_transactions.end())
                SCCFATAL(SCMOD) << "Invalid read transaction ID " << id;

            auto read_trans = it->second.front();
            if(read_trans->payload == nullptr)
                SCCERR(SCMOD) << "Invalid transaction";

            if(read_trans->phase == tlm::END_REQ || read_trans->phase == axi::BEGIN_PARTIAL_RESP) {
                auto data = r_data_i.read();
                for(size_t j = 0, k = 0; k < BUSWIDTH / 8; j += 8, ++k) {
                    *(uint8_t*)(read_trans->payload->get_data_ptr() + k) = data.range(j + 7, j).to_uint();
                }

                // The Master asserts RREADY, indicating the master is ready to receive data from the slave.
                r_ready_o.write(true);
                read_trans->phase = axi::BEGIN_PARTIAL_RESP;
                if(r_last_i.read()) {
                    read_trans->phase = tlm::BEGIN_RESP;
                }
                if(r_user_i.get_interface()) { // optional user interface
                    payload_type* p = read_trans->payload;
                    auto ext = p->get_extension<axi::axi4_extension>();
                    sc_assert(ext && "axi4_extension missing");
                    ext->set_user(common::id_type::DATA, r_user_i->read());
                }

                auto ret = input_socket->nb_transport_bw(*read_trans->payload, read_trans->phase, delay);
                SCCTRACE(SCMOD) << read_trans->phase << " R bit assignment trans " << std::hex << read_trans->payload << std::dec
                                << " (axi_id:" << id << ")";

                // EDN_RESP indicates the last phase of the AXI Read transaction
                if(ret == tlm::TLM_UPDATED && read_trans->phase == tlm::END_RESP) {
                    remove_trans(id, read_trans->payload->is_read());
                    if(read_trans->payload->has_mm())
                        read_trans->payload->release();
                }
            }
        }

        if(b_valid_i.read()) {
            unsigned id = b_id_i.read();
            auto it = active_w_transactions.find(id);
            if(it == active_w_transactions.end())
                SCCFATAL(SCMOD) << "Invalid transaction ID " << id;

            auto write_trans = it->second.front();
            if(write_trans->payload == nullptr)
                SCCERR(SCMOD) << "Invalid transaction";

            if(write_trans->phase == tlm::END_REQ) {
                sc_assert(b_resp_i.read() == axi::to_int(axi::resp_e::OKAY));
                if(b_user_i.get_interface()) { // optional user interface
                    payload_type* p = write_trans->payload;
                    auto ext = p->get_extension<axi::axi4_extension>();
                    sc_assert(ext && "axi4_extension missing");
                    ext->set_user(common::id_type::RESP, b_user_i->read());
                }
                b_ready_o.write(true);
                write_trans->phase = tlm::BEGIN_RESP;
                auto ret = input_socket->nb_transport_bw(*write_trans->payload, write_trans->phase, delay);
                SCCTRACE(SCMOD) << write_trans->phase << " B bit assignment trans " << std::hex << write_trans->payload << std::dec
                                << " (axi_id:" << id << ")";
                if(ret == tlm::TLM_UPDATED && write_trans->phase == tlm::END_RESP) {
                    remove_trans(id, write_trans->payload->is_read());
                }
            }
        }
    }
}

template <unsigned int BUSWIDTH, unsigned int ADDRWIDTH, unsigned int IDWIDTH, unsigned int USERWIDTH>
void axi_tlm2pin_adaptor<BUSWIDTH, ADDRWIDTH, IDWIDTH, USERWIDTH>::remove_trans(const unsigned id, const bool is_read) {
    if(is_read) {
        auto it = active_r_transactions.find(id);
        if(it == active_r_transactions.end())
            SCCFATAL(SCMOD) << "Invalid read transaction ID " << id;

        it->second.pop();
        if(it->second.empty())
            active_r_transactions.erase(id);
    } else {
        auto it = active_w_transactions.find(id);
        if(it == active_w_transactions.end())
            SCCFATAL(SCMOD) << "Invalid write transaction ID " << id;

        it->second.pop();
        if(it->second.empty())
            active_w_transactions.erase(id);
    }
}

template <unsigned int BUSWIDTH, unsigned int ADDRWIDTH, unsigned int IDWIDTH, unsigned int USERWIDTH>
void axi_tlm2pin_adaptor<BUSWIDTH, ADDRWIDTH, IDWIDTH, USERWIDTH>::register_trans(unsigned int axi_id,
                                                                                  payload_type& trans,
                                                                                  phase_type& phase) {
    auto th = std::make_shared<trans_handle>();
    th->payload = &trans;
    th->phase = phase;
    if(trans.is_read())
        active_r_transactions[axi_id].push(th);
    else
        active_w_transactions[axi_id].push(th);
}

} // namespace axi
