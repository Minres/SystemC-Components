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

#ifndef _SCC_REPORT_H_
#define _SCC_REPORT_H_

#include "utilities.h"
#include <iomanip>
#include <sstream>
#include <sysc/kernel/sc_time.h>
#include <sysc/utils/sc_report.h>
#include <util/ities.h>
#include <util/logging.h>

namespace scc {

/**
 * initializes the SystemC logging system with a particular logging level
 *
 * @param level the logging level
 * @param type_field_width width of the message type field in output, setting to zero suppresses the message type
 * @param print_time wheter to print the system time stamp
 */
void init_logging(logging::log_level level = logging::WARNING, unsigned type_field_width = 24, bool print_time = false);
/**
 * the configuration class for the logging setup
 */
struct LogConfig {
    logging::log_level level{logging::WARNING};
    unsigned msg_type_field_width{24};
    bool print_sys_time{false};
    bool print_sim_time{true};
    bool print_delta{false};
    bool print_severity{true};
    bool colored_output{true};
    std::string log_file_name{""};
    std::string log_filter_regex{""};
    bool log_async{true};
    bool dont_create_broker{false};

    LogConfig& logLevel(logging::log_level);
    LogConfig& msgTypeFieldWidth(unsigned);
    LogConfig& printSysTime(bool);
    LogConfig& printSimTime(bool);
    LogConfig& printDelta(bool);
    LogConfig& printSeverity(bool);
    LogConfig& coloredOutput(bool);
    LogConfig& logFileName(std::string&&);
    LogConfig& logFileName(const std::string&);
    LogConfig& logFilterRegex(std::string&&);
    LogConfig& logFilterRegex(const std::string&);
    LogConfig& logAsync(bool);
    LogConfig& dontCreateBroker(bool);
};
/**
 * initializes the SystemC logging system with a particular configuration
 *
 * @param log_config the logging configuration
 */
void init_logging(const LogConfig& log_config);
/**
 * sets the SystemC logging level
 *
 * @param level the logging level
 */
void set_logging_level(logging::log_level level);
/**
 * sets the cycle base for logging. If this is set the logging prints cycles instead of times
 *
 * @param level the logging level
 */
void set_cycle_base(sc_core::sc_time period);
/**
 * return the global verbosity level
 * @return
 */
inline sc_core::sc_verbosity get_log_verbosity(){
    return static_cast<sc_core::sc_verbosity>(::sc_core::sc_report_handler::get_verbosity_level());
}
/**
 * return a scope specific verbosity level if defined. Otherwise the global verbosity level
 * @param t
 * @return
 */
sc_core::sc_verbosity get_log_verbosity(std::string const& t);
/**
 * return an scope specific verbosity level if defined. Otherwise the global verbosity level
 * @param t
 * @return
 */
inline sc_core::sc_verbosity get_log_verbosity(char const* t){
    return get_log_verbosity(std::string(t));
}
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
    ScLogger(const char* file, int line, int verbosity = sc_core::SC_MEDIUM)
    : t(nullptr)
    , file(file)
    , line(line)
    , level(verbosity){};
    /**
     * no default constructor
     */
    ScLogger() = delete;
    /**
     * no copy constructor
     * @param
     */
    ScLogger(const ScLogger&) = delete;
    /**
     * no move constructor
     * @param
     */
    ScLogger(ScLogger&&) = delete;
    /**
     * no copy assignment
     * @param
     * @return
     */
    ScLogger& operator=(const ScLogger&) = delete;
    /**
     * no move assignment
     * @param
     * @return
     */
    ScLogger& operator=(ScLogger&&) = delete;
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
    inline ScLogger& type() {
        this->t = nullptr;
        return *this;
    }
    /**
     * set the category of the log entry
     *
     * @param t
     * @return
     */
    inline ScLogger& type(char const* t) {
        this->t = const_cast<char*>(t);
        return *this;
    }
    /**
     * set the category of the log entry
     *
     * @param t
     * @return
     */
    inline ScLogger& type(std::string const& t) {
        this->t = const_cast<char*>(t.c_str());
        return *this;
    }
    /**
     * return the underlying ostringstream
     *
     * @return the output stream collecting the log message
     */
    inline std::ostream& get() { return os; };

protected:
    std::ostringstream os;
    char* t;
    const char* file;
    const int line;
    const int level;
};

/**
 * logging macros
 */
//! macro for debug trace level output
#define SCCTRACEALL(...)                                                                                               \
    if(::scc::get_log_verbosity(__VA_ARGS__) >= sc_core::SC_DEBUG)                                       \
    ::scc::ScLogger<::sc_core::SC_INFO>(__FILE__, __LINE__, sc_core::SC_DEBUG/10).type(__VA_ARGS__).get()
