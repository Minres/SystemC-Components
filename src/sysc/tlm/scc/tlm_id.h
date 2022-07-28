/*******************************************************************************
 * Copyright 2021 - 2022 MINRES Technologies GmbH
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

#pragma once

#include <cstdint>
#include "tlm_gp_shared.h"

//! @brief SystemC TLM
namespace tlm {
//! @brief SCC TLM utilities
namespace scc {

struct tlm_id_extension : public tlm_extension<tlm_id_extension> {
    virtual tlm_extension_base* clone() const {
        tlm_id_extension* t = new tlm_id_extension(this->id);
        return t;
    }
    virtual void copy_from(tlm_extension_base const& from) { id = static_cast<tlm_id_extension const&>(from).id; }
    tlm_id_extension(tlm_gp_shared_ptr& i)
    : tlm_id_extension(reinterpret_cast<uintptr_t>(i.get())) {}
    tlm_id_extension(void* i)
    : tlm_id_extension(reinterpret_cast<uintptr_t>(i)) {}
    tlm_id_extension(uintptr_t i)
    : id(i) {}
    uintptr_t id;
};

inline uintptr_t getId(tlm::tlm_generic_payload& gp) {
    if(auto ext = gp.get_extension<tlm_id_extension>())
        return ext->id;
    else
        return (uintptr_t)&gp;
}

inline uintptr_t getId(tlm::tlm_generic_payload* gp) {
    if(!gp)
        return 0;
    if(auto ext = gp->get_extension<tlm_id_extension>())
        return ext->id;
    else
        return (uintptr_t)gp;
}

inline void setId(tlm::tlm_generic_payload& gp, uintptr_t id) {
    if(auto ext = gp.get_extension<tlm_id_extension>())
        ext->id = id;
    else if(gp.has_mm())
        gp.set_auto_extension(new tlm_id_extension(id));
    else
        gp.set_extension(new tlm_id_extension(id));
}
} // namespace scc
} // namespace tlm
