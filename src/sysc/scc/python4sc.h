/*******************************************************************************
 * Copyright 2025 MINRES Technologies GmbH
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

#ifndef _SCC_PYTHON4SC_H_
#define _SCC_PYTHON4SC_H_

#include "cci/cfg/cci_param_typed.h"
#include <cci_configuration>
#include <pybind11/pybind11.h>
#include <unordered_map>

/** \ingroup scc-sysc
 *  @{
 */
/**@{*/
//! @brief SCC SystemC utilities
namespace scc {
/**
 * @struct time2tick
 * @brief translate a tick-less clock (sc_time based) to boolean clock
 *
 */
struct python4sc : public sc_core::sc_module {
    //! yes, we have processes
#if SYSTEMC_VERSION < 20250221
    SC_HAS_PROCESS(python4sc); // NOLINT
#endif
    cci::cci_param<std::string> input_file_name{"input_file_name", "", "the name of the python file to be executed by the interpreter"};
    /**
     * the constructor
     *
     * @param nm the name
     */
    explicit python4sc(sc_core::sc_module_name nm)
    : sc_core::sc_module(nm) {
        SC_THREAD(run);
    }

    void register_module(std::string const& name, pybind11::module_&& mod) { mods.insert({name, mod}); }

private:
    void run();
    std::unordered_map<std::string, pybind11::module_> mods;
};
} // namespace scc
/** @} */ // end of scc-sysc
#endif    /* _SCC_PYTHON4SC_H_ */
