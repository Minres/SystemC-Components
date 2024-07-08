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
* \file   cci_callback.h
* \author Philipp A. Hartmann, Intel
* \author Enrico Galli, Intel
* \brief  Callback interface classes
*/

#ifndef CCI_CORE_CCI_CALLBACK_H_INCLUDED_
#define CCI_CORE_CCI_CALLBACK_H_INCLUDED_

#include "cci/core/cci_cmnhdr.h"
#include <algorithm> // std::swap

CCI_OPEN_NAMESPACE_

// forward declarations

template<typename ArgType, typename ResultType = void>
class cci_callback;

class cci_callback_untyped_handle;

template<typename ArgType, typename ResultType = void>
class cci_callback_typed_handle;

CCI_CLOSE_NAMESPACE_

// include here to propagate forward declarations
#include "cci/core/cci_callback_impl.h"

CCI_OPEN_NAMESPACE_

/* ------------------------------------------------------------------------ */

/**
 * \brief Callback wrapper class
 * \tparam ArgType Type of the (single) callback argument
 * \tparam ResultType Return type of the callback (default: \c void)
 *
 * This class provides a simple callback wrapper for any callable object
 * of the signature
 * \code
 *   ResultType ( ArgType )
 * \endcode
 *
 * As a special extension, callbacks can have a "generalization" relationship:
 * If the matching  \ref cci_callback_traits<ArgType,ResultType> template's boolean
 * member \ref is_generalizable equals to \c true and the nested types
 * \li \c generic_type
 * \li \c generic_wrap
 * exist, cci_callback instances of the signature
 * \code
 *  ResultType (Traits::generic_type)
 * \endcode
 * can be wrapped and calls are automatically forwarded to these callbacks
 * appropriately.
 *
 * \see cci_callback_untyped_handle, cci_callback_typed_handle
 * \todo Add support for callbacks without payloads (\c ArgType ==\c void)
 */
template<typename ArgType, typename ResultType>
class cci_callback
{
  template<typename,typename> friend class cci_callback;
  friend class cci_callback_untyped_handle; //<ResultType>;
  typedef cci_callback_traits<ArgType,ResultType> traits;

public:
  typedef cci_callback type;
  typedef ResultType signature(ArgType);

  cci_callback(const cci_callback& cb)
    : m_cb(cb.m_cb ? cb.m_cb->clone() : NULL)
  {}

#ifdef CCI_HAS_CXX_RVALUE_REFS
  cci_callback(cci_callback&& cb)
    : m_cb(cb.m_cb)
  {
    cb.m_cb = NULL;
  }
#endif // CCI_HAS_CXX_RVALUE_REFS

  /// construction from a "generalized" callback
  template< typename T >
  cci_callback( const cci_callback<T>& cb
#ifndef CCI_DOXYGEN_IS_RUNNING
              , typename cci_impl::enable_if<
                  cci_impl::callback_is_generalized<traits,T>::value
                >::type* = NULL
#endif // CCI_DOXYGEN_IS_RUNNING
              )
    : m_cb( new cci_impl::callback_generic_adapt<traits>(cb.m_cb) )
  {}

  cci_callback& operator=(cci_callback copy)
  {
    copy.swap(*this);
    return *this;
  }

  /// construction from a functor
  template< typename C
    // TODO: Add is_callable<C(ArgType)> restriction, see e.g.
    // http://talesofcpp.fusionfenix.com/post-11/true-story-call-me-maybe
  >
  cci_callback( C c )
    : m_cb( new cci_impl::callback_impl<C,traits>(CCI_MOVE_(c)) )
  {}

  void swap(cci_callback& that)
  {
    using std::swap;
    swap(m_cb, that.m_cb);
  }

  /// invoke callback
  ResultType operator()(ArgType arg) const
  {
    sc_assert( m_cb ); // can only happen in "moved-from" state
    // TODO: add perfect forwarding
    return m_cb->invoke(arg);
  }

  ~cci_callback()
  {
    if (m_cb)
      m_cb->release();
  }

private:
  typedef cci_impl::callback_typed_if<traits> impl_if;
  impl_if* m_cb;
};

/* ------------------------------------------------------------------------ */

