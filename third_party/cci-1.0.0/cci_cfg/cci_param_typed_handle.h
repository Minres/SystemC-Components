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

#ifndef CCI_CFG_CCI_PARAM_TYPED_HANDLE_H_INCLUDED_
#define CCI_CFG_CCI_PARAM_TYPED_HANDLE_H_INCLUDED_

#include "cci_cfg/cci_mutable_types.h"
#include "cci_cfg/cci_param_untyped_handle.h"

/**
 * @author Guillaume Delbergue, Ericsson / GreenSocs
 */

CCI_OPEN_NAMESPACE_

template<typename T, cci_param_mutable_type TM> class cci_param_typed;

/**
 * @brief Type-safe parameter handle
 * @tparam T underlying type of given parameter
 *
 * This class provides a typed handle to access a CCI parameter.
 * In addition to the untyped capabilities, it provides type-safe
 * access to the parameter value and allows registering typed
 * callbacks.
 *
 * @see cci_param_untyped_handle, cci_param_if
 */
template<typename T>
class cci_param_typed_handle : public cci_param_untyped_handle
{
public:
    /// The parameter's value type.
    typedef T value_type;

    /**
     * @brief Constructor to create a typed parameter handle
     * @param untyped Untyped parameter handle to restore type information on
     * @pre untyped.get_type_info() == typedif(T)
     *
     * If the destination type T does not match the underlying type of
     * the referenced parameter, the resulting handle is invalidated.
     *
     * @see cci_param_cast<T>
     */
    explicit cci_param_typed_handle(cci_param_untyped_handle untyped);

#if CCI_CPLUSPLUS >= 201103L
    cci_param_typed_handle(const cci_param_typed_handle&) = default;
    cci_param_typed_handle& operator=(const cci_param_typed_handle&) = default;

    cci_param_typed_handle(cci_param_typed_handle&& that)
      : cci_param_untyped_handle(CCI_MOVE_(that)) {}

    cci_param_typed_handle& operator=(cci_param_typed_handle&& that)
    {
        cci_param_untyped_handle::operator=(CCI_MOVE_(that));
        return *this;
    }
    ~cci_param_typed_handle() = default;
#endif // C++11

    ///Sets the stored value to a new value
    /**
     * @param value new value to assign
     */
    void set_value(const value_type& value);

    ///Sets the stored value to a new value
    /**
     * @param value new value to assign
     * @param pwd Password needed to unlock the param, ideally any pointer address known only by the locking entity, default = NULL
    */
    void set_value(const value_type & value, const void * pwd);

    /// Convenience shortcut to read the stored value
    const value_type& operator*() const;

    ///Gets the stored value
    const value_type& get_value() const;

    ///Get the value passed in via constructor
    const value_type & get_default_value() const;

#define CCI_PARAM_TYPED_HANDLE_CALLBACK_DECL_(name)                            \
    /** @copydoc cci_param_untyped::register_##name##_callback(const cci_param_##name##_callback_untyped, cci_untyped_tag) */ \
    cci_callback_untyped_handle register_##name##_callback(                    \
            const cci_param_##name##_callback_untyped &cb,                     \
            cci_untyped_tag);                                                  \
                                                                               \
    /** @copydoc cci_param_untyped::register_##name##_callback(cci_param_##name##_callback_untyped::signature, C*, cci_untyped_tag) */ \
    template<typename C>                                                       \
    cci_callback_untyped_handle register_##name##_callback(                    \
            cci_param_##name##_callback_untyped::signature (C::*cb), C* obj,   \
            cci_untyped_tag);                                                  \
                                                                               \
    /** @brief Typed name callback type */                                     \
    typedef typename cci_param_##name##_callback<value_type>::type             \
            cci_param_##name##_callback_typed;                                 \
                                                                               \
    /** @copydoc cci_param_typed::register_##name##_callback(const cci_param_##name##_callback_typed&, cci_typed_tag<value_type>) */ \
    cci_callback_untyped_handle register_##name##_callback(                    \
            const cci_param_##name##_callback_typed& cb,                       \
            cci_typed_tag<value_type> = cci_typed_tag<value_type>());          \
                                                                               \
    /** @copydoc cci_param_typed::register_##name##_callback(typename cci_param_##name##_callback_typed::signature, C*, cci_typed_tag<value_type>) */ \
    template<typename C>                                                       \
    cci_callback_untyped_handle register_##name##_callback(                    \
            typename cci_param_##name##_callback_typed::signature (C::*cb),    \
            C* obj, cci_typed_tag<value_type> = cci_typed_tag<value_type>())

    /// @name Pre write callback handling
    /// @{

    CCI_PARAM_TYPED_HANDLE_CALLBACK_DECL_(pre_write);

    /// @}

    /// @name Post write callback handling
    /// @{

    CCI_PARAM_TYPED_HANDLE_CALLBACK_DECL_(post_write);

    /// @}

    /// @name Pre read callback handling
    /// @{

    CCI_PARAM_TYPED_HANDLE_CALLBACK_DECL_(pre_read);

    /// @}

    /// @name Post read callback handling
    /// @{

    CCI_PARAM_TYPED_HANDLE_CALLBACK_DECL_(post_read);

    /// @}

