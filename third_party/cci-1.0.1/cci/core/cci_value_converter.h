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
#ifdef _MSC_VER // deliberately outside of #include guards
#pragma warning(push)
#pragma warning(disable: 4231) // extern template
#endif // _MSC_VER

#define CCI_VALUE_HAS_CONVERTER_(Type) \
  template<> struct cci_value_has_converter<Type> \
    : cci_impl::true_type {}

#ifndef CCI_CCI_VALUE_CONVERTER_H_INCLUDED_
#define CCI_CCI_VALUE_CONVERTER_H_INCLUDED_
/**
 * @file   cci_value_converter.h
 * @brief  conversions from and to a @ref cci::cci_value
 * @author Philipp A. Hartmann, OFFIS/Intel
 */

#include "cci/core/cci_cmnhdr.h"
#include "cci/core/cci_meta.h"
#include "cci/core/cci_value.h"

#include <cstring> // std::strncpy

CCI_OPEN_NAMESPACE_

/**
 * @brief opt-in for @ref cci_value_converter conversion
 *
 * To use the cci_value_converter based type conversion
 * for custom types to and from cci_value, specialize
 * this class via
 * @code
 * class my_type; // can be incomplete
 * namespace cci {
 *   template<> struct cci_value_has_converter<my_type>
 *     : cci::cci_true_type {};
 * } // namespace cci
 * @endcode
 *
 * @see cci_value_converter
 */
template<typename T> struct cci_value_has_converter : cci_false_type {};

/**
 * @class cci_value_converter
 * @brief traits class for cci_value conversions
 * @tparam T C++ datatype to convert to/from @ref cci_value
 *
 * Whenever a C++ type @c T is used in conjunction with a cci_value,
 * the requireed value conversion is performed by this traits class,
 * providing the two conversion functions @ref pack and @ref unpack.
 * Both functions return @c true upon success and @c false otherwise.
 * In case of a failing conversion, it is recommended to leave the given
 * destination object @c dst untouched.
 *
 * @note By default, the primary template is not implemented to
 *       enable instantiations with incomplete types.
 *
 * @note To enable type conversion via the cci_value_converter class,
 *       an opt-in via a specialization of @ref cci_value_has_converter
 *       is required.
 *
 * You need to implement the two functions @ref pack / @ref unpack
 * to enable conversion support for your custom datatype:
 * @code
 * struct my_int { int value; };
 *
 * namespace cci {
 * template<> struct cci_value_has_converter<my_int> : cci_true_type {};
 * template<> bool
 * cci_value_converter<my_int>::pack( cci_value::reference dst, type const & src )
 * {
 *    dst.set_int( src.value );
 *    return true;
 * }
 * template<> bool
 * cci_value_converter<my_int>::unpack( type & dst, cci_value::const_reference src )
 * {
 *    if( ! src.is_int() ) return false;
 *    dst.value  = src.get_int();
 *    return true;
 * }
 * } // namespace cci
 * @endcode
 *
 * To @em disable conversion support for a given type, you can refer
 * to the helper template @ref cci_value_converter_disabled.
 *
 * <b> Predefined converters </b>
 * @li C++ fundamental types:
 *     @c bool, @c (unsigned) @c char, @c (unsigned) @c short,
 *     @c (unsigned) @c int, @c (unsigned) @c long,
 *     @c float, @c double
 * @li @c std::string
 * @li SystemC data types:
 *     @c sc_dt::int64, @c sc_dt::uint64, @c sc_dt::sc_logic,
 *     @c sc_dt::sc_bv_base, @c sc_dt::sc_lv_base,
 *     @c sc_dt::sc_int_base, @c sc_dt::sc_uint_base,
 *     @c sc_dt::sc_signed, @c sc_dt::sc_unsigned,
 *     @c sc_dt::sc_fxval, @c sc_dt::sc_fxval_fast,
 *     @c sc_dt::sc_fix, @c sc_dt::sc_fix_fast,
 *     @c sc_dt::sc_ufix, @c sc_dt::sc_ufix_fast,
 *     and their templated variants
 * @li SystemC time (@c sc_core::sc_time)
 * @li Fixed-size C++ arrays and @c std::vector<T> of supported types
 */
template<typename T>
struct cci_value_converter
{
  typedef T type; ///< common type alias
  /// (optional) boolean tag to enable templated cci_value functions
  static const bool enabled = cci_value_has_converter<type>::value;
  /// convert from \ref type value to a cci_value
  static bool pack( cci_value::reference dst, type const & src );
  /// convert from cci_value to a \ref type value
  static bool unpack( type & dst, cci_value::const_reference src );
};

