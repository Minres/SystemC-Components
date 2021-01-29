#ifndef AXI_PIN2TLM_ADAPTOR_H_
#define AXI_PIN2TLM_ADAPTOR_H_

#include <axi/axi_tlm.h>
#include <scv4tlm/tlm_rec_initiator_socket.h>
#include <scc/initiator_mixin.h>
#include <tlm/tlm_mm.h>
#include <scc/report.h>
#include <tlm/tlm_id.h>

#include <systemc>
#include <tlm>

namespace axi_bfm {

template <unsigned int BUSWIDTH = 32>
class axi_pin2tlm_adaptor : public sc_core::sc_module {
public:
    using payload_type = axi::axi_protocol_types::tlm_payload_type;
    using phase_type = axi::axi_protocol_types::tlm_phase_type;

    SC_HAS_PROCESS(axi_pin2tlm_adaptor);

    axi_pin2tlm_adaptor(sc_core::sc_module_name nm);

    scc::initiator_mixin<scv4tlm::tlm_rec_initiator_socket<BUSWIDTH,axi::axi_protocol_types>,axi::axi_protocol_types> output_socket{"output_socket"};

    sc_core::sc_in<bool> clk_i{"clk_i"};
    sc_core::sc_in<bool> resetn_i{"resetn_i"}; // active low reset

    // Write address channel signals
    sc_core::sc_in<sc_dt::sc_uint<32>> aw_id_i{"aw_id_i"};
    sc_core::sc_in<sc_dt::sc_uint<32>> aw_addr_i{"aw_addr_i"};
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

    // write data channel signals
    sc_core::sc_in<sc_dt::sc_uint<BUSWIDTH>> w_data_i{"w_data_i"};
    sc_core::sc_in<sc_dt::sc_uint<4>> w_strb_i{"w_strb_i"};
    sc_core::sc_in<bool> w_last_i{"w_last_i"};
    sc_core::sc_in<bool> w_valid_i{"w_valid_i"};
    sc_core::sc_out<bool> w_ready_o{"w_ready_o"};

    // write response channel signals
    sc_core::sc_out<bool> b_valid_o{"b_valid_o"};
    sc_core::sc_in<bool> b_ready_i{"b_ready_i"};
    sc_core::sc_out<sc_dt::sc_uint<32>> b_id_o{"b_id_o"};
    sc_core::sc_out<sc_dt::sc_uint<2>> b_resp_o{"b_resp_o"};

    // read address channel signals
    sc_core::sc_in<sc_dt::sc_uint<32>> ar_id_i{"ar_id_i"};
    sc_core::sc_in<sc_dt::sc_uint<32>> ar_addr_i{"ar_addr_i"};
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

    // Read data channel signals
    sc_core::sc_out<sc_dt::sc_uint<32>> r_id_o{"r_id_o"};
    sc_core::sc_out<sc_dt::sc_uint<BUSWIDTH>> r_data_o{"r_data_o"};
    sc_core::sc_out<sc_dt::sc_uint<2>> r_resp_o{"r_resp_o"};
    sc_core::sc_out<bool> r_last_o{"r_last_o"};
    sc_core::sc_out<bool> r_valid_o{"r_valid_o"};
    sc_core::sc_in<bool> r_ready_i{"r_ready_i"};

    tlm::tlm_sync_enum nb_transport_bw(payload_type& trans, phase_type& phase, sc_core::sc_time& t);

private:
    void reset();
    void bus_thread();

    /**
     * a handle class holding the pointer to the current transaction and associated phase
     */
    struct trans_handle {
        //! pointer to the associated AXITLM payload
    	payload_type* payload = nullptr;
    	//! current protocol phase
    	phase_type    phase = tlm::UNINITIALIZED_PHASE;

