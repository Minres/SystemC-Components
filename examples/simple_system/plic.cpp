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
// CONTRACT, STRICT LIABILITY0x200004, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// Contributors:
//       eyck@minres.com - initial API and implementation
//
//
////////////////////////////////////////////////////////////////////////////////

// todo: truncate values beyond 7 (in prio_threshold write_cb)

#include "plic.h"
#include "gen/plic_regs.h"
#include "scc/utilities.h"
#include <scc/report.h>

namespace sysc {

plic::plic(sc_core::sc_module_name nm)
: sc_core::sc_module(nm)
, tlm_target<>(clk)
, NAMED(clk_i)
, NAMED(rst_i)
, NAMED(global_interrupts_i, 256)
, NAMED(core_interrupt_o)
, NAMEDD(regs, plic_regs)
{
    regs->registerResources(*this);
    // register callbacks
    init_callbacks();
    regs->claim_complete.set_write_cb(m_claim_complete_write_cb);

    // port callbacks
    SC_METHOD(global_int_port_cb);
    for (uint8_t i = 0; i < 255; i++) {
        sensitive << global_interrupts_i[i].pos();
    }
    dont_initialize();

    // register event callbacks
    SC_METHOD(clock_cb);
    sensitive << clk_i;
    SC_METHOD(reset_cb);
    sensitive << rst_i;
}

plic::~plic() = default;

void plic::init_callbacks() {
    m_claim_complete_write_cb = [=](scc::sc_register<uint32_t> reg, uint32_t v) -> bool {
        reg.put(v);
        reset_pending_int(v);
        // std::cout << "Value of register: 0x" << std::hex << reg << std::endl;
        // todo: reset related interrupt and find next high-prio interrupt
        return true;
    };
}

void plic::clock_cb() { this->clk = clk_i.read(); }

void plic::reset_cb() {
    if (rst_i.read())
        regs->reset_start();
    else
        regs->reset_stop();
}

// Functional handling of interrupts:
// - global_int_port_cb()
//   - set pending register bits
//   - called by: incoming global_int
// - handle_pending_int()
//   - update claim register content
//   - generate core-interrupt pulse
//   - called by:
//     - incoming global_int
//     - complete-register write access
// - reset_pending_int(int-id)
//   - reset pending bit
//   - call next handle_pending_int()
//   - called by:
//     - complete-reg write register content

void plic::global_int_port_cb() {

    // set related pending bit if enable is set for incoming global_interrupt

    // todo: extend up to 255 bits (limited to 32 right now)
    for (uint32_t i = 1; i < 32; i++) {
        uint32_t enable_bits = regs->r_enabled;
        bool enable = enable_bits & (0x1 << i); // read enable bit

        if (enable && global_interrupts_i[i].read() == 1) {
            regs->r_pending = regs->r_pending | (0x1 << i);
            SCCDEBUG("plic") << "pending interrupt identified: " << i;
        }
    }

    handle_pending_int();
}

void plic::handle_pending_int() {
    // identify high-prio pending interrupt and raise a core-interrupt
    uint32_t claim_int = 0;  // claim interrupt
    uint32_t claim_prio = 0; // related priority (highest prio interrupt wins the race)
    bool raise_int = false;
    uint32_t thold = regs->r_threshold.threshold; // threshold value

    // todo: extend up to 255 bits (limited to 32 right now)
    for (uint32_t i = 1; i < 32; i++) {
        uint32_t pending_bits = regs->r_pending;
        bool pending = (pending_bits & (0x1 << i)) ? true : false;
        uint32_t prio = regs->r_priority[i - 1].priority; // read priority value

        if (pending && thold < prio) {
            regs->r_pending = regs->r_pending | (0x1 << i);
            // below condition ensures implicitly that lowest id is selected in case of multiple identical
            // priority-interrupts
            if (prio > claim_prio) {
                claim_prio = prio;
                claim_int = i;
                raise_int = true;
                SCCDEBUG("plic") << "pending interrupt activated: " << i;
            }
        }
    }

    if (raise_int) {
        regs->r_claim_complete = claim_int;
        core_interrupt_o.write(true);
        // todo: evluate clock period
    } else {
        regs->r_claim_complete = 0;
        SCCDEBUG("plic") << "no further pending interrupt.";
    }
}

void plic::reset_pending_int(uint32_t irq) {
    // todo: evaluate enable register (see spec)
    // todo: make sure that pending is set, otherwise don't reset irq ... read spec.
    SCCDEBUG("plic") << "reset pending interrupt: " << irq;
    // reset related pending bit
    regs->r_pending &= ~(0x1 << irq);
    core_interrupt_o.write(false);

    // evaluate next pending interrupt
    handle_pending_int();
}

} /* namespace sysc */
