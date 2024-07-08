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
 * @file   systemc.h
 * @brief  include SystemC from within the CCI library
 * @author Thomas Goodfellow, OFFIS
 * @author Philipp A. Hartmann, OFFIS/Intel
 *
 * This file provides a local indirection to include SystemC from within
 * the CCI library.  Prefer this file over direct inclusion of the SystemC
 * header inside the CCI implementation(s).
 *
 * Some features not yet present in some (supported) versions of SystemC
 * may require local workarounds, e.g. @ref sc_core::sc_get_current_object()
 * (added in SystemC 2.3.1).
 *
 */

#ifndef CCI_CORE_SYSTEMC_H_INCLUDED_
#define CCI_CORE_SYSTEMC_H_INCLUDED_

#if defined(_MSC_VER)
#pragma warning( push )
#pragma warning( disable: 4244 )
#pragma warning( disable: 4267 )
#endif

// Required by CCI callback mechanism using sc_unnamed bind argument
#ifndef SC_INCLUDE_DYNAMIC_PROCESSES
# define SC_INCLUDE_DYNAMIC_PROCESSES
#endif

#include <systemc>

#if defined(_MSC_VER)
#pragma warning( pop )
#endif

#ifdef SC_VERSION_HELPER_
# define CCI_VERSION_HELPER_ \
    SC_VERSION_HELPER_
# else
# define CCI_VERSION_HELPER_(Major,Minor,Patch) \
    (((Major)*100000) + ((Minor)*100) + (Patch))
#endif

#ifdef SC_VERSION_CODE
# define CCI_SYSTEMC_VERSION_CODE_ \
    SC_VERSION_CODE

#elif defined(IEEE_1666_SYSTEMC)
# define CCI_SYSTEMC_VERSION_CODE_ \
    CCI_VERSION_HELPER_( SC_VERSION_MAJOR \
                       , SC_VERSION_MINOR \
                       , SC_VERSION_PATCH )
#else // pre 1666-2011
// assume 2.2.0 for now, eventually guess from SYSTEMC_VERSION (date)
# define CCI_SYSTEMC_VERSION_CODE_ \
    CCI_VERSION_HELPER_(2,2,0)

#endif // CCI_SYSTEMC_VERSION_CODE_

// sc_core::sc_get_current_object()
#if CCI_SYSTEMC_VERSION_CODE_ < CCI_VERSION_HELPER_(2,3,1)
namespace sc_core {
inline sc_object* sc_get_current_object()
{
  struct dummy_object : sc_object {} dummy;
  return dummy.get_parent_object();
}
} // namespace sc_core
#endif // sc_core::sc_get_current_object

#endif // CCI_CORE_SYSTEMC_H_INCLUDED_
