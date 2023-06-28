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

#ifndef CCI_CFG_CCI_MUTABLE_TYPES_H_INCLUDED_
#define CCI_CFG_CCI_MUTABLE_TYPES_H_INCLUDED_

#include "cci_core/cci_cmnhdr.h"

CCI_OPEN_NAMESPACE_

/**
 * Enumeration for cci_param_typed template specifying the parameter type
 * according the lock behavior
 */
enum cci_param_mutable_type {
    /// Mutable Parameter
    CCI_MUTABLE_PARAM = 0,
    /// Immutable Parameter
    CCI_IMMUTABLE_PARAM,
    /// Vendor specific/other Parameter type
    CCI_OTHER_MUTABILITY
};

CCI_CLOSE_NAMESPACE_

#endif // CCI_CFG_CCI_MUTABLE_TYPES_H_INCLUDED_