// ---------------------------------------------------------------------------
/**
 * @brief helper to disable cci_value conversion for a given type
 * @tparam T type without cci_value conversions
 *
 * In order to disable the conversion from/to a cci_value for a given type
 * @c T during @em run-time, you can simply inherit from this helper in
 * the specialization of cci_value_converter:
 * @code
 * struct my_type { ... };
 * template<>
 * struct cci_value_converter<my_type>
 *   : cci_value_converter_disabled<my_type> {};
 * @endcode
 *
 * @note In order to disable support for a given type at @em compile-time,
 *       the specialization of cci_value_converter can be left empty.
 */
template< typename T >
struct cci_value_converter_disabled
{
  typedef T type;
  static const bool enabled = true;
  static bool pack( cci_value::reference, T const & )      { return false; }
  static bool unpack( type &, cci_value::const_reference ) { return false; }
};

//@cond CCI_HIDDEN_FROM_DOXYGEN
namespace cci_impl {

// primary template - matches, if specialization does not contain "enabled" member
template<typename ConvT, typename = void >
struct value_converter_is_enabled : true_type {};

// specialization to check enabled member of cci_value_converter template
template<typename ConvT>
struct value_converter_is_enabled
< ConvT,
  typename always_void< integral_constant<bool, ConvT::enabled> >::type
> : integral_constant<bool, ConvT::enabled > {};

template<typename T, typename R>
struct value_converter_enable_if
  : enable_if< value_converter_is_enabled< cci_value_converter<T> >::value, R >
{};

} // namespace cci_impl
//@endcond
CCI_CLOSE_NAMESPACE_

///@cond CCI_HIDDEN_FROM_DOXYGEN
CCI_OPEN_NAMESPACE_

// ---------------------------------------------------------------------------
/// helper to convert compatible types (implementation artefact)
template< typename T, typename U >
struct cci_value_delegate_converter
{
  typedef T type;
  typedef cci_value_converter<U> traits_type;

  static bool pack( cci_value::reference dst, type const & src )
  {
    return traits_type::pack( dst, static_cast<U>(src) );
  }

  static bool unpack( type & dst, cci_value::const_reference src )
  {
      U u_dst;
      bool ret = traits_type::unpack( u_dst, src );
      if( ret )
        dst = static_cast<T>(u_dst);
      return ret;
  }
};

// --------------------------------------------------------------------------
// C++ builtin types
//
CCI_VALUE_HAS_CONVERTER_(bool);
CCI_VALUE_HAS_CONVERTER_(int);
CCI_VALUE_HAS_CONVERTER_(int64);
CCI_VALUE_HAS_CONVERTER_(unsigned);
CCI_VALUE_HAS_CONVERTER_(uint64);
CCI_VALUE_HAS_CONVERTER_(double);
CCI_VALUE_HAS_CONVERTER_(std::string);

#ifndef CCI_VALUE_BUILD // defined in cci_value_converter.cpp
CCI_TPLEXTERN_ template struct cci_value_converter<bool>;
CCI_TPLEXTERN_ template struct cci_value_converter<int>;
CCI_TPLEXTERN_ template struct cci_value_converter<int64>;
CCI_TPLEXTERN_ template struct cci_value_converter<unsigned>;
CCI_TPLEXTERN_ template struct cci_value_converter<uint64>;
CCI_TPLEXTERN_ template struct cci_value_converter<double>;
CCI_TPLEXTERN_ template struct cci_value_converter<std::string>;
#endif // CCI_VALUE_BUILD

// related numerical types
// (without range checks for now)

#define CCI_VALUE_CONVERTER_DERIVED_( UnderlyingType, SpecializedType ) \
  template<> \
  struct cci_value_converter<SpecializedType> \
    : cci_value_delegate_converter<SpecializedType, UnderlyingType > {}

CCI_VALUE_CONVERTER_DERIVED_( int, char );
CCI_VALUE_CONVERTER_DERIVED_( int, signed char );
CCI_VALUE_CONVERTER_DERIVED_( int, short );
CCI_VALUE_CONVERTER_DERIVED_( unsigned, unsigned char );
CCI_VALUE_CONVERTER_DERIVED_( unsigned, unsigned short );
CCI_VALUE_CONVERTER_DERIVED_( int64, long );
CCI_VALUE_CONVERTER_DERIVED_( uint64, unsigned long );
CCI_VALUE_CONVERTER_DERIVED_( double, float );

#undef CCI_VALUE_CONVERTER_DERIVED_
///@endcond

// ----------------------------------------------------------------------------
// C++ string literals

template<int N>
struct cci_value_converter<char[N]>
{
  typedef char type[N]; ///< common type alias
  static const bool enabled = true;

