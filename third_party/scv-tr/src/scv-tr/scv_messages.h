/* -*- C++ -*- <this line is for emacs to recognize it as C++ code> */
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

  scv_messages.h

  Original Authors (Cadence Design Systems, Inc):
  Norris Ip, Dean Shea, John Rose, Jasvinder Singh, William Paulsen,
  John Pierce, Rachida Kebichi, Ted Elkind, David Bailey
  2002-09-23

 *****************************************************************************/

/*****************************************************************************

  MODIFICATION LOG - modifiers, enter your name, affiliation, date and
  changes you are making here.

      Name, Affiliation, Date:
  Description of Modification:

 *****************************************************************************/

// The current maximum error code number is: 68
// If you add any error messages start at 69 and change these comments.
//

_SCV_DEFERR(ALL_USER_MESSAGES, 1, "", sc_core::SC_ERROR)

//
// Generic
//

_SCV_DEFERR(NOT_IMPLEMENTED_YET, 40, "The \"%s\" feature is not implemented yet.", sc_core::SC_ERROR)

_SCV_DEFERR(INTERNAL_ERROR, 41, "An internal error has occurred; please submit a bug report: %s.", sc_core::SC_ERROR)

//
// Transactions
//

_SCV_DEFERR(TRANSACTION_RECORDING_INTERNAL_INFO, 33, "TB Transaction Recording: %s'", sc_core::SC_INFO)

_SCV_DEFERR(TRANSACTION_RECORDING_INTERNAL, 34, "An internal error occurred in transaction recording:\n'%s'",
            sc_core::SC_ERROR)

_SCV_DEFERR(TRANSACTION_RECORDING_INTERNAL_FATAL, 51, "An internal error occurred in transaction recording:\n'%s'",
            sc_core::SC_FATAL)

//
// Introspection
//

_SCV_DEFERR(INTROSPECTION_INVALID_EXTENSIONS, 39,
            "A valid extension (scv_extensions<T>) has not been defined for your composite/enum type.",
            sc_core::SC_ERROR)

_SCV_DEFERR(INTROSPECTION_INVALID_INDEX, 42, "The value %d is an invalid index for %s %s.", sc_core::SC_ERROR)

_SCV_DEFERR(INTROSPECTION_NULL_POINTER, 43, "The NULL value in pointer %s cannot be dereferenced.", sc_core::SC_ERROR)

_SCV_DEFERR(INTROSPECTION_BAD_CALLBACK_REMOVAL, 44, "Cannot remove an invalid callback.", sc_core::SC_ERROR)

_SCV_DEFERR(INTROSPECTION_INVALID_DYNAMIC_EXTENSIONS, 45,
            "Cannot access dynamic extensions (%s) from objects other than scv_smart_ptr.", sc_core::SC_ERROR)

_SCV_DEFERR(INTROSPECTION_INVALID_EXPRESSION, 46, "Cannot generate expression from %s %s.", sc_core::SC_ERROR)

_SCV_DEFERR(INTROSPECTION_INVALID_READ_WRITE, 47, "Cannot use %s() to perform the %s operation on %s %s.",
            sc_core::SC_ERROR)

_SCV_DEFERR(INTROSPECTION_INVALID_ENUM_VALUE, 48, "Cannot obtain an enum string of type %s from integer value %d.",
            sc_core::SC_ERROR)

_SCV_DEFERR(
    INTROSPECTION_GET_INSTANCE_USAGE, 49,
    "You are accessing a data object through get_instance(), which bypasses the value change callback mechanism. If "
    "you are actually changing the data object, you should also call trigger_value_change_cb() explicitly.",
    sc_core::SC_WARNING)

_SCV_DEFERR(INTROSPECTION_INVALID_ENUM_STRING, 50, "Cannot assign invalid string \"%s\" to an enum.", sc_core::SC_ERROR)

_SCV_DEFERR(INTROSPECTION_SIZE_MISMATCH_FOR_WIDE_DATA, 57,
            "The %s argument does not have the same width as the data; illegal call to %s", sc_core::SC_ERROR)

_SCV_DEFERR(INTROSPECTION_EMPTY_DISTRIBUTION, 58,
            "Setting mode to DISTRIBUTION for '%s' without adding a scv_bag<T> OR scv_bag<pair<T, T> > object. The "
            "setting will be ignored.",
            sc_core::SC_ERROR)
