/*******************************************************************************
 * Copyright 2017, 2018 MINRES Technologies GmbH
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

#ifndef _SYSC_CONFIGURER_H_
#define _SYSC_CONFIGURER_H_

#include "report.h"
#include "utilities.h"
#ifdef WITH_CCI
#include <cci_configuration>
#endif
#include <json/json.h>

namespace scc {

void init_cci(std::string name = "Global Broker");
/**
 * @class configurer
 * @brief design configuration reader
 *
 * A class to configure a design hierarchy using a JSON input file. It reads a file and
 * and stores its values into a CCI broker. It can apply the value also to sc_attribute
 * once the design is installed.
 */
class configurer : public sc_core::sc_object {
public:
    using base_type = sc_core::sc_object;
    /**
     * create a configurer using a JSON input file
     * @param filename
     */
    configurer(const std::string& filename);

    configurer() = delete;

    configurer(const configurer&) = delete;

    configurer(configurer&&) = delete;

    configurer& operator=(const configurer&) = delete;

    configurer& operator=(configurer&&) = delete;
    /**
     * configure the design hierarchy using the input file. Apply the values to
     * sc_core::sc_attribute in th edsign hierarchy
     */
    void configure();
    /**
     * dump the design hierarchy as text
     *
     * @param os the output stream, std::cout by default
     * @param obj if not null specifies the root object of the dump
     */
    void dump_hierarchy(std::ostream& os = std::cout, sc_core::sc_object* obj = nullptr);
    /**
     * dump the parameters of a design hierarchy to output stream
     *
     * @param os the output stream, std::cout by default
     * @param obj if not null specifies the root object of the dump
     */
    void dump_configuration(std::ostream& os = std::cout, sc_core::sc_object* obj = nullptr);
    /**
     * set a value a some attribute (sc_attribute or cci_param)
     *
     * @param hier_name the hierarchical name of the attribute
     * @param value the value to put
     */
    template <typename T> void set_value(const std::string& hier_name, T value) {
#ifdef WITH_CCI
        cci::cci_param_handle param_handle = cci_broker.get_param_handle(hier_name);
        if(param_handle.is_valid()) {
            param_handle.set_cci_value(cci::cci_value(value));
        } else {
#endif
            size_t pos = hier_name.find_last_of('.');
            sc_core::sc_module* mod =
                dynamic_cast<sc_core::sc_module*>(sc_core::sc_find_object(hier_name.substr(0, pos).c_str()));
            if(mod != nullptr) {
                sc_core::sc_attribute<T>* attr =
                    dynamic_cast<sc_core::sc_attribute<T>*>(mod->get_attribute(hier_name.substr(pos + 1)));
                if(attr != nullptr)
                    attr->value = value;
                else
                    SCCERR() << "Could not set attribute value " << hier_name;
            }
#ifdef WITH_CCI
        }
#endif
    }
    /**
     * set a value of an sc_attribute from given configuration
     *
     * @param attr_base
     * @param owner
     */
    void set_configuration_value(sc_core::sc_attr_base* attr_base, sc_core::sc_object* owner);
    /**
     * find the configurer in the design hierarchy
     *
     * @return reference to the singleton
     */
    static configurer& instance() {
        configurer* inst = dynamic_cast<configurer*>(sc_core::sc_find_object("configurer"));
        sc_assert("No configurer instantiated when using it" && inst != nullptr);
        return *inst;
    }

protected:
    void dump_configuration(sc_core::sc_object* obj, Json::Value& node);

    void configure_sc_attribute_hierarchical(sc_core::sc_object* obj, Json::Value& hier_val);

    void set_value(sc_core::sc_attr_base* attr_base, Json::Value& hier_val);

    Json::Value& get_value_from_hierarchy(const std::string& hier_name);

    Json::Value& get_value_from_hierarchy(const std::string& hier_name, Json::Value& val);

    void configure_cci_hierarchical(const Json::Value& root, std::string prefix);

    bool config_valid{false};

    Json::Value root;

#ifdef WITH_CCI
    cci::cci_originator cci_originator;
    cci::cci_broker_handle cci_broker;
#endif
};
} // namespace scc

#endif /* _SYSC_CONFIGURER_H_ */