  static bool pack( cci_value::reference dst, type const & src )
  {
    dst.set_string( src );
    return true;
  }
  static bool unpack( type & dst, cci_value::const_reference src )
  {
    if( src.is_null() )
    {
      dst[0] = '\0'; // convert "null" to empty string
      return true;
    }
    if( !src.is_string() )
      return false;

    cci_value::const_string_reference str = src.get_string();
    std::strncpy( dst, str.c_str(), N-1 );
    dst[N-1] = '\0';
    return ( str.size() <= N-1 );
  }
};

// ----------------------------------------------------------------------------
// C++ arrays

template<typename T, int N>
struct cci_value_converter<T[N]>
{
  typedef T type[N]; ///< common type alias
  static const bool enabled = true;

  static bool pack( cci_value::reference dst, type const & src )
  {
    cci_value_list ret;
    ret.reserve( N );

    for( size_t i = 0; i < N; ++i )
      ret.push_back( src[i] );
    ret.swap( dst.set_list() );
    return true;
  }
  static bool unpack( type & dst, cci_value::const_reference src )
  {
    if( !src.is_list() )
      return false;

    cci_value::const_list_reference lst = src.get_list();
    size_t i = 0;
    for( ; i < N && i < lst.size() && lst[i].try_get<T>( dst[i] ); ++i ) {}

    return ( i == lst.size() );
  }
};

template<typename T, int N>
struct cci_value_converter<const T[N]> : cci_value_converter<T[N]>
{
  typedef const T type[N]; ///< common type alias
  // deliberately not implemented
  static bool unpack( type & dst, cci_value::const_reference src );
};

// ----------------------------------------------------------------------------
// std::vector<T, Alloc>

template< typename T, typename Alloc >
struct cci_value_converter< std::vector<T,Alloc> >
{
  typedef std::vector<T,Alloc> type; ///< common type alias
  static const bool enabled = true;

  static bool pack( cci_value::reference dst, type const & src )
  {
    cci_value_list ret;
    ret.reserve( src.size() );

    for( size_t i = 0; i < src.size(); ++i )
      ret.push_back( src[i] );
    ret.swap( dst.set_list() );
    return true;
  }
  static bool unpack( type & dst, cci_value::const_reference src )
  {
    if( !src.is_list() )
      return false;

    cci_value::const_list_reference lst = src.get_list();
    type ret;
    T    cur;
    size_t i = 0;
    ret.reserve( lst.size() );
    for( ; i < lst.size() && lst[i].try_get(cur); ++i )
      ret.push_back( cur );

    return ( i == lst.size() ) ? ( dst.swap(ret), true) : false;
  }
};

// ----------------------------------------------------------------------------
// SystemC builtin types

CCI_VALUE_HAS_CONVERTER_(sc_core::sc_time);
CCI_VALUE_HAS_CONVERTER_(sc_dt::sc_bit);
CCI_VALUE_HAS_CONVERTER_(sc_dt::sc_logic);
CCI_VALUE_HAS_CONVERTER_(sc_dt::sc_int_base);
CCI_VALUE_HAS_CONVERTER_(sc_dt::sc_uint_base);
CCI_VALUE_HAS_CONVERTER_(sc_dt::sc_signed);
CCI_VALUE_HAS_CONVERTER_(sc_dt::sc_unsigned);
CCI_VALUE_HAS_CONVERTER_(sc_dt::sc_bv_base);
CCI_VALUE_HAS_CONVERTER_(sc_dt::sc_lv_base);

#ifndef CCI_VALUE_BUILD // defined in cci_value_converter.cpp
CCI_TPLEXTERN_ template struct cci_value_converter<sc_core::sc_time>;
CCI_TPLEXTERN_ template struct cci_value_converter<sc_dt::sc_bit>;
CCI_TPLEXTERN_ template struct cci_value_converter<sc_dt::sc_logic>;
CCI_TPLEXTERN_ template struct cci_value_converter<sc_dt::sc_int_base>;
CCI_TPLEXTERN_ template struct cci_value_converter<sc_dt::sc_uint_base>;
CCI_TPLEXTERN_ template struct cci_value_converter<sc_dt::sc_signed>;
CCI_TPLEXTERN_ template struct cci_value_converter<sc_dt::sc_unsigned>;
CCI_TPLEXTERN_ template struct cci_value_converter<sc_dt::sc_bv_base>;
CCI_TPLEXTERN_ template struct cci_value_converter<sc_dt::sc_lv_base>;
#endif // CCI_VALUE_BUILD

/// @see cci_value_converter primary template
template<int N>
struct cci_value_converter< sc_dt::sc_int<N> >
  : cci_value_converter< sc_dt::sc_int_base >
{
  typedef sc_dt::sc_int<N> type;
};

