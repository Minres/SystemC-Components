
#ifndef AXI_BFM__INITIATOR_H_
#define AXI_BFM__INITIATOR_H_

#include <axi/axi_tlm.h>
#include "scv4tlm/tlm_rec_target_socket.h"
#include <scc/target_mixin.h>

#include <systemc>
#include <tlm>


// TODO: export functionality into base class
// TODO: data transfer

namespace axi_bfm {

template <unsigned int BUSWIDTH = 32>
class axi4_tlm2pin_adaptor : public sc_core::sc_module {
public:
    SC_HAS_PROCESS(axi4_tlm2pin_adaptor);

    using payload_type = axi::axi_protocol_types::tlm_payload_type;
    using phase_type = axi::axi_protocol_types::tlm_phase_type;

    axi4_tlm2pin_adaptor(sc_core::sc_module_name nm);

    scc::target_mixin<scv4tlm::tlm_rec_target_socket<BUSWIDTH,axi::axi_protocol_types>,axi::axi_protocol_types> input_socket{"input_socket"};

    sc_core::sc_in<bool> clk_i{"clk_i"};
    sc_core::sc_in<bool> resetn_i{"resetn_i"};

    // Write address channel signals
    sc_core::sc_out<bool> aw_id_o{"aw_id_o"};
    sc_core::sc_out<sc_dt::sc_uint<32>> aw_addr_o{"aw_addr_o"};
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

    // write data channel signals
    sc_core::sc_out<sc_dt::sc_uint<BUSWIDTH>> w_data_o{"w_data_o"};
    sc_core::sc_out<sc_dt::sc_uint<4>> w_strb_o{"w_strb_o"};
    sc_core::sc_out<bool> w_last_o{"w_last_o"};
    sc_core::sc_out<bool> w_valid_o{"w_valid_o"};
    sc_core::sc_in<bool> w_ready_i{"w_ready_i"};

    // write response channel signals
    sc_core::sc_in<bool> b_valid_i{"b_valid_i"};
    sc_core::sc_out<bool> b_ready_o{"b_ready_o"};
    sc_core::sc_in<bool> b_id_i{"b_id_i"};
    sc_core::sc_in<sc_dt::sc_uint<2>> b_resp_i{"b_resp_i"};

    // read address channel signals
    sc_core::sc_out<bool> ar_id_o{"ar_id_o"};
    sc_core::sc_out<sc_dt::sc_uint<32>> ar_addr_o{"ar_addr_o"};
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

    // Read data channel signals
    sc_core::sc_in<bool> r_id_i{"r_id_i"};
    sc_core::sc_in<sc_dt::sc_uint<BUSWIDTH>> r_data_i{"r_data_i"};
    sc_core::sc_in<sc_dt::sc_uint<2>> r_resp_i{"r_resp_i"};
    sc_core::sc_in<bool> r_last_i{"r_last_i"};
    sc_core::sc_in<bool> r_valid_i{"r_valid_i"};
    sc_core::sc_out<bool> r_ready_o{"r_ready_o"};

    tlm::tlm_sync_enum nb_transport_fw(payload_type& trans, phase_type& phase, sc_core::sc_time& t);

private:
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

    trans_handle write_trans;
    trans_handle read_trans;


    void reset();
    void bus_thread();

