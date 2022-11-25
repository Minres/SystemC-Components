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
* \file   cci_callback_impl.h
* \author Philipp A. Hartmann, Intel
* \brief  Callback implementation classes
*
* \note Any classes defined in the \ref cci::cci_impl namespace
*       are not part of the CCI standard.
*/
#ifndef CCI_CORE_CCI_CALLBACK_IMPL_H_INCLUDED_
#define CCI_CORE_CCI_CALLBACK_IMPL_H_INCLUDED_

#include "cci_core/cci_cmnhdr.h"
#include "cci_core/cci_meta.h"

CCI_OPEN_NAMESPACE_
///@cond CCI_HIDDEN_FROM_DOXYGEN
namespace cci_impl {

/// implementation-defined helper to define default callback traits
template<typename ArgType, typename ResultType, typename GenericType = void>
struct callback_traits
{
  typedef ArgType argument_type;
  typedef ResultType result_type;

  static const bool is_generalizable = false;
};

/// implementation-defined helper to define generalizable callback traits
template<typename ArgType, typename ResultType>
class callback_traits<ArgType, ResultType,
  typename cci_impl::always_void<
    typename cci_impl::remove_reference<ArgType>::type::generic_type
  >::type
>
{
  typedef typename cci_impl::remove_reference<ArgType>::type nonref_argtype;
public:
  typedef ArgType argument_type;
  typedef ResultType result_type;

  typedef typename nonref_argtype::generic_type generic_type;
  typedef typename nonref_argtype::generic_wrap generic_wrap;

  static const bool is_generalizable = true;
};

} // namespace cci_impl
///@endcond

/// default callback traits
template<typename ArgType, typename ResultType>
struct cci_callback_traits
  : cci_impl::callback_traits<ArgType, ResultType> {};

/* ------------------------------------------------------------------------ */

///@cond CCI_HIDDEN_FROM_DOXYGEN
namespace cci_impl {

struct callback_untyped_if
{
  virtual void acquire() = 0;
  virtual void release() = 0;

  virtual callback_untyped_if* clone() const = 0;
  virtual ~callback_untyped_if()  {}

  virtual const callback_untyped_if* get() const
    { return this; }

protected:
  callback_untyped_if() {}

private:
  callback_untyped_if(const callback_untyped_if&) /* = delete */;
  callback_untyped_if& operator=(const callback_untyped_if&) /* = delete */;
}; // callback_untyped_if<ResultType>

template<typename Traits>
struct callback_typed_if
  : callback_untyped_if //<typename Traits::result_type>
{
  typedef typename Traits::result_type result_type;
  typedef typename Traits::argument_type argument_type;

  virtual result_type invoke(argument_type) const = 0;

  virtual callback_typed_if* clone() const = 0;
  virtual ~callback_typed_if(){}

}; // callback_typed_if<Traits>

/* ------------------------------------------------------------------------ */

/// Implementation of a callback
template<typename Functor, typename Traits>
struct callback_impl
  : callback_typed_if<Traits>
{
  typedef callback_typed_if<Traits>        if_type;
  typedef typename if_type::argument_type  argument_type;
  typedef typename if_type::result_type    result_type;

  explicit callback_impl(const Functor& f)
    : m_func(f), m_refcnt(1) {}

#ifdef CCI_HAS_CXX_RVALUE_REFS
  explicit callback_impl(Functor&& f)
    : m_func(CCI_MOVE_(f)), m_refcnt(1) {}
#endif // CCI_HAS_CXX_RVALUE_REFS

  virtual result_type invoke(argument_type arg) const
  {
    return m_func(arg);
  }

  virtual callback_impl* clone() const
    { return new callback_impl(m_func); }

  virtual void acquire() { ++m_refcnt; }
  virtual void release() { if (!--m_refcnt) delete this; }

protected:
  Functor  m_func;
  unsigned m_refcnt;
}; // callback_impl<Functor,Traits>

/* ------------------------------------------------------------------------ */

template<typename Traits, typename GenericType
        , bool IsGeneralizable = Traits::is_generalizable>
struct callback_is_generalized;

template<typename Traits, typename GenericType>
struct callback_is_generalized<Traits, GenericType, false>
  : false_type {};

template<typename Traits, typename GenericType>
struct callback_is_generalized<Traits, GenericType, true>
  : is_same<typename Traits::generic_type, GenericType> {};

/* ------------------------------------------------------------------------ */

/// Implementation of a generic callback adapter
template<typename Traits, bool IsGeneralizable = Traits::is_generalizable>
struct callback_generic_adapt;

template<typename Traits>
struct callback_generic_adapt<Traits,false>
  : callback_untyped_if
{
  static callback_untyped_if* convert( const callback_untyped_if* )
    { return NULL; }

private:
  void acquire() {}
  void release() {}
  callback_untyped_if* clone() const { return NULL; }
};

template<typename Traits>
struct callback_generic_adapt<Traits,true>
  : callback_typed_if<Traits>
{
  typedef typename Traits::argument_type argument_type;
  typedef typename Traits::result_type   result_type;
  typedef typename Traits::generic_type  generic_type;

  typedef cci_callback_traits<generic_type,result_type> generic_traits;
  typedef callback_typed_if<generic_traits> if_type;

  static callback_untyped_if* convert( const callback_untyped_if* f )
  {
    if (!f)
      return NULL;

    // is this a generalized callback instance?
    const if_type* typed_if = dynamic_cast<const if_type*>(f);
    if (typed_if)
      return new callback_generic_adapt( const_cast<if_type*>(typed_if) );

    return NULL;
  }

  explicit callback_generic_adapt(if_type* f)
    : m_func(f), m_refcnt(1)
  {
    m_func->acquire();
  }

  virtual result_type invoke(argument_type arg) const
  {
    // convert to generic argument and call functor
    return m_func->invoke( typename Traits::generic_wrap(arg).get() );
  }

  virtual void acquire() { ++m_refcnt; }
  virtual void release() { if (!--m_refcnt) delete this; }

  virtual const callback_untyped_if* get() const
    { return m_func->get(); }

  virtual callback_generic_adapt* clone() const
    { return new callback_generic_adapt(m_func->clone()); }

protected:
  ~callback_generic_adapt()
  {
    m_func->release();
  }

  if_type* m_func;
  unsigned m_refcnt;
};

} // namespace cci_impl
///@endcond
CCI_CLOSE_NAMESPACE_

#endif // CCI_CORE_CCI_CALLBACK_IMPL_H_INCLUDED_