//! macro for trace level output
#define SCCTRACE(...)                                                                                                  \
    if(::scc::get_log_verbosity(__VA_ARGS__) >= sc_core::SC_FULL)                                        \
    ::scc::ScLogger<::sc_core::SC_INFO>(__FILE__, __LINE__, sc_core::SC_FULL/10).type(__VA_ARGS__).get()
//! macro for debug level output
#define SCCDEBUG(...)                                                                                                  \
    if(::scc::get_log_verbosity(__VA_ARGS__) >= sc_core::SC_HIGH)                                        \
    ::scc::ScLogger<::sc_core::SC_INFO>(__FILE__, __LINE__, sc_core::SC_HIGH/10).type(__VA_ARGS__).get()
//! macro for info level output
#define SCCINFO(...)                                                                                                   \
    if(::scc::get_log_verbosity(__VA_ARGS__) >= sc_core::SC_MEDIUM)                                      \
    ::scc::ScLogger<::sc_core::SC_INFO>(__FILE__, __LINE__, sc_core::SC_MEDIUM/10).type(__VA_ARGS__).get()
//! macro for warning level output
#define SCCWARN(...)                                                                                                   \
    ::scc::ScLogger<::sc_core::SC_WARNING>(__FILE__, __LINE__, sc_core::SC_MEDIUM).type(__VA_ARGS__).get()
//! macro for error level output
#define SCCERR(...) ::scc::ScLogger<::sc_core::SC_ERROR>(__FILE__, __LINE__, sc_core::SC_MEDIUM).type(__VA_ARGS__).get()
//! macro for fatal message output
#define SCCFATAL(...)                                                                                                  \
    ::scc::ScLogger<::sc_core::SC_FATAL>(__FILE__, __LINE__, sc_core::SC_MEDIUM).type(__VA_ARGS__).get()

#ifdef NDEBUG
#define SCC_ASSERT(expr) ((void)0)
#else
#define SCC_ASSERT(expr) ((void)((expr) ? 0 : (SC_REPORT_FATAL(::sc_core::SC_ID_ASSERTION_FAILED_, #expr), 0)))
#endif

#define SCMOD this->name()

/**
 * deprecated logging macros, may cause macro name clashes
 */
//! macro for debug trace level output
#ifndef SCDBGTRC
#define SCDBGTRC(...)                                                                                                  \
    if(::scc::get_log_verbosity() >= sc_core::SC_DEBUG)                                       \
    ::scc::ScLogger<::sc_core::SC_INFO>(__FILE__, __LINE__, sc_core::SC_DEBUG).type(__VA_ARGS__).get()
#endif
//! macro for trace level output
#ifndef SCTRACE
#define SCTRACE(...)                                                                                                   \
    if(::scc::get_log_verbosity() >= sc_core::SC_FULL)                                        \
    ::scc::ScLogger<::sc_core::SC_INFO>(__FILE__, __LINE__, sc_core::SC_FULL).type(__VA_ARGS__).get()
//! macro for debug level output
#endif
#ifndef SCDEBUG
#define SCDEBUG(...)                                                                                                   \
    if(::scc::get_log_verbosity() >= sc_core::SC_HIGH)                                        \
    ::scc::ScLogger<::sc_core::SC_INFO>(__FILE__, __LINE__, sc_core::SC_HIGH).type(__VA_ARGS__).get()
#endif
//! macro for info level output
#ifndef SCINFO
#define SCINFO(...)                                                                                                    \
    if(::scc::get_log_verbosity() >= sc_core::SC_MEDIUM)                                      \
    ::scc::ScLogger<::sc_core::SC_INFO>(__FILE__, __LINE__, sc_core::SC_MEDIUM).type(__VA_ARGS__).get()
//! macro for warning level output
#endif
#ifndef SCWARN
#define SCWARN(...)                                                                                                    \
    ::scc::ScLogger<::sc_core::SC_WARNING>(__FILE__, __LINE__, sc_core::SC_MEDIUM).type(__VA_ARGS__).get()
#endif
//! macro for error level output
#ifndef SCERR
#define SCERR(...) ::scc::ScLogger<::sc_core::SC_ERROR>(__FILE__, __LINE__, sc_core::SC_MEDIUM).type(__VA_ARGS__).get()
#endif
//! macro for fatal message output
#ifndef SCFATAL
#define SCFATAL(...)                                                                                                   \
    ::scc::ScLogger<::sc_core::SC_FATAL>(__FILE__, __LINE__, sc_core::SC_MEDIUM).type(__VA_ARGS__).get()
#endif
class stream_redirection : public std::stringbuf {
public:
    stream_redirection(std::ostream& os, logging::log_level level);
    ~stream_redirection();
    void reset();

protected:
    std::streamsize xsputn(const char_type* s, std::streamsize n) override;
    int sync() override;
    std::ostream& os;
    logging::log_level level;
    std::streambuf* old_buf;
};

} // namespace scc

#endif /* _SCC_REPORT_H_ */