    enum { OKAY = 0x0, EXOKAY = 0x1, SLVERR = 0x2, DECERR = 0x3 };
};


/////////////////////////////////////////////////////////////////////////////////////////
// Class definition
/////////////////////////////////////////////////////////////////////////////////////////
template <unsigned int BUSWIDTH>
inline axi4_tlm2pin_adaptor<BUSWIDTH>::axi4_tlm2pin_adaptor(sc_core::sc_module_name nm)
: sc_module(nm)
{
    input_socket.register_nb_transport_fw([this](payload_type& trans, phase_type& phase, sc_core::sc_time& t)
    		-> tlm::tlm_sync_enum { return nb_transport_fw(trans, phase, t); });

    SC_METHOD(reset);
    sensitive << resetn_i.neg();

    SC_METHOD(bus_thread)
    sensitive << clk_i.pos();
}

template <unsigned int BUSWIDTH>
inline tlm::tlm_sync_enum axi4_tlm2pin_adaptor<BUSWIDTH>::nb_transport_fw(payload_type& trans, phase_type& phase, sc_core::sc_time& t) {
    if(trans.has_mm())
        trans.acquire();

	SCCTRACE(SCMOD) << phase << " of "<<(trans.is_read()?"RD":"WR")<<" forward trans " << std::hex << &trans << std::dec <<" (axi_id:"<<axi::get_axi_id(trans)<<")";
    tlm::tlm_sync_enum status{tlm::TLM_ACCEPTED};

    if(phase == axi::BEGIN_PARTIAL_REQ || phase == tlm::BEGIN_REQ) {
    	if(trans.is_read()) {
    		read_trans.payload = &trans;
        	read_trans.phase = phase;
    	    // The Master puts an address on the Read Address channel as well as asserting ARVALID,indicating the address is valid.
    	    ar_addr_o.write(trans.get_address());
    	    ar_valid_o.write(true);
    	    ar_len_o.write(read_trans.payload->get_data_length());
    	    // The Master asserts RREADY, indicating the master is ready to receive data from the slave.
    	    r_ready_o.write(true);


    	    // The Slave asserts ARREADY, indicating that it is ready to receive the address on the bus.
    	    if(ar_ready_i.read()) {
    	    	phase = (phase==axi::BEGIN_PARTIAL_REQ) ? axi::END_PARTIAL_REQ : tlm::END_REQ;
    	    	read_trans.phase = phase;
            	status = tlm::TLM_UPDATED;
    	    }
    	} else {
            // write address
    	    auto ext = trans.get_extension<axi::axi4_extension>();
    	    sc_assert(ext && "axi4_extension missing");

    	    aw_id_o.write(ext->get_id());
    	    aw_addr_o.write(trans.get_address());
    	    aw_len_o.write(ext->get_length());
    	    aw_size_o.write(ext->get_size());
    	    aw_burst_o.write(static_cast<int>(ext->get_burst()));
    	    aw_lock_o.write(ext->is_exclusive());
    	    aw_cache_o.write(ext->get_cache());
    	    aw_prot_o.write(ext->get_prot());
    	    aw_qos_o.write(ext->get_qos());
    	    aw_region_o.write(ext->get_region());
            aw_valid_o.write(true);

            // write data
            sc_dt::sc_biguint<BUSWIDTH> data{0};
            for(size_t j = 0, k = 0, offset = 0; k < 32 / 8; j += 8, ++k, ++offset)
                data.range(j + 7, j) = *(uint8_t*)(trans.get_data_ptr() + offset);

            w_data_o.write(data);
            w_valid_o.write(true);

            if(w_ready_i.read() && phase==axi::BEGIN_PARTIAL_REQ) {
            	phase = axi::END_PARTIAL_REQ;
            	status = tlm::TLM_UPDATED;
                aw_valid_o.write(false);
                w_valid_o.write(false);
            }

    		write_trans.payload = &trans;
        	write_trans.phase = phase;
        }
    } else if(phase == axi::END_PARTIAL_RESP || phase == tlm::END_RESP) {
    	trans.set_response_status(tlm::TLM_OK_RESPONSE);
        if(trans.has_mm())
            trans.release();
        write_trans.reset();
        status = tlm::TLM_ACCEPTED;

    } else if(phase == tlm::END_REQ) { // snoop access resp
    } else if(phase == axi::BEGIN_PARTIAL_RESP || phase == tlm::BEGIN_RESP) {
    }
    return status;
}

template <unsigned int BUSWIDTH>
inline void axi4_tlm2pin_adaptor<BUSWIDTH>::bus_thread() {
	auto delay = sc_core::SC_ZERO_TIME;

    if(read_trans.payload) {
        if(read_trans.phase == tlm::BEGIN_REQ && ar_ready_i.read()) {
        	read_trans.phase = tlm::END_REQ;
        	ar_valid_o.write(false);
        	SCCTRACE(SCMOD) << read_trans.phase << " of backward trans " << std::hex << read_trans.payload << std::dec <<" (axi_id:"<<axi::get_axi_id(read_trans.payload)<<")";
        	auto ret = input_socket->nb_transport_bw(*read_trans.payload, read_trans.phase, delay);
        }
        // The Slave puts the requested data on the Read Data channel and asserts RVALID, indicating the data in the channel is valid.
        else if(read_trans.phase == tlm::END_REQ && r_valid_i.read()) {
        	SCCTRACE(SCMOD) << read_trans.phase << " of backward trans " << std::hex << read_trans.payload << std::dec <<" (axi_id:"<<axi::get_axi_id(read_trans.payload)<<")";
        	ar_valid_o.write(false);
            auto data = r_data_i.read();
            for(size_t j = 0, k = 0, offset = 0; k < 32 / 8; j += 8, ++k, ++offset)
                *(uint8_t*)(read_trans.payload->get_data_ptr() + offset) = data.range(j + 7, j).to_uint();

            // Since both RREADY and RVALID are asserted, the next rising clock edge completes the transaction.
            r_ready_o.write(false);
            read_trans.phase = tlm::BEGIN_RESP;
        	auto ret = input_socket->nb_transport_bw(*read_trans.payload, read_trans.phase, delay);
        	read_trans.payload = nullptr;

        	// EDN_RESP indicates the last phase of the AXI Read transaction
        	if(read_trans.phase == tlm::END_RESP && read_trans.payload->has_mm())
                read_trans.payload->release();
        }
    }


    if(write_trans.payload) {
    	if((write_trans.phase == axi::BEGIN_PARTIAL_REQ || write_trans.phase == tlm::BEGIN_REQ) && w_ready_i.read()) {
    		write_trans.phase = (write_trans.phase==axi::BEGIN_PARTIAL_REQ) ? axi::END_PARTIAL_REQ : tlm::END_REQ;
            if(write_trans.phase == tlm::END_REQ)
            	w_last_o.write(true);
        	auto ret = input_socket->nb_transport_bw(*write_trans.payload, write_trans.phase, delay);
        	SCCTRACE(SCMOD) << write_trans.phase << " of backward trans " << std::hex << write_trans.payload << std::dec <<" (axi_id:"<<axi::get_axi_id(write_trans.payload)<<")";
    	}
    	else if((write_trans.phase == tlm::END_REQ) && b_valid_i.read()) {
    	    sc_assert(b_resp_i.read() == OKAY);
            aw_valid_o.write(false);
            w_valid_o.write(false);
    	    b_ready_o.write(false);
            write_trans.phase = tlm::BEGIN_RESP;
        	auto ret = input_socket->nb_transport_bw(*write_trans.payload, write_trans.phase, delay);
        	SCCTRACE(SCMOD) << write_trans.phase << " of backward trans " << std::hex << write_trans.payload << std::dec <<" (axi_id:"<<axi::get_axi_id(write_trans.payload)<<")";
        	if (ret == tlm::TLM_UPDATED) {
        		write_trans.reset();
        	}
    	}
    }

}

template <unsigned int BUSWIDTH>
inline void axi4_tlm2pin_adaptor<BUSWIDTH>::reset() {
	SCCTRACE(SCMOD) << "Reset pins";
    aw_valid_o.write(false);
    w_valid_o.write(false);
    b_ready_o.write(true);
    r_ready_o.write(false);
    aw_len_o.write(1); // All AXI4-Lite transactions are of burst length 1
    ar_len_o.write(1); // All AXI4-Lite transactions are of burst length 1
}

} // namespace axi_bfm

#endif /* AXI_BFM__INITIATOR_H_ */