/**
 * \brief Generic callback handle class
 *
 * This class provides a generic callback handle to store arbitrary callback
 * instances (returning a the corresponding return type).
 *
 * The held callback can be invoked explicity via the two template functions
 * \li \ref cci_callback_untyped_handle::invoke
 * \li \ref cci_callback_untyped_handle::unchecked_invoke
 *
 * The lifetime of the held callback is extended until the last handle
 * instance pointing to the underlying callback object is destroyed
 * (shared ownership).
 *
 * \see cci_callback, cci_callback_typed_handle
 */
class cci_callback_untyped_handle
{
public:
  /// C++03 compatibility typedef
  typedef cci_callback_untyped_handle type;

  /// default constructor - creates invalid handle
  cci_callback_untyped_handle() : m_cb() {}

  /**
   * \brief Obtain a handle from a callback instance
   * \tparam ArgType Argument type / signature of the callback
   * \param cb Callback to obtain handle from
   */
  template<typename ResultType, typename ArgType>
  cci_callback_untyped_handle( const cci_callback<ArgType, ResultType>& cb )
    : m_cb(cb.m_cb)
  {
    if (m_cb)
      m_cb->acquire();
  }

  /// Copy constructor - both handles point to the same callback object
  cci_callback_untyped_handle(const cci_callback_untyped_handle& that)
    : m_cb(that.m_cb)
  {
    if (m_cb)
      m_cb->acquire();
  }

  /// Assignment - both handles point to the same callback object
  cci_callback_untyped_handle& operator=(const cci_callback_untyped_handle& that)
  {
    cci_callback_untyped_handle copy(that);
    copy.swap(*this);
    return *this;
  }

  /// Exchange contents of two untyped callback handles
  void swap(type& that)
  {
    using std::swap;
    swap(m_cb, that.m_cb);
  }

  ~cci_callback_untyped_handle()
  {
    if (m_cb)
      m_cb->release();
  }

  /// check, if handle currently holds a callback
  bool valid() const
    { return m_cb != NULL; }

  /**
   * \brief check, if handle currently holds a callback with the given signature
   * \tparam ResultType Return type of the wrapped callback
   * \tparam ArgType    Argument type of the wrapped callback
   */
  template<typename ResultType, typename ArgType>
  bool valid() const;

  /// clear held callback
  void clear()
    { reset(NULL); }

  /**
   * \brief invoke callback with given signature
   * \tparam ResultType Return type of the wrapped callback
   * \tparam ArgType    Argument type of the wrapped callback
   *
   * On valid handles, this function invokes the held callback after
   * checing that the expected signature matches.
   *
   * \throw sc_core::sc_report
   * \todo  Document error case.
   *
   * \see cci_callback_untyped_handle::unchecked_invoke
   */
  template<typename ResultType, typename ArgType>
  ResultType invoke(ArgType arg) const;

  /**
   * \brief invoke callback with given signature
   * \tparam ResultType Return type of the wrapped callback
   * \tparam ArgType    Argument type of the wrapped callback
   *
   * On valid handles, this function invokes the held callback
   * \b without checking that the expected signature matches.
   *
   * \warning If the held callback expects a different signature,
   *          the behavior is undefined.
   *
   * \see cci_callback_untyped_handle::invoke
   */
  template<typename ResultType, typename ArgType>
  ResultType unchecked_invoke(ArgType arg) const;

  /// compare two handles
  bool operator==( const cci_callback_untyped_handle& that ) const
  {
    return get() == that.get();
  }

protected:
  const cci_impl::callback_untyped_if * get() const
  {
    if (m_cb)
      return m_cb->get();
    return NULL;
  }

  void reset(cci_impl::callback_untyped_if* cb)
  {
    if (m_cb) m_cb->release();
    m_cb = cb;
    if (m_cb) m_cb->acquire();
  }

private:
  cci_impl::callback_untyped_if * m_cb;
};

/* ------------------------------------------------------------------------ */

