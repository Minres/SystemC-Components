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

#ifndef CCI_CFG_CCI_BROKER_HANDLE_H_INCLUDED_
#define CCI_CFG_CCI_BROKER_HANDLE_H_INCLUDED_

#include "cci/core/cci_cmnhdr.h"
#include "cci/core/cci_value.h"
#include "cci/cfg/cci_broker_types.h"
#include "cci/cfg/cci_originator.h"
#include "cci/cfg/cci_param_typed_handle.h"

/**
 * @author Guillaume Delbergue, Ericsson / GreenSocs
 */

CCI_OPEN_NAMESPACE_

// Forward declarations
class cci_param_if;
class cci_broker_if;
template<typename T> class cci_param_typed_handle;

// CCI Broker handle class
/**
* Broker handle class informing broker of originator when required.
* Provides a broker-like interface.
*/
class cci_broker_handle
{
public:
    /// Constructor to create handle with given originator.
    cci_broker_handle(cci_broker_if& broker,
                      const cci_originator& originator);

    cci_broker_handle(const cci_broker_handle& broker_handle)
      : m_broker(broker_handle.m_broker)
      , m_originator(promote_originator(broker_handle.m_originator)) {}

    cci_broker_handle& operator=(const cci_broker_handle& broker_handle)
    {
        m_broker = broker_handle.m_broker;
        // originator is preserved during assignment
        return *this;
    }

    cci_broker_handle(cci_broker_handle&& that)
      : m_broker(CCI_MOVE_(that.m_broker)) 
      , m_originator(promote_originator(that.m_originator)) {}

    cci_broker_handle& operator=(cci_broker_handle&& that)
    {
      m_broker = CCI_MOVE_(that.m_broker);
      // originator is preserved during assignment
      return *this;
    }

    ~cci_broker_handle() = default;

    /// @copydoc cci_broker_if::create_broker_handle
    cci_broker_handle
    create_broker_handle(const cci_originator &originator = cci_originator());

    /// Query the originator of the broker.
    /**
     * The originator identifies the entity accessing the broker
     * through this handle.
     */
    cci_originator get_originator() const;

    /// @copydoc cci_broker_if::name
    const char* name() const;

    /// @copydoc cci_broker_if::set_preset_cci_value
    void set_preset_cci_value(const std::string &parname,
                               const cci_value &cci_value);

    /// @copydoc cci_broker_if::get_preset_cci_value
    cci_value get_preset_cci_value(const std::string &parname) const;

    /// @copydoc cci_broker_if::get_unconsumed_preset_values()
    std::vector<cci_name_value_pair> get_unconsumed_preset_values() const;

    /// @copydoc cci_broker_if::has_preset_value
    bool has_preset_value(const std::string &parname) const;

    /// @copydoc cci_broker_if::get_unconsumed_preset_values(const cci_preset_value_predicate&)
    cci_preset_value_range get_unconsumed_preset_values(
            const cci_preset_value_predicate &pred) const;

    /// @copydoc cci_broker_if::ignore_unconsumed_preset_values
    void ignore_unconsumed_preset_values(
            const cci_preset_value_predicate &pred);

    /// @copydoc cci_broker_if::get_value_origin
    cci_originator
    get_value_origin(const std::string &parname) const;

    /// @copydoc cci_broker_if::get_preset_value_origin
    cci_originator
    get_preset_value_origin(const std::string &parname) const;

    /// @copydoc cci_broker_if::lock_preset_value
    void lock_preset_value(const std::string &parname);

    /// @copydoc cci_broker_if::get_cci_value
    cci_value get_cci_value(const std::string &parname) const;

    /// @copydoc cci_broker_if::add_param
    void add_param(cci_param_if *par);

    /// @copydoc cci_broker_if::remove_param
    void remove_param(cci_param_if *par);

    /// @copydoc cci_broker_if::get_param_handles(const std::string&)
    std::vector <cci_param_untyped_handle> get_param_handles() const;

