/*******************************************************************************
 * Copyright 2017 MINRES Technologies GmbH
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *******************************************************************************/
/*
 * configurer.h
 *
 *  Created on: 27.09.2017
 *      Author: eyck
 */

#ifndef _SYSC_CONFIGURER_H_
#define _SYSC_CONFIGURER_H_

#include <json/json.h>
#include "scc/report.h"
#include "scc/utilities.h"

namespace scc {

class configurer : public sc_core::sc_module {
public:
    using base_type = sc_core::sc_module;
    configurer(const std::string &filename);

    void configure() {
        for (auto *o : sc_core::sc_get_top_level_objects(sc_core::sc_curr_simcontext)) {
            Json::Value &val = root[o->name()];
            if (!val.isNull()) configure_sc_object(o, val);
        }
    }
    void dump_hierarchy(sc_core::sc_object *obj = nullptr, std::ostream &os = std::cout) {
        if (obj) {
            os << obj->name() << " of type " << typeid(*obj).name() << "\n";
            for (auto *o : obj->get_child_objects()) dump_hierarchy(o, os);
        } else {
            for (auto *o : sc_core::sc_get_top_level_objects(sc_core::sc_curr_simcontext)) dump_hierarchy(o, os);
        }
    }
    template <typename T> void set_value(const std::string &hier_name, T value) {
        size_t pos = hier_name.find_last_of('.');
        sc_core::sc_module *mod =
            dynamic_cast<sc_core::sc_module *>(sc_core::sc_find_object(hier_name.substr(0, pos).c_str()));
        if (mod != nullptr) {
            sc_core::sc_attribute<T> *attr =
                dynamic_cast<sc_core::sc_attribute<T> *>(mod->get_attribute(hier_name.substr(pos + 1)));
            if (attr != nullptr)
                attr->value = value;
            else
                LOG(ERROR) << "Could not set attribute value " << hier_name;
        }
    }

    void set_configuration_value(sc_core::sc_attr_base *attr_base, sc_core::sc_module *owner) {
        std::string name(owner->name());
        name += ".";
        name += attr_base->name();
        Json::Value &val = get_value_from_hierarchy(name);
        if (!val.isNull()) set_value(attr_base, val);
    }

    static configurer &instance() {
        configurer *inst = dynamic_cast<configurer *>(sc_core::sc_find_object("configurer"));
        assert("No configurer instantiated when using it" && inst != nullptr);
        return *inst;
    }

protected:
    void configure_sc_object(sc_core::sc_object *obj, Json::Value &hier_val);

    void set_value(sc_core::sc_attr_base *attr_base, Json::Value &hier_val);

    Json::Value &get_value_from_hierarchy(const std::string &hier_name);

    Json::Value &get_value_from_hierarchy(const std::string &hier_name, Json::Value &val);

    Json::Value root;
};
}

#endif /* _SYSC_CONFIGURER_H_ */
