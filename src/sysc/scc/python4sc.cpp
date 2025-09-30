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
#include "python4sc.h"
#include "report.h"
#include "utilities.h"
#include <fstream>
#include <pybind11/embed.h> // Everything needed for embedding#include <sysc/kernel/sc_time.h>
#include <systemc>
#ifdef CWR_SYSTEMC
#include "scml_clock/scml_clock_if.h"
#endif

namespace scc {
namespace py = pybind11;
// C++ class to expose to Python
class Logger {
public:
    Logger(const std::string& name)
    : name(name) {}
    void log(std::string const& msg) const { SCCINFO(name) << msg; }
    std::string get_name() const { return name; }

private:
    std::string name;
};

void wait(int value, std::string const& unit) {
    if(icompare(unit, "fs"))
        sc_core::wait(value * 1_fs);
    else if(icompare(unit, "ps"))
        sc_core::wait(value * 1_ps);
    else if(icompare(unit, "ns"))
        sc_core::wait(value * 1_ns);
    else if(icompare(unit, "us"))
        sc_core::wait(value * 1_us);
    else if(icompare(unit, "ms"))
        sc_core::wait(value * 1_ms);
    else if(icompare(unit, "s"))
        sc_core::wait(value * 1_sec);
    else
        SCCERR("Python::wait") << "Illegal time unit specification: " << unit;
}

/* example python script:
from sysc import Greeter
import sysc

g = Greeter("Alice")
g.greet()
print("Name from C++ class:", g.get_name())
sysc.wait(10, "ns")
*/
void python4sc::run() {
    wait(sc_core::SC_ZERO_TIME);
    if(input_file_name.get_value().empty())
        return;
    {
        std::ifstream ifs(input_file_name.get_value());
        if(!ifs)
            SCCERR(SCMOD) << "Cannot read " << input_file_name.get_value() << "!";
    }
    py::scoped_interpreter guard{};
    // Create a custom module to expose C++ class
    py::module_ mod = py::module_::create_extension_module("sysc", nullptr, new py::module_::module_def);
    // clang-format off
    // Bind class into the module
    py::class_<Logger>(mod, "Logger")
        .def(py::init<std::string>())
        .def("log", &Logger::log)
        .def("get_name", &Logger::get_name);
    // Expose free wait functions
    mod.def("wait", &::scc::wait);
    // clang-format on
    // Register the module so Python can import it
    py::module_::import("sys").attr("modules")["sysc"] = mod;
    for(auto& e : mods) {
        py::module_::import("sys").attr("modules")[e.first.c_str()] = e.second;
    }
    try {
        py::eval_file(input_file_name.get_value());
    } catch(py::error_already_set& e) {
        SCCERR(SCMOD) << "Python error: " << e.what();
    }
    return;
}
} // namespace scc