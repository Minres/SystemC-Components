/*******************************************************************************
 * Copyright 2016, 2017 MINRES Technologies GmbH
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

#include <tlm/scc/scv/tlm_recorder.h>
#include <tlm>

//! @brief SystemC TLM
namespace tlm {
//! @brief SCC TLM utilities
namespace scc {
//! @brief SCC SCV4TLM classes and functions
namespace scv {
template <unsigned int BUSWIDTH = 32, typename TYPES = tlm::tlm_base_protocol_types, int N = 1
#if !(defined SYSTEMC_VERSION & SYSTEMC_VERSION <= 20050714)
          ,
          sc_core::sc_port_policy POL = sc_core::SC_ONE_OR_MORE_BOUND
#endif
          >
class tlm_rec_target_socket : public tlm::tlm_target_socket<BUSWIDTH, TYPES, N
#if !(defined SYSTEMC_VERSION & SYSTEMC_VERSION <= 20050714)
                                                            ,
                                                            POL
#endif
                                                            > {
    static std::string gen_name(const char* first, const char* second) {
        std::stringstream ss;
        ss << first << "_" << second;
        return ss.str();
    }

public:
    using fw_interface_type = tlm::tlm_fw_transport_if<TYPES>;
    using bw_interface_type = tlm::tlm_bw_transport_if<TYPES>;
    using port_type = sc_core::sc_port<bw_interface_type, N
#if !(defined SYSTEMC_VERSION & SYSTEMC_VERSION <= 20050714)
                                       ,
                                       POL
#endif
                                       >;

    using export_type = sc_core::sc_export<fw_interface_type>;
    using base_initiator_socket_type = tlm::tlm_base_initiator_socket_b<BUSWIDTH, fw_interface_type, bw_interface_type>;
    using base_type = tlm::tlm_base_target_socket_b<BUSWIDTH, fw_interface_type, bw_interface_type>;

    tlm_rec_target_socket()
    : tlm::tlm_target_socket<BUSWIDTH, TYPES, N
#if !(defined SYSTEMC_VERSION & SYSTEMC_VERSION <= 20050714)
                             ,
                             POL
#endif
                             >()
    , recorder(this->name(), fw_port, this->get_base_port()) {
    }

    explicit tlm_rec_target_socket(const char* name)
    : tlm::tlm_target_socket<BUSWIDTH, TYPES, N
#if !(defined SYSTEMC_VERSION & SYSTEMC_VERSION <= 20050714)
                             ,
                             POL
#endif
                             >(name)
    , recorder(this->name(), fw_port, this->get_base_port()) {
    }

    virtual ~tlm_rec_target_socket() = default; // NOLINT

    virtual const char* kind() const { return "tlm_rec_target_socket"; }
    //
    // Bind target socket to target socket (hierarchical bind)
    // - Binds both the export and the port
    //
    virtual void bind(base_type& s) {
        // export
        (this->get_base_export())(s.get_base_export()); // will be handled by bind(fw_interface_type& ifs)
        // port
        (s.get_base_port())(recorder); // bind the recording interface to the port,
                                       // recording will use the m_port
    }
    //
    // Bind interface to socket
    // - Binds the interface to the export
    //
    virtual void bind(fw_interface_type& ifs) {
        export_type* exp = &this->get_base_export();
        if(this == exp) {
            export_type::bind(recorder); // non-virtual function call
            fw_port(ifs);
        } else {
            exp->bind(ifs);
        }
    }
    //
    // Forward to 'operator->()' of port class
    //
    bw_interface_type* operator->() { return &recorder; }

    void setExtensionRecording(tlm_extensions_recording_if<TYPES>* extensionRecording) {
        recorder.setExtensionRecording(extensionRecording);
    }

protected:
    sc_core::sc_port<fw_interface_type> fw_port{sc_core::sc_gen_unique_name("$$$__rec_fw__$$$")};
    scv::tlm_recorder<TYPES> recorder;
};
} // namespace scv
} // namespace scc
} // namespace tlm

#endif /* TLM_REC_TARGET_SOCKET_H_ */