#undef CCI_PARAM_TYPED_HANDLE_CALLBACK_DECL_

private:
    const value_type& typed_cast(const void* val) const
      { return *static_cast<const value_type *>(val); }
};

template <typename T>
const T& cci_param_typed_handle<T>::operator *() const
{
    return get_value();
}

template <typename T>
void cci_param_typed_handle<T>::set_value(const value_type& value)
{
    cci_param_untyped_handle::set_raw_value(&value);
}

template <typename T>
void cci_param_typed_handle<T>::set_value(const value_type& value, const void *pwd)
{
    cci_param_untyped_handle::set_raw_value(&value, pwd);
}

template <typename T>
const T& cci_param_typed_handle<T>::get_value() const
{
    return typed_cast(cci_param_untyped_handle::get_raw_value());
}

template <typename T>
const typename cci_param_typed_handle<T>::value_type&
cci_param_typed_handle<T>::get_default_value() const
{
    return typed_cast(cci_param_untyped_handle::get_raw_default_value());
}

#define CCI_PARAM_TYPED_HANDLE_CALLBACK_IMPL_(name)                            \
template <typename T>                                                          \
cci_callback_untyped_handle                                                    \
cci_param_typed_handle<T>::register_##name##_callback(                         \
        const cci_param_##name##_callback_untyped &cb,                         \
        cci_untyped_tag)                                                       \
{                                                                              \
    return cci_param_untyped_handle::register_##name##_callback(cb);           \
}                                                                              \
                                                                               \
template <typename T>                                                          \
template<typename C>                                                           \
cci_callback_untyped_handle                                                    \
cci_param_typed_handle<T>::register_##name##_callback(                         \
        cci_param_##name##_callback_untyped::signature (C::*cb), C* obj,       \
        cci_untyped_tag)                                                       \
{                                                                              \
    return cci_param_untyped_handle::register_##name##_callback(cb, obj);      \
}                                                                              \
                                                                               \
template <typename T>                                                          \
cci_callback_untyped_handle                                                    \
cci_param_typed_handle<T>::register_##name##_callback(                         \
        const cci_param_##name##_callback_typed &cb, cci_typed_tag<T>)         \
{                                                                              \
    return cci_param_untyped_handle::register_##name##_callback(cb,            \
           cci_typed_tag<void>());                                             \
}                                                                              \
                                                                               \
template <typename T>                                                          \
template <typename C>                                                          \
cci_callback_untyped_handle                                                    \
cci_param_typed_handle<T>::register_##name##_callback(                         \
        typename cci_param_##name##_callback_typed::signature (C::*cb),        \
        C* obj, cci_typed_tag<T>)                                              \
{                                                                              \
    return register_##name##_callback(sc_bind(cb, obj, sc_unnamed::_1));       \
}

CCI_PARAM_TYPED_HANDLE_CALLBACK_IMPL_(pre_write)

CCI_PARAM_TYPED_HANDLE_CALLBACK_IMPL_(post_write)

CCI_PARAM_TYPED_HANDLE_CALLBACK_IMPL_(pre_read)

CCI_PARAM_TYPED_HANDLE_CALLBACK_IMPL_(post_read)

template <typename T>
cci_param_typed_handle<T>::cci_param_typed_handle(cci_param_untyped_handle untyped)
 : cci_param_untyped_handle(CCI_MOVE_(untyped))
{
    if(is_valid() && typeid(T) != get_type_info()) {
        invalidate();
    }
}

/**
 * @brief Recover type information on a parameter handle
 * @tparam T Assumed underlying type of given parameter
 * @param untyped Untyped handle to convert to a typed handle
 * @return typed parameter handle, if types match; an invalid handle otherwise
 * @see cci_param_typed_handle<T>::cci_param_typed_handle(cci_param_untyped_handle)
 */
template <typename T>
cci_param_typed_handle<T>
cci_param_cast(const cci_param_untyped_handle& untyped)
  { return cci_param_typed_handle<T>(untyped); }

#undef CCI_PARAM_TYPED_HANDLE_CALLBACK_IMPL_

CCI_CLOSE_NAMESPACE_

#endif //CCI_CFG_CCI_BASE_PARAM_HANDLE_H_INCLUDED_
