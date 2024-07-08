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

#ifndef CCI_CCI_PARAM_UNTYPED_HANDLE_H_INCLUDED_
#define CCI_CCI_PARAM_UNTYPED_HANDLE_H_INCLUDED_

#include "cci/core/cci_core_types.h"
#include "cci/core/cci_value.h"
#include "cci/cfg/cci_originator.h"
#include "cci/cfg/cci_mutable_types.h"
#include "cci/cfg/cci_param_callbacks.h"

/**
 * @author Guillaume Delbergue, Ericsson / GreenSocs
 */

CCI_OPEN_NAMESPACE_

// Forward declaration
class cci_param_if;

// CCI Configuration parameter base class
/**
* Type independent base class all cci parameters derive from.
* This class gives some easier access to type independent functions.
*/
class cci_param_untyped_handle
{
public:

    /// @name Construction, destruction, assignment
    //@{
    /// Constructor to create handle with given originator.
    cci_param_untyped_handle(cci_param_if& param,
                             const cci_originator& originator);

    /// Constructor to create an invalid param handle with given originator.
    explicit cci_param_untyped_handle(const cci_originator& originator = cci_originator());

    /// Copy constructor
    cci_param_untyped_handle(const cci_param_untyped_handle& param_handle);

    /// Copy assignment - update handle to refer to a new parameter
    cci_param_untyped_handle&
    operator=(const cci_param_untyped_handle& param_handle);

#ifdef CCI_HAS_CXX_RVALUE_REFS
    /// Move constructor
    cci_param_untyped_handle(cci_param_untyped_handle&& that);
    /// Move assignment - update handle to refer to a new parameter
    cci_param_untyped_handle&
    operator=(cci_param_untyped_handle&& that);
#endif // CCI_HAS_CXX_RVALUE_REFS

    /// Destructor.
    ~cci_param_untyped_handle();

    //@}

    ///@name Description and metadata
    ///@{

    /// @copydoc cci_param_untyped::get_description
    std::string get_description() const;

    /// @copydoc cci_param_untyped::get_metadata
    cci_value_map get_metadata() const;

    ///@}

    ///@name (Untyped) parameter value access
    ///@{

    /**
     * @copydoc cci_param_if::set_cci_value(const cci_value&)
     * @note The @c originator is taken from the parameter handle here
     */
    void set_cci_value(const cci_value& val);

    /**
     * @copydoc cci_param_if::set_cci_value(const cci_value&,const void*)
     * @note The @c originator is taken from the parameter handle here
     */
    void set_cci_value(const cci_value& val, const void *pwd);

    /**
     * @copydoc cci_param_if::get_cci_value(const cci_originator&)
     * @note The @c originator is taken from the parameter handle here
     */
    cci_value get_cci_value() const;

    /**
     * @copydoc cci_param_if::get_default_cci_value
     * @note The @c originator is taken from the parameter handle here
     */
    cci_value get_default_cci_value() const;

    ///@}

    ///@name Parameter Value Status
    ///@{

    /// @copydoc cci_param_if::is_default_value
    bool is_default_value() const;


    /// @copydoc cci_param_if::is_preset_value
    bool is_preset_value() const;

    ///@}

    ///@name Originator queries
    ///@{

    /// @copydoc cci_param_if::get_originator
    cci_originator get_originator() const;

    /// @copydoc cci_param_if::get_value_origin
    cci_originator get_value_origin() const;

