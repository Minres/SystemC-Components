
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

#ifndef CCI_CFG_CCI_PARAM_IF_H_INCLUDED_
#define CCI_CFG_CCI_PARAM_IF_H_INCLUDED_

#include <typeinfo>

#include "cci/cfg/cci_param_callbacks.h"
#include "cci/cfg/cci_mutable_types.h"
#include "cci/cfg/cci_broker_handle.h"

/**
 * @author Enrico Galli, Intel
 * @author Philipp A. Hartmann, Intel
 */

CCI_OPEN_NAMESPACE_

class cci_param_untyped_handle;
class cci_originator;

/**
 * @brief CCI configuration parameter interface
 *
 * This class provides the core interfaces between parameter implementations
 * and the broker(s).  All interactions between the broker and the parameters
 * are routed through this interface, most commonly by using explicit
 * parameter handles, which include an additional cci_originator for access
 * tracking.
 *
 * The functions of this interface can be grouped as follows:
 * @li Basic information (name, documentation, metadata, type information)
 * @li Generic untyped access via cci_value
 * @li Generic callback handling
 * @li Access control (aka locking)
 *
 * The type-punned access functions are only needed for parameter
 * implementations (@c *_raw_value) and are marked @c private.
 *
 * @see cci_param_untyped_handle, cci_param_typed_handle<T>
 */
class cci_param_if : public cci_param_callback_if
{
    friend class cci_param_untyped_handle;
public:

    /// @name (Untyped) parameter value access
    //@{

    /**
     * @brief Set the parameter value from a given cci_value
     * @param val new value passed to the parameter
     * @exception cci_report_handler::cci_value_failure
     *            in case the conversion from the supplied cci_value to
     *            the parameter's underlying data type fails.
     * @exception cci_report_handler::set_param_failed
     *            in case of mutability, locked state or other failures
     *
     * This overload uses the parameter's own originator for access tracking.
     * @see get_originator()
     */
    void set_cci_value(const cci_value& val)
      { set_cci_value(val, NULL); }

    /**
     * @brief Set the parameter value from a given cci_value
     * @param val new value passed to the parameter
     * @param pwd the password, with which the parameter is currently locked
     *            (or NULL, if the parameter is not locked)
     * @exception cci_report_handler::cci_value_failure
     *            in case the conversion from the supplied cci_value to
     *            the parameter's underlying data type fails.
     * @exception cci_report_handler::set_param_failed
     *            in case of mutability, locked state or other failures
     *
     * This overload uses the parameter's own originator for access tracking.
     * @see get_originator()
     */
    void set_cci_value(const cci_value& val, const void* pwd)
      { set_cci_value(val, pwd, get_originator()); }

    /**
     * @brief Set the parameter value from a given cci_value
     * @param val new value passed to the parameter
     * @param originator the accessing originator
     * @exception cci_report_handler::cci_value_failure
     *            in case the conversion from the supplied cci_value to
     *            the parameter's underlying data type fails.
     * @exception cci_report_handler::set_param_failed
     *            in case of mutability, locked state or other failures
     */
    void set_cci_value(const cci_value &val, const cci_originator &originator)
      { set_cci_value(val, NULL, originator); }

    /**
     * @brief Set the parameter value from a given cci_value
     * @param val new value passed to the parameter
     * @param pwd the password, with which the parameter is currently locked
     *            (or NULL, if the parameter is not locked)
     * @param originator the accessing originator
     * @exception cci_report_handler::cci_value_failure
     *            in case the conversion from the supplied cci_value to
     *            the parameter's underlying data type fails.
     * @exception cci_report_handler::set_param_failed
     *            in case of mutability, locked state or other failures
     * @note This is the only overload that needs to be provided
     *       by parameter implementations.
     *
     * @see set_cci_value(const cci_value&),
     *      set_cci_value(const cci_value&, const void*),
     *      set_cci_value(const cci_value&, const cci_originator&)
     */
    virtual void set_cci_value(const cci_value &val, const void* pwd,
                               const cci_originator &originator) = 0;

    /**
     * @brief Get the parameter's value converted to cci_value
     * @return The current value of the parameter's underlying type
     *         converted to cci_value
     * @exception cci_report_handler::cci_value_failure
     *            in case the conversion to a cci_value fails
     *
     * This overload uses the parameter's own originator for access tracking.
     * @see get_originator()
     */
    cci_value get_cci_value() const
      { return get_cci_value(get_originator()); }

    /**
     * @brief Get the parameter's value converted to cci_value.
     * @param originator the accessing originator
     * @return The current value of the parameter's underlying type
     *         converted to cci_value
     * @exception cci_report_handler::cci_value_failure
     *            in case the conversion to a cci_value fails
     * @see get_cci_value()
     */
    virtual cci_value get_cci_value(const cci_originator& originator) const = 0;

    /**
     * @brief Get the parameter's default value converted to cci_value
     * @return The value of the parameter's underlying type
     *         as given to the constructor, converted to cci_value
     * @exception cci_report_handler::cci_value_failure
     *            in case the conversion to a cci_value fails
     */
    virtual cci_value get_default_cci_value() const = 0;
    //@}


    ///@name Description and metadata
    //@{

    /// @copydoc cci_param_untyped::get_description
    virtual std::string get_description() const = 0;

    /// @copydoc cci_param_untyped::set_description
    virtual void set_description(const std::string &desc) = 0;

    /// @copydoc cci_param_untyped::add_metadata
    virtual void add_metadata(const std::string &name, const cci_value &value,
                              const std::string &desc = "") = 0;

    /// @copydoc cci_param_untyped::get_metadata
    virtual cci_value_map get_metadata() const = 0;
    ///@}

