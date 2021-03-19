#pragma once

#include <axi/axi_tlm.h>
#include <scc/tlm/initiator_mixin.h>
#include <scc/report.h>
#include <scc/tlm/scv4tlm/tlm_rec_initiator_socket.h>
#include <scc/tlm/tlm_id.h>
#include <scc/tlm/tlm_mm.h>

#include <systemc>
#include <tlm>

#include <memory>
#include <queue>
#include <unordered_map>

namespace axi {

template <unsigned int BUSWIDTH = 32, unsigned int ADDRWIDTH = 32, unsigned int IDWIDTH = 32,
          unsigned int USERWIDTH = 0>
class axi_pin2tlm_adaptor : public sc_core::sc_module {
public:
    using payload_type = axi::axi_protocol_types::tlm_payload_type;
    using phase_type = axi::axi_protocol_types::tlm_phase_type;

    template <unsigned WIDTH = 0, typename TYPE = sc_dt::sc_uint<WIDTH>, int N = 1>
    using sc_in_opt = sc_core::sc_port<sc_core::sc_signal_in_if<TYPE>, N, sc_core::SC_ZERO_OR_MORE_BOUND>;
    template <unsigned WIDTH = 0, typename TYPE = sc_dt::sc_uint<WIDTH>, int N = 1>
    using sc_out_opt = sc_core::sc_port<sc_core::sc_signal_write_if<TYPE>, N, sc_core::SC_ZERO_OR_MORE_BOUND>;

    SC_HAS_PROCESS(axi_pin2tlm_adaptor);

    axi_pin2tlm_adaptor(sc_core::sc_module_name nm);

    scc::initiator_mixin<scv4tlm::tlm_rec_initiator_socket<BUSWIDTH, axi::axi_protocol_types>, axi::axi_protocol_types>
        output_socket{"output_socket"};

    sc_core::sc_in<bool> clk_i{"clk_i"};
    sc_core::sc_in<bool> resetn_i{"resetn_i"}; // active low reset

    // Write address channel signals
    sc_core::sc_in<sc_dt::sc_uint<IDWIDTH>> aw_id_i{"aw_id_i"};
    sc_core::sc_in<sc_dt::sc_uint<ADDRWIDTH>> aw_addr_i{"aw_addr_i"};
    sc_core::sc_out<bool> aw_ready_o{"aw_ready_o"};
    sc_core::sc_in<bool> aw_lock_i{"aw_lock_i"};
    sc_core::sc_in<bool> aw_valid_i{"aw_valid_i"};
    sc_core::sc_in<sc_dt::sc_uint<3>> aw_prot_i{"aw_prot_i"};
    sc_core::sc_in<sc_dt::sc_uint<3>> aw_size_i{"aw_size_i"};
    sc_core::sc_in<sc_dt::sc_uint<4>> aw_cache_i{"aw_cache_i"};
    sc_core::sc_in<sc_dt::sc_uint<2>> aw_burst_i{"aw_burst_i"};
    sc_core::sc_in<sc_dt::sc_uint<4>> aw_qos_i{"Aaw_qos_i"};
    sc_core::sc_in<sc_dt::sc_uint<4>> aw_region_i{"aw_region_i"};
    sc_core::sc_in<sc_dt::sc_uint<8>> aw_len_i{"aw_len_i"};
    sc_in_opt<USERWIDTH> aw_user_i{"aw_user_i"};

    // write data channel signals
    sc_core::sc_in<sc_dt::sc_biguint<BUSWIDTH>> w_data_i{"w_data_i"};
    sc_core::sc_in<sc_dt::sc_uint<BUSWIDTH / 8>> w_strb_i{"w_strb_i"};
    sc_core::sc_in<bool> w_last_i{"w_last_i"};
    sc_core::sc_in<bool> w_valid_i{"w_valid_i"};
    sc_core::sc_out<bool> w_ready_o{"w_ready_o"};
    sc_in_opt<USERWIDTH> w_user_i{"w_user_i"};

