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

class parallel_pe : public sc_core::sc_module, public intor_fw_nb {
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
    sc_core::sc_export<intor_fw_nb> fw_i{"fw_i"};

    sc_core::sc_port<intor_bw_nb> bw_o{"bw_o"};

    sc_core::sc_port<intor_fw_b> fw_o{"fw_o"};

    parallel_pe(sc_core::sc_module_name const& nm);

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
