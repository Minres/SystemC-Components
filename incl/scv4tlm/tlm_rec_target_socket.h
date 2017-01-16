/*******************************************************************************
 * Copyright 2016 MINRES Technologies GmbH
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

#ifndef TLM_REC_TARGET_SOCKET_H_
#define TLM_REC_TARGET_SOCKET_H_

#ifdef WITH_SCV
#include "tlm2_recorder.h"
#endif
#include <tlm_core/tlm_2/tlm_sockets/tlm_target_socket.h>

namespace scv4tlm {
template<unsigned int BUSWIDTH = 32, typename TYPES = tlm::tlm_base_protocol_types, int N = 1
#if !(defined SYSTEMC_VERSION & SYSTEMC_VERSION <= 20050714)
		, sc_core::sc_port_policy POL = sc_core::SC_ONE_OR_MORE_BOUND
#endif
		>
#ifndef WITH_SCV
using tlm_rec_target_socket  = tlm::tlm_target_socket<BUSWIDTH,TYPES,N,POL>;
#else
class tlm_rec_target_socket: public tlm::tlm_target_socket<BUSWIDTH
, TYPES
, N
#if !(defined SYSTEMC_VERSION & SYSTEMC_VERSION <= 20050714)
, POL
#endif
> {
	static std::string gen_name(const char* first, const char* second){
		std::stringstream ss;
		ss<<first<<"_"<<second;
		return ss.str();
	}
public:
	typedef tlm::tlm_fw_transport_if<TYPES>  fw_interface_type;
	typedef tlm::tlm_bw_transport_if<TYPES>  bw_interface_type;
	typedef sc_core::sc_port<bw_interface_type, N
#if !(defined SYSTEMC_VERSION & SYSTEMC_VERSION <= 20050714)
			, POL
#endif
			>   port_type;

	typedef sc_core::sc_export<fw_interface_type> export_type;
	typedef tlm::tlm_base_initiator_socket_b<BUSWIDTH, fw_interface_type, bw_interface_type>  base_initiator_socket_type;

	typedef tlm::tlm_base_target_socket_b<BUSWIDTH,	fw_interface_type, bw_interface_type> base_type;

	tlm_rec_target_socket() :
		tlm::tlm_target_socket<BUSWIDTH
		, TYPES
		, N
#if !(defined SYSTEMC_VERSION & SYSTEMC_VERSION <= 20050714)
		, POL
#endif
		>()
		,recorder()
	{
	}

	explicit tlm_rec_target_socket(const char* name) :
		tlm::tlm_target_socket<BUSWIDTH
		, TYPES
		, N
#if !(defined SYSTEMC_VERSION & SYSTEMC_VERSION <= 20050714)
		, POL
#endif
		>(name)
		,recorder(gen_name(name, "rec").c_str())
	{
	}

	virtual ~tlm_rec_target_socket(){}

	virtual const char* kind() const {
		return "tlm_rec_target_socket";
	}
	//
	// Bind target socket to target socket (hierarchical bind)
	// - Binds both the export and the port
	//
	virtual void bind(base_type& s) {
		// export
		(this->get_base_export())(s.get_base_export()); // will be handled by bind(fw_interface_type& ifs)
		// port
		(s.get_base_port())(recorder); // bind the recording interface to the port, recording will use the m_port
	}
	//
	// Bind interface to socket
	// - Binds the interface to the export
	//
	virtual void bind(fw_interface_type& ifs){
		export_type* exp = &this->get_base_export();
		if( this == exp ) {
			export_type::bind(recorder); // non-virtual function call
			recorder.fw_port(ifs);
			recorder.bw_port(this->get_base_port());
		} else {
			exp->bind( ifs );
		}

	}
	//
	// Forward to 'operator->()' of port class
	//
	bw_interface_type* operator->() {
		return &recorder;
	}

	scv4tlm::tlm2_recorder<TYPES>& get_recorder(){
		return recorder;
	}

protected:
	scv4tlm::tlm2_recorder<TYPES> recorder;
};
#endif
}
// namespace scv4tlm

#endif /* TLM_REC_TARGET_SOCKET_H_ */
