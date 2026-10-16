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

#ifndef TLM_SCC_LWTR_TLM_REC_TARGET_SOCKET_H_
#define TLM_SCC_LWTR_TLM_REC_TARGET_SOCKET_H_

#include "tlm/scc/lwtr/lwtr4tlm2_extension_registry.h"
#include "tlm2_lwtr.h"
#include <tlm>

//! @brief SystemC TLM
namespace tlm {
//! @brief SCC TLM utilities
namespace scc {
//! @brief SCC SCV4TLM classes and functions
namespace lwtr {
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
    : tlm_rec_target_socket("tlm_rec_target_socket") {}

    explicit tlm_rec_target_socket(const char* name)
    : tlm::tlm_target_socket<BUSWIDTH, TYPES, N
#if !(defined SYSTEMC_VERSION & SYSTEMC_VERSION <= 20050714)
                             ,
                             POL
#endif
                             >(name)
    , fw_export(sc_core::sc_gen_unique_name((std::string(name) + "_fw_export").c_str()))
    , bw_port(sc_core::sc_gen_unique_name((std::string(name) + "_bw_rec_port").c_str()))
    , proxy_port(sc_core::sc_gen_unique_name((std::string(name) + "_fw_rec_port").c_str()))
    , recorder((std::string(name) + "_rec").c_str(), proxy_port, bw_port) {
        fw_export(recorder);
        this->m_port(recorder);
    }

    virtual ~tlm_rec_target_socket() = default; // NOLINT
    //
    // Bind interface to socket
    // - Binds the interface to the export
    //
    virtual void bind(fw_interface_type& ifs) {
        export_type::bind(ifs); // non-virtual function call
        proxy_port(ifs);
    }
    //
    // Bind target socket to initiator socket
    // - Binds the port of the initiator socket to the proxy export of the target
    //   socket
    // - Binds the proxy port of the target socket to the export of the initiator
    //   socket
    //
    virtual void bind(base_initiator_socket_type& s) {
        // initiator.port -> target.export
        (s.get_base_port())(fw_export);
        // target.port -> initiator.export
        bw_port(s.get_base_interface());
    }
    //
    // Bind target socket to target socket (hierarchical bind)
    // - Binds both the export and the port
    //
    virtual void bind(base_type& s) {
        // export
        fw_export(s.get_base_export());
        // port
        (s.get_base_port())(bw_port);
    }

    // Implementation of tlm_base_target_socket_b functions
    virtual sc_core::sc_port_b<bw_interface_type>& get_base_port() { return bw_port; }
    virtual sc_core::sc_port_b<bw_interface_type> const& get_base_port() const { return bw_port; }

    virtual fw_interface_type& get_base_interface() { return fw_export; }
    virtual fw_interface_type const& get_base_interface() const { return fw_export; }

    virtual sc_core::sc_export<fw_interface_type>& get_base_export() { return fw_export; }
    virtual sc_core::sc_export<fw_interface_type> const& get_base_export() const { return fw_export; }

    void setExtensionRecording(lwtr4tlm2_extension_registry_if<TYPES>* extensionRecording) {
        recorder.setExtensionRecording(extensionRecording);
    }

protected:
    export_type fw_export;
    port_type bw_port;
    sc_core::sc_port<fw_interface_type> proxy_port;
    tlm2_lwtr_obj<TYPES> recorder;
};
} // namespace lwtr
} // namespace scc
} // namespace tlm

#endif /* TLM_SCC_LWTR_TLM_REC_TARGET_SOCKET_H_ */
