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

#ifndef CCI_UTILS_CONSUMING_BROKER_H_INCLUDED_
#define CCI_UTILS_CONSUMING_BROKER_H_INCLUDED_

#include <map>
#include <set>

#include "cci/core/cci_name_gen.h"
#include "cci/cfg/cci_broker_if.h"
#include "cci/cfg/cci_broker_manager.h"
#include "cci/cfg/cci_config_macros.h"
#include "cci/cfg/cci_param_if.h"
#include "cci/cfg/cci_report_handler.h"
#include "cci/cfg/cci_broker_callbacks.h"

namespace cci_utils {

  /// (Non-private) consuming_broker implementation
  /**
   * 
   * Default Consuming_broker declaration
   * See cci_broker_if for details of the implemented API.
   *
   * This broker consumes all parameters, and does not have any mechanism to
   * pass parameters to a 'global' broker. It is therefore a good candidate for
   * the global broker.
   * 
   */
  class consuming_broker: public cci::cci_broker_if
  {
protected:
    // This consuming_broker is specialized as the normal 'broker'
    // but can be used as a global broker if desired.

public:
    cci::cci_originator get_value_origin(
      const std::string &parname) const;

    cci::cci_originator get_preset_value_origin(
        const std::string &parname) const;

    /// Constructor
    explicit consuming_broker(const std::string& name);

    /// Destructor
    ~consuming_broker();

    /// Return the name of the broker
    const char* name() const;

    /// Return the preset value of a parameter (by name)
    cci::cci_value get_preset_cci_value(const std::string &parname) const;

    /// Set the preset value of a parameter (by name, requires originator)
    void set_preset_cci_value(
      const std::string &parname,
      const cci::cci_value &cci_value,
      const cci::cci_originator& originator);

    /// Lock parameter
    void lock_preset_value(const std::string &parname);

    /// Get a full list of unconsumed preset values.
    std::vector<cci::cci_name_value_pair> get_unconsumed_preset_values() const;

    /// get vector of unconsumed preset values
    cci::cci_preset_value_range get_unconsumed_preset_values(
      const cci::cci_preset_value_predicate &pred) const;

    void ignore_unconsumed_preset_values(
      const cci::cci_preset_value_predicate &pred);

    /// Get current cci_value
    cci::cci_value get_cci_value(const std::string &parname,
        const cci::cci_originator& originator = cci::cci_originator()) const;
    
    /// return a handle with which to access a parameter
    cci::cci_param_untyped_handle get_param_handle(
      const std::string &parname,
      const cci::cci_originator& originator) const;

    std::vector<cci::cci_param_untyped_handle> get_param_handles(
      const cci::cci_originator& originator) const;

    cci::cci_param_range get_param_handles(cci::cci_param_predicate& pred,
                                      const cci::cci_originator& originator) const;

    bool has_preset_value(const std::string &parname) const;

    void add_param(cci::cci_param_if* par);

    void remove_param(cci::cci_param_if* par);
    
    virtual bool is_global_broker() const;

    cci::cci_param_create_callback_handle
    register_create_callback(const cci::cci_param_create_callback& cb,
                             const cci::cci_originator& orig);

    bool
    unregister_create_callback(const cci::cci_param_create_callback_handle& cb,
                               const cci::cci_originator& orig);

    cci::cci_param_destroy_callback_handle
    register_destroy_callback(const cci::cci_param_destroy_callback& cb,
                              const cci::cci_originator& orig);

    bool
    unregister_destroy_callback(const cci::cci_param_destroy_callback_handle& cb,
                                const cci::cci_originator& orig);

    bool unregister_all_callbacks(const cci::cci_originator& orig);

    bool has_callbacks() const;
    
  protected:
    /// Get original parameter (internal method)
    cci::cci_param_if* get_orig_param(const std::string &parname) const;

    std::string m_name;

    // These are used as a database of _preset_ values.
    std::map<std::string, cci::cci_param_if*> m_param_registry;
    std::map<std::string, cci::cci_value> m_unused_value_registry;
    std::map<std::string, cci::cci_value> m_used_value_registry;
    
    // store the list of locked param's (guessing there are many fewer of these
    // than actual params, so hopefully this is more efficient than adding a
    // boolean above)
    std::set<std::string> locked;

    /// Map to save the latest write originator when preset values are set
    std::map<std::string, cci::cci_originator> m_preset_value_originator_map;

    template<class T>
    struct callback_obj {
      callback_obj(T cb, const cci::cci_originator& orig):
              callback(cb), originator(orig) {}
      T callback;
      cci::cci_originator originator;
    };

    typedef callback_obj<typename cci::cci_param_create_callback_handle::type>
          create_callback_obj_t;

    /// Create callbacks
    std::vector<create_callback_obj_t> m_create_callbacks;

    typedef callback_obj<typename cci::cci_param_destroy_callback_handle::type>
          destroy_callback_obj_t;

    /// Destroy callbacks
    std::vector<destroy_callback_obj_t> m_destroy_callbacks;

    /// Ignored unconsumed preset cci values
    std::vector<cci::cci_preset_value_predicate> m_ignored_unconsumed_predicates;
  };

}
#endif
