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

#ifndef CCI_CFG_CCI_BROKER_IF_H_INCLUDED_
#define CCI_CFG_CCI_BROKER_IF_H_INCLUDED_

#include "cci_cfg/cci_broker_types.h"
#include "cci_cfg/cci_param_untyped_handle.h"

CCI_OPEN_NAMESPACE_

// forward declaration
class cci_param_if;

// forward declaration
template<class T>
class cci_param_typed_handle;

// forward declaration for friend class
class cci_broker_handle;

/**
 * @brief CCI configuration broker interface
 *
 * This class provides the core interface for "tools" accessing
 * the availabe configuration parameters in a model.  Parameters
 * are accessed via brokers, implementing this interface.
 *
 * Usually, brokers are not accessed directly, but via broker
 * handles, including an additional cci_originator for access
 * tracking.
 *
 * The functions of this interface can be grouped as follows:
 * @li Providing initial preset values for parameters (configuration)
 * @li Parameter registration and unregistration
 * @li Parameter lookup and enumeration
 * @li Callback handling
 *
 * @see cci_broker_handle, cci_get_broker(), cci_register_broker()
 *
 * For convenience, the CCI PoC implementation provides two
 * simple brokers:
 * @see cci_utils::broker, cci_utils::consuming_broker
 */
class cci_broker_if
 : public cci_broker_callback_if
{
public:
    friend class cci_broker_handle;
    friend class cci_param_if;

    /**
     * @brief Convenience function to create a new broker handle
     * @param originator Originator for access tracking (forwarded to parameter handles)
     * @return broker handle for the given (or current) originator
     * @note  The default argument can only be reliably used from within
     *        the SystemC hierarchy during elaboration.
     * @see cci_originator
     */
    cci_broker_handle
    create_broker_handle(const cci_originator &originator = cci_originator()) const;

    /// Name of this broker
    /**
     * Shall be a system-wide unique broker name.
     * Broker handles shall return their underlying broker's name.
     *
     * @return broker name
     */
    virtual const char* name() const = 0;

    // //////////////////////////////////////////////////////////////////// //
    // ///////////   Access Parameters and Values   /////////////////////// //

    /// Set a parameter's preset value.
    /**
     * The preset value has priority to the default value being set by the owner!
     *
     * @exception        cci::cci_report::set_param_failed Setting parameter
     *                   object failed
     * @param parname    Full hierarchical parameter name.
     * @param cci_value  cci::cci_value representation of the preset value the
     *                   parameter has to be set to.
     * @param originator originator reference to the originator
     *                   (not applicable in case of broker handle)
     */
    virtual void set_preset_cci_value(const std::string &parname,
                                       const cci::cci_value &cci_value,
                                       const cci_originator& originator) = 0;

    /// Get a parameter's preset value.
    /**
     *
     * @param parname    Full hierarchical parameter name.
     * @return           CCI value of the parameter's preset value. Empty value is returned when parameter is not existing or its preset value is not existing
     */
    virtual cci::cci_value
    get_preset_cci_value(const std::string &parname) const = 0;

    /// Get unconsumed preset values
    /**
     * Querying of unconsumed values. An unconsumed value is an "preset" value
     * supplied to a broker for which no parameter is ever created.
     *
     * @return           Vector of unconsumed parameter's preset value.
     */
    virtual std::vector<cci_name_value_pair> get_unconsumed_preset_values() const = 0;

    /// Get unconsumed preset values with a user-defined predicate callback
    /**
     * Querying of unconsumed values. An unconsumed value is an "preset" value
     * supplied to a broker for which no parameter is ever created.
     *
     * @param pred       Callback to filter unconsumed preset values.
     * @return           Vector of unconsumed parameter's preset value.
     */
    virtual cci_preset_value_range get_unconsumed_preset_values(
            const cci_preset_value_predicate &pred) const = 0;

    /// Ignore unconsumed preset values
    /**
     * Filtering of unconsumed values. An unconsumed value is an "preset" value
     * supplied to a broker for which no parameter is ever created. This method
     * affects future calls to @see get_unconsumed_preset_values().
     *
     * @param pred       Callback to ignore unconsumed preset values.
     */
    virtual void ignore_unconsumed_preset_values(
            const cci_preset_value_predicate &pred) = 0;

    /// Returns the originator that wrote the parameter's current value.
    /**
     * @param parname  Name of a parameter.
     * @return originator 
     */
    virtual cci_originator
    get_value_origin(const std::string &parname) const = 0;

    /// Returns originator of the preset value if a preset value exists, otherwise unknown originator.
    /**
    * @param parname  Name of a parameter.
    * @return originator which will be the unknown originator when no preset value exists
    */
    virtual cci_originator
        get_preset_value_origin(const std::string &parname) const = 0;

    /// Lock a parameter's preset value.
    /**
     * Lock so that this parameter's preset value cannot be overwritten by
     * any subsequent set_preset_value call. This allows to emulate a hierarchical
     * precendence since a top-level module can prevent the childs from setting
     * preset values by locking the preset value before creating the subsystem.
     *
     * Throws (and does not lock) if no preset value exists
     * that can be locked or if a preset value is already locked or if the
     * parameter is already existing as object.
     *
     * @exception     cci::cci_report::set_param_failed Locking parameter object failed
     * @param parname Hierarchical parameter name.
     */
    virtual void lock_preset_value(const std::string &parname) = 0;

    /// Get a parameter's value (CCI value representation).
    /**
     * Returns the value of the named parameter.
     *
     * @param parname  Full hierarchical name of the parameter whose value should be returned.
     * @param   originator Reference to the originator
     * @return  CCI value of the parameter
     */
    virtual cci::cci_value get_cci_value(const std::string &parname,
        const cci_originator &originator = cci_originator()) const = 0;
    
    /// Get a parameter handle.
    /**
     * This returns not the owner's parameter object but a handle.
     *
     * @param   parname    Full hierarchical parameter name.
     * @param   originator Reference to the originator
     * @return  Parameter handle object (invalid if not existing).
     */
    virtual cci_param_untyped_handle
    get_param_handle(const std::string &parname,
                     const cci_originator& originator) const = 0;

    /// Returns if the parameter has a preset value
    /**
     * @param parname  Full hierarchical parameter name.
     * @return If the parameter has a preset value set
     */
    virtual bool has_preset_value(const std::string &parname) const = 0;

    // //////////////////////////////////////////////////////////////////// //
    // ///////////////   Registry Functions   ///////////////////////////// //

    /// Adds a parameter to the registry.
    /**
     * Note: addPar (and all related methods) must not call any of the
     *       pure virtual functions in cci_param_untyped_handle because
     *       this method may be called by the cci_param_untyped_handle
     *       constructor.
     *
     * Note: This function shall never been called for any parameter handle
     *       objects but only for "real" parameter objects.
     *
     * @exception cci::cci_report::add_param_failed Adding parameter object
     *            failed
     * @param par Parameter (including name and value).
     */
    virtual void add_param(cci_param_if *par) = 0;

    /// Removes a parameter from the registry. May only be called by the
    /// parameter destructor, must not be called by anyone else.
    /**
     * It should be ensured this is not being called from elsewhere than the
     * parameter destructor (e.g. by user).
     *
     * Note: This function shall never been called for any parameter handle
     *       objects but only for "real" parameter objects.
     *
     * @exception cci::cci_report::remove_param_failed Remove parameter object
     *            failed
     * @param par Parameter pointer.
     */
    virtual void remove_param(cci_param_if *par) = 0;

    /// Return a list of all parameter handles in the given scope
    /**
     *
     * This returns not the owner's parameter objects but handles.
     *
     * @param pattern Specifies the parameters to be returned.
     * @return Vector with parameter handles.
     */
    virtual std::vector <cci_param_untyped_handle>
    get_param_handles(const cci_originator& originator = cci_originator()) const = 0;

    /// Search parameters with a user-defined predicate callback
    /**
     * @param pred Callback to filter parameters
     * @return cci_iterable Iterable parameters
     */
    virtual cci_param_range
    get_param_handles(cci_param_predicate& pred,
                      const cci_originator& originator) const = 0;

    ///If this broker is a private broker (or handle)
    /**
     * @return If this broker is the global broker
     */
    virtual bool is_global_broker() const = 0;

protected:
    /// Default Constructor
    cci_broker_if() {}

    /// Destructor
    virtual ~cci_broker_if() {}

    cci_originator unknown_originator() const
      { return cci_originator( cci_originator::unknown_tag() ); }

    static cci_broker_if &unwrap_broker(cci_broker_handle h)
    { return h.ref(); }

private:
    // Disabled
    cci_broker_if(const cci_broker_if&);
    cci_broker_if& operator=(const cci_broker_if&);
#ifdef CCI_HAS_CXX_RVALUE_REFS
    cci_broker_if(cci_broker_if&&);
    cci_broker_if& operator=(cci_broker_if&&);
#endif
};

inline cci_broker_handle
cci_broker_if::create_broker_handle(const cci_originator& originator) const
{
  return cci_broker_handle(*const_cast<cci_broker_if*>(this), originator);
}

CCI_CLOSE_NAMESPACE_

#endif // CCI_CFG_CCI_BROKER_IF_H_INCLUDED_
