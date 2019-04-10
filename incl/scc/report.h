/*******************************************************************************
 * Copyright 2016, 2018 MINRES Technologies GmbH
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

#ifndef _SYSC_REPORT_H_
#define _SYSC_REPORT_H_

#include "utilities.h"
#include <iomanip>
#include <sstream>
#include <sysc/kernel/sc_time.h>
#include <sysc/utils/sc_report.h>
#include <util/logging.h>

namespace logging {
class SystemC {};
class STDIO {};
}
namespace scc {

/**
 * initializes the SystemC logging system to use logging::Logger with a particular logging level
 *
 * @param level the logging level
 */
void init_logging(logging::log_level level = logging::WARNING, bool print_time = false);
/**
 * the logger class
 */
template <sc_core::sc_severity SEVERITY> struct ScLogger {
    /**
     * the constructor
     *
     * @param file file where the log entry originates
     * @param line the line where the log entry originates
     * @param level the log level
     */
    ScLogger(const char *file, int line, sc_core::sc_verbosity level = sc_core::SC_MEDIUM)
    : t(nullptr)
    , file(file)
    , line(line)
    , level(level){};
    /**
     * no default constructor
     */
    ScLogger() = delete;
    /**
     * no copy constructor
     * @param
     */
    ScLogger(const ScLogger &) = delete;
    /**
     * no move constructor
     * @param
     */
    ScLogger(ScLogger &&) = delete;
    /**
     * no copy assignment
     * @param
     * @return
     */
    ScLogger &operator=(const ScLogger &) = delete;
    /**
     * no move assignment
     * @param
     * @return
     */
    ScLogger &operator=(ScLogger &&) = delete;
    /**
     * the destructor generating the SystemC report
     */
    virtual ~ScLogger() {
        ::sc_core::sc_report_handler::report(SEVERITY, t ? t : "SystemC", os.str().c_str(), level, file, line);
    }
    /**
     * reset the category of the log entry
     *
     * @return
     */
    inline ScLogger &type() {
        this->t=nullptr;
        return *this;
    }
    /**
     * set the category of the log entry
     *
     * @param t
     * @return
     */
    inline ScLogger &type(const char *t) {
        this->t = const_cast<char *>(t);
        return *this;
    }
    /**
     * set the category of the log entry
     *
     * @param t
     * @return
     */
    inline ScLogger &type(const std::string& t) {
        this->t = const_cast<char *>(t.c_str());
        return *this;
    }
    /**
     * return the underlying ostringstream
     *
     * @return the output stream collecting the log message
     */
    inline std::ostream &get() { return os; };

protected:
    std::ostringstream os;
    char *t;
    const char *file;
    const int line;
    const sc_core::sc_verbosity level;
};
//! macro for debug trace lavel output
#define SCDBGTRC(...)                                                                                                  \
    if (::sc_core::sc_report_handler::get_verbosity_level() >= sc_core::SC_DEBUG)                                      \
    ::scc::ScLogger<::sc_core::SC_INFO>(__FILE__, __LINE__, sc_core::SC_DEBUG).type(__VA_ARGS__).get()
//! macro for trace level output
#define SCTRACE(...)                                                                                                   \
    if (::sc_core::sc_report_handler::get_verbosity_level() >= sc_core::SC_FULL)                                       \
    ::scc::ScLogger<::sc_core::SC_INFO>(__FILE__, __LINE__, sc_core::SC_FULL).type(__VA_ARGS__).get()
//! macro for debug level output
#define SCDEBUG(...)                                                                                                   \
    if (::sc_core::sc_report_handler::get_verbosity_level() >= sc_core::SC_HIGH)                                       \
    ::scc::ScLogger<::sc_core::SC_INFO>(__FILE__, __LINE__, sc_core::SC_HIGH).type(__VA_ARGS__).get()
//! macro for info level output
#define SCINFO(...)                                                                                                    \
    if (::sc_core::sc_report_handler::get_verbosity_level() >= sc_core::SC_MEDIUM)                                     \
    ::scc::ScLogger<::sc_core::SC_INFO>(__FILE__, __LINE__, sc_core::SC_MEDIUM).type(__VA_ARGS__).get()
//! macro for warning level output
#define SCWARN(...)                                                                                                    \
    ::scc::ScLogger<::sc_core::SC_WARNING>(__FILE__, __LINE__, sc_core::SC_MEDIUM).type(__VA_ARGS__).get()
//! macro for error level output
#define SCERR(...) ::scc::ScLogger<::sc_core::SC_ERROR>(__FILE__, __LINE__, sc_core::SC_MEDIUM).type(__VA_ARGS__).get()
//! macro for fatal message output
#define SCFATAL(...)                                                                                                   \
    ::scc::ScLogger<::sc_core::SC_FATAL>(__FILE__, __LINE__, sc_core::SC_MEDIUM).type(__VA_ARGS__).get()

inline std::string padded(std::string str, size_t width, bool show_ellipsis=true) {
    assert(width>7);
    if (str.length() > width) {
        if (show_ellipsis){
            auto pos = str.size()-(width-6);
            return str.substr(0, 3) + "..."+str.substr(pos, str.size()-pos);
        } else
            return str.substr(0, width);
    } else {
        return str+std::string(width-str.size(), ' ');
    }
}

class stream_redirection: public std::stringbuf {
public:
    stream_redirection(std::ostream& os, logging::log_level level);
    ~stream_redirection();
    void reset();
protected:
    std::streamsize xsputn(const char_type* s, std::streamsize n) override;
    std::ostream& os;
    logging::log_level level;
    std::streambuf* old_buf;
};

}

#endif /* _SYSC_REPORT_H_ */
