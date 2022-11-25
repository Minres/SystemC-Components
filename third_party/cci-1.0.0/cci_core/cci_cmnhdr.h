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

/**
 * \file cci_cmnhdr.h
 * \brief Some platform helpers other implementation-defined parts
 *
 * @author Enrico Galli, Intel
 * @author Philipp A. Hartmann, Intel
 *
 * \note The contents of this file are entirely implementation-defined.
 */
#ifndef CCI_CORE_CCI_CMNHDR_H_INCLUDED_
#define CCI_CORE_CCI_CMNHDR_H_INCLUDED_

#include "cci_core/systemc.h"

/**
 * @def CCI_NAMESPACE
 * @brief Main CCI namespace (overridable)
 * @hideinitializer
 * @see cci
 */
#ifndef CCI_NAMESPACE
# define CCI_NAMESPACE cci
#endif // CCI_NAMESPACE

/// Opening a CCI namespace block (internal)
# define CCI_OPEN_NAMESPACE_  namespace CCI_NAMESPACE {
/// Closing a CCI namespace block (internal)
# define CCI_CLOSE_NAMESPACE_ } /* namespace cci */

/* ------------------------------------------------------------------------ */

/** @namespace cci
 *  @brief Accellera Configuration, Control & Inspection (CCI) standard
 */

/** @namespace cci_utils
 *  @brief CCI utilities (not part of the interoperability standard)
 */

/* ------------------------------------------------------------------------ */

/**
 * @def CCI_CPLUSPLUS
 * @brief Selected C++ standard baseline
 * @hideinitializer
 *
 * This macro can be used inside the library sources to make certain assumptions
 * on the available features in the underlying C++ implementation.
 *
 * Supported values are
 *  @li @c 199711L (C++03, ISO/IEC 14882:1998, 14882:2003)
 *  @li @c 201103L (C++11, ISO/IEC 14882:2011)
 *  @li @c 201402L (C++14, ISO/IEC 14882:2014)
 *  @li @c 201703L (C++17, ISO/IEC 14882:2017)
 *
 * Starting with SystemC 2.3.2, this value should match the @c SC_CPLUSPLUS
 * macro in order to build compatible models.
 */
#ifndef CCI_CPLUSPLUS
# ifdef SC_CPLUSPLUS // as defined by SystemC >= 2.3.2
#   define CCI_CPLUSPLUS SC_CPLUSPLUS
# else // copy deduction from SystemC 2.3.2

#   ifdef _MSC_VER // don't rely on __cplusplus for MSVC
    // Instead, we select the C++ standard with reasonable support.
    // If some features still need to be excluded on specific MSVC
    // versions, we'll do so at the point of definition.

#     if defined(_MSVC_LANG) // MSVC'2015 Update 3 or later, use compiler setting
#       define CCI_CPLUSPLUS _MSVC_LANG
#     elif _MSC_VER < 1800   // MSVC'2010 and earlier, assume C++03
#       define CCI_CPLUSPLUS 199711L
#     elif _MSC_VER < 1900   // MSVC'2013, assume C++11
#       define CCI_CPLUSPLUS 201103L
#     else                   // MSVC'2015 before Update 3, assume C++14
#       define CCI_CPLUSPLUS 201402L
#     endif
#   else // not _MSC_VER
      // use compiler setting
#     define CCI_CPLUSPLUS __cplusplus
#   endif // not _MSC_VER

# endif // not SC_CPLUSPLUS
#endif // CCI_CPLUSPLUS

/* ------------------------------------------------------------------------ */

// Macros to check if certain C++ features are supported
#ifndef __has_feature        // Optional of course.
# define __has_feature(x) 0  // Compatibility with non-clang compilers.
#endif
#ifndef __has_extension
# define __has_extension __has_feature // Compatibility with pre-3.0 compilers.
#endif

/* ------------------------------------------------------------------------ */
// Support for C++ rvalue-references / perfect forwarding

#ifndef CCI_HAS_CXX_RVALUE_REFS
# if CCI_CPLUSPLUS >= 201103L
#	 define CCI_HAS_CXX_RVALUE_REFS
# endif
#endif // detect: CCI_HAS_CXX_RVALUE_REFS

#ifdef CCI_HAS_CXX_RVALUE_REFS
# include <utility>
# define CCI_MOVE_(Obj)::std::move(Obj)
# define CCI_FORWARD_(Type,Obj) ::std::forward<Type>(Obj)
#else
# define CCI_MOVE_(Obj) Obj
# define CCI_FORWARD_(Type,Obj) Obj
#endif // CCI_HAS_CXX_RVALUE_REFS

/* ------------------------------------------------------------------------ */
// Extern templates (supported by all major compilers even before C++11)

#ifndef CCI_TPLEXTERN_
# if defined(__GNUC__) && CCI_CPLUSPLUS < 201103L
#   define CCI_TPLEXTERN_ __extension__ extern
# else
#   define CCI_TPLEXTERN_ extern
# endif
#endif // CCI_TPLEXTERN_

#endif // CCI_CORE_CCI_CMNHDR_H_INCLUDED_
