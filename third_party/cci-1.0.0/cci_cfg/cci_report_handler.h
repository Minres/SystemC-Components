/*****************************************************************************

  Licensed to Accellera Systems Initiative Inc. (Accellera) under one or
  more contributor license agreements.  See the NOTICE file distributed
  with this work for additional information regarding copyright ownership.
  Accellera licenses this file to you under the Apache License, Version 2.0
  (the "License"); you may not use this file except in compliance with the
  License.  You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
  implied.  See the License for the specific language governing
  permissions and limitations under the License.

 ****************************************************************************/

#ifndef CCI_CCI_REPORT_HANDLER_H_INCLUDED_
#define CCI_CCI_REPORT_HANDLER_H_INCLUDED_

#include "cci_core/cci_cmnhdr.h"

CCI_OPEN_NAMESPACE_

enum cci_param_failure
{
  CCI_NOT_FAILURE = 0,

  CCI_SET_PARAM_FAILURE,
  CCI_GET_PARAM_FAILURE,
  CCI_ADD_PARAM_FAILURE,
  CCI_REMOVE_PARAM_FAILURE,
  CCI_VALUE_FAILURE,
  CCI_UNDEFINED_FAILURE,

  CCI_ANY_FAILURE = CCI_UNDEFINED_FAILURE
};

/* ------------------------------------------------------------------------ */

class cci_report_handler : public sc_core::sc_report_handler
{
public:
  static void report( sc_core::sc_severity severity
                    , const char* msg_type, const char* msg
                    , const char* file, int line);

  //functions that throw a report for each cci_param_failure type
  static void
  set_param_failed(const char* msg="", const char* file=NULL, int line = 0);

  static void
  get_param_failed(const char* msg="", const char* file=NULL, int line = 0);

  static void
  add_param_failed(const char* msg="", const char* file=NULL, int line = 0);

  static void
  remove_param_failed(const char* msg="", const char* file=NULL, int line = 0);

  static void
  cci_value_failure(const char* msg="", const char* file=NULL, int line = 0);

  // function to return cci_param_failure that matches thrown (or cached) report
  static cci_param_failure
  decode_param_failure(const sc_core::sc_report& rpt);

}; // class cci_report_handler

/* ------------------------------------------------------------------------ */

/**
 * Helper function to handle CCI parameter failure exceptions
 *
 * @param expect A cci_param_failure category to handle (default: CCI_ANY_FAILURE)
 * @return Current cci_param_failure (if matching expect), rethrows otherwise
 *
 * This function inspects the currently active exception and compares it against
 * an expected cci_param_failure value.  If the current exception is @b not
 * a CCI failure (or doesn't match the expected value), the exception is rethrown.
 * Otherwise, the decoded param failure is returned.
 *
 * @note This function must be called within a @c catch clause, i.e. there has
 *       to be a currently active exception - usually an @ref sc_core::sc_report
 *
 * @b Example
 * \code
 * try {
 *   cci::cci_value val; // null value
 *   val.get_string();   // invalid cci_value access
 * } catch (...) {
 *   cci::cci_param_failure err = cci::cci_handle_exception();
 *   SC_REPORT_WARNING( "Example/Warning", "Caught CCI param failure");
 *   sc_assert( err == CCI_VALUE_FAILURE );
 * }
 * \endcode
 *
 * @see cci_report_handler::decode_param_failure
 */
cci_param_failure
cci_handle_exception(cci_param_failure expect = CCI_ANY_FAILURE);

/* ------------------------------------------------------------------------ */
// cci_abort - abort simulation after irrecoverable error

#if CCI_CPLUSPLUS >= 201103L && (!defined(_MSC_VER) || _MSC_VER >= 1900)
// C++11: use standard C++ attribute
# define CCI_NORETURN_ [[noreturn]]
#else
# if defined(_MSC_VER)
#    define CCI_NORETURN_ __declspec(noreturn)
# elif defined(__GNUC__) || defined(__MINGW32__) || defined(__clang__)
#    define CCI_NORETURN_ __attribute__((noreturn))
# else
#    define CCI_NORETURN_ /* nothing */
# endif
#endif // CCI_NORETURN_

/// abort simulation
CCI_NORETURN_ void cci_abort();

#undef CCI_NORETURN_

/* ------------------------------------------------------------------------ */
// CCI report macro helpers

#define CCI_REPORT_INFO(_id, _message) \
  CCI_NAMESPACE::cci_report_handler::  \
    report(sc_core::SC_INFO,_id,_message,__FILE__,__LINE__)

#define CCI_REPORT_WARNING(_id, _message) \
  CCI_NAMESPACE::cci_report_handler::     \
    report(sc_core::SC_WARNING,_id,_message,__FILE__,__LINE__)

#define CCI_REPORT_ERROR(_id, _message) \
  CCI_NAMESPACE::cci_report_handler::   \
    report(sc_core::SC_ERROR,_id,_message,__FILE__,__LINE__)

#define CCI_REPORT_FATAL(_id, _message) \
  CCI_NAMESPACE::cci_report_handler::   \
    report(sc_core::SC_FATAL,_id,_message,__FILE__,__LINE__)

CCI_CLOSE_NAMESPACE_

#endif // CCI_CFG_CCI_REPORT_HANDLER_H_INCLUDED_
