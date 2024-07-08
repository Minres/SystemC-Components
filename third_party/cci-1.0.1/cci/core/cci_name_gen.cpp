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

#include <sstream>
#include <cstring>
#include <map>

#include "cci/core/cci_name_gen.h"
#include "cci/cfg/cci_report_handler.h"

CCI_OPEN_NAMESPACE_

#if CCI_SYSTEMC_VERSION_CODE_ < CCI_VERSION_HELPER_(2,3,2)
enum cci_name_state {
    cci_name_free,
    cci_name_used
};

std::map<std::string, std::pair<int, cci_name_state> >&
cci_get_cci_unique_names()
{
    /// CCI unique names map used when SystemC < 2.3.2
    static std::map<std::string, std::pair<int, cci_name_state> >
            cci_unique_names;
    return cci_unique_names;
}
#endif

#define CCI_NAME_CONFLICT_WITH_SYSTEMC_WARNING_(old_name, new_name)            \
        std::stringstream msg;                                                 \
        msg << name << " is already used in the SystemC hierarchy, using "     \
            << new_name << " instead";                                         \
        CCI_REPORT_WARNING("cci_name_gen/gen_unique_name", msg.str().c_str())

const char* cci_gen_unique_name(const char *name)
{
    if(!name || !*name) {
        CCI_REPORT_ERROR("cci_name_gen/gen_unique_name",
                         "empty name is not allowed");
        cci_abort(); // cannot recover from here
    }
#if CCI_SYSTEMC_VERSION_CODE_ >= CCI_VERSION_HELPER_(2,3,2)
    if (!sc_core::sc_register_hierarchical_name(name)) {
        const char* new_name = sc_core::sc_gen_unique_name(name);
        sc_core::sc_register_hierarchical_name(new_name);
        CCI_NAME_CONFLICT_WITH_SYSTEMC_WARNING_(name, new_name);
        return sc_core::sc_get_hierarchical_name(new_name);
    }
    return sc_core::sc_get_hierarchical_name(name);
#else
    bool systemc_conflict = false;
    std::pair<std::map<std::string,
            std::pair<int, cci_name_state> >::iterator, bool> ret;
    ret = cci_get_cci_unique_names().insert(std::pair<std::string,
                                  std::pair<int, cci_name_state> >(name,
                    std::make_pair(0, cci_name_used)));
    if(sc_core::sc_find_object(name)) {
        systemc_conflict = true;
    }
    if (!ret.second || systemc_conflict) {
        std::stringstream new_name;
        new_name << name << "_" << ret.first->second.first;
        ret.first->second.first++;
        cci_gen_unique_name(new_name.str().c_str());
        if (systemc_conflict) {
            CCI_NAME_CONFLICT_WITH_SYSTEMC_WARNING_(name, new_name.str());
        }
        return cci_get_cci_unique_names().find(new_name.str())->first.c_str();
    }
    return cci_get_cci_unique_names().find(name)->first.c_str();
#endif
}

const char* cci_get_name(const char *name)
{
#if CCI_SYSTEMC_VERSION_CODE_ >= CCI_VERSION_HELPER_(2,3,2)
    return sc_core::sc_get_hierarchical_name(name);
#else
    std::map<std::string, std::pair<int, cci_name_state> >::iterator
            it = cci_get_cci_unique_names().find(name);
    if (it != cci_get_cci_unique_names().end()) {
        return it->first.c_str();
    } else {
        return NULL;
    }
#endif
}

bool cci_unregister_name(const char *name)
{
#if CCI_SYSTEMC_VERSION_CODE_ >= CCI_VERSION_HELPER_(2,3,2)
    return sc_core::sc_unregister_hierarchical_name(name);
#else
    std::map<std::string, std::pair<int, cci_name_state> >::iterator
            it = cci_get_cci_unique_names().find(name);
    if (it != cci_get_cci_unique_names().end()) {
        it->second.second = cci_name_free;
        return true;
    } else {
        return false;
    }
#endif
}

#undef CCI_NAME_CONFLICT_WITH_SYSTEMC_WARNING_

CCI_CLOSE_NAMESPACE_
