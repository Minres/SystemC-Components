////////////////////////////////////////////////////////////////////////////////
// Copyright 2017 eyck@minres.com
//
// Licensed under the Apache License, Version 2.0 (the "License"); you may not
// use this file except in compliance with the License.  You may obtain a copy
// of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
// WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
// License for the specific language governing permissions and limitations under
// the License.
////////////////////////////////////////////////////////////////////////////////
/*
 * sc_main.cpp
 *
 *  Created on: 17.09.2017
 *      Author: eyck@minres.com
 */

#include "simple_system.h"
#include <scc/report.h>
#include <scc/tracer.h>
#include <boost/program_options.hpp>

using namespace sysc;
using namespace scc;
namespace po = boost::program_options;

namespace {
const size_t ERROR_IN_COMMAND_LINE = 1;
const size_t SUCCESS = 0;
const size_t ERROR_UNHANDLED_EXCEPTION = 2;
} // namespace

int sc_main(int argc, char *argv[]) {
    sc_core::sc_report_handler::set_actions( "/IEEE_Std_1666/deprecated", sc_core::SC_DO_NOTHING );
    sc_core::sc_report_handler::set_actions(sc_core::SC_ID_MORE_THAN_ONE_SIGNAL_DRIVER_, sc_core::SC_DO_NOTHING);
    ///////////////////////////////////////////////////////////////////////////
    // CLI argument parsing
    ///////////////////////////////////////////////////////////////////////////
    po::options_description desc("Options");
    // clang-format off
    desc.add_options()
    		("help,h",  "Print help message")
			("debug,d", "set debug level")
			("trace,t", "trace SystemC signals");
    // clang-format on
    po::variables_map vm;
    try {
        po::store(po::parse_command_line(argc, argv, desc), vm); // can throw
        // --help option
        if (vm.count("help")) {
            std::cout << "JIT-ISS simulator for AVR" << std::endl << desc << std::endl;
            return SUCCESS;
        }
        po::notify(vm); // throws on error, so do after help in case
        // there are any problems
    } catch (po::error &e) {
        std::cerr << "ERROR: " << e.what() << std::endl << std::endl;
        std::cerr << desc << std::endl;
        return ERROR_IN_COMMAND_LINE;
    }
    ///////////////////////////////////////////////////////////////////////////
    // configure logging
    ///////////////////////////////////////////////////////////////////////////
    scc::init_logging(vm.count("debug")?scc::log::DEBUG:scc::log::INFO);
    ///////////////////////////////////////////////////////////////////////////
    // set up tracing & transaction recording
    ///////////////////////////////////////////////////////////////////////////
    tracer trace("simple_system", tracer::COMPRESSED, vm.count("trace"));
    // todo: fix displayed clock period in VCD

    ///////////////////////////////////////////////////////////////////////////
    // instantiate top level
    ///////////////////////////////////////////////////////////////////////////
    simple_system i_simple_system("i_simple_system");

    ///////////////////////////////////////////////////////////////////////////
    // run simulation
    ///////////////////////////////////////////////////////////////////////////
    sc_start(sc_core::sc_time(1, sc_core::SC_MS));
    // todo: provide end-of-simulation macros

    if (!sc_core::sc_end_of_simulation_invoked()) {
        SCCERR() << "simulation timed out";
        sc_core::sc_stop();
    }
    return SUCCESS;
}
