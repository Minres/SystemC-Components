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
            // if (strlen(category))
            //    this->os << "[" << std::setw(10) << category<<"]";
            // else
            //    this->os << "            ";
            this->os << std::right;
        }
        this->os << " [" << std::setw(20) << time2string(sc_core::sc_time_stamp()) << "] ";
        this->os.copyfmt(init);
        log::Log<T>::get_last_log_level() = level;
        return this->os;
    };
protected:
    std::string time2string(const sc_core::sc_time& t) const{
        const std::array<const char*, 6> time_units{"fs", "ps", "ns", "us", "ms", "s "};
        const std::array<uint64_t, 6> multiplier{1ULL, 1000ULL, 1000ULL*1000, 1000ULL*1000*1000, 1000ULL*1000*1000*1000, 1000ULL*1000*1000*1000*1000};
        std::ostringstream oss;
        const sc_core::sc_time_tuple tt{t};
        const auto val = tt.value();
        if ( !val ) {
            oss << "0 s";
        } else {
            const unsigned scale = tt.unit();
            const auto fs_val = val*multiplier[scale];
            for(int j = multiplier.size()-1; j>=scale; --j){
                if(fs_val>multiplier[j]){
                    const auto i = val/multiplier[j-scale];
                    const auto f = val%multiplier[j-scale];
                    oss<<i<<'.'<<std::setw(3*(j-scale))<<std::setfill('0')<<std::left<<f<<' ' << time_units[j];
                    break;
                }
            }
        }
        return oss.str();
    }
};

template <typename CATEGORY = log::SystemC> class FILELOG_DECLSPEC Logger : public Log<log::Output2FILE<CATEGORY>> {
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
    if (logging::LEVEL <= logging::Log<logging::Output2FILE<logging::SystemC>>::reporting_level() &&                   \
        LOG_OUTPUT(SystemC)::stream())                                                                                 \
    scc::Log<logging::Output2FILE<logging::SystemC>>().get(logging::LEVEL, "SystemC")

#ifdef CLOG
#undef CLOG
#endif
#define CLOG(LEVEL, CATEGORY)                                                                                          \
    if (logging::LEVEL <= LOGGER(CATEGORY)::reporting_level() && LOG_OUTPUT(CATEGORY)::stream())                       \
    scc::Log<logging::Output2FILE<logging::CATEGORY>>().get(logging::LEVEL, #CATEGORY)

#endif /* _SYSC_REPORT_H_ */
