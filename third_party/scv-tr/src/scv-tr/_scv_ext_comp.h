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

  _scv_ext_comp.h -- A collection of general code to implement the
 introspection facility.  This file includes the files "_scv_ext_*.h".

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

// ----------------------------------------
// macro for common inlined check for _is_dynamic()
// ----------------------------------------
#define _SCV_CHECK_DYNAMIC(feature, return_statement)                                                                  \
    if(!this->_is_dynamic()) {                                                                                         \
        _scv_message::message(_scv_message::INTROSPECTION_INVALID_DYNAMIC_EXTENSIONS, #feature);                       \
        return_statement;                                                                                              \
    }

// ----------------------------------------
// error message
// ----------------------------------------
#define _SCV_RW_ERROR(feature, type, obj)                                                                              \
    _scv_message::message(_scv_message::INTROSPECTION_INVALID_READ_WRITE, #feature, #type, #obj, this->get_name())

// ----------------------------------------
// common data objects for dynamic extensions
// ----------------------------------------
#include <cstdio>

inline std::string _scv_ext_util_get_string(int i) {
    char tmp[128];
    std::sprintf(tmp, "%d", i);
    return tmp;
}

inline const char* _scv_ext_util_get_name(const char* format, const char* name, int N) {
    static char tmp[1024];
    std::sprintf(tmp, format, name, N);
    return strdup(tmp);
}

inline const char* _scv_ext_util_get_name(const char* format, const char* name) {
    static char tmp[1024];
    std::sprintf(tmp, format, name);
    return strdup(tmp);
}

// ----------------------------------------
// others
// ----------------------------------------
#if defined(SYSTEMC_INCLUDED) || defined(IEEE_1666_SYSTEMC)
#define _SCV_SYSTEMC_BASIC_TYPE_SPECIALIZATION()                                                                       \
    _SCV_COMPONENT_1(sc_dt::sc_bit);                                                                                   \
    _SCV_COMPONENT_1(sc_dt::sc_logic);                                                                                 \
    _SCV_COMPONENT_N(sc_dt::sc_int);                                                                                   \
    _SCV_COMPONENT_N(sc_dt::sc_uint);                                                                                  \
    _SCV_COMPONENT_N(sc_dt::sc_bigint);                                                                                \
    _SCV_COMPONENT_N(sc_dt::sc_biguint);                                                                               \
    _SCV_COMPONENT_N(sc_dt::sc_bv);                                                                                    \
    _SCV_COMPONENT_N(sc_dt::sc_lv);                                                                                    \
    _SCV_COMPONENT(sc_dt::sc_signed);                                                                                  \
    _SCV_COMPONENT(sc_dt::sc_unsigned);                                                                                \
    _SCV_COMPONENT(sc_dt::sc_int_base);                                                                                \
    _SCV_COMPONENT(sc_dt::sc_uint_base);                                                                               \
    _SCV_COMPONENT(sc_dt::sc_lv_base);                                                                                 \
    _SCV_COMPONENT(sc_dt::sc_bv_base);

// SCV_COMPONENT_N(tag,sc_fixed);
// SCV_COMPONENT_N(tag,sc_ufixed);

#else
#define _SCV_SYSTEMC_BASIC_TYPE_SPECIALIZATION()
#endif

#define _SCV_BASIC_TYPE_SPECIALIZATION()                                                                               \
    _SCV_COMPONENT(bool);                                                                                              \
    _SCV_COMPONENT(char);                                                                                              \
    _SCV_COMPONENT(unsigned char);                                                                                     \
    _SCV_COMPONENT(short);                                                                                             \
    _SCV_COMPONENT(unsigned short);                                                                                    \
    _SCV_COMPONENT(int);                                                                                               \
    _SCV_COMPONENT(unsigned int);                                                                                      \
    _SCV_COMPONENT(long);                                                                                              \
    _SCV_COMPONENT(unsigned long);                                                                                     \
    _SCV_COMPONENT(long long);                                                                                         \
    _SCV_COMPONENT(unsigned long long);                                                                                \
    _SCV_COMPONENT(float);                                                                                             \
    _SCV_COMPONENT(double);                                                                                            \
    _SCV_COMPONENT(std::string);                                                                                       \
    _SCV_COMPONENT_N(test_uint);                                                                                       \
    _SCV_SYSTEMC_BASIC_TYPE_SPECIALIZATION()

// implementation details
// clang-format off
#include "_scv_ext_util.h"
#include "_scv_ext_type.h"
#include "_scv_ext_rw.h"
// clang-format on

#undef _SCV_SYSTEMC_BASIC_TYPE_SPECIALIZATION
#undef _SCV_BASIC_TYPE_SPECIALIZATION
