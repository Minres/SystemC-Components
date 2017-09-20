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

#include <util/logging.h>
#include <sysc/utilities.h>
#include <sysc/utils/sc_report.h>
#include <sstream>
#include <iomanip>


namespace sysc {

namespace log=logging;
/**
 *
 */
void init_logging();

/**
 *
 */
template <typename T>
class Log: public logging::Log<T>{
public:
    Log(){};

    /**
     *
     * @param level
     * @return
     */
    std::ostringstream& get(logging::log_level level = logging::INFO){
        std::ios init(NULL);
        init.copyfmt(this->os);
        this->os << logging::now_time() << " " << std::setw(7) <<std::left << logging::Log<T>::to_string(level)<<std::right
                << " ["<<std::setw(20)<<sc_core::sc_time_stamp()<<"] ";
        logging::Log<T>::get_last_log_level()=level;
        this->os.copyfmt(init);
        return this->os;
    };

};

class FILELOG_DECLSPEC Logger : public Log<logging::Output2FILE> {
    static std::once_flag once;
public:
    /**
     *
     *
     * @return
     */
    static logging::log_level& reporting_level(){
        std::call_once(once, [](){ init_logging();});
        return logging::Log<logging::Output2FILE>::reporting_level();
    }
};

}

#undef LOG
#define LOG(level) \
        if (level > FILELOG_MAX_LEVEL) ;\
        else if (level > logging::Logger::reporting_level() || !logging::Output2FILE::stream()) ; \
        else sysc::Logger().get(level)


#endif /* _SYSC_REPORT_H_ */
