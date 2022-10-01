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
//             *      spi_regs.h Author: <RDL Generator>
//
////////////////////////////////////////////////////////////////////////////////

#ifndef _SPI_REGS_H_
#define _SPI_REGS_H_

#include <util/bit_field.h>
#include "scc/register.h"
#include "scc/tlm_target.h"
#include "scc/utilities.h"

namespace sysc {

class spi_regs : public sc_core::sc_module, public scc::resetable {
protected:
    // storage declarations
    BEGIN_BF_DECL(sckdiv_t, uint32_t);
    BF_FIELD(div, 0, 12);
    END_BF_DECL() r_sckdiv;

    BEGIN_BF_DECL(sckmode_t, uint32_t);
    BF_FIELD(pha, 0, 1);
    BF_FIELD(pol, 1, 1);
    END_BF_DECL() r_sckmode;

    uint32_t r_csid;

    uint32_t r_csdef;

    BEGIN_BF_DECL(csmode_t, uint32_t);
    BF_FIELD(mode, 0, 2);
    END_BF_DECL() r_csmode;

    BEGIN_BF_DECL(delay0_t, uint32_t);
    BF_FIELD(cssck, 0, 8);
    BF_FIELD(sckcs, 16, 8);
    END_BF_DECL() r_delay0;

    BEGIN_BF_DECL(delay1_t, uint32_t);
    BF_FIELD(intercs, 0, 16);
    BF_FIELD(interxfr, 16, 8);
    END_BF_DECL() r_delay1;

    BEGIN_BF_DECL(fmt_t, uint32_t);
    BF_FIELD(proto, 0, 2);
    BF_FIELD(endian, 2, 1);
    BF_FIELD(dir, 3, 1);
    BF_FIELD(len, 16, 4);
    END_BF_DECL() r_fmt;

    BEGIN_BF_DECL(txdata_t, uint32_t);
    BF_FIELD(data, 0, 8);
    BF_FIELD(full, 31, 1);
    END_BF_DECL() r_txdata;

    BEGIN_BF_DECL(rxdata_t, uint32_t);
    BF_FIELD(data, 0, 8);
    BF_FIELD(empty, 31, 1);
    END_BF_DECL() r_rxdata;

    BEGIN_BF_DECL(txmark_t, uint32_t);
    BF_FIELD(txmark, 0, 3);
    END_BF_DECL() r_txmark;

    BEGIN_BF_DECL(rxmark_t, uint32_t);
    BF_FIELD(rxmark, 0, 3);
    END_BF_DECL() r_rxmark;

    BEGIN_BF_DECL(fctrl_t, uint32_t);
    BF_FIELD(en, 0, 1);
    END_BF_DECL() r_fctrl;

    BEGIN_BF_DECL(ffmt_t, uint32_t);
    BF_FIELD(cmd_en, 0, 1);
    BF_FIELD(addr_len, 1, 2);
    BF_FIELD(pad_cnt, 3, 4);
    BF_FIELD(cmd_proto, 7, 2);
    BF_FIELD(addr_proto, 9, 2);
    BF_FIELD(data_proto, 11, 2);
    BF_FIELD(cmd_code, 16, 8);
    BF_FIELD(pad_code, 24, 8);
    END_BF_DECL() r_ffmt;

    BEGIN_BF_DECL(ie_t, uint32_t);
    BF_FIELD(txwm, 0, 1);
    BF_FIELD(rxwm, 1, 1);
    END_BF_DECL() r_ie;

    BEGIN_BF_DECL(ip_t, uint32_t);
    BF_FIELD(txwm, 0, 1);
    BF_FIELD(rxwm, 1, 1);
    END_BF_DECL() r_ip;

    // register declarations
    scc::sc_register<sckdiv_t> sckdiv;
    scc::sc_register<sckmode_t> sckmode;
    scc::sc_register<uint32_t> csid;
    scc::sc_register<uint32_t> csdef;
    scc::sc_register<csmode_t> csmode;
    scc::sc_register<delay0_t> delay0;
    scc::sc_register<delay1_t> delay1;
    scc::sc_register<fmt_t> fmt;
    scc::sc_register<txdata_t> txdata;
    scc::sc_register<rxdata_t> rxdata;
    scc::sc_register<txmark_t> txmark;
    scc::sc_register<rxmark_t> rxmark;
    scc::sc_register<fctrl_t> fctrl;
    scc::sc_register<ffmt_t> ffmt;
    scc::sc_register<ie_t> ie;
    scc::sc_register<ip_t> ip;

public:
    spi_regs(sc_core::sc_module_name nm);

    template <unsigned BUSWIDTH = 32> void registerResources(scc::tlm_target<BUSWIDTH> &target);
};
}
//////////////////////////////////////////////////////////////////////////////
// member functions
//////////////////////////////////////////////////////////////////////////////

inline sysc::spi_regs::spi_regs(sc_core::sc_module_name nm)
: sc_core::sc_module(nm)
, NAMED(sckdiv, r_sckdiv, 0, *this)
, NAMED(sckmode, r_sckmode, 0, *this)
, NAMED(csid, r_csid, 0, *this)
, NAMED(csdef, r_csdef, 0, *this)
, NAMED(csmode, r_csmode, 0, *this)
, NAMED(delay0, r_delay0, 0, *this)
, NAMED(delay1, r_delay1, 0, *this)
, NAMED(fmt, r_fmt, 0, *this)
, NAMED(txdata, r_txdata, 0, *this)
, NAMED(rxdata, r_rxdata, 0, *this)
, NAMED(txmark, r_txmark, 0, *this)
, NAMED(rxmark, r_rxmark, 0, *this)
, NAMED(fctrl, r_fctrl, 0, *this)
, NAMED(ffmt, r_ffmt, 0, *this)
, NAMED(ie, r_ie, 0, *this)
, NAMED(ip, r_ip, 0, *this) {}

template <unsigned BUSWIDTH> inline void sysc::spi_regs::registerResources(scc::tlm_target<BUSWIDTH> &target) {
    target.addResource(sckdiv, 0x0UL);
    target.addResource(sckmode, 0x4UL);
    target.addResource(csid, 0x10UL);
    target.addResource(csdef, 0x14UL);
    target.addResource(csmode, 0x18UL);
    target.addResource(delay0, 0x28UL);
    target.addResource(delay1, 0x2cUL);
    target.addResource(fmt, 0x40UL);
    target.addResource(txdata, 0x48UL);
    target.addResource(rxdata, 0x4cUL);
    target.addResource(txmark, 0x50UL);
    target.addResource(rxmark, 0x54UL);
    target.addResource(fctrl, 0x60UL);
    target.addResource(ffmt, 0x64UL);
    target.addResource(ie, 0x70UL);
    target.addResource(ip, 0x74UL);
}

#endif // _SPI_REGS_H_
