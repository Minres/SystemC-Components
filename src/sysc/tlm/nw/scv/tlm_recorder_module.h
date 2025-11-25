/*******************************************************************************
 * Copyright 2016, 2025 MINRES Technologies GmbH
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

#ifndef TLM_NW_SCV_TLM_RECORDER_MODULE_H
#define TLM_NW_SCV_TLM_RECORDER_MODULE_H
#include "tlm/nw/tlm_network_sockets.h"
#include "tlm_recorder.h"
#include <tlm_utils/simple_initiator_socket.h>
#include <tlm_utils/simple_target_socket.h>

//! @brief SystemC TLM
namespace tlm {
//! @brief SCC TLM utilities
namespace nw {
//! @brief SCC SCV4TLM classes and functions
namespace scv {
/*! \brief The TLM2 transaction recorder
 *
 * This module records all TLM transaction to a SCV transaction stream for
 * further viewing and analysis.
 * The handle of the created transaction is storee in an tlm_extension so that
 * another instance of the scv_tlm_recorder
 * e.g. further down the opath can link to it.
 * The transaction recorder is simply bound between an existing pair of
 * initiator and target sockets
 */
template <typename CMDENUM, unsigned int FLIT_WITH, typename TYPES> class tlm_recorder_module : public sc_core::sc_module {
public:
    SC_HAS_PROCESS(tlm_recorder_module); // NOLINT
    //! The target socket of the recorder to be bound to the initiator
    tlm::nw::tlm_network_target_socket<FLIT_WITH, CMDENUM, TYPES> ts{"ts"};
    //! The initiator to be bound to the target socket
    tlm::nw::tlm_network_initiator_socket<FLIT_WITH, CMDENUM, TYPES> is{"is"};
    /*! \brief The constructor of the component
     *
     * \param name is the SystemC module name of the recorder
     * \param recording_enabled if true the recorder is enabled after construction
     * \param tr_db is a pointer to a transaction recording database. If none is
     * provided the default one is retrieved.
     *        If this database is not initialized (e.g. by not calling
     * scv_tr_db::set_default_db() ) recording is disabled.
     */
    tlm_recorder_module(sc_core::sc_module_name name, bool recording_enabled = true,
                        SCVNS scv_tr_db* tr_db = SCVNS scv_tr_db::get_default_db())
    : sc_module(name)
    , recorder(sc_core::sc_object::name(), is.get_base_port(), ts.get_base_port(), recording_enabled, tr_db) {
        // bind the sockets to the module
        is.bind(recorder);
        ts.bind(recorder);
    }

    virtual ~tlm_recorder_module() {}

    tlm_recorder<TYPES> recorder;

private:
    void start_of_simulation() override { recorder.initialize_streams(); }
};
} // namespace scv
} // namespace nw
} // namespace tlm

#endif /* TLM_NW_SCV_TLM_RECORDER_MODULE_H */
