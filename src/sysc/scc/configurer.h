/*******************************************************************************
 * Copyright 2017-2022 MINRES Technologies GmbH
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
#include <cci_configuration>
#include <regex>

/** \ingroup scc-sysc
 *  @{
 */
/**@{*/
//! @brief SCC SystemC utilities
namespace scc {

bool init_cci(std::string name = "Global Broker");
/**
 * @class configurer
 * @brief design configuration reader
 *
 * A class to configure a design hierarchy using a JSON input file. It reads a file and
 * and stores its values into a CCI broker. It can apply the value also to sc_attribute
 * once the design is installed.
 */
class configurer : public sc_core::sc_module {
    struct ConfigHolder;

public:
    using base_type = sc_core::sc_module;
    using broker_t = cci::cci_broker_handle;
    using cci_param_cln = std::vector<std::pair<cci::cci_param_post_write_callback_untyped, std::unique_ptr<cci::cci_param_untyped>>>;
    enum { NEVER = 0, BEFORE_END_OF_ELABORATION = 1, END_OF_ELABORATION = 2, START_OF_SIMULATION = 4 };
    /**
     * create a configurer using an input file
     * @param filename the input file to read containing the values to apply
     * @param sc_attr_config_phases defines when to apply the values to sc_attribute instances
     */
    configurer(std::string const& filename, unsigned sc_attr_config_phases = BEFORE_END_OF_ELABORATION);

    configurer() = delete;

    configurer(const configurer&) = delete;

    configurer(configurer&&) = delete;

    ~configurer();

    configurer& operator=(const configurer&) = delete;

    configurer& operator=(configurer&&) = delete;

    void read_input_file(std::string const& filename);
    /**
     * configure the design hierarchy using the input file. Apply the values to
     * sc_core::sc_attribute in th edsign hierarchy
     */
    void configure();
    /**
     * Schedule the dump of the parameters of a design hierarchy to a file
     * during start_of_simulation()
     *
     * @param file_name the output stream, std::cout by default
     */
    void dump_configuration(std::string const& file_name, bool with_description = false, bool complete = true,
                            std::vector<std::string> stop_list = std::vector<std::string>{}) {
        dump_file_name = file_name;
        this->with_description = with_description;
        this->stop_list = stop_list;
        this->complete = complete;
    }
    /**
     * Immediately dumps the parameters of a design hierarchy to the given output stream
     * As this dumps the parameters immediately it should only be called during start_of_simulation since
     * many parameters are created during before_end_of_elaboration or end_of_elaboration.
     *
     * @param os the output stream, std::cout by default
     * @param obj if not null specifies the root object of the dump
     */
    void dump_configuration(std::ostream& os = std::cout, bool as_yaml = true, bool with_description = false, bool complete = true,
                            sc_core::sc_object* obj = nullptr);
    /**
     * set a value of some property (sc_attribute or cci_param) directly
     *
     * In case the configurer is being used without CCI the function can only be called after
     * the simulation objects are instantiated since the sc_attributes have to exist.
     *
     * @param hier_name the hierarchical name of the property
     * @param value the value to put
     */
    template <typename T> void set_value(std::string const& hier_name, T value) { set_value(hier_name, cci::cci_value(value)); }
    /**
     * set a value of some property (sc_attribute or cci_param) directly
     *
     * this version automatically converts the string into the needed target data type
     *
     * In case the configurer is being used without CCI the function can only be called after
     * the simulation objects are instantiated since the sc_attributes have to exist.
     *
     * @param hier_name the hierarchical name of the property
     * @param value the value to put
     */
    void set_value_from_str(const std::string& hier_name, const std::string& value);
    /**
     * set a value of an sc_attribute from given configuration. This is being used by the scc::ext_attribute
     * which allows to use config values during construction
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
    static configurer& get() {
        configurer* inst = dynamic_cast<configurer*>(sc_core::sc_find_object("$$$configurer$$$"));
        if(!inst)
            SCCFATAL() << "No configurer instantiated when using it";
        return *inst;
    }

protected:
    unsigned const config_phases;
    std::string dump_file_name{""};
    bool with_description{false};
    bool complete{true};
    std::vector<std::string> stop_list{};
    configurer(std::string const& filename, unsigned sc_attr_config_phases, sc_core::sc_module_name nm);
    void config_check();
    void before_end_of_elaboration() override {
        if(config_phases & BEFORE_END_OF_ELABORATION)
            configure();
    }
    void end_of_elaboration() override {
        if(config_phases & END_OF_ELABORATION)
            configure();
    }
    void start_of_simulation() override;
    broker_t cci_broker;
    void set_value(const std::string& hier_name, cci::cci_value value);
    cci_param_cln cci2sc_attr;
    cci::cci_originator cci_originator;
    std::unique_ptr<ConfigHolder> root;
};

} // namespace scc
/** @} */ // end of scc-sysc
#endif    /* _SYSC_CONFIGURER_H_ */
