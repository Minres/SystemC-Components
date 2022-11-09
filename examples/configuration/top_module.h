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
 *  @file     top_module.h
 *  @brief    Implementation the TOP_MODULE.
 *            This header contains code related to the top module which decides
 *            the model hierarchy for example#9.
 *  @author   P V S Phaneendra, CircuitSutra Technologies   <pvs@circuitsutra.com>
 *            Girish Verma, CircuitSutra Technologies      <girish@circuitsutra.com>
 *            Parvinder Pal Singh, CircuitSutra Technologies   <parvinder@circuitsutra.com>
 *  @date     29th April, 2011 (Friday)
 */

#ifndef EXAMPLES_EX09_HIERARCHICAL_OVERRIDE_OF_PARAMETER_VALUES_TOP_MODULE_H_
#define EXAMPLES_EX09_HIERARCHICAL_OVERRIDE_OF_PARAMETER_VALUES_TOP_MODULE_H_

#include <cci_configuration>
#include <tlm>
#include <vector>
#include <sstream>
#include <scc/report.h>

#include "router.h"
#include "initiator.h"
#include "target.h"

/**
 *  @class  top_module
 *  @brief  This module instantiated a initiator, target, and router and binds them correctly for communication.
 */
SC_MODULE(top_module) {
public:
    /**
     *  @fn     top_module
     *  @brief  The class constructor
     */
    SC_CTOR(top_module)
            :
            n_initiators("number_of_initiators", 0), n_targets("number_of_targets", 0), m_broker(cci::cci_get_broker()) {
        std::stringstream ss;

        SCCINFO(SCMOD) << "[TOP_MODULE C_TOR] -- [TOP MODULE CONSTRUCTOR BEGINS HERE]";

        SCCINFO(SCMOD) << "[TOP_MODULE C_TOR] :  Number of initiators : " << n_initiators.get_value();
        SCCINFO(SCMOD) << "[TOP_MODULE C_TOR] :  Number of targets : " << n_targets.get_value();

        // Set and lock the number of initiators in Router Table
        // to value passed from 'sc_main'
        m_broker.set_preset_cci_value("top_module_inst.RouterInstance.r_initiators", n_initiators.get_cci_value());
        m_broker.lock_preset_value("top_module_inst.RouterInstance.r_initiators");

        // Set and lock the number of targets in Router Table
        // to value passed from 'sc_main'
        m_broker.set_preset_cci_value("top_module_inst.RouterInstance.r_targets", n_targets.get_cci_value());
        m_broker.lock_preset_value("top_module_inst.RouterInstance.r_targets");

        // Declaring and defining router module
        char routerName[15] = "RouterInstance";
        SCCINFO(SCMOD) << "[TOP_MODULE C_TOR] : Creating Router : " << routerName;
        routerInstance = new router(routerName);

        // Top_Module begins construction of the model hierarchy from here
        // ----------------------------------------------------------------

        cci::cci_param_handle r_addr_limit_handle = m_broker.get_param_handle("top_module_inst.RouterInstance.addr_limit");
        if (r_addr_limit_handle.is_valid()) {
            r_addr_max = atoi((r_addr_limit_handle.get_cci_value().to_json()).c_str());

            SCCINFO(SCMOD) << "[TOP_MODULE C_TOR] : Router's maximum addressable limit : " << r_addr_max;
        }

        /// Creating instances of initiator(s)
        for (int i = 0; i < n_initiators; i++) {
            snprintf(initiatorName, sizeof(initiatorName), "initiator_%d", i);
            SCCINFO(SCMOD) << "[TOP_MODULE C_TOR] : Creating initiator : " << initiatorName;

            snprintf(stringMisc, sizeof(stringMisc), "%s.%s.initiator_ID", name(), initiatorName);

            snprintf(initiatorName, sizeof(initiatorName), "\"initiator_%d\"", i);
            m_broker.set_preset_cci_value(stringMisc, cci::cci_value::from_json(initiatorName));
            snprintf(initiatorName, sizeof(initiatorName), "initiator_%d", i);
            initiatorList.push_back(new initiator(initiatorName));

            //     Binding of initiator to Router
            SCCINFO(SCMOD) << "[TOP MODULE C_TOR] : Binding Router_Initiator to " << initiatorName;
            initiatorList[i]->initiator_socket.bind(routerInstance->Router_target);
        }

        // Defining target size
        targetSize = 128;

        // Creating instances of target(s)
        for (int i = 0; i < n_targets; i++) {
            snprintf(targetName, sizeof(targetName), "target_%d", i);
            SCCINFO(SCMOD) << "[TOP_MODULE C_TOR] : Creating target : " << targetName;

            snprintf(stringMisc, sizeof(stringMisc), "%s.%s.target_ID", name(), targetName);
            snprintf(targetName, sizeof(targetName), "\"target_%d\"", i);
            m_broker.set_preset_cci_value(stringMisc, cci::cci_value::from_json(targetName));
            snprintf(targetName, sizeof(targetName), "target_%d", i);

            // Set preset value for maximum target size(memory)
            snprintf(stringMisc, sizeof(stringMisc), "%s.%s.s_size", name(), targetName);
            ss.clear();
            ss.str("");
            ss << targetSize;

            m_broker.set_preset_cci_value(stringMisc, cci::cci_value::from_json(ss.str()));
            targetList.push_back(new target(targetName));

            // Binding Router to target
            SCCINFO(SCMOD) << "[TOP MODULE C_TOR] : Binding Router_Initiator to " << targetName;
            routerInstance->Router_initiator.bind(targetList[i]->target_socket);
        }

        // Try re-setting locked values for Router Table contents
        for (int i = 0; i < n_targets; i++) {
            snprintf(targetName, sizeof(targetName), "%s.RouterInstance.r_index_%d", name(), i);
            ss.clear();
            ss.str("");
            ss << i;

            try {
                SCCINFO(SCMOD) << "[TOP_MODULE C_TOR] : Re-setting fields of target_" << i;
                m_broker.set_preset_cci_value(targetName, cci::cci_value::from_json(ss.str()));
            } catch (sc_core::sc_report const &exception) {
                SCCINFO(SCMOD) << "[ROUTER : Caught] : " << exception.what();
            }

            snprintf(targetName, sizeof(targetName), "%s.RouterInstance.r_sa_%d", name(), i);
            ss.clear();
            ss.str("");
            ss << (i * targetSize);

            snprintf(targetBaseAddr, sizeof(targetBaseAddr), "%s.target_%d.s_base_addr", name(), i);

            cci::cci_param_untyped_handle h = m_broker.get_param_handle(targetBaseAddr);
            h.set_cci_value(cci::cci_value::from_json(ss.str()));

            try {
                SCCINFO(SCMOD) << "[TOP_MODULE C_TOR] : Re-setting start addr of target_" << i;
                m_broker.set_preset_cci_value(targetName, cci::cci_value::from_json(ss.str()));
            } catch (sc_core::sc_report const &exception) {
                SCCINFO(SCMOD) << "[ROUTER : Caught] : " << exception.what();
            }

            snprintf(targetName, sizeof(targetName), "%s.RouterInstance.r_ea_%d", name(), i);
            ss.clear();
            ss.str("");
            ss << ((i + 1) * targetSize - 1);

            try {
                SCCINFO(SCMOD) << "[TOP_MODULE C_TOR] : Re-setting end addr of target_" << i;
                m_broker.set_preset_cci_value(targetName, cci::cci_value::from_json(ss.str()));
            } catch (sc_core::sc_report const &exception) {
                SCCINFO(SCMOD) << "[ROUTER : Caught] : " << exception.what();
            }
        }
    }

    /**
     *  @fn     ~top_module()
     *  @brief  The class destructor
     *  @return void
     */
    ~top_module() {
        if (!initiatorList.empty()) {
            for (std::vector<initiator*>::iterator it = initiatorList.begin(); it != initiatorList.end(); ++it) {
                delete (*it);
            }
            initiatorList.clear();
        }

        if (!targetList.empty()) {
            for (std::vector<target*>::iterator it = targetList.begin(); it != targetList.end(); ++it) {
                delete (*it);
            }
            targetList.clear();
        }
    }

private:
    // Immutable type cci-parameters
    cci::cci_param<int, cci::CCI_IMMUTABLE_PARAM> n_initiators; ///< Number of initiators to be instantiated
    cci::cci_param<int, cci::CCI_IMMUTABLE_PARAM> n_targets;  ///< Number of targets to be instantiated

    cci::cci_broker_handle m_broker; ///< Configuration broker handle

    router *routerInstance;  ///< Declaration of a router pointer

    // STD::VECTORs for creating instances of initiator and target
    std::vector<initiator*> initiatorList;   ///< STD::VECTOR for initiators
    std::vector<target*> targetList; ///< STD::VECTOR for targets

    char initiatorName[50]; ///< initiator_ID
    char targetName[50];  ///< target_ID
    char stringMisc[50];  ///< String to be used for misc things
    char targetBaseAddr[50];  ///< The base address of the target

    int addrValue
    { 0 };  ///< Address Value
    int targetSize; ///< Maximum target Size (preset value)
    int r_addr_max; ///< Maximum Router Table's memory range
};
// top_module

#endif  // EXAMPLES_EX09_HIERARCHICAL_OVERRIDE_OF_PARAMETER_VALUES_TOP_MODULE_H_