/**
 * \brief Typed callback handle class
 * \tparam ArgType Type of the (single) callback argument
 * \tparam ResultType Return type of the wrapped callback (default: \c void)
 *
 * This class provides a typed callback handle to store arbitrary callback
 * instances (returning a the corresponding return type).
 *
 * The held callback can be invoked explicity via the
 * \ref cci_callback_typed_handle::invoke function.
 *
 * The lifetime of the held callback is extended until the last handle
 * instance pointing to the underlying callback object is destroyed
 * (shared ownership).
 *
 * \see cci_callback, cci_callback_untyped_handle
 */
template<typename ArgType, typename ResultType>
class cci_callback_typed_handle
  : public cci_callback_untyped_handle
{
  typedef cci_callback_untyped_handle      base_type;
  typedef cci_callback<ArgType,ResultType> cb_type;

  // disallow untyped/templated overloads
  using base_type::invoke;
  using base_type::unchecked_invoke;

public:
  /// C++03 compatibility typedef
  typedef cci_callback_typed_handle type;

  /// default constructor - creates invalid handle
  cci_callback_typed_handle()
    : base_type() {}

  /// construct from a callback instance
  cci_callback_typed_handle(const cb_type& cb)
    : base_type(cb) {}

  /**
   * \brief Construct from an untyped callback handle, iff signature matches
   *
   * This constructor is used for type recovery from untyped handles.
   * If the underlying signature of the source handle matches the
   * expected signature of the typed handle, the conversion is successful
   * and the constructed handle is valid.
   *
   * Otherwise, an invalid handle is constructed.
   */
  explicit cci_callback_typed_handle(const base_type& cb);

  /**
   * \brief invoke the held callback
   * \see cci_callback_untyped_handle::invoke
   */
  ResultType invoke(ArgType arg) const;
};

/* ------------------------------------------------------------------------ */

template<typename ResultType, typename ArgType>
bool
cci_callback_untyped_handle::valid() const
{
  typedef cci_callback_traits<ArgType,ResultType> traits;
  typedef cci_impl::callback_typed_if<traits> if_type;
  return m_cb && dynamic_cast<const if_type*>(m_cb);
}

template<typename ResultType, typename ArgType>
ResultType
cci_callback_untyped_handle::invoke(ArgType arg) const
{
  typedef cci_callback_traits<ArgType,ResultType> traits;
  typedef cci_impl::callback_typed_if<traits> if_type;
  const if_type* typed_cb = dynamic_cast<const if_type*>(m_cb);

  sc_assert( m_cb );            /// \todo Add proper error handling
  sc_assert( typed_cb );
  return typed_cb->invoke(arg); /// \todo [cxx11] add perfect forwarding
}

template<typename ResultType, typename ArgType>
ResultType
cci_callback_untyped_handle::unchecked_invoke(ArgType arg) const
{
  typedef cci_callback_traits<ArgType,ResultType> traits;
  typedef cci_impl::callback_typed_if<traits> if_type;

  sc_assert( m_cb );            /// \todo Add proper error handling
  const if_type* typed_cb = static_cast<const if_type*>(m_cb);
  return typed_cb->invoke(arg); /// \todo [cxx11] add perfect forwarding
}

/* ------------------------------------------------------------------------ */

template<typename ArgType, typename ResultType>
cci_callback_typed_handle<ArgType,ResultType>::
  cci_callback_typed_handle(const cci_callback_untyped_handle& cb)
    : base_type(cb)
{
  if (!this->valid()) // invalid source
    return;

  if (this->template valid<ResultType,ArgType>()) // matching source
    return;

  // check, if convertible from a generic handle
  typedef cci_callback_traits<ArgType,ResultType> traits;
  cci_impl::callback_untyped_if* cb_adapt
    = cci_impl::callback_generic_adapt<traits>::convert(this->get());
  this->reset(cb_adapt);
  if (cb_adapt) cb_adapt->release();
}

template<typename ArgType, typename ResultType>
ResultType
cci_callback_typed_handle<ArgType,ResultType>::invoke(ArgType arg) const
{
  /// \todo [cxx11] add perfect forwarding
  return base_type::template unchecked_invoke<ResultType,ArgType>(arg);
}

CCI_CLOSE_NAMESPACE_

#endif // CCI_CORE_CCI_CALLBACK_H_INCLUDED_
