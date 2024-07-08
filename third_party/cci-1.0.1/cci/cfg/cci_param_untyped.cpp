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
 * @author Guillaume Delbergue, GreenSocs / Ericsson
 */

#include "cci/cfg/cci_param_untyped.h"
#include "cci/cfg/cci_param_if.h"
#include "cci/cfg/cci_broker_handle.h"
#include "cci/cfg/cci_param_untyped_handle.h"
#include "cci/cfg/cci_report_handler.h"
#include "cci/core/cci_name_gen.h"

#include <sstream>

CCI_OPEN_NAMESPACE_

cci_param_untyped::cci_param_untyped(const std::string& name,
                                     cci_name_type name_type,
                                     cci_broker_handle broker_handle,
                                     const std::string& desc,
                                     const cci_originator& originator)
    : m_description(desc), m_lock_pwd(NULL),
      m_broker_handle(broker_handle), m_value_origin(originator),
      m_originator(originator), fast_read(false),fast_write(false)
{
    if(name_type == CCI_ABSOLUTE_NAME) {
        m_name = name;
    } else {
        sc_core::sc_object* current_obj = sc_core::sc_get_current_object();
        for (sc_core::sc_process_handle current_proc(current_obj);
             current_proc.valid();
             current_proc = sc_core::sc_process_handle(current_obj)) { 
            current_obj = current_proc.get_parent_object(); 
        }
        if(current_obj) {
            m_name = std::string(current_obj->name()) +
                sc_core::SC_HIERARCHY_CHAR + name;
        } else {
            m_name = name;
        }
    }

    // Handle name collision and destruction / resurrection
    std::string unique_name = std::string(cci_gen_unique_name(m_name.c_str()));
    if (unique_name != m_name
        && (sc_core::sc_hierarchical_name_exists(m_name.c_str())
            || broker_handle.has_preset_value(m_name))) {
        m_name = unique_name;
    }
}

cci_param_untyped::~cci_param_untyped()
{
    // should have been invalidated from within typed implementation
    // (through call to cci_param_if::destroy)
    sc_assert( m_param_handles.empty() );

    if(!m_name.empty()) {
        cci_unregister_name(name());
    }
}

void cci_param_untyped::set_description(const std::string& desc)
{
    m_description = desc;
}

std::string cci_param_untyped::get_description() const
{
    return m_description;
}

void cci_param_untyped::add_metadata(const std::string &name,
                                     const cci_value &value,
                                     const std::string &desc)
{
    metadata.push_entry(name, cci_value_list().push_back(value)
            .push_back(desc));
}

cci_value_map cci_param_untyped::get_metadata() const
{
    return metadata;
}

bool cci_param_untyped::is_preset_value() const
{
  if (!m_broker_handle.has_preset_value(name())) return false;
  cci_value init_value = m_broker_handle.get_preset_cci_value(name());
  return init_value == get_cci_value(m_originator);
}

cci_originator cci_param_untyped::get_value_origin() const
{
    return m_value_origin;
}

bool
cci_param_untyped::set_cci_value_allowed(cci_param_mutable_type mutability)
{
  if (mutability==CCI_IMMUTABLE_PARAM) {
    std::stringstream ss;
    ss << "Parameter (" << name() << ") is immutable.";
    cci_report_handler::set_param_failed(ss.str().c_str(), __FILE__, __LINE__);
    return false;
  }
  return true;
}

#define CCI_PARAM_UNTYPED_CALLBACK_IMPL_(name)                                 \
cci_callback_untyped_handle                                                    \
cci_param_untyped::register_##name##_callback(                                 \
        const cci_callback_untyped_handle &cb,                                 \
        const cci_originator &orig)                                            \
{                                                                              \
    fast_read=false;                                                           \
    fast_write=false;                                                          \
    m_##name##_callbacks.vec.push_back(name##_callback_obj_t(cb, orig));       \
    return cb;                                                                 \
}                                                                              \
                                                                               \
cci_callback_untyped_handle                                                    \
cci_param_untyped::register_##name##_callback(                                 \
        const cci_param_##name##_callback_untyped& cb,                         \
        cci_untyped_tag)                                                       \
{                                                                              \
    return cci_param_untyped::register_##name##_callback(cb, m_originator);    \
}                                                                              \
                                                                               \
