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

#define SC_INCLUDE_DYNAMIC_PROCESSES
#include "parallel_pe.h"

namespace tlm {
namespace scc {
namespace pe {
using namespace sc_core;

parallel_pe::parallel_pe(sc_core::sc_module_name const& nm)
: sc_module(nm) {
    fw_i.bind(*this);
}

parallel_pe::~parallel_pe() = default;

void parallel_pe::transport(tlm::tlm_generic_payload& payload, bool lt_transport) {
    if(!waiting_ids.size()) {
        auto id = threads.size();
        threads.resize(threads.size() + 1);
        thread_unit& tu = threads.back();
        tu.hndl = sc_core::sc_spawn(
            [this, id]() -> void {
                auto& tu = threads[id];
                while(true) {
                    fw_o->transport(*tu.gp, tu.lt_transport);
                    bw_o->transport(*tu.gp);
                    if(tu.gp->has_mm())
                        tu.gp->release();
                    tu.gp = nullptr;
                    waiting_ids.push_back(id);
                    wait(tu.evt);
                    assert(tu.gp);
                }
            },
            sc_core::sc_gen_unique_name("execute"));
        tu.gp = &payload;
    } else {
        auto& tu = threads[waiting_ids.front()];
        waiting_ids.pop_front();
        tu.gp = &payload;
        tu.lt_transport = lt_transport;
        tu.evt.notify();
    }
    if(payload.has_mm())
        payload.acquire();
}

} /* namespace pe */
} // namespace scc
} /* namespace tlm */
