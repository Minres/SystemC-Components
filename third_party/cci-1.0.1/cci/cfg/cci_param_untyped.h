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
 * @author Enrico Galli, Intel
 */
#ifndef CCI_CFG_CCI_PARAM_UNTYPED_H_INCLUDED_
#define CCI_CFG_CCI_PARAM_UNTYPED_H_INCLUDED_

#include <string>
#include <vector>

#include "cci/core/cci_core_types.h"
#include "cci/core/cci_value.h"
#include "cci/cfg/cci_param_if.h"
#include "cci/cfg/cci_originator.h"
#include "cci/cfg/cci_broker_handle.h"

CCI_OPEN_NAMESPACE_

// CCI Configuration parameter base class
/**
* Type independent base class all cci parameters derive from.
* This class gives some easier access to type independent functions.
*/
class cci_param_untyped : public cci_param_if
{

public:

    /// Destructor.
    virtual ~cci_param_untyped();

    ///@name Description
    ///@{

    /// Set parameter description.
    /**
     * Set the parameter's description describing purpose and
     * intended use, allowed value range etc. in a human readable way.
     *
     * @param desc Human readable description
     */
    virtual void set_description(const std::string& desc);
    
    /// Get the parameter's description.
    /**
     * @return Description
     */
    virtual std::string get_description() const;

    ///@}

    ///@name Metadata
    ///@{

    /// Add metadata
    /**
     * Add metadata to the parameter identified by its name, value
     * and a human readable description.
     *
     * @param name Name of the metadata
     * @param value Value of the metadata
     * @param desc Human readable description
     */
    void add_metadata(const std::string &name, const cci_value &value,
                      const std::string &desc = "");

    /// Return the metadata value
    /**
     * Return value of the metadata by its given name.
     *
     * @return name Name of the metadata
     * @return Metadata value
     */
    cci_value_map get_metadata() const;

    ///@}

    ///@name Parameter Value Status
    ///@{

    /// Indicates whether the value provided at parameter construction persists.
    /**
     * True if the value was supplied as a constructor argument and not
     * subsequently changed.
     *
     * Note: false is returned even if the current value matches the constructor
     * supplied default but has undergone intermediate changes.
     *
     * @return false if the parameter received a preset value or its value has
     *         changed; otherwise, true
     */
    virtual bool is_default_value() const =0; // note this is pure virtual here,
                                              // as the function isn't possible
                                              // without a typed instance (to
                                              // have a defualt value).


    /// Indicates that the parameter received a preset value that has not since been modified.
    /**
     * True if the value was supplied using the broker's
     * set_preset_cci_value function and not subsequently changed.
     *
     * Note: false is returned even if the current value matches the preset
     * value but has undergone intermediate changes.
     *
     * @return fase if no preset value was supplied or the parameter's value has
     *         changed; otherwise, true
     */
    virtual bool is_preset_value() const;

    ///@}


    ///@name Miscellaneous
    ///@{

    /// Returns the originator of the parameter's current value. 
    /**
     * This initially reflects originator of the parameter's starting value,
     * e.g. the owning module or startup configuration file.  It is
     * subsequently updated to reflect the originator of any value changes.
     *
     * The originator is updated on successful calls to the following functions:
     * set_cci_value(), cci_param_typed::set_value(), cci_param_typed::operator=()
     */
    cci_originator get_value_origin() const;

    ///@}

    /// @name Post write callback handling
    /// @{

    /// Register an untyped write callback.
    /**
     * @param cb Untyped post write callback
     * @param cci_untyped_tag Untyped tag to avoid compiler ambiguity
     *
     * @return Untyped callback handle
     */
    cci_callback_untyped_handle
    register_post_write_callback(
            const cci_param_post_write_callback_untyped& cb,
            cci_untyped_tag = cci_untyped_tag());

    /// Register an untyped post write callback with a method as callback
    /**
     * @param cb Untyped post write callback method
     * @param obj Associated object instance pointer
     * @param cci_untyped_tag Untyped tag to avoid compiler ambiguity
     *
     * @return Untyped callback handle
     */
    template<typename C>
    cci_callback_untyped_handle
    register_post_write_callback(
            cci_param_post_write_callback_untyped::signature (C::*cb), C* obj,
            cci_untyped_tag = cci_untyped_tag())
    {
        return register_post_write_callback(sc_bind(cb, obj, sc_unnamed::_1));
    }

