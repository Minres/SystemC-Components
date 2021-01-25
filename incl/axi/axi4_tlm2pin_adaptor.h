
#ifndef AXI_BFM__INITIATOR_H_
#define AXI_BFM__INITIATOR_H_

#include <axi/axi_tlm.h>
#include "scv4tlm/tlm_rec_target_socket.h"
#include <scc/target_mixin.h>

#include <systemc>
#include <tlm>

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

    void b_transport(tlm::tlm_generic_payload& trans, sc_core::sc_time& delay);
    tlm::tlm_sync_enum nb_transport_fw(payload_type& trans, phase_type& phase, sc_core::sc_time& t);

private:
    //! pointer to the associated AXITLM payload
    axi::axi_protocol_types::tlm_payload_type* write_trans{nullptr};
    axi::axi_protocol_types::tlm_payload_type* read_trans{nullptr};

    void reset();
    void bus_thread();

    void write_addr_channel(const tlm::tlm_generic_payload& trans);
    void write_data_channel(const tlm::tlm_generic_payload& trans);
    void write_resp_channel();
    void read_addr_channel(const tlm::tlm_generic_payload& trans);
    void read_data_channel(tlm::tlm_generic_payload& trans);

    enum { OKAY = 0x0, EXOKAY = 0x1, SLVERR = 0x2, DECERR = 0x3 };
};


/////////////////////////////////////////////////////////////////////////////////////////
// Class definition
/////////////////////////////////////////////////////////////////////////////////////////
template <unsigned int BUSWIDTH>
inline axi4_tlm2pin_adaptor<BUSWIDTH>::axi4_tlm2pin_adaptor(sc_core::sc_module_name nm)
: sc_module(nm)
{
    input_socket.register_b_transport([this](tlm::tlm_generic_payload& generic_payload,
                                             sc_core::sc_time& delay) -> void { b_transport(generic_payload, delay); });

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

	SCCTRACE(SCMOD) << phase << " of "<<(trans.is_read()?"RD":"WR")<<" trans " << std::hex << &trans << std::dec <<" (axi_id:"<<axi::get_axi_id(trans)<<")";
    tlm::tlm_sync_enum status{tlm::TLM_ACCEPTED};

    if(phase == axi::BEGIN_PARTIAL_REQ || phase == tlm::BEGIN_REQ) {
    	if(trans.is_read()) {
    		read_trans = &trans;
    	    // The Master puts an address on the Read Address channel as well as asserting ARVALID,indicating the address is valid.
    	    ar_addr_o.write(trans.get_address());
    	    ar_valid_o.write(true);
    	    // The Master asserts RREADY, indicating the master is ready to receive data from the slave.
    	    r_ready_o.write(true);


    	    // The Slave asserts ARREADY, indicating that it is ready to receive the address on the bus.
    	    if(ar_ready_i.read()) {
    	    	phase = (phase==axi::BEGIN_PARTIAL_REQ) ? axi::END_PARTIAL_REQ : tlm::END_REQ;
            	status = tlm::TLM_UPDATED;
    	    }
    	} else {
    		write_trans = &trans;

            aw_addr_o.write(trans.get_address());
            aw_valid_o.write(true);
            if(aw_ready_i.read()) {
    	    	phase = (phase==axi::BEGIN_PARTIAL_REQ) ? axi::END_PARTIAL_REQ : tlm::END_REQ;
            	status = tlm::TLM_UPDATED;
            }
        }
    } else if(phase == axi::END_PARTIAL_RESP || phase == tlm::END_RESP) {
        if(read_trans->has_mm())
            read_trans->release();
    } else if(phase == tlm::END_REQ) { // snoop access resp
    } else if(phase == axi::BEGIN_PARTIAL_RESP || phase == tlm::BEGIN_RESP) {
    }
    return status;
}

template <unsigned int BUSWIDTH>
inline void axi4_tlm2pin_adaptor<BUSWIDTH>::bus_thread() {
	auto delay = sc_core::SC_ZERO_TIME;
	phase_type phase{tlm::END_REQ};

    if(read_trans) {
        if(ar_valid_o.read() && ar_ready_i.read()) {
        	SCCTRACE(SCMOD) << phase << " of trans " << std::hex << read_trans << std::dec <<" (axi_id:"<<axi::get_axi_id(read_trans)<<")";
        	ar_valid_o.write(false);
        	auto ret = input_socket->nb_transport_bw(*read_trans, phase, delay);
        }

        // The Slave puts the requested data on the Read Data channel and asserts RVALID, indicating the data in the channel is valid.
        if(r_ready_o.read() && r_valid_i.read()) {
            auto data = r_data_i.read();
            for(size_t j = 0, k = 0, offset = 0; k < 32 / 8; j += 8, ++k, ++offset)
                *(uint8_t*)(read_trans->get_data_ptr() + offset) = data.range(j + 7, j).to_uint();

            // Since both RREADY and RVALID are asserted, the next rising clock edge completes the transaction.
            r_ready_o.write(false);
            phase = tlm::BEGIN_RESP;
        	auto ret = input_socket->nb_transport_bw(*read_trans, phase, delay);
        	read_trans = nullptr;
        }
    }


    if(write_trans && aw_valid_o.read() && aw_ready_i.read()) {
    	aw_valid_o.write(false);
    	auto ret = input_socket->nb_transport_bw(*read_trans, phase, delay);
    }

}


