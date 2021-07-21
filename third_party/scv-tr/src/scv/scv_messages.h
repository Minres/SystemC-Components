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

_SCV_DEFERR(ALL_USER_MESSAGES,1,"",_scv_message::ERROR,"none")

//
// Generic
//

_SCV_DEFERR(NOT_IMPLEMENTED_YET,40,\
           "The \"%s\" feature is not implemented yet.",\
           _scv_message::ERROR,"short")

_SCV_DEFERR(INTERNAL_ERROR,41,\
           "An internal error has occurred; please submit a bug report: %s.",\
           _scv_message::ERROR,"short")

//
// Bags
//

_SCV_DEFERR(BAG_ZERO_UNMARKED_OBJECTS,2,\
           "There are no marked objects in scv_bag '%s'.",\
           _scv_message::WARNING,"short")

_SCV_DEFERR(BAG_ZERO_MARKED_OBJECTS,3,\
           "There are no unmarked objects in scv_bag '%s'.", \
           _scv_message::WARNING,"short")

_SCV_DEFERR(BAG_INVALID_PEEK_ERASE,4,\
           "An object can be removed from a bag only if it was referenced by peek_next or peek_random. \nThere is no such object in scv_bag '%s'.",\
           _scv_message::WARNING,"short")

_SCV_DEFERR(BAG_INVALID_PEEK_RETURN,5,
           "An object can be returned in a bag only if it was referenced by peek_next or peek_random. \nThere is no such object in scv_bag '%s'.",\
           _scv_message::ERROR,"short")

_SCV_DEFERR(BAG_INVALID_ADD_ARGUMENT,6,\
           "The second argument for scv_bag::add must be a non-zero positive integer; the actual argument passed in this call was '%d'.",\
           _scv_message::ERROR,"none")

_SCV_DEFERR(BAG_INVALID_PEEK_MARK,7,\
           "An object can be marked in a bag only if it was referenced by peek_next or peek_random. \nThere is no such object in scv_bag '%s'.",\
           _scv_message::WARNING,"short")

_SCV_DEFERR(BAG_INVALID_PEEK_UNMARK,8,\
           "An object can be unmarked in a bag only if it was referenced by peek_next or peek_random. \nThere is no such object in scv_bag '%s'.",\
           _scv_message::WARNING,"short")

//
// Empty data structures
//

_SCV_DEFERR(EMPTY_BAG,9,"There is no object to peek at and return from scv_bag '%s'.",\
           _scv_message::ERROR,"short")

_SCV_DEFERR(EMPTY_LIST,10,"%s was called on an empty %s, '%s'.",_scv_message::ERROR,"short")

//
// Transactions
//

_SCV_DEFERR(TRANSACTION_RECORDING_INTERNAL,34,\
	"An internal error occurred in transaction recording:\n'%s'",\
	_scv_message::ERROR,"short")

_SCV_DEFERR(TRANSACTION_RECORDING_INTERNAL_FATAL,51,\
        "An internal error occurred in transaction recording:\n'%s'",\
        _scv_message::FATAL,"short")

//
// Expression
//

_SCV_DEFERR(EXPRESSION_ILLEGAL_EXTRACTION,38,\
           "%s cannot be extracted from an expression.",\
           _scv_message::ERROR,"short")

//
// Introspection
//

_SCV_DEFERR(INTROSPECTION_INVALID_EXTENSIONS,39,\
	  "A valid extension (scv_extensions<T>) has not been defined for your composite/enum type.",
	  _scv_message::ERROR,"short")

_SCV_DEFERR(INTROSPECTION_INVALID_INDEX,42,\
	  "The value %d is an invalid index for %s %s.",\
	  _scv_message::ERROR,"short")

_SCV_DEFERR(INTROSPECTION_NULL_POINTER,43,\
	  "The NULL value in pointer %s cannot be dereferenced.",\
	  _scv_message::ERROR,"short")

_SCV_DEFERR(INTROSPECTION_BAD_CALLBACK_REMOVAL,44,\
	  "Cannot remove an invalid callback.",\
	  _scv_message::ERROR,"short")

_SCV_DEFERR(INTROSPECTION_INVALID_DYNAMIC_EXTENSIONS,45,\
	  "Cannot access dynamic extensions (%s) from objects other than scv_smart_ptr.",\
	  _scv_message::ERROR,"short")

_SCV_DEFERR(INTROSPECTION_INVALID_EXPRESSION,46,\
	  "Cannot generate expression from %s %s.",\
	  _scv_message::ERROR,"short")

_SCV_DEFERR(INTROSPECTION_INVALID_READ_WRITE,47,\
	  "Cannot use %s() to perform the %s operation on %s %s.",\
	  _scv_message::ERROR,"short")

_SCV_DEFERR(INTROSPECTION_INVALID_ENUM_VALUE,48,\
	  "Cannot obtain an enum string of type %s from integer value %d.",\
	  _scv_message::ERROR,"short")

_SCV_DEFERR(INTROSPECTION_GET_INSTANCE_USAGE,49,\
	  "You are accessing a data object through get_instance(), which bypasses the value change callback mechanism. If you are actually changing the data object, you should also call trigger_value_change_cb() explicitly.",\
	  _scv_message::WARNING,"short")

_SCV_DEFERR(INTROSPECTION_INVALID_ENUM_STRING,50,\
	  "Cannot assign invalid string \"%s\" to an enum.",\
	  _scv_message::ERROR,"short")

_SCV_DEFERR(INTROSPECTION_SIZE_MISMATCH_FOR_WIDE_DATA,57,\
	  "The %s argument does not have the same width as the data; illegal call to %s",\
	  _scv_message::ERROR,"short")

_SCV_DEFERR(INTROSPECTION_EMPTY_DISTRIBUTION,58,\
	  "Setting mode to DISTRIBUTION for '%s' without adding a scv_bag<T> OR scv_bag<pair<T, T> > object. The setting will be ignored.",\
	  _scv_message::ERROR,"short")

