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

#ifndef SC_INCLUDE_DYNAMIC_PROCESSES
#define SC_INCLUDE_DYNAMIC_PROCESSES
#endif

/**
 *  @file     main.cpp
 *  @brief    Testbench file
 *            This file declares and implements the functionality of the target.
 *            Few of the parameters of the target sc_module are configured by the
 *            router sc_module.
 *  @author   P V S Phaneendra, CircuitSutra Technologies   <pvs@circuitsutra.com>
 *  @date     29th April, 2011 (Friday)
 */

#include <cci_configuration>
#include <cci_utils/broker.h>
#include <string>
#include "top_module.h"

/**
 *  @fn     int sc_main(int argc, char* argv[])
 *  @brief  The testbench for the hierarchical override of parameter values example
 *  @param  argc  The number of input arguments
 *  @param  argv  The list of input arguments
 *  @return An integer for the execution status
 */
int sc_main(int sc_argc, char *sc_argv[]) {
    scc::init_logging(scc::log::INFO);
    cci::cci_originator me = cci::cci_originator("sc_main");
    // Get handle to the default broker
    cci::cci_broker_handle myGlobalBroker = cci::cci_get_global_broker(me);
    myGlobalBroker.set_preset_cci_value("top_module_inst.**.log_level", cci::cci_value(3));
    SCCINFO("sc_main") << "[MAIN] : Setting preset value of the number of initiators to 2";
    // Set preset value to the number of initiator(s) (within top_module)
    std::string initiatorHierarchicalName = "top_module_inst.number_of_initiators";
    myGlobalBroker.set_preset_cci_value(initiatorHierarchicalName, cci::cci_value(2));
    SCCINFO("sc_main") << "[MAIN] : Setting preset value of the number of initiators to 1";
    // The program considers only the last set preset value
    myGlobalBroker.set_preset_cci_value(initiatorHierarchicalName, cci::cci_value(1));
    SCCINFO("sc_main") << "[MAIN] : Setting preset value of the number of targets to 4";
    // Set preset value to the number of target(s) (within top_module)
    std::string targetHierarchicalName = "top_module_inst.number_of_targets";
    myGlobalBroker.set_preset_cci_value(targetHierarchicalName, cci::cci_value(4));
    // Set the maximum addressing limit for the router
    myGlobalBroker.set_preset_cci_value("top_module_inst.RouterInstance.addr_max", cci::cci_value(1024));
    // Set and lock the Router Table presets values for target_1
    //  These values have again been tried to set within the Top_MODULE
    //  @see top_module.h
    SCCINFO("sc_main") << "[MAIN] : Set and lock Router Table target_1 contents";
    myGlobalBroker.set_preset_cci_value("top_module_inst.RouterInstance.r_index_1", cci::cci_value(1));
    myGlobalBroker.lock_preset_value("top_module_inst.RouterInstance.r_index_1");
    SCCINFO("sc_main") << "[MAIN] : Set and lock Router Table Start Address for target_1 to 128";
    myGlobalBroker.set_preset_cci_value("top_module_inst.RouterInstance.r_sa_1", cci::cci_value(128));
    myGlobalBroker.lock_preset_value("top_module_inst.RouterInstance.r_sa_1");
    SCCINFO("sc_main") << "[MAIN] : Set and lock Router Table End Address for target_1 to 255";
    myGlobalBroker.set_preset_cci_value("top_module_inst.RouterInstance.r_ea_1", cci::cci_value(255));
    myGlobalBroker.lock_preset_value("top_module_inst.RouterInstance.r_ea_1");
    SCCINFO("sc_main") << "[MAIN] : Instantiate top module after setting preset values to top_module, router and target parameters";
    // Instantiate TOP_MODULE responsible for creating the model hierarchy
    top_module top_mod("top_module_inst");
    // Start the simulation
    SCCINFO("sc_main") << "Begin Simulation.";
    sc_core::sc_start(1140, sc_core::SC_NS);
    SCCINFO("sc_main") << "End Simulation.";

    return EXIT_SUCCESS;
}  // End of 'sc_main'
