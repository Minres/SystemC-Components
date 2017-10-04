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

#include <iomanip>
#include <sstream>
#include <sysc/utilities.h>
#include <sysc/utils/sc_report.h>
#include <util/logging.h>

namespace logging {
struct SystemC {};
}
namespace sysc {

namespace log = logging;
/**
 *
 */
void init_logging();

/**
 *
 */
template <typename T> class Log : public log::Log<T> {
public:
    Log() = default;

    Log(const Log &) = delete;

    Log &operator=(const Log &) = delete;

    /**
     *
     * @param level
     * @return
     */
    std::ostringstream &get(log::log_level level = log::INFO, const char *category = "") {
        std::ios init(NULL);
        init.copyfmt(this->os);
        if (this->print_time()) this->os << "- " << log::now_time();
        if (this->print_severity()) {
            this->os << " " << std::setw(7) << std::left << this->to_string(level);
            //if (strlen(category))
            //    this->os << "[" << std::setw(10) << category<<"]";
            //else
            //    this->os << "            ";
            this->os << std::right;
        }
        this->os << " [" << std::setw(20) << sc_core::sc_time_stamp() << "] ";
        this->os.copyfmt(init);
        log::Log<T>::get_last_log_level() = level;
        return this->os;
    };
};

template<typename CATEGORY = log::SystemC>
class FILELOG_DECLSPEC Logger : public Log<log::Output2FILE<CATEGORY>> {
public:
    /**
     *
     *
     * @return
     */
    static log::log_level &reporting_level() {
        static std::once_flag once;
        std::call_once(once, []() { init_logging(); });
        return log::Log<log::Output2FILE<log::SystemC>>::reporting_level();
    }
};
}

#ifdef LOG
#undef LOG
#endif
#define LOG(LEVEL)                                                                                                     \
    if (logging::LEVEL <= logging::Log<logging::Output2FILE<logging::SystemC>>::reporting_level() && LOG_OUTPUT(SystemC)::stream())  \
        sysc::Log<logging::Output2FILE<logging::SystemC>>().get(logging::LEVEL, "SystemC")

#ifdef CLOG
#undef CLOG
#endif
#define CLOG(LEVEL, CATEGORY)                                                                    \
    if (logging::LEVEL <= LOGGER(CATEGORY)::reporting_level() && LOG_OUTPUT(CATEGORY)::stream()) \
        sysc::Log<logging::Output2FILE<logging::CATEGORY>>().get(logging::LEVEL, #CATEGORY)

#endif /* _SYSC_REPORT_H_ */
