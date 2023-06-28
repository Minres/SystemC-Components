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

#ifndef CCI_CFG_CCI_PARAM_CALLBACKS_H_INCLUDED_
#define CCI_CFG_CCI_PARAM_CALLBACKS_H_INCLUDED_

#include "cci_core/cci_callback.h"
#include "cci_core/cci_value.h"
#include "cci_core/cci_value_converter.h"

CCI_OPEN_NAMESPACE_

template<typename T = void>
struct cci_param_write_event;

/// Forward declaration
class cci_param_untyped_handle;
class cci_originator;

/// Payload for type-independent pre write and post write callbacks
template<>
struct cci_param_write_event<void>
{
  typedef cci_param_write_event type;
  typedef cci_value value_type;

  cci_param_write_event( const value_type&     old_
                       , const value_type&     new_
                       , const cci_originator& orig_
                       , const cci_param_untyped_handle& handle_ );

  /// Old parameter value
  const value_type& old_value;
  /// New parameter value
  const value_type& new_value;
  /// Originator of new value
  const cci_originator& originator;
  /// Parameter handle
  const cci_param_untyped_handle& param_handle;
};

/// Payload for pre write and post write callbacks
template<typename T>
struct cci_param_write_event
{
  typedef cci_param_write_event type;
  typedef T value_type;

  cci_param_write_event( const value_type&     old_
                       , const value_type&     new_
                       , const cci_originator& orig_
                       , const cci_param_untyped_handle& handle_ );
  /// Old parameter value
  const value_type& old_value;
  /// New parameter value
  const value_type& new_value;
  /// Originator of new value
  const cci_originator& originator;
  /// Parameter handle
  const cci_param_untyped_handle& param_handle;

  /// Type-punned specialization
  typedef const cci_param_write_event<>& generic_type;

  /// Conversion to a generic (type independent) event
  struct generic_wrap
  {
    generic_type get() const { return wrapped_value; }
    explicit generic_wrap(const type& payload);

    cci_value old_value;
    cci_value new_value;
    typename cci_impl::remove_reference<generic_type>::type wrapped_value;
  }; // struct generic_wrap

}; // cci_param_write_event

template<typename T = void>
struct cci_param_read_event;

/// Payload for type-independent read callbacks
template<>
struct cci_param_read_event<void>
{
    typedef cci_param_read_event type;
    typedef cci_value value_type;

    cci_param_read_event( const value_type&     val_
                        , const cci_originator& orig_
                        , const cci_param_untyped_handle& handle_ );

    /// Parameter value
    const value_type& value;
    /// Originator of new value
    const cci_originator& originator;
    /// Parameter handle
    const cci_param_untyped_handle& param_handle;
};

/// Payload for read callbacks
template<typename T>
struct cci_param_read_event
{
    typedef cci_param_read_event type;
    typedef T value_type;

    cci_param_read_event( const value_type&     val_
                        , const cci_originator& orig_
                        , const cci_param_untyped_handle& handle_ );

    /// Parameter value
    const value_type& value;
    /// Originator of new value
    const cci_originator& originator;
    /// Parameter handle
    const cci_param_untyped_handle& param_handle;

    /// Type-punned specialization
    typedef const cci_param_read_event<>& generic_type;

    /// Conversion to a generic (type independent) event
    struct generic_wrap
    {
        generic_type get() const { return wrapped_value; }
        explicit generic_wrap(const type& payload);

        cci_value value;
        typename cci_impl::remove_reference<generic_type>::type wrapped_value;
    }; // struct generic_wrap

}; // cci_param_read_event

/* ------------------------------------------------------------------------ */

#if CCI_CPLUSPLUS >= 201103L

/// Parameter pre write callback
template <typename T = void>
using cci_param_pre_write_callback
  = cci_callback<const cci_param_write_event<T>&, bool>;

/// Parameter pre write callback handle
template <typename T = void>
using cci_param_pre_write_callback_handle
  = cci_callback_typed_handle<const cci_param_write_event<T>&, bool>;

/// Parameter post write callback
template <typename T = void>
using cci_param_post_write_callback
  = cci_callback<const cci_param_write_event<T>&>;

/// Parameter post write callback handle
template <typename T = void>
using cci_param_post_write_callback_handle
  = cci_callback_typed_handle<const cci_param_write_event<T>&>;

/// Parameter pre read callback
template <typename T = void>
using cci_param_pre_read_callback
  = cci_callback<const cci_param_read_event<T>&>;

/// Parameter pre read callback handle
template <typename T = void>
using cci_param_pre_read_callback_handle
  = cci_callback_typed_handle<const cci_param_read_event<T>&>;

/// Parameter post read callback
template <typename T = void>
using cci_param_post_read_callback
  = cci_callback<const cci_param_read_event<T>&>;

/// Parameter post read callback handle
template <typename T = void>
using cci_param_post_read_callback_handle
  = cci_callback_typed_handle<const cci_param_read_event<T>&>;

#else // CCI_CPLUSPLUS >= 201103L (no alias templates)

/// Parameter pre write callback
template <typename T = void>
struct cci_param_pre_write_callback
        : cci_callback<const cci_param_write_event<T>&, bool> {};

