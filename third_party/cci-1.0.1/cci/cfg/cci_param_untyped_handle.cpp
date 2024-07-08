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

#include "cci/cfg/cci_param_untyped_handle.h"
#include "cci/cfg/cci_broker_manager.h"
#include "cci/cfg/cci_report_handler.h"
#include "cci/core/cci_name_gen.h"
#include "cci/cfg/cci_param_if.h"

CCI_OPEN_NAMESPACE_

cci_param_untyped_handle::
  cci_param_untyped_handle( cci_param_if & param
                          , const cci_originator & originator )
  : m_param(&param)
  , m_originator(originator)
{
    m_param->add_param_handle(this);
}

cci_param_untyped_handle::
  cci_param_untyped_handle(const cci_originator & originator)
  : m_param(NULL)
  , m_originator(originator)
{}

cci_param_untyped_handle::~cci_param_untyped_handle()
{
    invalidate();
}

cci_param_untyped_handle::
  cci_param_untyped_handle(const cci_param_untyped_handle& param_handle)
    : m_param(param_handle.m_param)
    , m_originator(promote_originator(param_handle.m_originator))
{
    if(m_param) {
        m_param->add_param_handle(this);
    }
}

#ifdef CCI_HAS_CXX_RVALUE_REFS
cci_param_untyped_handle::
  cci_param_untyped_handle(cci_param_untyped_handle&& param_handle)
    : m_param(CCI_MOVE_(param_handle.m_param))
    , m_originator(promote_originator(param_handle.m_originator))
{
    if(m_param) {
        m_param->add_param_handle(this);
    }
}
#endif // CCI_HAS_CXX_RVALUE_REFS

cci_param_untyped_handle&
cci_param_untyped_handle::operator=(const cci_param_untyped_handle& param_handle)
{
    if (this == &param_handle)
        return *this;

    if (m_param) {
        m_param->remove_param_handle(this);
    }

    m_param      = param_handle.m_param;
    // The originator is explicitly not assigned here.
    
    if (m_param) {
        m_param->add_param_handle(this);
    }
    return *this;
}

#ifdef CCI_HAS_CXX_RVALUE_REFS
cci_param_untyped_handle&
cci_param_untyped_handle::operator=(cci_param_untyped_handle&& param_handle)
{
    if (this == &param_handle)
        return *this;

    if (m_param) {
        m_param->remove_param_handle(this);
    }

    m_param      = CCI_MOVE_(param_handle.m_param);
    // The originator is explicitly not assigned here.
    
    if (m_param) {
        m_param->add_param_handle(this);
    }
    return *this;
}
#endif // CCI_HAS_CXX_RVALUE_REFS

std::string cci_param_untyped_handle::get_description() const
{
    check_is_valid();
    return m_param->get_description();
}

cci_value_map cci_param_untyped_handle::get_metadata() const
{
    check_is_valid();
    return m_param->get_metadata();
}

void cci_param_untyped_handle::set_cci_value(const cci_value& val)
{
    check_is_valid();
    m_param->set_cci_value(val, NULL, m_originator);
}

void cci_param_untyped_handle::set_cci_value(const cci_value& val, const void *pwd)
{
    check_is_valid();
    m_param->set_cci_value(val, pwd, m_originator);
}

cci_value cci_param_untyped_handle::get_cci_value() const
{
    check_is_valid();
    return m_param->get_cci_value(m_originator);
}

cci_param_mutable_type cci_param_untyped_handle::get_mutable_type() const
{
    check_is_valid();
    return m_param->get_mutable_type();
}

cci_value cci_param_untyped_handle::get_default_cci_value() const
{
    check_is_valid();
    return m_param->get_default_cci_value();
}

bool cci_param_untyped_handle::is_default_value() const
{
    check_is_valid();
    return m_param->is_default_value();
}

bool cci_param_untyped_handle::is_preset_value() const
{
    check_is_valid();
    return m_param->is_preset_value();
}

cci_originator
cci_param_untyped_handle::get_value_origin() const
{
    check_is_valid();
    return m_param->get_value_origin();
}