    	void reset() {
    		payload = nullptr;
    		phase   = tlm::UNINITIALIZED_PHASE;
    	}
    };
    // 2 trans_handles because write and read can be done simultaneously
    trans_handle write_trans;
    trans_handle read_trans;
    enum { OKAY = 0x0, EXOKAY = 0x1, SLVERR = 0x2, DECERR = 0x3 };
};


/////////////////////////////////////////////////////////////////////////////////////////
// Class definition
/////////////////////////////////////////////////////////////////////////////////////////
template <unsigned int BUSWIDTH>
inline axi_pin2tlm_adaptor<BUSWIDTH>::axi_pin2tlm_adaptor::axi_pin2tlm_adaptor(sc_core::sc_module_name nm)
: sc_module(nm) {
    output_socket.register_nb_transport_bw([this](payload_type& trans, phase_type& phase, sc_core::sc_time& t)
    		-> tlm::tlm_sync_enum { return nb_transport_bw(trans, phase, t); });

    SC_METHOD(reset);
    sensitive << resetn_i.neg();

    SC_THREAD(bus_thread)
    sensitive << clk_i.pos();
}


template <unsigned int BUSWIDTH>
inline void axi_pin2tlm_adaptor<BUSWIDTH>::axi_pin2tlm_adaptor::bus_thread() {
    constexpr auto buswidth_in_bytes = BUSWIDTH / 8;

    sc_core::sc_time delay = sc_core::SC_ZERO_TIME;
    sc_dt::sc_biguint<BUSWIDTH> read_beat{0};
    sc_dt::sc_biguint<BUSWIDTH> write_data{0};
    uint64_t output_data[8]{0};
    while(true) {
        wait();

		if(r_ready_i.read()) {
			r_valid_o.write(false);
			r_last_o.write(false);
		}
		ar_ready_o.write(false);

		if (ar_valid_i.read() && read_trans.phase == tlm::UNINITIALIZED_PHASE) { // IDLE state
		    auto trans = tlm::tlm_mm<axi::axi_protocol_types>::get().allocate<axi::axi4_extension>();
			auto addr = ar_addr_i.read();
		    auto ext = trans->get_extension<axi::axi4_extension>();
			auto length   = ar_len_i.read();
			auto size     = 1 << ar_size_i.read();
			auto buf_size = size * (length+1);
			uint8_t* r_data_buf = new uint8_t[buf_size];

		    read_trans.phase  = tlm::BEGIN_REQ;
			read_trans.payload= trans;

		    trans->acquire();
			trans->set_address(addr);
		    trans->set_data_length(buf_size);
		    trans->set_streaming_width(buf_size);
			trans->set_command(tlm::TLM_READ_COMMAND);
			trans->set_data_ptr(r_data_buf);
		    ext->set_size(ar_size_i.read());
		    ext->set_length(length);
		    ext->set_burst(axi::burst_e::INCR);
		    ext->set_id(ar_id_i.read());
			output_socket->nb_transport_fw(*read_trans.payload, read_trans.phase, delay);
			ar_ready_o.write(true);
		}

		if (read_trans.payload && (read_trans.phase == axi::BEGIN_PARTIAL_RESP || read_trans.phase == tlm::BEGIN_RESP)) { // send a single beat
			axi::axi4_extension* ext;
			read_trans.payload->get_extension(ext);
			sc_assert(ext && "axi4_extension missing");

			read_trans.phase = (read_trans.phase == axi::BEGIN_PARTIAL_RESP) ? axi::END_PARTIAL_RESP : tlm::END_RESP;

			for(size_t i = 0, j = 0; j < ext->get_size(); i += 8, j++) {
				read_beat.range(i + 7, i) = *(uint8_t*)(read_trans.payload->get_data_ptr() + j);
			}

			r_id_o.write(ext->get_id());
			r_data_o.write(read_beat);
			r_valid_o.write(true);

			output_socket->nb_transport_fw(*read_trans.payload, read_trans.phase, delay);
			read_trans.payload->set_address(read_trans.payload->get_address() + buswidth_in_bytes);

			// EDN_RESP indicates the last phase of the AXI Read transaction
			if(read_trans.phase == tlm::END_RESP) {
				r_last_o.write(true);
				read_trans.payload->release();
				read_trans.reset();
			}
		}

		// Handle write request
		if(aw_valid_i.read() && w_valid_i.read()) {
			aw_ready_o.write(true);
			b_valid_o.write(false);
			w_ready_o.write(true);
			wait();

			auto length = aw_len_i.read();
			auto addr = aw_addr_i.read();
			auto num_bytes = 1 << aw_size_i.read();

//			payload.set_data_length(num_bytes);
//			payload.set_streaming_width(num_bytes);
//			payload.set_command(tlm::TLM_WRITE_COMMAND);
//			payload.set_address(addr);

			for (int l = 0; l <= length; l++) {
				w_ready_o.write(false);
				write_data = w_data_i.read();
				for (size_t i=0, j = 0; i < 8; j += num_bytes, i++) {
					output_data[i] = write_data.range(j + num_bytes - 1, j).to_uint64();
//                    	SCCDEBUG(SCMOD) << "Addr: 0x" << std::hex << payload.get_address() << " Value: 0x" << output_data[i];
				}
//				payload.set_data_ptr((unsigned char *)&output_data);
//
//				output_socket->b_transport(payload, delay);
//				addr += num_bytes;
//				payload.set_address(addr);

				w_ready_o.write(true);
				if (w_last_i.read()) {
					aw_ready_o.write(false);
					w_ready_o.write(false);
					b_resp_o.write(OKAY);
					b_valid_o.write(true);
				}
				wait();
			}
			b_valid_o.write(false);
		}
    }
}

template <unsigned int BUSWIDTH>
inline tlm::tlm_sync_enum axi_pin2tlm_adaptor<BUSWIDTH>::axi_pin2tlm_adaptor::nb_transport_bw(payload_type& trans, phase_type& phase, sc_core::sc_time& t) {
	SCCTRACE(SCMOD) << phase << " of "<<(trans.is_read()?"RD":"WR")<<" trans (axi_id:"<<axi::get_axi_id(trans)<<")";

	if(trans.is_read()){
		read_trans.phase = phase;
	} else {
		write_trans.phase= phase;
	}

	return tlm::TLM_ACCEPTED;
}

template <unsigned int BUSWIDTH>
inline void axi_pin2tlm_adaptor<BUSWIDTH>::axi_pin2tlm_adaptor::reset() {
	SCCTRACE(SCMOD) << "Reset adapter";
    r_valid_o.write(false);
    r_last_o.write(false);
    b_valid_o.write(false);
    ar_ready_o.write(true);
}

} // namespace axi_bfm

#endif /* AXI_PIN2TLM_ADAPTOR_H_ */
