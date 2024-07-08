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
 * \file   cci_meta.h
 * \author Philipp A. Hartmann, Intel
 * \brief  Some template meta-programming helpers
 *
 * \note Only \ref cci::cci_tag<T> et.al. are part of the standard.
 */
#ifndef CCI_CORE_CCI_META_H_INCLUDED_
#define CCI_CORE_CCI_META_H_INCLUDED_

#include "cci/core/systemc.h"
#include "cci/core/cci_cmnhdr.h"

# include <type_traits> // C++11

CCI_OPEN_NAMESPACE_

/// Typed tag class
template<typename T> struct cci_tag { typedef T type; };

/// Explicitly typed tag class
template<typename T> struct cci_typed_tag : cci_tag<T> {};

/// Tag alias for an untyped tag
typedef cci_tag<void> cci_untyped_tag;

//@cond CCI_HIDDEN_FROM_DOXYGEN
namespace cci_impl {

// Note: C++11 minimal required version, should be enforced by build scripts
using std::enable_if;
using std::integral_constant;
using std::true_type;
using std::false_type;
using std::is_same;
using std::remove_reference;

/// C++03 implementation of std::void_t (from C++17)
template<typename T> struct always_void { typedef void type; };

} // namespace cci_impl
//@endcond

/// Tag type to represent a boolean @c true value
typedef cci_impl::true_type  cci_true_type;
/// Tag type to represent a boolean @c false value
typedef cci_impl::false_type cci_false_type;

CCI_CLOSE_NAMESPACE_

#endif // CCI_CORE_CCI_META_H_INCLUDED_