#define CCI_PARAM_UNTYPED_HANDLE_CALLBACK_IMPL_(name)                          \
cci_callback_untyped_handle                                                    \
cci_param_untyped_handle::register_##name##_callback(                          \
        const cci_param_##name##_callback_untyped &cb,                         \
        cci_untyped_tag)                                                       \
{                                                                              \
    check_is_valid();                                                          \
    return m_param->register_##name##_callback(cb, m_originator);              \
}                                                                              \
                                                                               \
cci_callback_untyped_handle                                                    \
cci_param_untyped_handle::register_##name##_callback(                          \
        const cci_callback_untyped_handle& cb, cci_typed_tag<void>)            \
{                                                                              \
    check_is_valid();                                                          \
    return m_param->register_##name##_callback(cb, m_originator);              \
}                                                                              \
                                                                               \
bool cci_param_untyped_handle::unregister_##name##_callback(                   \
        const cci_callback_untyped_handle &cb)                                 \
{                                                                              \
    check_is_valid();                                                          \
    return m_param->unregister_##name##_callback(cb, m_originator);            \
}

// Pre write callback
CCI_PARAM_UNTYPED_HANDLE_CALLBACK_IMPL_(pre_write)

// Post write callback
CCI_PARAM_UNTYPED_HANDLE_CALLBACK_IMPL_(post_write)

// Pre read callback
CCI_PARAM_UNTYPED_HANDLE_CALLBACK_IMPL_(pre_read)

// Post read callback
CCI_PARAM_UNTYPED_HANDLE_CALLBACK_IMPL_(post_read)

bool cci_param_untyped_handle::unregister_all_callbacks()
{
    check_is_valid();
    return m_param->unregister_all_callbacks(m_originator);
}

bool cci_param_untyped_handle::has_callbacks() const
{
    check_is_valid();
    return m_param->has_callbacks();
}

bool cci_param_untyped_handle::lock(const void* pwd)
{
    check_is_valid();
    return m_param->lock(pwd);
}

bool cci_param_untyped_handle::unlock(const void* pwd)
{
    check_is_valid();
    return m_param->unlock(pwd);
}

bool cci_param_untyped_handle::is_locked() const
{
    check_is_valid();
    return m_param->is_locked();
}

cci_param_data_category cci_param_untyped_handle::get_data_category() const
{
    check_is_valid();
    return m_param->get_data_category();
}

const char* cci_param_untyped_handle::name() const
{
    check_is_valid();
    return m_param->name();
}

cci_originator cci_param_untyped_handle::get_originator() const
{
    return m_originator;
}

const void* cci_param_untyped_handle::get_raw_value() const
{
    check_is_valid();
    return m_param->get_raw_value(m_originator);
}

const void* cci_param_untyped_handle::get_raw_default_value() const
{
    check_is_valid();
    return m_param->get_raw_default_value();
}

void cci_param_untyped_handle::set_raw_value(const void* vp)
{
    set_raw_value(vp, NULL);
}

void cci_param_untyped_handle::set_raw_value(const void* vp, const void* pwd)
{
    check_is_valid();
    m_param->set_raw_value(vp, pwd, m_originator);
}

const std::type_info& cci_param_untyped_handle::get_type_info() const
{
    check_is_valid();
    return m_param->get_type_info();
}

bool cci_param_untyped_handle::is_valid() const
{
    return m_param != NULL;
}

void cci_param_untyped_handle::invalidate() {
    if(m_param) {
        m_param->remove_param_handle(this);
    }
    m_param = NULL;
}

void cci_param_untyped_handle::check_is_valid() const
{
    bool invalid_error = !is_valid();
    if(invalid_error) {
        CCI_REPORT_ERROR("cci_param_untyped_handle/check_is_valid",
                         "The handled parameter is not valid.");
        cci_abort(); // cannot recover from here
    }
}

#undef CCI_PARAM_UNTYPED_HANDLE_CALLBACK_IMPL_

CCI_CLOSE_NAMESPACE_
