////////////////////////////////////////////////////////////////////////////////
// Copyright (C) 2017, MINRES Technologies GmbH
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice,
//    this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its contributors
//    may be used to endorse or promote products derived from this software
//    without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// Created on: Wed Sep 20 22:30:45 CEST 2017
//             *      plic_regs.h Author: <RDL Generator>
//
////////////////////////////////////////////////////////////////////////////////

#ifndef _PLIC_REGS_H_
#define _PLIC_REGS_H_

#include <util/bit_field.h>
#include "scc/register.h"
#include "scc/tlm_target.h"
#include "scc/utilities.h"

namespace sysc {

class plic_regs : public sc_core::sc_module, public scc::resetable {
public:
    // storage declarations
    BEGIN_BF_DECL(priority_t, uint32_t);
    BF_FIELD(priority, 0, 3);
    END_BF_DECL();
    std::array<priority_t, 255> r_priority;

    uint32_t r_pending;

    uint32_t r_enabled;

    BEGIN_BF_DECL(threshold_t, uint32_t);
    BF_FIELD(threshold, 0, 3);
    END_BF_DECL() r_threshold;

    uint32_t r_claim_complete;

    // register declarations
    scc::sc_register_indexed<priority_t, 255> priority;
    scc::sc_register<uint32_t> pending;
    scc::sc_register<uint32_t> enabled;
    scc::sc_register<threshold_t> threshold;
    scc::sc_register<uint32_t> claim_complete;

    plic_regs(sc_core::sc_module_name nm);

    template <unsigned BUSWIDTH = 32> void registerResources(scc::tlm_target<BUSWIDTH> &target);
};
}
//////////////////////////////////////////////////////////////////////////////
// member functions
//////////////////////////////////////////////////////////////////////////////

inline sysc::plic_regs::plic_regs(sc_core::sc_module_name nm)
: sc_core::sc_module(nm)
, NAMED(priority, r_priority, 0, *this)
, NAMED(pending, r_pending, 0, *this)
, NAMED(enabled, r_enabled, 0, *this)
, NAMED(threshold, r_threshold, 0, *this)
, NAMED(claim_complete, r_claim_complete, 0, *this) {}

template <unsigned BUSWIDTH> inline void sysc::plic_regs::registerResources(scc::tlm_target<BUSWIDTH> &target) {
    target.addResource(priority, 0x4UL);
    target.addResource(pending, 0x1000UL);
    target.addResource(enabled, 0x2000UL);
    target.addResource(threshold, 0x00200000UL);
    target.addResource(claim_complete, 0x00200004UL);
}

#endif // _PLIC_REGS_H_
