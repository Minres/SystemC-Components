/*******************************************************************************
 * Copyright 2016 MINRES Technologies GmbH
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
 * logging.h
 *
 *  Created on: Nov 24, 2016
 *      Author: developer
 */

#ifndef _SYSC_REPORT_H_
#define _SYSC_REPORT_H_

#include <sysc/utils/sc_report.h>
#include "scc/utilities.h"
#include <util/logging.h>
#include <sysc/kernel/sc_time.h>
#include <iomanip>
#include <sstream>

namespace logging {
class SystemC {};
}
namespace scc {

/**
 * initializes the SystemC logging system to use logging::Logger with a particular logging level
 */
void init_logging(logging::log_level level=logging::WARNING);

template<sc_core::sc_severity SEVERITY>
struct ScLogger {
    ScLogger(const char* file, int line, sc_core::sc_verbosity level=sc_core::SC_MEDIUM):t(nullptr), file(file), line(line), level(level){};

    ScLogger() = delete;

    ScLogger(const ScLogger &) = delete;

    ScLogger &operator=(const ScLogger &) = delete;

    virtual ~ScLogger() {
      ::sc_core::sc_report_handler::report(SEVERITY, t?t:"SystemC", os.str().c_str(), level, file , line );
    }

    inline
    ScLogger& type(){return *this;}

    inline
    ScLogger& type(const char* t){this->t=const_cast<char*>(t); return *this;}

    inline
    std::ostringstream& get() {return os;};

protected:
    std::ostringstream os;
    char* t;
    const char* file;
    const int line;
    const sc_core::sc_verbosity level;
};

#define SCDBGTRC(...) if (::sc_core::sc_report_handler::get_verbosity_level() >= sc_core::SC_DEBUG)   ::scc::ScLogger<::sc_core::SC_INFO>(__FILE__,__LINE__, sc_core::SC_DEBUG).type(__VA_ARGS__).get()
#define SCTRACE(...)  if (::sc_core::sc_report_handler::get_verbosity_level() >= sc_core::SC_FULL)   ::scc::ScLogger<::sc_core::SC_INFO>(__FILE__,__LINE__, sc_core::SC_FULL).type(__VA_ARGS__).get()
#define SCDEBUG(...)  if (::sc_core::sc_report_handler::get_verbosity_level() >= sc_core::SC_HIGH)  ::scc::ScLogger<::sc_core::SC_INFO>(__FILE__,__LINE__, sc_core::SC_HIGH).type(__VA_ARGS__).get()
#define SCINFO(...)   if (::sc_core::sc_report_handler::get_verbosity_level() >= sc_core::SC_MEDIUM) ::scc::ScLogger<::sc_core::SC_INFO>(__FILE__,__LINE__, sc_core::SC_MEDIUM).type(__VA_ARGS__).get()
#define SCWARN(...)   ::scc::ScLogger<::sc_core::SC_WARNING>(__FILE__,__LINE__,sc_core::SC_MEDIUM).type(__VA_ARGS__).get()
#define SCERR(...)    ::scc::ScLogger<::sc_core::SC_ERROR>(__FILE__,__LINE__,sc_core::SC_MEDIUM).type(__VA_ARGS__).get()

}

#endif /* _SYSC_REPORT_H_ */