bool                                                                           \
cci_param_untyped::unregister_##name##_callback(                               \
        const cci_callback_untyped_handle &cb,                                 \
        const cci_originator &orig)                                            \
{                                                                              \
    std::vector<name##_callback_obj_t>::iterator it;                           \
    for(it=m_##name##_callbacks.vec.begin() ;                                  \
        it < m_##name##_callbacks.vec.end();                                   \
        it++)                                                                  \
    {                                                                          \
        if(it->callback == cb && it->originator == orig) {                     \
            m_##name##_callbacks.vec.erase(it);                                \
            return true;                                                       \
        }                                                                      \
    }                                                                          \
    return false;                                                              \
}                                                                              \
                                                                               \
bool                                                                           \
cci_param_untyped::unregister_##name##_callback(                               \
        const cci_callback_untyped_handle &cb)                                 \
{                                                                              \
    return unregister_##name##_callback(cb, m_originator);                     \
}

CCI_PARAM_UNTYPED_CALLBACK_IMPL_(pre_write)

CCI_PARAM_UNTYPED_CALLBACK_IMPL_(post_write)

CCI_PARAM_UNTYPED_CALLBACK_IMPL_(pre_read)

CCI_PARAM_UNTYPED_CALLBACK_IMPL_(post_read)

bool cci_param_untyped::unregister_all_callbacks(const cci_originator &orig)
{
    bool result = false;
    for (std::vector<pre_write_callback_obj_t>::iterator it =
            m_pre_write_callbacks.vec.begin();
         it < m_pre_write_callbacks.vec.end(); it++)
    {
        if(it->originator == orig) {
            m_pre_write_callbacks.vec.erase(it);
            result = true;
        }
    }
    for(std::vector<post_write_callback_obj_t>::iterator it =
            m_post_write_callbacks.vec.begin();
        it < m_post_write_callbacks.vec.end(); it++)
    {
        if(it->originator == orig) {
            m_post_write_callbacks.vec.erase(it);
            result = true;
        }
    }
    for(std::vector<pre_read_callback_obj_t>::iterator it =
            m_pre_read_callbacks.vec.begin();
        it < m_pre_read_callbacks.vec.end(); it++)
    {
        if(it->originator == orig) {
            m_pre_read_callbacks.vec.erase(it);
            result = true;
        }
    }
    for(std::vector<post_read_callback_obj_t>::iterator it =
            m_post_read_callbacks.vec.begin();
        it < m_post_read_callbacks.vec.end(); it++)
    {
        if(it->originator == orig) {
            m_post_read_callbacks.vec.erase(it);
            result = true;
        }
    }
    return result;
}

bool cci_param_untyped::unregister_all_callbacks()
{
    return unregister_all_callbacks(m_originator);
}

bool cci_param_untyped::has_callbacks() const
{
    return (!m_post_write_callbacks.vec.empty() ||
            !m_pre_write_callbacks.vec.empty());
}

bool cci_param_untyped::lock(const void* pwd)
{
    if (!pwd) pwd=this;
    if(pwd != m_lock_pwd && m_lock_pwd != NULL) {
        return false;
    } else {
        m_lock_pwd = pwd;
        return true;
    }
}

bool cci_param_untyped::unlock(const void* pwd)
{
    if (!pwd) pwd=this;
    if(pwd == m_lock_pwd) {
        m_lock_pwd = NULL;
        return true;
    }
    return false;
}

bool cci_param_untyped::is_locked() const
{
    return m_lock_pwd != NULL;
}

const char* cci_param_untyped::name() const
{
    return m_name.c_str();
}

cci_originator cci_param_untyped::get_originator() const
{
    return m_originator;
}

void cci_param_untyped::add_param_handle(cci_param_untyped_handle* param_handle)
{
    m_param_handles.push_back(param_handle);
}

void cci_param_untyped::remove_param_handle(
        cci_param_untyped_handle* param_handle)
{
    m_param_handles.erase(std::remove(m_param_handles.begin(),
                                      m_param_handles.end(),
                                      param_handle),
                          m_param_handles.end());
}

void
cci_param_untyped::invalidate_all_param_handles()
{
    while( !m_param_handles.empty() )
        m_param_handles.front()->invalidate(); // removes itself from the list
}

CCI_CLOSE_NAMESPACE_
