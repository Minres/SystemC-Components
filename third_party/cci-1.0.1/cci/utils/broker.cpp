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

#include "cci/cfg/cci_param_if.h"
#include "cci/utils/broker.h"
#include "cci/core/cci_name_gen.h"

namespace cci_utils {
  using namespace cci;
  

/*
 * private function to determine if we send to the parent broker or not
 */
  bool broker::sendToParent(const std::string &parname) const
  {
    return  ((expose.find(parname) != expose.end()) && (!is_global_broker()));
  }

/*
 * public interface functions
 */
  broker::broker(const std::string& name)
    : consuming_broker(name),
    m_parent(get_parent_broker()) // local convenience function
    { 
      sc_assert (name.length() > 0 && "Name must not be empty");
    }

  broker::~broker()
  {
  }

  cci_originator broker::get_value_origin(const std::string &parname) const
  {
    if (sendToParent(parname)) {
      return m_parent.get_value_origin(parname);
    } else {
      return consuming_broker::get_value_origin(parname);
    }
  }
  
  bool broker::has_preset_value(const std::string &parname) const
  {
    if (sendToParent(parname)) {
      return m_parent.has_preset_value(parname);
    } else {
      return consuming_broker::has_preset_value(parname);
    }
  }

  cci_value broker::get_preset_cci_value(const std::string &parname) const
  {
    if (sendToParent(parname)) {
      return m_parent.get_preset_cci_value(parname);
    } else {
      return consuming_broker::get_preset_cci_value(parname);
    }
  }

  void broker::lock_preset_value(const std::string &parname)
  {
    if (sendToParent(parname)) {
      return m_parent.lock_preset_value(parname);
    } else {
      return consuming_broker::lock_preset_value(parname);
    }
  }

  cci_value broker::get_cci_value(const std::string &parname,
    const cci_originator &originator) const
  {
    if (sendToParent(parname)) {
      return m_parent.get_cci_value(parname);
    } else {
      return consuming_broker::get_cci_value(parname);
    }
  }

  void broker::add_param(cci_param_if* par)
  {
    if (sendToParent(par->name())) {
      return m_parent.add_param(par);
    } else {
      return consuming_broker::add_param(par);
    }
  }

  void broker::remove_param(cci_param_if* par) {
    if (sendToParent(par->name())) {
      return m_parent.remove_param(par);
    } else {
      return consuming_broker::remove_param(par);
    }
  }

// Functions below here require an orriginator to be passed to the local
// method variant.

  void broker::set_preset_cci_value(
    const std::string &parname,
    const cci_value &cci_value,
    const cci_originator& originator)
  {
    if (sendToParent(parname)) {
      return m_parent.set_preset_cci_value(parname,cci_value, originator);
    } else {
      return consuming_broker::set_preset_cci_value(parname,cci_value,originator);
    }
  }
  cci_param_untyped_handle broker::get_param_handle(
    const std::string &parname,
    const cci_originator& originator) const
  {
    if (sendToParent(parname)) {
      return m_parent.get_param_handle(parname, originator);
    }
    cci_param_if* orig_param = get_orig_param(parname);
    if (orig_param) {
      return cci_param_untyped_handle(*orig_param, originator);
    }
    if (has_parent) {
      return m_parent.get_param_handle(parname, originator);
    }
    return cci_param_untyped_handle(originator);
  }


  std::vector<cci_param_untyped_handle>
    broker::get_param_handles(const cci_originator& originator) const
  {
    if (has_parent) {
      std::vector<cci_param_untyped_handle> p_param_handles=m_parent.get_param_handles();
      std::vector<cci_param_untyped_handle> param_handles=consuming_broker::get_param_handles(originator);
      // this is likely to be more efficient the other way round, but it keeps
      // things consistent and means the local (mre useful) params will be at the
      // head of the list.
      param_handles.insert(param_handles.end(),p_param_handles.begin(), p_param_handles.end());
      return param_handles;
    } else {
      return consuming_broker::get_param_handles(originator);
    }
  }

  bool broker::is_global_broker() const
  {
    return  (!has_parent);
  }

}
