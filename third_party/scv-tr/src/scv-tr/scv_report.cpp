//  -*- C++ -*- <this line is for emacs to recognize it as C++ code>
/*****************************************************************************

  Licensed to Accellera Systems Initiative Inc. (Accellera)
  under one or more contributor license agreements.  See the
  NOTICE file distributed with this work for additional
  information regarding copyright ownership. Accellera licenses
  this file to you under the Apache License, Version 2.0 (the
  "License"); you may not use this file except in compliance
  with the License.  You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing,
  software distributed under the License is distributed on an
  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
  KIND, either express or implied.  See the License for the
  specific language governing permissions and limitations
  under the License.

 *****************************************************************************/

/*****************************************************************************

  scv_report.cpp -- A trace/debug facility for the SystemC Verification
  Standard library

  Original Authors (Cadence Design Systems, Inc):
  Norris Ip, Dean Shea, John Rose, Jasvinder Singh, William Paulsen,
  John Pierce, Rachida Kebichi, Ted Elkind, David Bailey
  2002-09-23

 *****************************************************************************/

/*****************************************************************************

  MODIFICATION LOG - modifiers, enter your name, affiliation, date and
  changes you are making here.

      Name, Affiliation, Date: Torsten Maehne
                               Universite Pierre et Marie Curie, 2013-12-02
  Description of Modification: Drop scv_report and its implementation details
                               in favor of sc_core::sc_report.

 *****************************************************************************/

//
// This must be included before any system headers.
//
#include "scv_report.h"

#include <array>
#include <cstdarg>
#include <map>
#include <string>

//
// Implementation of _scv_message
//
namespace scv_tr {
#define _SCV_DEFERR(code, number, string, severity)                                                                    \
    _scv_message_desc* _scv_message::code##_base = nullptr;                                                            \
    _scv_message_desc** _scv_message::code = &_scv_message::code##_base;
#include "scv_messages.h"
#undef _SCV_DEFERR

void _scv_message::message(_scv_message_desc** desc_pp, ...) {
    // make sure desc is defined
    static bool res = _scv_message::setup();

    _scv_message_desc* desc_p = *desc_pp;

    const char* tag = desc_p->get_tag();
    const char* format = desc_p->get_format();
    sc_core::sc_severity severity = desc_p->get_severity();
    sc_core::sc_actions actions = desc_p->get_actions();

    sc_core::sc_actions hold = sc_core::sc_report_handler::force(0);
    sc_core::sc_report_handler::force(hold | actions);

    static std::array<char, 20000> formattedMessageString;
    std::va_list ap;
    va_start(ap, desc_pp);
    vsprintf(formattedMessageString.data(), format, ap);
    va_end(ap);

    sc_core::sc_report_handler::report(severity, tag, formattedMessageString.data(), "unknown", 0);

    sc_core::sc_report_handler::force(hold);
}

bool _scv_message::setup() {
#define _SCV_DEFERR(code, number, string, severity)                                                                    \
    code##_base = new _scv_message_desc(#code, string, severity, sc_core::SC_DO_NOTHING);
#include "scv_messages.h"
#undef _SCV_DEFERR
    return true;
}
} // namespace scv_tr
