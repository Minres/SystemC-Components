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
// Sparse arrays
//

_SCV_DEFERR(SPARSE_ARRAY_ILLEGAL_INDEX,11,\
           "Index = %d cannot be used to index scv_sparse_array '%s'. \nThe valid range is lower = %d, upper = %d.",\
           _scv_message::ERROR,"short")

//
// Random
//

_SCV_DEFERR(RANDOM_NULL_ALGORITHM,12,\
	   "Although CUSTOM is selected in '%s', the pointer to the custom algorithm is NULL.\n RAND48 will be used, instead.",\
	   _scv_message::WARNING,"short")

_SCV_DEFERR(RANDOM_OUT_OF_ORDER_SEED,13,\
	   "The seed for '%s' was retrieved out of order from the \"%s\" seed information file. \nThe order in which the instances were stored has changed.",\
	   _scv_message::WARNING,"none")

_SCV_DEFERR(RANDOM_CANNOT_MATCH_SEED,14,\
	   "No matching seed found for instance '%s' from the '%s' seed information file.",\
	   _scv_message::WARNING,"none")

_SCV_DEFERR(RANDOM_RETRIEVING_SEED_WITH_SAME_NAME,15,\
	   "Identical names, '%s', found when retrieving seeds from the '%s' seed information file.\nThere might be a problem in matching the seed to \nits corresponding generator instance if the order of instantiation has changed.",\
	   _scv_message::WARNING,"none")

_SCV_DEFERR(RANDOM_STORING_SEED_WITH_SAME_NAME,16,\
	   "Identical names, '%s', founf when storing seeds to the '%s' seed information file.\n\nThere might be a problem in matching the seed to \nits corresponding generator instance if the order of instantiation has changed.",\
	   _scv_message::WARNING,"none")

_SCV_DEFERR(RANDOM_SEED_MONITOR_NOT_OFF,17,\
	   "The seed_monitor_on method was called a second time before closing seed file '%s' by calling seed_monitor_off.",\
	   _scv_message::WARNING,"short")

_SCV_DEFERR(RANDOM_SEED_NOT_EXHAUSTED,18,\
	     "The seed file '%s' was not exhausted when it was closed.",\
	     _scv_message::WARNING,"none")

_SCV_DEFERR(RANDOM_CANNOT_OPEN_SEED_FILE,19,\
	   "Seed file '%s' cannot be opened.",\
	   _scv_message::WARNING,"none")

_SCV_DEFERR(RANDOM_TYPE_NOT_SUPPORTED,68,\
	   "Randomization for '%s' type not supported. Use disable_randomization to turn off this WARNING message.",\
	   _scv_message::WARNING,"none")

//
// Constraint Solver messages
//

_SCV_DEFERR(CONSTRAINT_ERROR_INTERNAL,21,\
           "An internal error occurred; %s.", \
           _scv_message::ERROR,"short")

_SCV_DEFERR(CONSTRAINT_WARNING_EQUAL_4_STATE,22,\
           "The 'X' and 'Z' values in signal '%s' will be converted to 0 to obtain a value for '%s' that will satisfy the constraints.",\
           _scv_message::WARNING,"none")

_SCV_DEFERR(CONSTRAINT_ERROR_NOTIMPLEMENTED,23,\
           "The '%s' facility is not implemented yet.",\
           _scv_message::ERROR,"none")

_SCV_DEFERR(CONSTRAINT_ERROR_OVER_CONSTRAINED,24,\
           "Constraints for over-constrained object '%s' will be ignored.",\
           _scv_message::ERROR,"none")

_SCV_DEFERR(CONSTRAINT_WARNING_IGNORE_SOFT_CONSTRAINT,25,\
           "Soft constraints for over-constrained  object '%s' will be ignored. \n",\
           _scv_message::WARNING,"none")

_SCV_DEFERR(CONSTRAINT_INVALID_SCAN,26,\
           "The scan interval for the avoid-duplicate mode of generator '%s' is invalid; \nhence, it will be ignored.",\
           _scv_message::ERROR,"short")

_SCV_DEFERR(CONSTRAINT_INVALID_DISTANCE,27,\
           "The distance for the avoid-duplicate mode of generator '%s' is invalid; \nhence, it will be ignored.",\
           _scv_message::ERROR,"short")

_SCV_DEFERR(CONSTRAINT_BAD_BAG,28,\
           "The distribution that was specified for '%s' has no legal value.\nThis problem was detected during %s.",\
           _scv_message::ERROR,"short")

_SCV_DEFERR(CONSTRAINT_EXPRESSION_TYPEMISMATCHED,29,\
           "A binary decision diagram for expression '%s' cannot be created. \nSubexpressions that have different types cannot be compared. ",\
           _scv_message::ERROR,"none")

_SCV_DEFERR(CONSTRAINT_INVALID_MODE,59,\
	  "Cannot set mode to distribution for constraint object '%s'. %s. The setting will be ignored.",\
	  _scv_message::ERROR,"short")

_SCV_DEFERR(CONSTRAINT_INVALID_RANGE,67,\
           "The upper bound of the constraint interval for smart_ptr '%s' is less than the lower bound. ",\
           _scv_message::ERROR,"none")

_SCV_DEFERR(CONSTRAINT_INVALID_COMBINATION_COMPLEX,60,\
	  "Cannot set complex constraints on an object having range constraints set using keep_only/keep_out. The constraint expression on scv_smart_ptr '%s' in constraint object %s will be ignored.",\
	  _scv_message::WARNING,"short")

_SCV_DEFERR(CONSTRAINT_INVALID_COMBINATION_RANGE,61,\
	  "Cannot set range constraints on an object having complex constraints. The constraint set using keep_only/keep_out on scv_smart_ptr '%s' in constraint object %s will be ignored.",\
	  _scv_message::WARNING,"short")

_SCV_DEFERR(CONSTRAINT_DISTRIBUTION_OVERWRITTEN,62,\
	  "Setting complex constraint expression on scv_smart_ptr %s in constraint object '%s'. The distribution setting will be saved. You can use set_mode to use distribution instead of complex constraints.",\
	  _scv_message::WARNING,"short")

_SCV_DEFERR(CONSTRAINT_TYPE_MISMATCH,63,\
	  "Objects must be of the same type in use_constraint. The scv_smart_ptr %s is of type %s, while scv_smart_ptr %s is of type %s. This use_constraint will be ignored.",\
	  _scv_message::ERROR,"short")

_SCV_DEFERR(CONSTRAINT_INVALID_USE_CONSTRAINT,64,\
	  "Cannot use use_constraint on scv_smart_ptr '%s' in constraint object '%s'.",\
	  _scv_message::ERROR,"short")

_SCV_DEFERR(CONSTRAINT_DEFAULT_CONSTRAINT,65,\
	  "Trying to set a constraint from an object with a default constraint will not have any effect. scv_smart_ptr %s in current mode has no constraint.",\
	  _scv_message::WARNING,"short")

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