    /// Unregister a post write callback handle
    /**
     * @param cb Untyped post write callback handle
     *
     * @return True if unregister is a success. Otherwise False.
     */
    bool
    unregister_post_write_callback(const cci_callback_untyped_handle &cb);

    /// @}

    /// @name Pre write callback handling
    /// @{

    /// Register an untyped pre write callback.
    /**
     * @param cb Untyped pre write callback
     * @param cci_untyped_tag Untyped tag to avoid compiler ambiguity
     *
     * @return Untyped callback handle
     */
    cci_callback_untyped_handle
    register_pre_write_callback(
            const cci_param_pre_write_callback_untyped& cb,
            cci_untyped_tag = cci_untyped_tag());

    /// Register an untyped pre write callback with a method as callback
    /**
     * @param cb Untyped validate write callback method
     * @param obj Associated object instance pointer
     * @param cci_untyped_tag Untyped tag to avoid compiler ambiguity
     *
     * @return Untyped callback handle
     */
    template<typename C>
    cci_callback_untyped_handle
    register_pre_write_callback(
            cci_param_pre_write_callback_untyped::signature(C::*cb),
            C* obj, cci_untyped_tag = cci_untyped_tag())
    {
        return register_pre_write_callback(
                sc_bind(cb, obj, sc_unnamed::_1));
    }

    /// Unregister a pre write callback handle
    /**
     * @param cb Untyped pre write callback handle
     *
     * @return True if unregister is a success. Otherwise False.
     */
    bool
    unregister_pre_write_callback(const cci_callback_untyped_handle &cb);

    /// @}

    /// @name Pre read callback handling
    /// @{

    /// Register an untyped pre read callback.
    /**
     * @param cb Untyped pre read callback
     * @param cci_untyped_tag Untyped tag to avoid compiler ambiguity
     *
     * @return Untyped callback handle
     */
    cci_callback_untyped_handle
    register_pre_read_callback(const cci_param_pre_read_callback_untyped& cb,
                               cci_untyped_tag = cci_untyped_tag());

    /// Register an untyped pre read callback with a method as callback
    /**
     * @param cb Untyped pre read callback method
     * @param obj Associated object instance pointer
     * @param cci_untyped_tag Untyped tag to avoid compiler ambiguity
     *
     * @return Untyped callback handle
     */
    template<typename C>
    cci_callback_untyped_handle
    register_pre_read_callback(cci_param_pre_read_callback_untyped::signature
                               (C::*cb), C* obj,
                               cci_untyped_tag = cci_untyped_tag())
    {
        return register_pre_read_callback(sc_bind(cb, obj, sc_unnamed::_1));
    }

    /// Unregister a pre read callback handle
    /**
     * @param cb Untyped pre read callback handle
     *
     * @return True if unregister is a success. Otherwise False.
     */
    bool
    unregister_pre_read_callback(const cci_callback_untyped_handle &cb);

    /// @}

    /// @name Post read callback handling
    /// @{

    /// Register an untyped post read callback.
    /**
     * @param cb Untyped post read callback
     * @param cci_untyped_tag Untyped tag to avoid compiler ambiguity
     *
     * @return Untyped callback handle
     */
    cci_callback_untyped_handle
    register_post_read_callback(const cci_param_post_read_callback_untyped& cb,
                               cci_untyped_tag = cci_untyped_tag());

    /// Register an untyped post read callback with a method as callback
    /**
     * @param cb Untyped post read callback method
     * @param obj Associated object instance pointer
     * @param cci_untyped_tag Untyped tag to avoid compiler ambiguity
     *
     * @return Untyped callback handle
     */
    template<typename C>
    cci_callback_untyped_handle
    register_post_read_callback(cci_param_post_read_callback_untyped::signature
                               (C::*cb), C* obj,
                               cci_untyped_tag = cci_untyped_tag())
    {
        return register_post_read_callback(sc_bind(cb, obj, sc_unnamed::_1));
    }