    // write response channel signals
    sc_core::sc_out<bool> b_valid_o{"b_valid_o"};
    sc_core::sc_in<bool> b_ready_i{"b_ready_i"};
    sc_core::sc_out<sc_dt::sc_uint<IDWIDTH>> b_id_o{"b_id_o"};
    sc_core::sc_out<sc_dt::sc_uint<2>> b_resp_o{"b_resp_o"};
    sc_out_opt<USERWIDTH> b_user_o{"b_user_o"};

    // read address channel signals
    sc_core::sc_in<sc_dt::sc_uint<IDWIDTH>> ar_id_i{"ar_id_i"};
    sc_core::sc_in<sc_dt::sc_uint<ADDRWIDTH>> ar_addr_i{"ar_addr_i"};
    sc_core::sc_in<sc_dt::sc_uint<8>> ar_len_i{"ar_len_i"};
    sc_core::sc_in<sc_dt::sc_uint<3>> ar_size_i{"ar_size_i"};
    sc_core::sc_in<sc_dt::sc_uint<2>> ar_burst_i{"ar_burst_i"};
    sc_core::sc_in<bool> ar_lock_i{"ar_lock_i"};
    sc_core::sc_in<sc_dt::sc_uint<4>> ar_cache_i{"ar_cache_i"};
    sc_core::sc_in<sc_dt::sc_uint<3>> ar_prot_i{"ar_prot_i"};
    sc_core::sc_in<sc_dt::sc_uint<4>> ar_qos_i{"ar_qos_i"};
    sc_core::sc_in<sc_dt::sc_uint<4>> ar_region_i{"ar_region_i"};
    sc_core::sc_in<bool> ar_valid_i{"ar_valid_i"};
    sc_core::sc_out<bool> ar_ready_o{"ar_ready_o"};
    sc_in_opt<USERWIDTH> ar_user_i{"ar_user_i"};

    // Read data channel signals
    sc_core::sc_out<sc_dt::sc_uint<IDWIDTH>> r_id_o{"r_id_o"};
    sc_core::sc_out<sc_dt::sc_biguint<BUSWIDTH>> r_data_o{"r_data_o"};
    sc_core::sc_out<sc_dt::sc_uint<2>> r_resp_o{"r_resp_o"};
    sc_core::sc_out<bool> r_last_o{"r_last_o"};
    sc_core::sc_out<bool> r_valid_o{"r_valid_o"};
    sc_core::sc_in<bool> r_ready_i{"r_ready_i"};
    sc_out_opt<USERWIDTH> r_user_o{"r_user_o"};

    tlm::tlm_sync_enum nb_transport_bw(payload_type& trans, phase_type& phase, sc_core::sc_time& t);

private:
    void bus_thread();

    /**
     * a handle class holding the pointer to the current transaction and associated phase
     */
    struct trans_handle {
        //! pointer to the associated AXITLM payload
        payload_type* payload = nullptr;
        //! current protocol phase
        phase_type phase = tlm::UNINITIALIZED_PHASE;
        //! beat counter
        unsigned beat_cnt = 0;
        //! transaction status
        bool running = false;
    };

