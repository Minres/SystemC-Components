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

#include "../../../../scc/third_party/cci-1.0.0/cci_core/cci_cmnhdr.h"
#include "../../../../scc/third_party/cci-1.0.0/cci_core/systemc.h"

#if CCI_CPLUSPLUS >= 201103L
# include <type_traits>
#endif

CCI_OPEN_NAMESPACE_

/// Typed tag class
template<typename T> struct cci_tag { typedef T type; };

/// Explicitly typed tag class
template<typename T> struct cci_typed_tag : cci_tag<T> {};

/// Tag alias for an untyped tag
typedef cci_tag<void> cci_untyped_tag;

///@cond CCI_HIDDEN_FROM_DOXYGEN
namespace cci_impl {

#if CCI_CPLUSPLUS >= 201103L
using std::enable_if;
using std::integral_constant;
using std::true_type;
using std::false_type;
using std::is_same;
using std::remove_reference;

#else // use Boost/local implementation for type traits

template<bool Cond, typename T = void>
struct enable_if : sc_boost::enable_if_c<Cond, T> {};

/// A type encoding for an integral value
template<typename IntegralType, IntegralType V> struct integral_constant
{
  typedef IntegralType value_type;
  typedef integral_constant type;
  static const value_type value = V;
};

typedef integral_constant<bool,true>  true_type;
typedef integral_constant<bool,false> false_type;

template<typename T, typename U> struct is_same : false_type {};
template<typename T> struct is_same<T,T> : true_type {};

template<typename T> struct remove_reference { typedef T type; };
template<typename T> struct remove_reference<T&> { typedef T type; };

#endif // CCI_CPLUSPLUS >= 201103L

/// C++03 implementation of std::void_t (from C++17)
template<typename T> struct always_void { typedef void type; };

} // namespace cci_impl
///@endcond
CCI_CLOSE_NAMESPACE_

#endif // CCI_CORE_CCI_META_H_INCLUDED_