    ///@}

#define CCI_PARAM_UNTYPED_HANDLE_CALLBACK_DECL_(name)                          \
    /* @copydoc cci_param_untyped::register_##name##_callback(const cci_param_##name##_callback_untyped, cci_untyped_tag) */ \
    cci_callback_untyped_handle register_##name##_callback(                    \
            const cci_param_##name##_callback_untyped &cb,                     \
            cci_untyped_tag = cci_untyped_tag());                              \
                                                                               \
    /* @copydoc cci_param_untyped::register_##name##_callback(cci_param_##name##_callback_untyped::signature, C*, cci_untyped_tag) */ \
    template<typename C>                                                                        \
    cci_callback_untyped_handle register_##name##_callback(                    \
            cci_param_##name##_callback_untyped::signature (C::*cb), C* obj,   \
            cci_untyped_tag = cci_untyped_tag())                               \
    {                                                                          \
        return register_##name##_callback(sc_bind(cb, obj, sc_unnamed::_1));   \
    }                                                                          \
                                                                               \
    /* @copydoc cci_param_untyped::unregister_##name##_callback(const cci_callback_untyped_handle) */ \
    bool unregister_##name##_callback(const cci_callback_untyped_handle &cb);  \
                                                                               \
    /* TODO: doc */                                                            \
    cci_callback_untyped_handle register_##name##_callback(                    \
            const cci_callback_untyped_handle& cb, cci_typed_tag<void>)

    /// @name Pre write callback handling
    /// @{

    CCI_PARAM_UNTYPED_HANDLE_CALLBACK_DECL_(pre_write);

    /// @}

    /// @name Post write callback handling
    /// @{

    CCI_PARAM_UNTYPED_HANDLE_CALLBACK_DECL_(post_write);

    /// @}

    /// @name Pre read callback handling
    /// @{

    CCI_PARAM_UNTYPED_HANDLE_CALLBACK_DECL_(pre_read);

    /// @}

    /// @name Post read callback handling
    /// @{

    CCI_PARAM_UNTYPED_HANDLE_CALLBACK_DECL_(post_read);

    /// @}

#undef CCI_PARAM_UNTYPED_HANDLE_CALLBACK_DECL_

    /// @name CCI callback handling
    /// @{

    /// @copydoc cci_param_untyped::unregister_all_callbacks
    bool unregister_all_callbacks();

    /// @copydoc cci_param_untyped::has_callbacks
    bool has_callbacks() const;

    /// @}

    ///@name Write-access control
    ///@{

    /// @copydoc cci_param_untyped::lock
    bool lock(const void* pwd = NULL);

    /// @copydoc cci_param_untyped::unlock
    bool unlock(const void* pwd = NULL);

    /// @copydoc cci_param_untyped::is_locked
    bool is_locked() const;

    ///@}

    ///@name Query parameter type and name
    ///@{

    /// @copydoc cci_param_typed::get_data_category
    cci_param_data_category get_data_category() const;

    /// @copydoc cci_param_untyped::name
    const char* name() const;

    /// @copydoc cci_param_typed::get_type_info
    const std::type_info& get_type_info() const;

    /// @copydoc cci_param_typed::get_mutable_type
    cci_param_mutable_type get_mutable_type() const;

    ///@}

    /**
     * @brief  Indicates if the handled parameter is valid or not
     * @return whether handle currently points to a valid parameter
     */
    bool is_valid() const;

    /// Invalidate the parameter handle
    /**
     *        remove the parameter handle from the original
     *        parameter before invalidating. Otherwise, just invalidate.
     */
    void invalidate();

protected:
    ///@name Type-punned value operations
    ///@{

    /// @copydoc cci_param_typed::get_raw_value
    const void* get_raw_value() const;

    /// @copydoc cci_param_typed::get_raw_default_value
    const void* get_raw_default_value() const;

    /// @copydoc cci_param_typed::set_raw_value(const void*)
    void set_raw_value(const void* vp);

    /// @copydoc cci_param_typed::set_raw_value(const void*, const void*)
    void set_raw_value(const void* vp, const void* pwd);

    ///@}

    /// Promote a gifted originator to one that represents the current context
    /// when possible (i.e. when within the module hierarchy)
    /**
    * @param gifted_originator associated with the copy ctor broker argument
    * @return context originator if possible; otherwise, the gifted_originator
    */
    inline const cci_originator promote_originator(const cci_originator &gifted_originator);

private:
    cci_param_if*  m_param;
    cci_originator m_originator;

    /// Check handled parameter is valid
    /**
     * In case the handled parameter is no more valid, it will report an error.
     */
    void check_is_valid() const;
};

/// Convenience shortcut for untyped parameter handles
typedef cci_param_untyped_handle cci_param_handle ;


const cci_originator cci_param_untyped_handle::promote_originator(
    const cci_originator &gifted_originator)
{
    if (sc_core::sc_get_current_object())
        return cci_originator();
    else
        return gifted_originator;
}

CCI_CLOSE_NAMESPACE_

#endif //CCI_BASE_PARAM_HANDLE_H_INCLUDED_
