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

  scv_report.h --  The public interface for error messages.

  Original Authors (Cadence Design Systems, Inc):
  Norris Ip, Dean Shea, John Rose, Jasvinder Singh, William Paulsen,
  John Pierce, Rachida Kebichi, Ted Elkind, David Bailey
  2002-09-23

 *****************************************************************************/

/*****************************************************************************

  MODIFICATION LOG - modifiers, enter your name, affiliation, date and
  changes you are making here.

      Name, Affiliation, Date: Stephan Schulz, Fraunhofer IIS-EAS, 2013-02-21
  Description of Modification: Undefined ERROR macro from wingdi.h to support
                               mingw32

      Name, Affiliation, Date: Torsten Maehne,
                               Universite Pierre et Marie Curie, 2013-12-02
  Description of Modification: Drop scv_report in favor of sc_core::sc_report.

 *****************************************************************************/

#ifndef _SCV_TR_REPORT_H
#define _SCV_TR_REPORT_H

#include <systemc>

//! @brief SystemC Verification Library (SCV) Transaction Recording
namespace scv_tr {
//
// Translation layer for _scv_message messages
//

class _scv_message_desc {
public:
    _scv_message_desc(std::string tag, std::string format, sc_core::sc_severity severity, sc_core::sc_actions actions)
    : _tag(tag)
    , _format(format)
    , _severity(severity)
    , _actions(actions) {}
    const char* get_tag() const { return _tag.c_str(); }
    const char* get_format() const { return _format.c_str(); }
    sc_core::sc_severity get_severity() const { return _severity; }
    unsigned get_actions() const { return _actions; }

private:
    std::string _tag;
    std::string _format;
    sc_core::sc_severity _severity;
    sc_core::sc_actions _actions;
};

class _scv_message {
public:
// Message types are actually pointers to descriptors
#define _SCV_DEFERR(code, number, string, severity)                                                                    \
    static _scv_message_desc* code##_base;                                                                             \
    static _scv_message_desc** code;
#include "scv_messages.h"
#undef _SCV_DEFERR

    // Used internally by the SystemC Verification Standard to report exceptions
    static void message(_scv_message_desc** desc_pp, ...);

private:
    static bool setup();
};
} // namespace scv_tr
#endif // ! _SCV_TR_REPORT_H
