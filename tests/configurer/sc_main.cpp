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

#include <scc/cci_util.h>
#include <scc/configurer.h>
#include <string>

DEFINE_ENUM4CCI(trace_lvl, (NONE)(LOW)(MEDIUM)(HIGH)(FULL))
DEFINE_NS_ENUM4CCI(test, log_lvl, (NONE)(LOW)(MEDIUM)(HIGH)(FULL))

/**
 *  @fn     int sc_main(int argc, char* argv[])
 *  @brief  The testbench for the hierarchical override of parameter values example
 *  @param  argc  The number of input arguments
 *  @param  argv  The list of input arguments
 *  @return An integer for the execution status
 */
int sc_main(int sc_argc, char* sc_argv[]) {
    scc::init_logging(scc::log::INFO);
    scc::configurer cfg(sc_argc == 2 ? sc_argv[1] : "test.yaml");
    cfg.dump_configuration("dump.yaml", true);
    cci::cci_originator sc_main_orig("SC_MAIN");
    cci::cci_param<int> int_param0{"int_param0", 0, "This is parameter 1", cci::CCI_ABSOLUTE_NAME, sc_main_orig};
    cci::cci_param<int> int_param1{"int_param1", 1, "This is parameter 3", cci::CCI_ABSOLUTE_NAME, sc_main_orig};
    cci::cci_param<int64_t> int64_param0{"int64_param0", 10, "This is parameter 2", cci::CCI_ABSOLUTE_NAME, sc_main_orig};
    cci::cci_param<int64_t> int64_param1{"int64_param1", 10, "This is parameter 4", cci::CCI_ABSOLUTE_NAME, sc_main_orig};
    cci::cci_param<unsigned> unsigned_param{"unsigned_param", 1, "This is parameter 5", cci::CCI_ABSOLUTE_NAME, sc_main_orig};
    cci::cci_param<uint64_t> uint64_param{"uint64_param", 1, "This is parameter 6", cci::CCI_ABSOLUTE_NAME, sc_main_orig};
    cci::cci_param<float> float_param{"float_param", 4, "This is parameter 7", cci::CCI_ABSOLUTE_NAME, sc_main_orig};
    cci::cci_param<double> double_param{"double_param", 4, "This is parameter 7", cci::CCI_ABSOLUTE_NAME, sc_main_orig};
    cci::cci_param<std::string> string_param{"string_param", "", "This is parameter 7", cci::CCI_ABSOLUTE_NAME, sc_main_orig};
    cci::cci_param<sc_core::sc_time> sc_time_param{"sc_time_param", sc_core::SC_ZERO_TIME, "This is parameter 7", cci::CCI_ABSOLUTE_NAME,
                                                   sc_main_orig};
    cci::cci_param<trace_lvl> trace_lvl_param{"trace_lvl_param", trace_lvl::NONE, "This is parameter 8", cci::CCI_ABSOLUTE_NAME,
                                              sc_main_orig};
    cci::cci_param<test::log_lvl> test_log_lvl_param{"test_log_lvl_param", test::log_lvl::NONE, "This is parameter 9",
                                                     cci::CCI_ABSOLUTE_NAME, sc_main_orig};
    // Start the simulation
    SCCINFO("sc_main") << "Begin Simulation.";
    sc_core::sc_start(sc_core::SC_ZERO_TIME);
    SCCINFO("sc_main") << "End Simulation.";

    return sc_core::sc_report_handler::get_count(sc_core::SC_ERROR) + sc_core::sc_report_handler::get_count(sc_core::SC_WARNING);
} // End of 'sc_main'
