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
//             *      uart_regs.h Author: <RDL Generator>
//
////////////////////////////////////////////////////////////////////////////////

#ifndef _UART_REGS_H_
#define _UART_REGS_H_

#include <util/bit_field.h>
#include "scc/register.h"
#include "scc/tlm_target.h"
#include "scc/utilities.h"

namespace sysc {

class uart_regs : public sc_core::sc_module, public scc::resetable {
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

    BEGIN_BF_DECL(txctrl_t, uint32_t);
    BF_FIELD(txen, 0, 1);
    BF_FIELD(nstop, 1, 1);
    BF_FIELD(reserved, 2, 14);
    BF_FIELD(txcnt, 16, 3);
    END_BF_DECL() r_txctrl;

    BEGIN_BF_DECL(rxctrl_t, uint32_t);
    BF_FIELD(rxen, 0, 1);
    BF_FIELD(reserved, 1, 15);
    BF_FIELD(rxcnt, 16, 3);
    END_BF_DECL() r_rxctrl;

    BEGIN_BF_DECL(ie_t, uint32_t);
    BF_FIELD(txwm, 0, 1);
    BF_FIELD(rxwm, 1, 1);
    END_BF_DECL() r_ie;

    BEGIN_BF_DECL(ip_t, uint32_t);
    BF_FIELD(txwm, 0, 1);
    BF_FIELD(rxwm, 1, 1);
    END_BF_DECL() r_ip;

    BEGIN_BF_DECL(div_t, uint32_t);
    BF_FIELD(div, 0, 16);
    END_BF_DECL() r_div;

    // register declarations
    scc::sc_register<txdata_t> txdata;
    scc::sc_register<rxdata_t> rxdata;
    scc::sc_register<txctrl_t> txctrl;
    scc::sc_register<rxctrl_t> rxctrl;
    scc::sc_register<ie_t> ie;
    scc::sc_register<ip_t> ip;
    scc::sc_register<div_t> div;

public:
    uart_regs(sc_core::sc_module_name nm);

    template <unsigned BUSWIDTH = 32> void registerResources(scc::tlm_target<BUSWIDTH> &target);
};
}
//////////////////////////////////////////////////////////////////////////////
// member functions
//////////////////////////////////////////////////////////////////////////////

inline sysc::uart_regs::uart_regs(sc_core::sc_module_name nm)
: sc_core::sc_module(nm)
, NAMED(txdata, r_txdata, 0, *this)
, NAMED(rxdata, r_rxdata, 0, *this)
, NAMED(txctrl, r_txctrl, 0, *this)
, NAMED(rxctrl, r_rxctrl, 0, *this)
, NAMED(ie, r_ie, 0, *this)
, NAMED(ip, r_ip, 0, *this)
, NAMED(div, r_div, 0, *this) {}

template <unsigned BUSWIDTH> inline void sysc::uart_regs::registerResources(scc::tlm_target<BUSWIDTH> &target) {
    target.addResource(txdata, 0x0UL);
    target.addResource(rxdata, 0x4UL);
    target.addResource(txctrl, 0x8UL);
    target.addResource(rxctrl, 0xcUL);
    target.addResource(ie, 0x10UL);
    target.addResource(ip, 0x14UL);
    target.addResource(div, 0x18UL);
}

#endif // _UART_REGS_H_