    /// Unregister a post read callback handle
    /**
     * @param cb Untyped post read callback handle
     *
     * @return True if unregister is a success. Otherwise False.
     */
    bool
    unregister_post_read_callback(const cci_callback_untyped_handle &cb);

    /// @}

    /// @name Callback handling
    /// @{

    /// Unregister all callbacks
    /**
     * @return True if success, otherwise False
     */
    bool unregister_all_callbacks();

    /// Returns if the parameter has registered callbacks.
    /**
     * @return True if the parameter has registered callbacks, otherwise False
     */
    bool has_callbacks() const;

    /// @}

    /// Add parameter handle
    /**
     * Add a parameter handle associated with this parameter.
     *
     * @param param_handle Parameter handle to add.
     */
    void add_param_handle(cci_param_untyped_handle* param_handle);

    /// Remove parameter handle
    /**
     * Remove a parameter handle associated with this parameter.
     *
     * @param param_handle Parameter handle to remove.
     */
    void remove_param_handle(cci_param_untyped_handle* param_handle);


protected:
    /// @name Post callback handling implementation
    /// @{

    /// Register a post write callback handle
    /**
     * @param cb Untyped post write callback handle
     * @param cci_originator Originator
     *
     * @return Untyped callback handle
     */
    cci_callback_untyped_handle
    register_post_write_callback(const cci_callback_untyped_handle &cb,
                                 const cci_originator &orig);

    /// Unregister a post write callback handle
    /**
     * @param cb Untyped post write callback handle
     * @param cci_originator Originator
     *
     * @return True if unregister is a success. Otherwise False.
     */
    bool
    unregister_post_write_callback(const cci_callback_untyped_handle &cb,
                                   const cci_originator &orig);

    /// @}

    /// @name Pre write callback handling implementation
    /// @{

    /// Register a pre write callback handle
    /**
     * @param cb Untyped pre write callback handle
     * @param cci_originator Originator
     *
     * @return Untyped callback handle
     */
    cci_callback_untyped_handle
    register_pre_write_callback(const cci_callback_untyped_handle &cb,
                                     const cci_originator &orig);

    /// Unregister a pre write callback handle
    /**
     * @param cb Untyped pre write callback handle
     * @param cci_originator Originator
     *
     * @return True if unregister is a success. Otherwise False.
     */
    bool
    unregister_pre_write_callback(const cci_callback_untyped_handle &cb,
                                       const cci_originator &orig);

    /// @}

    /// @name Pre read callback handling implementation
    /// @{

    /// Register a pre read callback handle
    /**
     * @param cb Untyped pre read callback handle
     * @param cci_originator Originator
     *
     * @return Untyped callback handle
     */
    cci_callback_untyped_handle
    register_pre_read_callback(const cci_callback_untyped_handle &cb,
                               const cci_originator &orig);

    /// Unregister a pre read callback handle
    /**
     * @param cb Untyped pre read callback handle
     * @param cci_originator Originator
     *
     * @return True if unregister is a success. Otherwise False.
     */
    bool
    unregister_pre_read_callback(const cci_callback_untyped_handle &cb,
                                 const cci_originator &orig);

    /// @}

    /// @name Post read callback handling implementation
    /// @{

    /// Register a post read callback handle
    /**
     * @param cb Untyped post read callback handle
     * @param cci_originator Originator
     *
     * @return Untyped callback handle
     */
    cci_callback_untyped_handle
    register_post_read_callback(const cci_callback_untyped_handle &cb,
                                const cci_originator &orig);

    /// Unregister a post read callback handle
    /**
     * @param cb Untyped post read callback handle
     * @param cci_originator Originator
     *
     * @return True if unregister is a success. Otherwise False.
     */
    bool
    unregister_post_read_callback(const cci_callback_untyped_handle &cb,
                                  const cci_originator &orig);

    /// @}

    /// @name Callback handling implementation
    /// @{

    /// Unregister all callbacks (write, validate write and read callbacks).
    /**
     * @param cci_originator Originator
     *
     * @return True if unregister is a success. Otherwise False.
     */
    bool unregister_all_callbacks(const cci_originator &orig);