    std::unordered_map<uint8_t, std::queue<std::shared_ptr<trans_handle>>> active_w_transactions;
    std::unordered_map<uint8_t, std::queue<std::shared_ptr<trans_handle>>> active_r_transactions;
    void register_trans(unsigned int axi_id, payload_type& trans, phase_type phase);
};

/////////////////////////////////////////////////////////////////////////////////////////
// Class definition
/////////////////////////////////////////////////////////////////////////////////////////
template <unsigned int BUSWIDTH, unsigned int ADDRWIDTH, unsigned int IDWIDTH, unsigned int USERWIDTH>
inline axi_pin2tlm_adaptor<BUSWIDTH, ADDRWIDTH, IDWIDTH, USERWIDTH>::axi_pin2tlm_adaptor::axi_pin2tlm_adaptor(
    sc_core::sc_module_name nm)
: sc_module(nm) {
    output_socket.register_nb_transport_bw(
        [this](payload_type& trans, phase_type& phase, sc_core::sc_time& t) -> tlm::tlm_sync_enum {
            return nb_transport_bw(trans, phase, t);
        });

    SC_METHOD(bus_thread)
    sensitive << clk_i.pos() << resetn_i.neg();
}

template <unsigned int BUSWIDTH, unsigned int ADDRWIDTH, unsigned int IDWIDTH, unsigned int USERWIDTH>
inline void axi_pin2tlm_adaptor<BUSWIDTH, ADDRWIDTH, IDWIDTH, USERWIDTH>::axi_pin2tlm_adaptor::bus_thread() {
    sc_core::sc_time delay = sc_core::SC_ZERO_TIME;
    sc_dt::sc_biguint<BUSWIDTH> write_data{0};

    if(!resetn_i.read()) { // active-low reset
        r_valid_o.write(false);
        r_last_o.write(false);
        b_valid_o.write(false);
        ar_ready_o.write(false);
        aw_ready_o.write(false);
    } else {
        if(r_ready_i.read()) {
            r_valid_o.write(false);
            r_last_o.write(false);
        }
        if(b_ready_i.read())
            b_valid_o.write(false);

        w_ready_o.write(false);
        ar_ready_o.write(false);
        aw_ready_o.write(false);

        if(ar_valid_i.read()) {
            unsigned id = ar_id_i.read();

            auto it = active_r_transactions.find(id);
            if(it == active_r_transactions.end()) {
                payload_type* payload = tlm::tlm_mm<axi::axi_protocol_types>::get().allocate<axi::axi4_extension>();
                auto ext = payload->get_extension<axi::axi4_extension>();
                auto addr = ar_addr_i.read();
                auto length = ar_len_i.read();
                auto size = 1 << ar_size_i.read();
                auto buf_size = size * (length + 1);
                uint8_t* r_data_buf = new uint8_t[buf_size];

                payload->acquire();
                payload->set_address(addr);
                payload->set_data_length(buf_size);
                payload->set_streaming_width(buf_size);
                payload->set_command(tlm::TLM_READ_COMMAND);
                payload->set_data_ptr(r_data_buf);
                ext->set_size(ar_size_i.read());
                ext->set_length(length);
                ext->set_burst(axi::into<axi::burst_e>(ar_burst_i.read().to_uint()));
                ext->set_id(ar_id_i.read());
                ext->set_exclusive(ar_lock_i.read());
                ext->set_cache(ar_cache_i.read());
                ext->set_prot(ar_prot_i.read());
                ext->set_qos(ar_qos_i.read());
                ext->set_region(ar_region_i.read());
                if(ar_user_i.get_interface()) // optional user interface
                    ext->set_user(common::id_type::CTRL, ar_user_i->read());

                ar_ready_o.write(true);
                phase_type phase = tlm::BEGIN_REQ;
                output_socket->nb_transport_fw(*payload, phase, delay);
                SCCTRACE(SCMOD) << phase << " of RD trans (axi_id:" << id << ")";
                register_trans(id, *payload, phase);
            }
        }

        // R channel
        for(auto& it : active_r_transactions) {
            auto read_trans = it.second.front();
            if(read_trans->phase == axi::BEGIN_PARTIAL_RESP ||
               read_trans->phase == tlm::BEGIN_RESP) { // send a single beat
                sc_dt::sc_biguint<BUSWIDTH> read_beat{0};
                payload_type* p = read_trans->payload;
                auto ext = p->get_extension<axi::axi4_extension>();
                sc_assert(ext && "axi4_extension missing");

                read_trans->phase =
                    (read_trans->phase == axi::BEGIN_PARTIAL_RESP) ? axi::END_PARTIAL_RESP : tlm::END_RESP;

                auto beat_size = 1 << ar_size_i.read();
                for(size_t i = 0, j = 0; j < beat_size; i += 8, j++) {
                    auto offset = beat_size * read_trans->beat_cnt + j;
                    read_beat.range(i + 7, i) = *(p->get_data_ptr() + offset);
                }

                auto id = ext->get_id();
                r_id_o.write(id);
                r_resp_o.write(axi::to_int(ext->get_resp()));
                if(r_user_o.get_interface()) // optional user interface
                    r_user_o->write(ext->get_user(common::id_type::DATA));

                r_data_o.write(read_beat);
                r_valid_o.write(true);

                output_socket->nb_transport_fw(*p, read_trans->phase, delay);
                SCCTRACE(SCMOD) << read_trans->phase << " of RD trans (axi_id:" << id
                                << ") beat:" << read_trans->beat_cnt;
                read_trans->beat_cnt++;

                // EDN_RESP indicates the last phase of the AXI Read transaction
                if(read_trans->phase == tlm::END_RESP) {
                    r_last_o.write(true);
                    p->release();
                    auto it = active_r_transactions.find(id);
                    if(it == active_r_transactions.end())
                        SCCFATAL(SCMOD) << "Invalid read transaction ID " << id;
                    auto trans_queue = it->second;
                    trans_queue.pop();
                    if(trans_queue.empty())
                        active_r_transactions.erase(id);
                }
                break;
            }
        }

        if(aw_valid_i.read()) {
            unsigned id = aw_id_i.read();
            auto it = active_w_transactions.find(id);
            if(it == active_w_transactions.end()) {
                payload_type* trans = tlm::tlm_mm<axi::axi_protocol_types>::get().allocate<axi::axi4_extension>();
                register_trans(id, *trans, axi::BEGIN_PARTIAL_REQ);
                it = active_w_transactions.find(id);
            }
            auto write_trans = it->second.front();
            payload_type* p = write_trans->payload;
            if(!write_trans->running) {
                write_trans->running = true;
                aw_ready_o.write(true);
                w_ready_o.write(true);
                auto ext = p->get_extension<axi::axi4_extension>();
                auto length = aw_len_i.read() + 1;
                auto addr = aw_addr_i.read();
                auto num_bytes = 1 << aw_size_i.read();
                auto buf_size = num_bytes * length;
                uint8_t* w_data_buf = new uint8_t[buf_size];

                p->acquire();
                p->set_address(addr);
                p->set_data_length(buf_size);
                p->set_streaming_width(buf_size);
                p->set_command(tlm::TLM_WRITE_COMMAND);
                p->set_data_ptr(w_data_buf);
                ext->set_size(aw_size_i.read());
                ext->set_length(length);
                ext->set_burst(axi::into<axi::burst_e>(aw_burst_i.read().to_uint()));
                ext->set_id(aw_id_i.read());
                ext->set_exclusive(aw_lock_i.read());
                ext->set_cache(aw_cache_i.read());
                ext->set_prot(aw_prot_i.read());
                ext->set_qos(aw_qos_i.read());
                ext->set_region(aw_region_i.read());
                if(aw_user_i.get_interface()) // optional user interface
                    ext->set_user(common::id_type::CTRL, aw_user_i->read());
            }
        }

        if(w_valid_i.read()) {
            unsigned id = aw_id_i.read();
            auto it = active_w_transactions.find(id);
            if(it != active_w_transactions.end()) {
                auto write_trans = it->second.front();
                payload_type* p = write_trans->payload;
                auto ext = p->get_extension<axi::axi4_extension>();
                sc_assert(ext && "axi4_extension missing");
                w_ready_o.write(true);
                write_data = w_data_i.read();
                auto num_bytes = 1 << aw_size_i.read();
                p->set_byte_enable_length(w_strb_i.read());

                if(w_user_i.get_interface()) // optional user interface
                    ext->set_user(common::id_type::DATA, w_user_i->read());

                auto data_ptr = write_trans->payload->get_data_ptr();

                if(w_last_i.read()) {
                    write_trans->phase = tlm::BEGIN_REQ;
                    write_trans->running = false;
                    SCCTRACE(SCMOD) << "Write last beat " << write_trans->beat_cnt;
                }

                for(size_t i = 0, j = num_bytes * write_trans->beat_cnt; j < (num_bytes * (write_trans->beat_cnt + 1));
                    i += 8, j++) {
                    sc_assert(p->get_data_length() > j);
                    *(p->get_data_ptr() + j) = write_data.range(i + 7, i).to_uint64();
                }
                output_socket->nb_transport_fw(*write_trans->payload, write_trans->phase, delay);
                SCCTRACE(SCMOD) << "FW: " << write_trans->phase << " of WR (axi_id:" << id
                                << "). Beat:" << write_trans->beat_cnt;
                write_trans->beat_cnt++;
            }
        }

        // WR RESPONSE channel
        for(auto& it : active_w_transactions) {
            auto write_trans = it.second.front();
            if(write_trans->phase == tlm::BEGIN_RESP) {
                payload_type* p = write_trans->payload;
                auto ext = p->get_extension<axi::axi4_extension>();
                sc_assert(ext && "axi4_extension missing");
                auto id = ext->get_id();

                write_trans->phase = tlm::END_RESP;

                if(b_user_o.get_interface()) // optional user interface
                    b_user_o->write(ext->get_user(common::id_type::RESP));

                b_valid_o.write(true);
                b_id_o.write(id);
                b_resp_o.write(axi::to_int(ext->get_resp()));
                output_socket->nb_transport_fw(*p, write_trans->phase, delay);
                SCCTRACE(SCMOD) << "FW: " << write_trans->phase << " of WR (axi_id:" << id << ")";

                p->release();
                it.second.pop();
                if(it.second.empty())
                    active_w_transactions.erase(id);

                break;
            }
        }
    }
}

template <unsigned int BUSWIDTH, unsigned int ADDRWIDTH, unsigned int IDWIDTH, unsigned int USERWIDTH>
inline tlm::tlm_sync_enum
axi_pin2tlm_adaptor<BUSWIDTH, ADDRWIDTH, IDWIDTH, USERWIDTH>::axi_pin2tlm_adaptor::nb_transport_bw(
    payload_type& trans, phase_type& phase, sc_core::sc_time& t) {
    auto id = axi::get_axi_id(trans);
    SCCTRACE(SCMOD) << "BW: " << phase << " of " << (trans.is_read() ? "RD" : "WR") << " trans (axi_id:" << id << ")";
    axi::axi4_extension* ext;
    trans.get_extension(ext);
    sc_assert(ext && "axi4_extension missing");
    if(trans.is_read()) {
        auto it = active_r_transactions.find(id);
        if(it == active_r_transactions.end())
            SCCERR(SCMOD) << "Invalid read transaction ID " << id;
        auto active_trans = it->second.front();
        active_trans->phase = phase;
    } else { // WRITE transaction
        auto it = active_w_transactions.find(id);
        if(it == active_w_transactions.end())
            SCCERR(SCMOD) << "Invalid write transaction ID " << id;
        auto active_trans = it->second.front();
        active_trans->phase = phase;
    }
    return tlm::TLM_ACCEPTED;
}

template <unsigned int BUSWIDTH, unsigned int ADDRWIDTH, unsigned int IDWIDTH, unsigned int USERWIDTH>
void axi_pin2tlm_adaptor<BUSWIDTH, ADDRWIDTH, IDWIDTH, USERWIDTH>::register_trans(unsigned int axi_id,
                                                                                  payload_type& trans,
                                                                                  phase_type phase) {
    auto th = std::make_shared<trans_handle>();
    th->payload = &trans;
    th->phase = phase;
    if(trans.is_read())
        active_r_transactions[axi_id].push(th);
    else
        active_w_transactions[axi_id].push(th);
}

} // namespace axi