/// @see cci_value_converter primary template
template<int N>
struct cci_value_converter< sc_dt::sc_uint<N> >
  : cci_value_converter< sc_dt::sc_uint_base >
{
  typedef sc_dt::sc_uint<N> type;
};

/// @see cci_value_converter primary template
template<int N>
struct cci_value_converter< sc_dt::sc_bigint<N> >
  : cci_value_converter< sc_dt::sc_signed >
{
  typedef sc_dt::sc_bigint<N> type;
};

/// @see cci_value_converter primary template
template<int N>
struct cci_value_converter< sc_dt::sc_biguint<N> >
  : cci_value_converter< sc_dt::sc_unsigned >
{
  typedef sc_dt::sc_biguint<N> type;
};

/// @see cci_value_converter primary template
template<int N>
struct cci_value_converter< sc_dt::sc_bv<N> >
  : cci_value_converter< sc_dt::sc_bv_base >
{
  typedef sc_dt::sc_bv<N> type;
};

/// @see cci_value_converter primary template
template<int N>
struct cci_value_converter< sc_dt::sc_lv<N> >
  : cci_value_converter< sc_dt::sc_lv_base >
{
  typedef sc_dt::sc_lv<N> type;
};

CCI_CLOSE_NAMESPACE_
#endif // CCI_CCI_VALUE_CONVERTER_H_INCLUDED_

///@todo add support for SystemC fixpoint types
#if defined(SC_INCLUDE_FX) && !defined(CCI_CNF_CCI_VALUE_CONVERTER_H_INCLUDED_FX_)
#define CCI_CNF_CCI_VALUE_CONVERTER_H_INCLUDED_FX_

CCI_OPEN_NAMESPACE_

CCI_VALUE_HAS_CONVERTER_(sc_dt::sc_fxval);
CCI_VALUE_HAS_CONVERTER_(sc_dt::sc_fxval_fast);
CCI_VALUE_HAS_CONVERTER_(sc_dt::sc_fix);
CCI_VALUE_HAS_CONVERTER_(sc_dt::sc_fix_fast);
CCI_VALUE_HAS_CONVERTER_(sc_dt::sc_ufix);
CCI_VALUE_HAS_CONVERTER_(sc_dt::sc_ufix_fast);

#ifndef CCI_VALUE_BUILD // defined in cci_value_converter.cpp
CCI_TPLEXTERN_ template struct cci_value_converter<sc_dt::sc_fxval>;
CCI_TPLEXTERN_ template struct cci_value_converter<sc_dt::sc_fxval_fast>;
CCI_TPLEXTERN_ template struct cci_value_converter<sc_dt::sc_fix>;
CCI_TPLEXTERN_ template struct cci_value_converter<sc_dt::sc_fix_fast>;
CCI_TPLEXTERN_ template struct cci_value_converter<sc_dt::sc_ufix>;
CCI_TPLEXTERN_ template struct cci_value_converter<sc_dt::sc_ufix_fast>;
#endif // CCI_VALUE_BUILD

template<int W, int I, sc_dt::sc_q_mode Q, sc_dt::sc_o_mode O, int N >
struct cci_value_converter< sc_dt::sc_fixed<W,I,Q,O,N> >
  : cci_value_converter< sc_dt::sc_fix >
{
  typedef sc_dt::sc_fixed<W,I,Q,O,N> type;
};

template<int W, int I, sc_dt::sc_q_mode Q, sc_dt::sc_o_mode O, int N >
struct cci_value_converter< sc_dt::sc_fixed_fast<W,I,Q,O,N> >
  : cci_value_converter< sc_dt::sc_fix_fast >
{
  typedef sc_dt::sc_fixed_fast<W,I,Q,O,N> type;
};

template<int W, int I, sc_dt::sc_q_mode Q, sc_dt::sc_o_mode O, int N >
struct cci_value_converter< sc_dt::sc_ufixed<W,I,Q,O,N> >
  : cci_value_converter< sc_dt::sc_ufix >
{
  typedef sc_dt::sc_ufixed<W,I,Q,O,N> type;
};

template<int W, int I, sc_dt::sc_q_mode Q, sc_dt::sc_o_mode O, int N >
struct cci_value_converter< sc_dt::sc_ufixed_fast<W,I,Q,O,N> >
  : cci_value_converter< sc_dt::sc_ufix_fast >
{
  typedef sc_dt::sc_ufixed_fast<W,I,Q,O,N> type;
};

CCI_CLOSE_NAMESPACE_

#endif // SC_INCLUDE_FX && ! CCI_CNF_CCI_VALUE_CONVERTER_H_INCLUDED_FX_

#undef CCI_VALUE_HAS_CONVERTER_

#ifdef _MSC_VER
#pragma warning(pop)
#endif // _MSC_VER
