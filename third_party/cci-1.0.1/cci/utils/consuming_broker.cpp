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
 * @author GreenSocs
 */

#include "cci/utils/consuming_broker.h"


namespace cci_utils {
  using namespace cci;
  
// NB this broker must be instanced and registered in the same place
//
  consuming_broker::consuming_broker(const std::string& name)
    : m_name(cci_gen_unique_name(name.c_str()))
    {
      sc_assert (name.length() > 0 && "Name must not be empty");
    }

  consuming_broker::~consuming_broker()
  {
  }

  const char* consuming_broker::name() const
  {
    return m_name.c_str();
  }

  void consuming_broker::set_preset_cci_value(
    const std::string &parname,
    const cci_value & value,
    const cci_originator& originator)
  {
    if (locked.find(parname) != locked.end()) {
      cci_report_handler::set_param_failed("Setting preset value failed (parameter locked).");
      return;
    }

    std::map<std::string,cci_value>::const_iterator iter =
      m_used_value_registry.find(parname);
    if (iter != m_used_value_registry.end() ) {
      m_used_value_registry[parname]=value; // kiss a zombee
      /* here, one could build a broker that, e.g. allowed writes to a param
         during elaboration. We choose not to, the user may reset the value at eoe
         if they choose*/
    } else {
      m_unused_value_registry[parname] = value;
    }

    // Store originator of the preset value. Can't use index operator since
    // null construction of an originator is prohibited outside the module hierarchy.
    // m_preset_value_originator_map[parname] = originator;
    std::map<std::string, cci_originator>::iterator it;
    it = m_preset_value_originator_map.find(parname);
    if (it != m_preset_value_originator_map.end())
        it->second = originator;
    else 
        m_preset_value_originator_map.insert(
            std::pair<std::string, cci_originator>(parname, originator));
  }

  std::vector<cci_name_value_pair> consuming_broker::get_unconsumed_preset_values() const
  {
    std::vector<cci_name_value_pair> unconsumed_preset_cci_values;
    std::map<std::string, cci_value>::const_iterator iter;
    std::vector<cci_preset_value_predicate>::const_iterator pred;

    for( iter = m_unused_value_registry.begin(); iter != m_unused_value_registry.end(); ++iter ) {
      for (pred =  m_ignored_unconsumed_predicates.begin(); pred !=  m_ignored_unconsumed_predicates.end(); ++pred) {
        const cci_preset_value_predicate &p=*pred; // get the actual predicate
        if (p(std::make_pair(iter->first, iter->second))) {
          break;
        }
      }
      if (pred==m_ignored_unconsumed_predicates.end()) {
        unconsumed_preset_cci_values.push_back(std::make_pair(iter->first, iter->second));
      }
    }
    return unconsumed_preset_cci_values;
  }

  cci_preset_value_range consuming_broker::get_unconsumed_preset_values(
    const cci_preset_value_predicate &pred) const
  {
    return cci_preset_value_range(pred, get_unconsumed_preset_values());
  }

  void consuming_broker::ignore_unconsumed_preset_values(const cci_preset_value_predicate &pred)
  {
    m_ignored_unconsumed_predicates.push_back(pred);
  }

  cci_originator consuming_broker::get_value_origin(const std::string &parname) const
  {
    cci_param_if* p = get_orig_param(parname);
    if (p) {
      return p->get_value_origin();
    }
    std::map<std::string, cci_originator>::const_iterator it;
    it = m_preset_value_originator_map.find(parname);
    if (it != m_preset_value_originator_map.end()) {
      return it->second;
    }
    // if the param doesn't exist, we should return 'unkown_originator'
    return cci_broker_if::unknown_originator();
  }

  cci_originator consuming_broker::get_preset_value_origin(const std::string &parname) const
  {
    std::map<std::string, cci_originator>::const_iterator it;
    it = m_preset_value_originator_map.find(parname);
    if (it != m_preset_value_originator_map.end())
      return it->second;
    // if no preset value, return 'unknown originator'
    return cci_broker_if::unknown_originator();
  }

  cci_value consuming_broker::get_preset_cci_value(const std::string &parname) const
  {
    {
      std::map<std::string,cci_value>::const_iterator iter =
        m_used_value_registry.find(parname);
      if (iter != m_used_value_registry.end() ) {
        return iter->second;
      }
    }
    {
      std::map<std::string,cci_value>::const_iterator iter =
        m_unused_value_registry.find(parname);
      if (iter != m_unused_value_registry.end() ) {
        return iter->second;
      }
    }
// If there is nothing in the database, return NULL.
    return cci_value();
  }

  void consuming_broker::lock_preset_value(const std::string &parname)
  {
    // no error is possible. Even if the parameter does not yet exist.
    locked.insert(parname);
  }

  cci_value consuming_broker::get_cci_value(const std::string &parname,
    const cci_originator &originator) const
  {
    cci_param_if* p = get_orig_param(parname);
    if(p) {
      return p->get_cci_value(originator);
    } else {
      std::map<std::string,cci_value>::const_iterator iter =
        m_unused_value_registry.find(parname);
      if (iter != m_unused_value_registry.end() ) {
        return iter->second;
      }
      cci_report_handler::get_param_failed("Unable to find the parameter to get value");
      return cci_value();
    }
  }

  cci_param_if* consuming_broker::get_orig_param(
    const std::string &parname) const
  {
    std::map<std::string,cci_param_if*>::const_iterator iter =
      m_param_registry.find(parname);
    if( iter != m_param_registry.end() ) {
      cci_param_if* ret = iter->second;
      sc_assert(ret != NULL && "This param shall be a cci_param_if!");
      return ret;
    }
    else return NULL;
  }

/*
 * This entire broker can be re-used as a 'greedy' private broker, the broker
 * should remain the same, except for when a model asks for a param handle for a
 * param that this broker has no knowledge of, at which point it should ask up
 * the broker tree. This case is _NOT_ handled below. The intention is that a well
 * behaved private broker can then 'wrap' this broker, passing 'public' params up
 * the broker tree and bypassing this broker, while this broker will handle all
 * 'private' params. To see the mechanism, see the private broker.
 */

