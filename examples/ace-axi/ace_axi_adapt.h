/*
 * ace_axi_adapt.h
 *
 *  Created on: Jul 16, 2020
 *      Author: eyckj
 */

#ifndef _ACE_INTOR_ACE_AXI_ADAPT_H_
#define _ACE_INTOR_ACE_AXI_ADAPT_H_

#include <axi/axi_tlm.h>

template<unsigned BUSWIDTH=32>
class ace_axi_adapt: public sc_core::sc_module, public axi::ace_fw_transport_if<axi::axi_protocol_types>, public axi::axi_bw_transport_if<axi::axi_protocol_types> {
public:
	using transaction_type = axi::axi_protocol_types::tlm_payload_type;
	using phase_type = axi::axi_protocol_types::tlm_phase_type;

	axi::ace_target_socket<BUSWIDTH> tsckt{"tsckt"};
	axi::axi_initiator_socket<BUSWIDTH> isckt{"isckt"};

	ace_axi_adapt(sc_core::sc_module_name const& nm): sc_core::sc_module(nm){
		tsckt.bind(*this);
		isckt.bind(*this);
	}

	void b_transport(transaction_type& trans, sc_core::sc_time& t) override {}

	bool get_direct_mem_ptr(transaction_type& trans, tlm::tlm_dmi& dmi_data) override { return false; }

	unsigned int transport_dbg(transaction_type& trans) override { return 0; }

	tlm::tlm_sync_enum nb_transport_fw(transaction_type& trans,phase_type& phase, sc_core::sc_time& t) override {
		return isckt->nb_transport_fw(trans, phase, t);
	}

	tlm::tlm_sync_enum nb_transport_bw(transaction_type& trans,phase_type& phase, sc_core::sc_time& t) override {
		return tsckt->nb_transport_bw(trans, phase, t);
	}

	void invalidate_direct_mem_ptr(sc_dt::uint64 start_range, sc_dt::uint64 end_range) override {}
};


#endif /* _ACE_INTOR_ACE_AXI_ADAPT_H_ */
