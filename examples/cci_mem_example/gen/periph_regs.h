/*******************************************************************************
 * Copyright 2023 MINRES Technologies GmbH
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

#ifndef __PERIPH_REGS_H_
#define __PERIPH_REGS_H_

#include <util/bit_field.h>
#include <scc/register.h>
#include <scc/tlm_target.h>
#include <scc/utilities.h>

namespace sysc {

class periph_regs : public sc_core::sc_module, public scc::resetable {
protected:
    // storage declarations
    BEGIN_BF_DECL(txdata_t, uint32_t);
    BF_FIELD(data, 0, 8);
    BF_FIELD(full, 31, 1);
    END_BF_DECL() r_txdata;

    BEGIN_BF_DECL(rxdata_t, uint32_t);
    BF_FIELD(data, 0, 8);
    BF_FIELD(empty, 31, 1);
    END_BF_DECL() r_rxdata;

    BEGIN_BF_DECL(priority_t, uint32_t);
    BF_FIELD(priority, 0, 3);
    END_BF_DECL();
    std::array<priority_t, 16> r_priority;

    // register declarations
    scc::sc_register<txdata_t> txdata;
    scc::sc_register<rxdata_t> rxdata;
    scc::sc_register_indexed<priority_t, 16> priority;

public:
    periph_regs(sc_core::sc_module_name nm);

    template <unsigned BUSWIDTH = 32> void registerResources(scc::tlm_target<BUSWIDTH> &target);
};
}
//////////////////////////////////////////////////////////////////////////////
// member functions
//////////////////////////////////////////////////////////////////////////////

inline sysc::periph_regs::periph_regs(sc_core::sc_module_name nm)
: sc_core::sc_module(nm)
, NAMED(txdata, r_txdata, 0, *this)
, NAMED(rxdata, r_rxdata, 0, *this)
, NAMED(priority, r_priority, 0, *this)
{}

template <unsigned BUSWIDTH> inline void sysc::periph_regs::registerResources(scc::tlm_target<BUSWIDTH> &target) {
    target.addResource(txdata, 0x0UL);
    target.addResource(rxdata, 0x4UL);
    target.addResource(priority, 0x100UL);
}

#endif // __PERIPH_REGS_H_
