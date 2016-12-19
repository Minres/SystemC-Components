/*
 * logging.h
 *
 *  Created on: Nov 24, 2016
 *      Author: developer
 */

#ifndef INCL_SYSC_LOGGING_H_
#define INCL_SYSC_LOGGING_H_

#include <util/logging.h>
#include <systemc>

namespace logging {
template <typename T>
class ScLog: public Log<T>{
public:
    ScLog(){};
    std::ostringstream& Get(LogLevel level = info){
        this->os << "- " << NowTime();
        this->os << " " << Log<T>::ToString(level) << " - ["<<sc_core::sc_time_stamp()<<"] ";
        Log<T>::getLastLogLevel()=level;
        return this->os;
    };
};

class FILELOG_DECLSPEC ScLogger : public ScLog<Output2FILE> {};

#undef LOG
#define LOG(level) \
    if (level > FILELOG_MAX_LEVEL) ;\
    else if (level > logging::Logger::ReportingLevel() || !logging::Output2FILE::Stream()) ; \
    else logging::ScLogger().Get(level)


}

#endif /* INCL_SYSC_LOGGING_H_ */