    /// @copydoc cci_broker_if::get_param_handles(cci_param_predicate&)
    cci_param_range
    get_param_handles(cci_param_predicate& pred) const;

    /// @copydoc cci_broker_if::get_param_handle
    cci_param_untyped_handle get_param_handle(const std::string &parname) const;

    /// Convenience function to get a typed parameter handle.
    /**
     * @param   parname   Full hierarchical parameter name.
     * @return  Parameter handle (invalid if not existing or the type is not correct)
     */
    template<class T>
    cci_param_typed_handle<T> get_param_handle(const std::string &parname) const {
        return cci_param_typed_handle<T>(get_param_handle(parname));
    }

    /** @name Parameter creation/destruction callbacks */
    //@{
    /// @copydoc cci_broker_callback_if::register_create_callback
    cci_param_create_callback_handle
    register_create_callback(const cci_param_create_callback& cb);

    /**
     * @brief Convenience overload to register callback for a member function
     * @param cb  Member function pointer to be used as callback
     * @param obj Object to invoke member function on
     * @see register_create_callback(const cci_param_create_callback&)
     */
    template<typename Owner>
    cci_param_create_callback_handle
    register_create_callback( cci_param_create_callback::signature (Owner::*cb)
                            , Owner* obj )
      { return register_create_callback( sc_bind( cb, obj, sc_unnamed::_1 ) ); }

    /// @copydoc cci_broker_callback_if::unregister_create_callback
    bool unregister_create_callback(const cci_param_create_callback_handle& cb);

    /// @copydoc cci_broker_callback_if::register_destroy_callback
    cci_param_destroy_callback_handle
    register_destroy_callback(const cci_param_destroy_callback& cb);

    /**
     * @brief Convenience overload to register callback for a member function
     * @param cb  Member function pointer to be used as callback
     * @param obj Object to invoke member function on
     * @see register_destroy_callback(const cci_param_destroy_callback&)
     */
    template<typename Owner>
    cci_param_destroy_callback_handle
    register_destroy_callback( cci_param_destroy_callback::signature (Owner::*cb)
                             , Owner* obj )
      { return register_destroy_callback( sc_bind( cb, obj, sc_unnamed::_1 ) ); }

    /// @copydoc cci_broker_callback_if::unregister_destroy_callback
    bool
    unregister_destroy_callback(const cci_param_destroy_callback_handle& cb);

    /// @copydoc cci_broker_callback_if::unregister_all_callbacks
    bool unregister_all_callbacks();

    /// @copydoc cci_broker_callback_if::has_callbacks
    bool has_callbacks() const;
    //@}

    /// @copydoc cci_broker_if::is_global_broker
    bool is_global_broker() const;

    /// equality between this handle and a broker pointer (typically 'this'
    /// broker)
    /**
     * @param pointer to a broker
     * @return equality
     */
    bool operator==(const cci_broker_if *b) const {
      return m_broker==b;
    }

    /**
     * @param pointer to a broker
     * @return inequality
     */
    bool operator!=(const cci_broker_if *b) const {
      return m_broker!=b;
    }
protected:
    /// Promote a gifted originator to one that represents the current context
    /// when possible (i.e. when within the module hierarchy)
    /**
     * @param gifted_originator associated with the copy ctor broker argument
     * @return context originator if possible; otherwise, the gifted_originator 
     */
    inline const cci_originator promote_originator(const cci_originator &gifted_originator);

private:
    friend class cci_broker_if;
    friend class cci_param_if;
    cci_broker_if&       ref()       { return *m_broker; }
    const cci_broker_if& ref() const { return *m_broker; }

private:
    cci_broker_if* m_broker;
    cci_originator m_originator;
};



const cci_originator cci_broker_handle::promote_originator(
    const cci_originator &gifted_originator)
{
    if (sc_core::sc_get_current_object())
        return cci_originator();
    else
        return gifted_originator;
}

CCI_CLOSE_NAMESPACE_
#endif // CCI_CFG_CCI_BROKER_HANDLE_H_INCLUDED_
