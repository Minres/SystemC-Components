/*******************************************************************************
 * Copyright 2020-2022 MINRES Technologies GmbH
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

#ifndef _TLM_SCC_PE_PARALLEL_PE_H_
#define _TLM_SCC_PE_PARALLEL_PE_H_

#include "intor_if.h"
#include <deque>
#include <tlm>
//! @brief SystemC TLM
namespace tlm {
//! @brief SCC TLM utilities
namespace scc {
//! @brief SCC protocol engines
namespace pe {
/*!
 * Parallel Protocol Engine
 *
 * This module implements a parallel protocol engine that can handle multiple
 * transactions in parallel.
 *
 * @note This implementation assumes that the provided interfaces are compatible
 */
class parallel_pe : public sc_core::sc_module, public intor_fw_nb {
    template <class IF> using sc_port_opt = sc_core::sc_port<IF, 1, sc_core::SC_ZERO_OR_MORE_BOUND>;

    struct thread_unit {
        sc_core::sc_event evt;
        tlm::tlm_generic_payload* gp{nullptr};
        bool lt_transport{false};
        sc_core::sc_process_handle hndl{};
        thread_unit(const thread_unit& o)
        : gp(o.gp)
        , lt_transport(o.lt_transport)
        , hndl(o.hndl) {}
        thread_unit() = default;
    };

public:
    /*!
     * upstream forward interface
     *
     * This interface is used to connect the protocol engine to the upstream forward port
     */
    sc_core::sc_export<intor_fw_nb> fw_i{"fw_i"};
    /*!
     * upstream backward interface
     *
     * This interface is used to connect the protocol engine to the upstream backward export
     */
    sc_port_opt<intor_bw_nb> bw_o{"bw_o"};
    /*!
     * downstream forward interface
     *
     * This interface is used to connect the protocol engine to the downstream forward export
     */
    sc_core::sc_port<intor_fw_b> fw_o{"fw_o"};
    /*!
     * Constructor
     *
     * @param nm Name of the module
     *
     * @note This constructor is private and should only be called from the
     * sc_module constructor.
     */
    parallel_pe(sc_core::sc_module_name const& nm);
    /*!
     * virtual destructor
     */
    virtual ~parallel_pe();

private:
    void transport(tlm::tlm_generic_payload& payload, bool lt_transport = false) override;

    void snoop_resp(tlm::tlm_generic_payload& payload, bool sync) override { fw_o->snoop_resp(payload, sync); }

    std::deque<unsigned> waiting_ids;
    std::vector<thread_unit> threads;
};

} /* namespace pe */
} // namespace scc
} /* namespace tlm */

#endif /* _TLM_SCC_PE_PARALLEL_PE_H_ */
