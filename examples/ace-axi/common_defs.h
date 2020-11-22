/*****************************************************************************
 *
 *   The following code is derived, directly or indirectly, from the SystemC
 *   source code Copyright (c) 1996-2014 by all Contributors.
 *   All Rights reserved.
 *
 *   The contents of this file are subject to the restrictions and limitations
 *   set forth in the SystemC Open Source License (the "License");
 *   You may not use this file except in compliance with such restrictions and
 *   limitations. You may obtain instructions on how to receive a copy of the
 *   License at http://www.accellera.org/. Software distributed by Contributors
 *   under the License is distributed on an "AS IS" basis, WITHOUT WARRANTY OF
 *   ANY KIND, either express or implied. See the License for the specific
 *   language governing rights and limitations under the License.
 *
 *   *****************************************************************************/
/*
 * @file
 *  reporting.h
 *
 * @brief
 * Convenience macros to simplify reporting.
 */
#pragma once
#include "axi/axi_tlm.h" ///< AXI TLM headers
#include "scc/report.h"
#include "tlm.h" ///< TLM headers
#include <iomanip>
#include <sstream>
#include <stdio.h>
//#include "coda.h" // need for AXI_phit definition, but ugly...

using std::setfill;
using std::setw;
using namespace std;

#define SC_TRACE_SIG(x) sc_trace(tf, x, std::string(sc_module::name()) + "." + #x)
#define SC_TRACE_VAR(x) sc_trace(tf, x, std::string(sc_module::name()) + "." + #x)
#define SC_TRACE_ATT(x) sc_trace(tf, x.value, std::string(sc_module::name()) + "." + #x)

struct TLM2Group {
    axi::axi_protocol_types::tlm_payload_type* trans;
    axi::axi_protocol_types::tlm_phase_type phase;
  sc_core::sc_time delay;
};
