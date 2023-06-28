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

#ifndef CCI_CFG_CCI_BROKER_CALLBACKS_H_INCLUDED_
#define CCI_CFG_CCI_BROKER_CALLBACKS_H_INCLUDED_

#include "cci_core/cci_callback.h"
#include "cci_core/cci_value.h"

CCI_OPEN_NAMESPACE_

class cci_param_untyped_handle;
class cci_originator;

/// Parameter creation callback
typedef cci_callback<const cci_param_untyped_handle& >
  cci_param_create_callback;

/// Parameter creation callback handle
typedef cci_callback_typed_handle<const cci_param_untyped_handle& >
  cci_param_create_callback_handle;

/// Parameter destruction callback
typedef cci_callback<const cci_param_untyped_handle& >
  cci_param_destroy_callback;

/// Parameter destruction callback handle
typedef cci_callback_typed_handle<const cci_param_untyped_handle& >
  cci_param_destroy_callback_handle;

/// Parameter predicate
typedef cci_callback<const cci_param_untyped_handle&, bool >
        cci_param_predicate;

/// Parameter predicate handle
typedef cci_callback_typed_handle<const cci_param_untyped_handle&, bool >
        cci_param_predicate_handle;

/// Preset value predicate
typedef cci_callback<const std::pair<std::string, cci_value>&, bool >
        cci_preset_value_predicate;

/// Preset value predicate handle
typedef cci_callback_typed_handle<const std::pair<std::string, cci_value>&,
        bool > cci_preset_value_predicate_handle;

/* ------------------------------------------------------------------------ */

/// Callback API of CCI brokers
struct cci_broker_callback_if
{
  virtual cci_param_create_callback_handle
  register_create_callback( const cci_param_create_callback& cb
                          , const cci_originator& orig ) = 0;
  virtual bool
  unregister_create_callback( const cci_param_create_callback_handle& cb
                            , const cci_originator& orig ) = 0;

  virtual cci_param_destroy_callback_handle
  register_destroy_callback( const cci_param_destroy_callback& cb
                           , const cci_originator& orig ) = 0;
  virtual bool
  unregister_destroy_callback( const cci_param_destroy_callback_handle& cb
                             , const cci_originator& orig ) = 0;

  virtual bool unregister_all_callbacks(const cci_originator& orig) = 0;

  virtual bool has_callbacks() const = 0;
};

CCI_CLOSE_NAMESPACE_

#endif // CCI_CFG_CCI_BROKER_CALLBACKS_H_INCLUDED_
