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

#include "cci_cfg/cci_originator.h"

#include <cstring>

#include "cci_cfg/cci_config_macros.h"
#include "cci_cfg/cci_report_handler.h"

CCI_OPEN_NAMESPACE_

cci_originator::cci_originator(const std::string& originator_name)
  : m_originator_obj()
  , m_originator_str()
{
    if (current_originator_object()) {
      m_originator_obj = current_originator_object();
      if (originator_name.length() > 0) {
        CCI_REPORT_ERROR("cci_originator/name_inside_hierarchy","Originators must not be named inside the SystemC hierarchy");
      }
    } else {
      if (originator_name.length() > 0) {
        m_originator_str = new std::string(originator_name);
      } else {
        CCI_REPORT_ERROR("cci_originator/noname_outside_hierarchy","An originator name must be given outside the SystemC hierarchy");
      }
    }
    check_is_valid();
}

cci_originator::cci_originator(const char* originator_name)
  : m_originator_obj()
  , m_originator_str()
{
    if (current_originator_object()) {
      m_originator_obj = current_originator_object();
      if (originator_name && *originator_name) {
        CCI_REPORT_ERROR("cci_originator/name_inside_hierarchy","Originators must not be named inside the SystemC hierarchy");
      }
    } else {
      if (originator_name && *originator_name) {
        m_originator_str = new std::string(originator_name);
      } else {
        CCI_REPORT_ERROR("cci_originator/noname_outside_hierarchy","An originator name must be given outside the SystemC hierarchy");
      }
    }
    check_is_valid();
}

cci_originator::cci_originator(const cci_originator& originator)
  : m_originator_obj(originator.m_originator_obj)
  , m_originator_str()
{
    if (originator.m_originator_str)
        m_originator_str = new std::string(*originator.m_originator_str);
}

#ifdef CCI_HAS_CXX_RVALUE_REFS
cci_originator::cci_originator(cci_originator&& originator)
  : m_originator_obj(CCI_MOVE_(originator.m_originator_obj))
  , m_originator_str(CCI_MOVE_(originator.m_originator_str))
{
    // invalidate incoming originator
    originator.m_originator_obj = NULL;
    originator.m_originator_str = NULL;
}
#endif // CCI_HAS_CXX_RVALUE_REFS

const sc_core::sc_object *cci_originator::get_object() const
{
    return m_originator_obj;
}

const char* cci_originator::name() const
{
    static const char* default_name = CCI_UNKNOWN_ORIGINATOR_STRING_;
    if (m_originator_obj) {
        return m_originator_obj->name();
    } else if(m_originator_str) {
        return m_originator_str->c_str();
    }
    return default_name;
}

const char* cci_originator::string_name() const {
    if(m_originator_str)
        return m_originator_str->c_str();
    return NULL;
}

void cci_originator::swap(cci_originator& that) {
    std::swap(m_originator_obj, that.m_originator_obj);
    std::swap(m_originator_str, that.m_originator_str);
}

cci_originator &cci_originator::operator=(const cci_originator& originator) {
    cci_originator copy(originator);
    copy.swap(*this);
    return *this;
}

#ifdef CCI_HAS_CXX_RVALUE_REFS
cci_originator& cci_originator::operator=(cci_originator&& originator)
{
    // guard delete against self-move, invalidate incoming originator
    const sc_core::sc_object* orig_obj = CCI_MOVE_(originator.m_originator_obj);
    const std::string*        orig_str = CCI_MOVE_(originator.m_originator_str);
    originator.m_originator_obj = NULL;
    originator.m_originator_str = NULL;
    delete m_originator_str;
    m_originator_obj = CCI_MOVE_(orig_obj);
    m_originator_str = CCI_MOVE_(orig_str);
    return *this;
}
#endif // CCI_HAS_CXX_RVALUE_REFS

bool cci_originator::operator==( const cci_originator& originator ) const {
    if(this->get_object() || originator.get_object()) {
        return this->get_object() == originator.get_object();
    }
    return !std::strcmp(name(), originator.name());
}

bool cci_originator::operator<(const cci_originator& originator) const {
    return std::strcmp(name(), originator.name()) < 0;
}

sc_core::sc_object *cci_originator::current_originator_object() {
    return sc_core::sc_get_current_object();
}

void cci_originator::check_is_valid() const {
    if (is_unknown()) {
        CCI_REPORT_ERROR("cci_originator/check_is_valid",
                         "It is forbidden to build an originator without "
                         "information (no SystemC hierarchy or empty name)!");
        // may continue, if suppressed
    }
}

bool cci_originator::is_unknown() const
{
    return !m_originator_obj && !m_originator_str;
}

cci_originator::~cci_originator() {
    delete m_originator_str;
}

CCI_CLOSE_NAMESPACE_
