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

#ifndef CCI_CFG_CCI_BROKER_TYPES_H_INCLUDED_
#define CCI_CFG_CCI_BROKER_TYPES_H_INCLUDED_

#include "cci/core/cci_value.h"
#include "cci/core/cci_filtered_range.h"
#include "cci/cfg/cci_broker_callbacks.h"
//
#include <string>
#include <utility> // std::pair

CCI_OPEN_NAMESPACE_

// forward declaration
class cci_param_untyped_handle;

/// CCI parameter filter iterator type
typedef cci_filtered_range<cci_param_untyped_handle, cci_param_predicate>
        cci_param_range;

/// CCI value with parameter name pair type
typedef std::pair<std::string, cci_value> cci_name_value_pair;

/// CCI preset value filter iterator type
typedef cci_filtered_range<cci_name_value_pair, cci_preset_value_predicate>
        cci_preset_value_range;

CCI_CLOSE_NAMESPACE_

#endif // CCI_CFG_CCI_BROKER_TYPES_H_INCLUDED_