/// Parameter pre write callback handle
template <typename T = void>
struct cci_param_pre_write_callback_handle
        : cci_callback_typed_handle<const cci_param_write_event<T>&, bool> {};

/// Parameter post write callback
template <typename T = void>
struct cci_param_post_write_callback
  : cci_callback<const cci_param_write_event<T>& > {};

/// Parameter post write callback handle
template <typename T = void>
struct cci_param_post_write_callback_handle
  : cci_callback_typed_handle<const cci_param_write_event<T>&> {};

/// Parameter pre read callback
template <typename T = void>
struct cci_param_pre_read_callback
        : cci_callback<const cci_param_read_event<T>& > {};

/// Parameter pre read callback handle
template <typename T = void>
struct cci_param_pre_read_callback_handle
        : cci_callback_typed_handle<const cci_param_read_event<T>&> {};

/// Parameter post read callback
template <typename T = void>
struct cci_param_post_read_callback
        : cci_callback<const cci_param_read_event<T>& > {};

/// Parameter post read callback handle
template <typename T = void>
struct cci_param_post_read_callback_handle
        : cci_callback_typed_handle<const cci_param_read_event<T>&> {};

#endif // CCI_CPLUSPLUS >= 201103L

/// Untyped parameter write event
typedef cci_param_write_event<>::type
        cci_param_write_event_untyped;

/// Untyped parameter pre write callback
typedef cci_param_pre_write_callback<>::type
  cci_param_pre_write_callback_untyped;

/// Untyped parameter post write callback
typedef cci_param_post_write_callback<>::type
        cci_param_post_write_callback_untyped;

/// Untyped parameter read event
typedef cci_param_read_event<>::type
        cci_param_read_event_untyped;

/// Untyped parameter pre read callback
typedef cci_param_pre_read_callback<>::type
        cci_param_pre_read_callback_untyped;

/// Untyped parameter post read callback
typedef cci_param_post_read_callback<>::type
        cci_param_post_read_callback_untyped;

/* ------------------------------------------------------------------------ */

/// Callback API of CCI parameter implementations
struct cci_param_callback_if
{
  friend class cci_param_untyped_handle;

  virtual bool has_callbacks() const = 0;

protected:
  virtual cci_callback_untyped_handle
  register_pre_write_callback( const cci_callback_untyped_handle& cb
                             , const cci_originator& orig ) = 0;
  virtual bool
  unregister_pre_write_callback( const cci_callback_untyped_handle& cb
                               , const cci_originator& orig ) = 0;

  virtual cci_callback_untyped_handle
  register_post_write_callback( const cci_callback_untyped_handle& cb
                              , const cci_originator& orig ) = 0;
  virtual bool
  unregister_post_write_callback( const cci_callback_untyped_handle& cb
                                , const cci_originator& orig ) = 0;

  virtual cci_callback_untyped_handle
  register_pre_read_callback( const cci_callback_untyped_handle& cb
                            , const cci_originator& orig ) = 0;
  virtual bool
  unregister_pre_read_callback( const cci_callback_untyped_handle& cb
                              , const cci_originator& orig ) = 0;

  virtual cci_callback_untyped_handle
  register_post_read_callback( const cci_callback_untyped_handle& cb
                             , const cci_originator& orig ) = 0;
  virtual bool
  unregister_post_read_callback( const cci_callback_untyped_handle& cb
                               , const cci_originator& orig ) = 0;

  virtual bool unregister_all_callbacks(const cci_originator& orig) = 0;
};

/* ------------------------------------------------------------------------ */

inline
cci_param_write_event<void>::
  cci_param_write_event( const value_type&     old_
                       , const value_type&     new_
                       , const cci_originator& orig_
                       , const cci_param_untyped_handle& handle_ )
    : old_value(old_)
    , new_value(new_)
    , originator(orig_)
    , param_handle(handle_)
{}

template<typename T>
cci_param_write_event<T>::
  cci_param_write_event( const value_type&     old_
                       , const value_type&     new_
                       , const cci_originator& orig_
                       , const cci_param_untyped_handle& handle_ )
    : old_value(old_)
    , new_value(new_)
    , originator(orig_)
    , param_handle(handle_)
{}

template<typename T>
cci_param_write_event<T>::
  generic_wrap::generic_wrap( const type& payload )
    : old_value( payload.old_value )
    , new_value( payload.new_value )
    , wrapped_value(old_value, new_value, payload.originator, payload.param_handle)
{}

inline
cci_param_read_event<void>::
  cci_param_read_event( const value_type&     val_
                      , const cci_originator& orig_
                      , const cci_param_untyped_handle& handle_ )
    : value(val_)
    , originator(orig_)
    , param_handle(handle_)
{}

template<typename T>
cci_param_read_event<T>::
  cci_param_read_event( const value_type&     val_
                      , const cci_originator& orig_
                      , const cci_param_untyped_handle& handle_ )
    : value(val_)
    , originator(orig_)
    , param_handle(handle_)
{}

template<typename T>
cci_param_read_event<T>::
  generic_wrap::generic_wrap( const type& payload )
        : value( payload.value )
        , wrapped_value(value, payload.originator, payload.param_handle)
{}

CCI_CLOSE_NAMESPACE_

#endif // CCI_CFG_CCI_PARAM_CALLBACKS_H_INCLUDED_