    ///@name Parameter Value Status
    //@{

    /// @copydoc cci_param_untyped::is_default_value
    virtual bool is_default_value() const = 0;

    /// @copydoc cci_param_untyped::is_preset_value
    virtual bool is_preset_value() const = 0;

    ///@}

    ///@name Originator queries
    //@{

    /// @copydoc cci_param_untyped::get_originator
    virtual cci_originator get_originator() const = 0;

    /// @copydoc cci_param_untyped::get_value_origin
    virtual cci_originator get_value_origin() const = 0;

    //@}

    ///@name Write-access control
    //@{

    /// @copydoc cci_param_untyped::lock
    virtual bool lock(const void *pwd = NULL) = 0;

    /// @copydoc cci_param_untyped::unlock
    virtual bool unlock(const void *pwd = NULL) = 0;

    /// @copydoc cci_param_untyped::is_locked
    virtual bool is_locked() const = 0;

    //@}

    ///@name Query parameter type and name
    //@{

    /// @copydoc cci_param_untyped::name
    virtual const char* name() const = 0;

    /// @copydoc cci_param_typed::get_mutable_type
    virtual cci_param_mutable_type get_mutable_type() const = 0;

    /// @copydoc cci_param_typed::get_type_info
    virtual const std::type_info& get_type_info() const = 0;

    /// @copydoc cci_param_typed::get_data_category
    virtual cci_param_data_category get_data_category() const = 0;

    //@}

    /**
     * @brief Convenience function to create a new parameter handle
     * @param originator Originator for access tracking
     * @return parameter handle for the given (or current) originator
     * @note  The default argument can only be reliably used from within the
     *        SystemC hierarchy during elaboration.
     * @see cci_originator
     */
    cci_param_untyped_handle
    create_param_handle(const cci_originator& originator = cci_originator()) const;

    /**
     * @brief  Convenience function to restore parameter's initial value
     * @return true if successful; false if unsuccessful due to parameter being locked
     * @note   Upon success, the value originator will be updated to reflect the
     *         origin of the initial value.
     */
    virtual bool reset() = 0;

protected:
    /// helper function, returning an unknown/invalid originator
    cci_originator unknown_originator() const
      { return cci_originator( cci_originator::unknown_tag() ); }

    /** Extract typed value from a parameter.
     *  @param  rhs parameter to read value from
     *  @pre    rhs.get_type_info() == typeid(T)
     *  @return rhs.get_raw_value() - converted back to reference of type T
     */
    template<typename T>
    const T& get_typed_value(const cci_param_if& rhs) const
      { return *static_cast<const T*>( rhs.get_raw_value(get_originator()) ); }

    ///@name Initialization and destruction of parameter object
    //@{
    /**
     * Initialize parameter object.
     * @param broker Broker to register this parameter with.
     *
     * This function shall be called from the constructor of the final
     * parameter object where @b all virtual functions are available.
     *
     * This function will register the parameter with the supplied broker
     * and will afterwards apply the any preset value available from the
     * broker.
     *
     * @see preset_cci_value, cci_broker_handle::add_param
     */
    void init(cci_broker_handle broker);

    /**
     * Destroy parameter object.
     * @param broker Broker to unregister this parameter from.
     *
     * This function shall be called from the destructor of the final
     * parameter object where @b all virtual functions are still available.
     *
     * This function will unregister the parameter from the supplied broker
     * and will afterwards invalidate all parameter handles.
     *
     * @see invalidate_all_param_handles, cci_broker_handle::remove_param
     */
    void destroy(cci_broker_handle broker);

    /// Default Constructor
    cci_param_if() {}

    /// Destruction from base pointer is disallowed
    virtual ~cci_param_if() {}

private:
    /**
     * Initialize value of parameter from broker preset, if present.
     * @param preset     Preset value supplied by broker
     * @param originator Originator of preset value
     *
     * The default implementation if simply calls @ref set_cci_value,
     * parameter implementations may choose to customize the behavior
     * when applying a preset value.
     *
     * Called from @ref init.
     */
    virtual void
    preset_cci_value(const cci_value& preset, const cci_originator& originator);

    /**
     * Invalidate all parameter handles.
     *
     * This function shall call @ref cci_param_untyped_handle::invalidate
     * on all currently registered handles of this parameter.
     *
     * Called from @ref destroy.
     */
    virtual void
    invalidate_all_param_handles() = 0;
    //@}

    ///@name Type-punned value operations
    //@{

    /// @copydoc cci_param_typed::set_raw_value(const void*, const void*, const cci_originator&)
    virtual void set_raw_value(const void *vp, const void *pwd,
                               const cci_originator &originator) = 0;

    /// @copydoc cci_param_typed::get_raw_value
    virtual const void *get_raw_value(const cci_originator &originator) const = 0;

    /// @copydoc cci_param_typed::get_raw_default_value
    virtual const void *get_raw_default_value() const = 0;

    /// @copydoc cci_param_untyped::add_param_handle
    virtual void add_param_handle(cci_param_untyped_handle* param_handle) = 0;

    /// @copydoc cci_param_untyped::remove_param_handle
    virtual void remove_param_handle(cci_param_untyped_handle* param_handle) = 0;

    //@}

    // Disabled
    cci_param_if(const cci_param_if&);
    cci_param_if& operator=(const cci_param_if&);
#ifdef CCI_HAS_CXX_RVALUE_REFS
    cci_param_if(cci_param_if&&);
    cci_param_if& operator=(cci_param_if&&);
#endif

}; // class cci_param_if

CCI_CLOSE_NAMESPACE_
#endif // CCI_CFG_CCI_PARAM_IF_H_INCLUDED_