  cci_param_untyped_handle consuming_broker::get_param_handle(
    const std::string &parname,
    const cci_originator& originator) const
  {
    cci_param_if* orig_param = get_orig_param(parname);
    if (orig_param) {
      return cci_param_untyped_handle(*orig_param, originator);
    }
    return cci_param_untyped_handle(originator);
  }

  bool consuming_broker::has_preset_value(const std::string &parname) const
  {
    {
      std::map<std::string,cci_value>::const_iterator iter =
        m_used_value_registry.find(parname);
      if (iter != m_used_value_registry.end() ) {
        return true;
      }
    }
    {
      std::map<std::string,cci_value>::const_iterator iter =
        m_unused_value_registry.find(parname);
      if (iter != m_unused_value_registry.end() ) {
        return true;
      }
    }
    return false;
  }

  cci_param_create_callback_handle
    consuming_broker::register_create_callback(
      const cci_param_create_callback &cb,
      const cci_originator &orig) {
    m_create_callbacks.push_back(create_callback_obj_t(cb, orig));
    return cb;
  }

  bool
    consuming_broker::unregister_create_callback(
      const cci_param_create_callback_handle &cb,
      const cci_originator &orig) {
    std::vector<create_callback_obj_t>::iterator it;
    for(it=m_create_callbacks.begin() ; it < m_create_callbacks.end(); it++ )
    {
      if(it->callback == cb && it->originator == orig) {
        m_create_callbacks.erase(it);
        return true;
      }
    }
    return false;
  }

  cci_param_destroy_callback_handle
    consuming_broker::register_destroy_callback(
      const cci_param_destroy_callback &cb,
      const cci_originator& orig) {
    m_destroy_callbacks.push_back(destroy_callback_obj_t(cb, orig));
    return cb;
  }

  bool
    consuming_broker::unregister_destroy_callback(
      const cci_param_destroy_callback_handle &cb,
      const cci_originator &orig) {
    std::vector<destroy_callback_obj_t>::iterator it;
    for(it=m_destroy_callbacks.begin() ; it < m_destroy_callbacks.end(); it++ )
    {
      if(it->callback == cb && it->originator == orig) {
        m_destroy_callbacks.erase(it);
        return true;
      }
    }
    return false;
  }

  bool consuming_broker::unregister_all_callbacks(
    const cci_originator &orig) {
    bool result = false;
    std::vector<create_callback_obj_t>::iterator it;
    for(it=m_create_callbacks.begin() ; it < m_create_callbacks.end(); it++ )
    {
      if(it->originator == orig) {
        m_create_callbacks.erase(it);
        result = true;
      }
    }
    std::vector<destroy_callback_obj_t>::iterator itt;
    for (itt = m_destroy_callbacks.begin();
         itt < m_destroy_callbacks.end(); itt++)
    {
      if(itt->originator == orig) {
        m_destroy_callbacks.erase(itt);
        result = true;
      }
    }
    return result;
  }

  bool consuming_broker::has_callbacks() const {
    return (!m_create_callbacks.empty() ||
            !m_destroy_callbacks.empty());
  }

  void consuming_broker::add_param(cci_param_if* par) {
    sc_assert(par != NULL && "Unable to add a NULL parameter");
    const std::string &par_name = par->name();
    bool new_element = m_param_registry.insert(
      std::pair<std::string, cci_param_if*>(par_name, par)).second;
    sc_assert(new_element && "The same parameter had been added twice!!");

    std::map<std::string,cci_value>::iterator iter =
      m_unused_value_registry.find(par_name);
    if (iter != m_unused_value_registry.end()  ) {
      m_used_value_registry.insert(std::make_pair(iter->first, iter->second));
      m_unused_value_registry.erase(iter);
    }
    // Create callbacks
    for (unsigned i = 0; i < m_create_callbacks.size(); ++i) {
      m_create_callbacks[i].callback.invoke(
        par->create_param_handle(par->get_originator()));
    }
  }

  void consuming_broker::remove_param(cci_param_if* par) {
    sc_assert(par != NULL && "Unable to remove a NULL parameter");
    m_param_registry.erase(par->name());

    // Destroy callbacks
    for (unsigned i = 0; i < m_destroy_callbacks.size(); ++i) {
        m_destroy_callbacks[i].callback.invoke(
            par->create_param_handle(par->get_originator()));
    }

    std::map<std::string,cci_value>::iterator iter =
      m_used_value_registry.find(par->name());
    if (iter != m_used_value_registry.end()  ) {
      m_unused_value_registry.insert(std::make_pair(iter->first, iter->second));
      m_used_value_registry.erase(iter);    
    }
  }

  std::vector<cci_param_untyped_handle>
    consuming_broker::get_param_handles(const cci_originator& originator) const
  {
    std::vector<cci_param_untyped_handle> param_handles;
    std::map<std::string,cci_param_if*>::const_iterator it;
    for (it=m_param_registry.begin(); it != m_param_registry.end(); ++it) {
      cci_param_if* p = it->second;
      param_handles.push_back(cci_param_untyped_handle(*p, originator));
    }
    return param_handles;
  }

  cci_param_range consuming_broker::get_param_handles(
    cci_param_predicate& pred,
    const cci_originator& originator) const
  {
    return cci_param_range(pred,
                           get_param_handles(originator));
  }

  bool consuming_broker::is_global_broker() const
  {
    return false;
  }

}
