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

#include "report.h"
#include "utilities.h"
#include <cci_configuration>
#include <json/json.h>

namespace scc {

class configurer : public sc_core::sc_object {
public:
    using base_type = sc_core::sc_object;

    configurer(const std::string &filename);

    configurer() = delete;

    configurer(const configurer&) = delete;

    configurer(configurer&&) = delete;

    configurer& operator=(const configurer&) = delete;

    configurer& operator=(configurer&&) = delete;

    void configure();

    void dump_hierarchy(std::ostream &os = std::cout, sc_core::sc_object *obj = nullptr);

    void dump_configuration(std::ostream &os = std::cout, sc_core::sc_object *obj = nullptr);

    template <typename T> void set_value(const std::string &hier_name, T value) {
  	    cci::cci_param_handle param_handle = cci_broker.get_param_handle(hier_name);
  	    if(param_handle.is_valid()) {
  	    	param_handle.set_cci_value(cci::cci_value(value));
  	    } else {
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
    }

    void set_configuration_value(sc_core::sc_attr_base *attr_base, sc_core::sc_object *owner);

    static configurer &instance() {
        configurer *inst = dynamic_cast<configurer *>(sc_core::sc_find_object("configurer"));
        sc_assert("No configurer instantiated when using it" && inst != nullptr);
        return *inst;
    }

protected:
    void dump_configuration(std::ostream& os, sc_core::sc_object* obj, Json::Value& node);

    void configure_sc_attribute_hierarchical(sc_core::sc_object *obj, Json::Value &hier_val);

    void set_value(sc_core::sc_attr_base *attr_base, Json::Value &hier_val);

    Json::Value &get_value_from_hierarchy(const std::string &hier_name);

    Json::Value &get_value_from_hierarchy(const std::string &hier_name, Json::Value &val);

    void configure_cci_hierarchical(const Json::Value &root, std::string prefix);

    Json::Value root;

    cci::cci_originator cci_originator;
    cci::cci_broker_handle cci_broker;
};
}

#endif /* _SYSC_CONFIGURER_H_ */