    ///@}

public:

    ///@name Write-access control
    ///@{

    /// Locking this parameter, optionally with a password.
    /**
     * Makes a parameter read-only.
     *
     * Returns false
     * - if this parameter was already locked with a different password (this call has no effect)
     *
     * Returns true
     * - if the parameter was not locked (and is locked now) or
     * - if the parameter was locked without a password. Then it is locked now with the given password
     * - if the parameter was locked with the given password pwd. Then it is still locked now with the given password.
     *
     * @param pwd Password needed to unlock the param, ideally any pointer
     * address known only by the locking entity. If no key is given
     * an internal 'key' (unique to the parameter) will be used.
     * @return If the lock was successful.
     */
    bool lock(const void* pwd = NULL);

    /// Unlocking this parameter, optionally with a password if needed.
    /**
     * @param pwd Password to unlock the param (if needed),
     * If no key is given an internal 'key' will be used.
     * @return If the parameter is unlocked now.
     */
    bool unlock(const void* pwd = NULL);

    /// If this parameter is locked.
    /**
    * @return If this parameter is locked
    */
    bool is_locked() const;

    ///@}

    ///@name Query parameter type and name
    ///@{

    /// Get the name of this parameter.
    /**
     * @return   Name of the parameter.
     */
    const char* name() const;

    ///@}

    ///@name Originator query
    ///@{

    /// Gets cci_originator of the parameter.
    /**
     * The originator reflects ownership of the parameter proxy, which points
     * to an implementation.  For a handle, the originator identifies the
     * entity accessing the parameter.  Otherwise, the originator reflects
     * the parameter's creator.
     */
    cci_originator get_originator() const;

    ///@}

protected:
    /// Constructor to create new parameter with given originator.
    cci_param_untyped(const std::string& name, cci_name_type name_type,
                      cci_broker_handle broker_handle, const std::string& desc,
                      const cci_originator& originator);

    /// check mutability
    bool set_cci_value_allowed(cci_param_mutable_type mutability);

protected:
    /// Name
    std::string m_name;

    /// Description
    std::string m_description;

    /// Metadata
    cci_value_map metadata;

    /// Passwort needed to unlock the parameter or override the lock
    const void* m_lock_pwd;

    /// Broker handle
    cci_broker_handle m_broker_handle;

    /// Stores the originator of the latest successful write access
    mutable cci_originator m_value_origin;

    /// Callback object
    template<class T>
    struct callback_obj {
        callback_obj(T cb, const cci_originator& orig):
                callback(cb), originator(orig) {}
        T callback;
        cci_originator originator;
    };

    /// Callback object vector with tag to avoid nested callback
    template<class T>
    struct callback_obj_vector {
      callback_obj_vector():oncall(false){};
      std::vector<T> vec;
      mutable bool oncall;
    };

    /// Pre write callbacks
    typedef callback_obj<typename cci_callback_untyped_handle::type>
            pre_write_callback_obj_t;

    callback_obj_vector<pre_write_callback_obj_t> m_pre_write_callbacks;

    /// Post write callbacks
    typedef callback_obj<typename cci_callback_untyped_handle::type>
            post_write_callback_obj_t;

    callback_obj_vector<post_write_callback_obj_t> m_post_write_callbacks;

    /// Pre read callbacks
    typedef callback_obj<typename cci_callback_untyped_handle::type>
            pre_read_callback_obj_t;

    callback_obj_vector<pre_read_callback_obj_t> m_pre_read_callbacks;

    /// Post read callbacks
    typedef callback_obj<typename cci_callback_untyped_handle::type>
            post_read_callback_obj_t;

    callback_obj_vector<pre_read_callback_obj_t> m_post_read_callbacks;

    /// Originator of the parameter
    const cci_originator m_originator;

private:
    /// @copydoc cci_param_if::invalidate_all_param_handles
    virtual void invalidate_all_param_handles();

    /// Parameter handles
    std::vector<cci_param_untyped_handle*> m_param_handles;

protected:
    bool fast_read, fast_write;
};

CCI_CLOSE_NAMESPACE_
#endif // CCI_CFG_CCI_PARAM_UNTYPED_H_INCLUDED_