template <unsigned int BUSWIDTH>
inline void axi4_tlm2pin_adaptor<BUSWIDTH>::b_transport(tlm::tlm_generic_payload& trans, sc_core::sc_time& delay) {
    if(trans.has_mm())
        trans.acquire();

    if(trans.get_command() == tlm::TLM_WRITE_COMMAND) {
        b_ready_o.write(true);
        write_addr_channel(trans);
        write_data_channel(trans);
        write_resp_channel();
    } else {
        read_addr_channel(trans);
        read_data_channel(trans);
    }
    trans.set_response_status(tlm::TLM_OK_RESPONSE);
}

template <unsigned int BUSWIDTH>
inline void axi4_tlm2pin_adaptor<BUSWIDTH>::reset() {
	SCCDEBUG(SCMOD) << "Reset pins";
    aw_valid_o.write(false);
    w_valid_o.write(false);
    b_ready_o.write(true);
    r_ready_o.write(false);
    aw_len_o.write(1); // All AXI4-Lite transactions are of burst length 1
    ar_len_o.write(1); // All AXI4-Lite transactions are of burst length 1
}

template <unsigned int BUSWIDTH>
inline void axi4_tlm2pin_adaptor<BUSWIDTH>::write_addr_channel(const tlm::tlm_generic_payload& trans) {
    aw_addr_o.write(trans.get_address());
    aw_valid_o.write(true);
    if(!aw_ready_i.read()) {
        wait(aw_ready_i.posedge_event());
    } else {
        wait(clk_i.posedge_event());
    }
    aw_valid_o.write(false);
}

template <unsigned int BUSWIDTH>
inline void axi4_tlm2pin_adaptor<BUSWIDTH>::write_data_channel(const tlm::tlm_generic_payload& trans) {
    sc_dt::sc_biguint<512> data{0};
    w_valid_o.write(true);
    for(size_t j = 0, k = 0, offset = 0; k < 32 / 8; j += 8, ++k, ++offset)
        data.range(j + 7, j) = *(uint8_t*)(trans.get_data_ptr() + offset);
    w_data_o.write(data);
    if(!w_ready_i.read()) {
        wait(w_ready_i.posedge_event());
    } else {
        wait(clk_i.posedge_event());
    }
    w_valid_o.write(false);
}

template <unsigned int BUSWIDTH>
inline void axi4_tlm2pin_adaptor<BUSWIDTH>::write_resp_channel() {
    // The slave must wait for AWVALID, AWREADY, WVALID, and WREADY to be asserted before asserting BVALID
    if(!b_valid_i.read())
        wait(b_valid_i.posedge_event());
    sc_assert(b_resp_i.read() == OKAY);
    b_ready_o.write(false);
}

template <unsigned int BUSWIDTH>
inline void axi4_tlm2pin_adaptor<BUSWIDTH>::read_addr_channel(const tlm::tlm_generic_payload& trans) {
    // The Master puts an address on the Read Address channel as well as asserting ARVALID,indicating the address is valid.
    ar_addr_o.write(trans.get_address());
    ar_valid_o.write(true);

    // The Slave asserts ARREADY, indicating that it is ready to receive the address on the bus.
    if(!ar_ready_i.read()) {
        wait(ar_ready_i.posedge_event());
    } else {
        wait(clk_i.posedge_event());
    }
    ar_valid_o.write(false);
}

template <unsigned int BUSWIDTH>
inline void axi4_tlm2pin_adaptor<BUSWIDTH>::read_data_channel(tlm::tlm_generic_payload& trans) {
    sc_dt::sc_biguint<512> data{0};
    // The Master asserts RREADY, indicating the master is ready to receive data from the slave.
    r_ready_o.write(true);

    // The Slave puts the requested data on the Read Data channel and asserts RVALID, indicating the data in the channel is valid.
    if(!r_valid_i.read()) {
        wait(r_valid_i.posedge_event());
    }

    data = r_data_i.read();
    for(size_t j = 0, k = 0, offset = 0; k < 32 / 8; j += 8, ++k, ++offset)
        *(uint8_t*)(trans.get_data_ptr() + offset) = data.range(j + 7, j).to_uint();

    // Since both RREADY and RVALID are asserted, the next rising clock edge completes the transaction.
    wait(clk_i.posedge_event());

    r_ready_o.write(false);
}

} // namespace axi_bfm

#endif /* AXI_BFM__INITIATOR_H_ */
