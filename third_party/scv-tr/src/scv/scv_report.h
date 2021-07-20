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

#ifndef _SCV_REPORT_H
#define _SCV_REPORT_H

// Previous SCV versions provided an own scv_report infrastructure,
// which has been superseeded by SystemC sc_core::sc_report.

typedef sc_core::sc_actions scv_actions;
typedef sc_core::sc_report scv_report;
typedef sc_core::sc_report_handler scv_report_handler;
typedef sc_core::sc_severity scv_severity;
typedef const char *scv_msg_type;

#define SCV_INFO SC_INFO
#define SCV_WARNING SC_WARNING
#define SCV_ERROR SC_ERROR
#define SCV_FATAL SC_FATAL

#define SCV_UNSPECIFIED SC_UNSPECIFIED
#define SCV_DO_NOTHING SC_DO_NOTHING
#define SCV_THROW SC_THROW
#define SCV_LOG SC_LOG
#define SCV_DISPLAY SC_DISPLAY
#define SCV_CACHE_REPORT SC_CACHE_REPORT
#define SCV_INTERRUPT SC_INTERRUPT
#define SCV_STOP SC_STOP
#define SCV_ABORT SC_ABORT

#define SCV_DEFAULT_INFO_ACTIONS SC_DEFAULT_INFO_ACTIONS
#define SCV_DEFAULT_WARNING_ACTIONS SC_DEFAULT_WARNING_ACTIONS
#define SCV_DEFAULT_ERROR_ACTIONS SC_DEFAULT_ERROR_ACTIONS
#define SCV_DEFAULT_FATAL_ACTIONS SC_DEFAULT_FATAL_ACTIONS

#define SCV_REPORT_INFO SC_REPORT_INFO
#define SCV_REPORT_WARNING SC_REPORT_WARNING
#define SCV_REPORT_ERROR SC_REPORT_ERROR
#define SCV_REPORT_FATAL SC_REPORT_FATAL


//
// Translation layer for _scv_message messages
//


class _scv_message_desc {
public:
  _scv_message_desc(
    std::string tag, std::string format, scv_severity severity, scv_actions actions
  ) : _tag(tag), _format(format), _severity(severity), _actions(actions) {}
  const char *get_tag() const { return _tag.c_str(); }
  const char *get_format() const { return _format.c_str(); }
  scv_severity get_severity() const { return _severity; }
  unsigned get_actions() const { return _actions; }
private:
  std::string _tag;
  std::string _format;
  scv_severity _severity;
  scv_actions _actions;
};


#if defined(_MSC_VER) || defined(_WIN32)
#  ifdef ERROR
#    undef ERROR //defined in wingdi.h
#  endif
#endif

class _scv_message {
public:

  enum severity_level { INFO, WARNING, ERROR, FATAL };

  enum response_level {
    NONE_SPECIFIED, ENABLE_MESSAGE, SUPPRESS_MESSAGE
  };

// Message types are actually pointers to descriptors
#define _SCV_DEFERR(code, number, string, severity, actions) \
  static _scv_message_desc *code##_base; \
  static _scv_message_desc **code;
#include "scv/scv_messages.h"
#undef _SCV_DEFERR

  // Used internally by the SystemC Verification Standard to report exceptions
  static void message(_scv_message_desc **desc_pp, ... );
  static scv_severity xlat_severity(severity_level severity);

private:
  static void setup();
};


#endif // ! _SCV_REPORT_H 
