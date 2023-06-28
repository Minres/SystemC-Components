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
// Created on: Wed Sep 20 11:47:24 CEST 2017
//             *      gpio_regs.h Author: <RDL Generator>
//
////////////////////////////////////////////////////////////////////////////////

#ifndef _GPIO_REGS_H_
#define _GPIO_REGS_H_

#include <util/bit_field.h>
#include "scc/register.h"
#include "scc/tlm_target.h"
#include "scc/utilities.h"

namespace sysc {

class gpio_regs : public sc_core::sc_module, public scc::resetable {
public:
    // storage declarations
    uint32_t r_value;

    uint32_t r_input_en;

    uint32_t r_output_en;

    uint32_t r_port;

    uint32_t r_pue;

    uint32_t r_ds;

    uint32_t r_rise_ie;

    uint32_t r_rise_ip;

    uint32_t r_fall_ie;

    uint32_t r_fall_ip;

    uint32_t r_high_ie;

    uint32_t r_high_ip;

    uint32_t r_low_ie;

    uint32_t r_low_ip;

    uint32_t r_iof_en;

    uint32_t r_iof_sel;

    uint32_t r_out_xor;

    // register declarations
    scc::sc_register<uint32_t> value;
    scc::sc_register<uint32_t> input_en;
    scc::sc_register<uint32_t> output_en;
    scc::sc_register<uint32_t> port;
    scc::sc_register<uint32_t> pue;
    scc::sc_register<uint32_t> ds;
    scc::sc_register<uint32_t> rise_ie;
    scc::sc_register<uint32_t> rise_ip;
    scc::sc_register<uint32_t> fall_ie;
    scc::sc_register<uint32_t> fall_ip;
    scc::sc_register<uint32_t> high_ie;
    scc::sc_register<uint32_t> high_ip;
    scc::sc_register<uint32_t> low_ie;
    scc::sc_register<uint32_t> low_ip;
    scc::sc_register<uint32_t> iof_en;
    scc::sc_register<uint32_t> iof_sel;
    scc::sc_register<uint32_t> out_xor;

public:
    gpio_regs(sc_core::sc_module_name nm);

    template <unsigned BUSWIDTH = 32> void registerResources(scc::tlm_target<BUSWIDTH> &target);
};
}
//////////////////////////////////////////////////////////////////////////////
// member functions
//////////////////////////////////////////////////////////////////////////////

inline sysc::gpio_regs::gpio_regs(sc_core::sc_module_name nm)
: sc_core::sc_module(nm)
, NAMED(value, r_value, 0, *this)
, NAMED(input_en, r_input_en, 0, *this)
, NAMED(output_en, r_output_en, 0, *this)
, NAMED(port, r_port, 0, *this)
, NAMED(pue, r_pue, 0, *this)
, NAMED(ds, r_ds, 0, *this)
, NAMED(rise_ie, r_rise_ie, 0, *this)
, NAMED(rise_ip, r_rise_ip, 0, *this)
, NAMED(fall_ie, r_fall_ie, 0, *this)
, NAMED(fall_ip, r_fall_ip, 0, *this)
, NAMED(high_ie, r_high_ie, 0, *this)
, NAMED(high_ip, r_high_ip, 0, *this)
, NAMED(low_ie, r_low_ie, 0, *this)
, NAMED(low_ip, r_low_ip, 0, *this)
, NAMED(iof_en, r_iof_en, 0, *this)
, NAMED(iof_sel, r_iof_sel, 0, *this)
, NAMED(out_xor, r_out_xor, 0, *this) {}

template <unsigned BUSWIDTH> inline void sysc::gpio_regs::registerResources(scc::tlm_target<BUSWIDTH> &target) {
    target.addResource(value, 0x0UL);
    target.addResource(input_en, 0x4UL);
    target.addResource(output_en, 0x8UL);
    target.addResource(port, 0xcUL);
    target.addResource(pue, 0x10UL);
    target.addResource(ds, 0x14UL);
    target.addResource(rise_ie, 0x18UL);
    target.addResource(rise_ip, 0x1cUL);
    target.addResource(fall_ie, 0x20UL);
    target.addResource(fall_ip, 0x24UL);
    target.addResource(high_ie, 0x28UL);
    target.addResource(high_ip, 0x2cUL);
    target.addResource(low_ie, 0x30UL);
    target.addResource(low_ip, 0x34UL);
    target.addResource(iof_en, 0x38UL);
    target.addResource(iof_sel, 0x3cUL);
    target.addResource(out_xor, 0x40UL);
}

#endif // _GPIO_REGS_H_
