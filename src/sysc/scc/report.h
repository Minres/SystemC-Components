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
#include <cstring>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <sysc/kernel/sc_time.h>
#include <sysc/utils/sc_report.h>
#include <util/ities.h>

#if defined(_MSC_VER) && defined(ERROR)
#undef ERROR
#endif

#define SCC_LOG_LEVEL_PARAM_NAME "log_level"
namespace scc {
//! \brief array holding string representations of log levels
static std::array<const char* const, 8> buffer = {
    {"NONE", "FATAL", "ERROR", "WARNING", "INFO", "DEBUG", "TRACE", "TRACEALL"}};
//! \brief enum defining the log levels
enum class log { NONE, FATAL, ERROR, WARNING, INFO, DEBUG, TRACE, TRACEALL, DBGTRACE = TRACEALL };
/**
 * @fn log as_log(int)
 * @brief safely convert an integer into a log level
 *
 * @param logLevel the logging level
 * @return the log level
 */
inline log as_log(int logLevel) {
    assert(logLevel >= static_cast<int>(log::NONE) && logLevel <= static_cast<int>(log::TRACEALL));
    std::array<const log, 8> m = {
        {log::NONE, log::FATAL, log::ERROR, log::WARNING, log::INFO, log::DEBUG, log::TRACE, log::TRACEALL}};
    return m[logLevel];
}
/**
 * @fn std::istream& operator >>(std::istream&, log&)
 * @brief read a log level from input stream e.g. used by boost::lexical_cast
 *
 * @param is input stream holding the string representation
 * @param val the value holding the resulting value
 * @return the input stream
 */
inline std::istream& operator>>(std::istream& is, log& val) {
    std::string buf;
    is >> buf;
    for(auto i = 0U; i <= static_cast<unsigned>(log::TRACEALL); ++i) {
        if(std::strcmp(buf.c_str(), buffer[i]) == 0) {
            val = as_log(i);
            return is;
        }
    }
    return is;
}
/**
 * @fn std::ostream& operator <<(std::ostream&, const log&)
 * @brief output the textual representation of the log level
 *
 * @param os output stream
 * @param val logging level
 * @return reference to the stream for chaining
 */
inline std::ostream& operator<<(std::ostream& os, log const& val) {
    os << buffer[static_cast<unsigned>(val)];
    return os;
}
/**
 * @fn void init_logging(log=log::WARNING, unsigned=24, bool=false)
 * @brief initializes the SystemC logging system with a particular logging level
 *
 * @param level the log level
 * @param type_field_width the with of the type field in the output
 * @param print_time whether to print the system time stamp
 */
void init_logging(log level = log::WARNING, unsigned type_field_width = 24, bool print_time = false);
/**
 * @fn void init_logging(log=log::WARNING, unsigned=24, bool=false)
 * @brief initializes the SystemC logging system with a particular logging level
 *
 * @param level the log level
 * @param type_field_width the with of the type field in the output
 * @param print_time whether to print the system time stamp
 */
void reinit_logging(log level = log::WARNING);
/**
 * @struct LogConfig
 * @brief the configuration class for the logging setup
 *
 * using this class allows to configure the logging output in many aspects. The class follows the builder pattern.
 */
struct LogConfig {
    log level{log::WARNING};
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
    bool report_only_first_error{false};

    LogConfig& logLevel(log);
    LogConfig& msgTypeFieldWidth(unsigned);
    LogConfig& printSysTime(bool = true);
    LogConfig& printSimTime(bool = true);
    LogConfig& printDelta(bool = true);
    LogConfig& printSeverity(bool = true);
    LogConfig& coloredOutput(bool = true);
    LogConfig& logFileName(std::string&&);
    LogConfig& logFileName(const std::string&);
    LogConfig& logFilterRegex(std::string&&);
    LogConfig& logFilterRegex(const std::string&);
    LogConfig& logAsync(bool = true);
    LogConfig& dontCreateBroker(bool = true);
    LogConfig& reportOnlyFirstError(bool = true);
};
/**
 * @fn void init_logging(const LogConfig&)
 * @brief initializes the SystemC logging system with a particular configuration
 *
 * @param log_config the logging configuration
 */
void init_logging(const LogConfig& log_config);
/**
 * @fn void set_logging_level(log)
 * @brief sets the SystemC logging level
 *
 * @param level the logging level
 */
void set_logging_level(log level);
/**
 * @fn log get_logging_level()
 * @brief get the SystemC logging level
 *
 * @return the logging level
 */
log get_logging_level();
/**
 * @fn void set_cycle_base(sc_core::sc_time)
 * @brief sets the cycle base for cycle based logging
 *
 * if this is set to a non-SC_ZERO_TIME value all logging timestamps are printed as cyles (multiple of this value)
 *
 * @param period the cycle period
 */
void set_cycle_base(sc_core::sc_time period);
/**
 * @fn sc_core::sc_verbosity get_log_verbosity()
 * @brief get the global verbosity level
 *
 * @return the global verbosity level
 */
inline sc_core::sc_verbosity get_log_verbosity() {
    return static_cast<sc_core::sc_verbosity>(::sc_core::sc_report_handler::get_verbosity_level());
}
/**
 * @fn sc_core::sc_verbosity get_log_verbosity(const char*)
 * @brief get the scope-based verbosity level
 *
 * The function returns a scope specific verbosity level if defined (e.g. by using a CCI param named "log_level").
 * Otherwise the global verbosity level is being returned
 *
 * @param t the SystemC hierarchy scope name
 * @return the verbosity level
 */
sc_core::sc_verbosity get_log_verbosity(char const* t);
/**
 * @fn sc_core::sc_verbosity get_log_verbosity(const char*)
 * @brief get the scope-based verbosity level
 *
 * The function returns a scope specific verbosity level if defined (e.g. by using a CCI param named "log_level").
 * Otherwise the global verbosity level is being returned
 *
 * @param t the SystemC hierarchy scope name
 * @return the verbosity level
 */
inline sc_core::sc_verbosity get_log_verbosity(std::string const& t) { return get_log_verbosity(t.c_str()); }
/**
 * @struct ScLogger
 * @brief the logger class
 *
 * The ScLogger creates a RTTI based output stream to be used similar to std::cout
 *
 * @tparam SEVERITY
 */
template <sc_core::sc_severity SEVERITY> struct ScLogger {
    /**
     * @fn  ScLogger(const char*, int, int=sc_core::SC_MEDIUM)
     * @brief
     *
     * @param file where the log entry originates
     * @param line number where the log entry originates
     * @param verbosity the log level
     */
    ScLogger(const char* file, int line, int verbosity = sc_core::SC_MEDIUM)
    : t(nullptr)
    , file(file)
    , line(line)
    , level(verbosity){};

    ScLogger() = delete;

    ScLogger(const ScLogger&) = delete;

    ScLogger(ScLogger&&) = delete;

    ScLogger& operator=(const ScLogger&) = delete;

    ScLogger& operator=(ScLogger&&) = delete;
    /**
     * @fn  ~ScLogger()
     * @brief the destructor generating the SystemC report
     *
     */
    virtual ~ScLogger() {
        ::sc_core::sc_report_handler::report(SEVERITY, t ? t : "SystemC", os.str().c_str(), level, file, line);
    }
    /**
     * @fn ScLogger& type()
     * @brief reset the category of the log entry
     *
     * @return reference to self for chaining
     */
    inline ScLogger& type() {
        this->t = nullptr;
        return *this;
    }
    /**
     * @fn ScLogger& type(const char*)
     * @brief set the category of the log entry
     *
     * @param t type of th elog entry
     * @return reference to self for chaining
     */
    inline ScLogger& type(char const* t) {
        this->t = const_cast<char*>(t);
        return *this;
    }
    /**
     * @fn ScLogger& type(std::string const&)
     * @brief set the category of the log entry
     *
     * @param t type of th elog entry
     * @return reference to self for chaining
     */
    inline ScLogger& type(std::string const& t) {
        this->t = const_cast<char*>(t.c_str());
        return *this;
    }
    /**
     * @fn std::ostream& get()
     * @brief  get the underlying ostringstream
     *
     * @return the output stream collecting the log message
     */
    inline std::ostream& get() { return os; };

protected:
    std::ostringstream os{};
    char* t{nullptr};
    const char* file;
    const int line;
    const int level;
};

/**
 * logging macros
 */
//! macro for debug trace level output
#define SCCTRACEALL(...)                                                                                               \
    if(::scc::get_log_verbosity(__VA_ARGS__) >= sc_core::SC_DEBUG)                                                     \
    ::scc::ScLogger<::sc_core::SC_INFO>(__FILE__, __LINE__, sc_core::SC_DEBUG / 10).type(__VA_ARGS__).get()
//! macro for trace level output
#define SCCTRACE(...)                                                                                                  \
    if(::scc::get_log_verbosity(__VA_ARGS__) >= sc_core::SC_FULL)                                                      \
    ::scc::ScLogger<::sc_core::SC_INFO>(__FILE__, __LINE__, sc_core::SC_FULL / 10).type(__VA_ARGS__).get()
//! macro for debug level output
#define SCCDEBUG(...)                                                                                                  \
    if(::scc::get_log_verbosity(__VA_ARGS__) >= sc_core::SC_HIGH)                                                      \
    ::scc::ScLogger<::sc_core::SC_INFO>(__FILE__, __LINE__, sc_core::SC_HIGH / 10).type(__VA_ARGS__).get()
//! macro for info level output
#define SCCINFO(...)                                                                                                   \
    if(::scc::get_log_verbosity(__VA_ARGS__) >= sc_core::SC_MEDIUM)                                                    \
    ::scc::ScLogger<::sc_core::SC_INFO>(__FILE__, __LINE__, sc_core::SC_MEDIUM / 10).type(__VA_ARGS__).get()
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
//! get the name of the sc_object/sc_module
#define SCMOD this->name()
/**
 * @class stream_redirection
 * @brief stream redirector
 *
 * the stream redirector allows to redirect std::cout and std::cerr into SystemC log messages
 *
 */
class stream_redirection : public std::stringbuf {
public:
    /**
     * @fn  stream_redirection(std::ostream&, scc::log)
     * @brief constructor redirecting the given stream to a SystemC log message of given llog level
     *
     * @param os the stream to be redirected
     * @param level the log level to use
     */
    stream_redirection(std::ostream& os, scc::log level);

    stream_redirection(scc::stream_redirection const&) = delete;

    stream_redirection& operator=(scc::stream_redirection const&) = delete;

    stream_redirection(scc::stream_redirection&&) = delete;

    stream_redirection& operator=(scc::stream_redirection&&) = delete;
    /**
     * @fn  ~stream_redirection()
     * @brief destructor restoring the output stream buffer
     *
     */
    ~stream_redirection();
    /**
     * @fn void reset()
     * @brief reset the stream redirection and restore output buffer of the stream
     *
     */
    void reset();

protected:
    std::streamsize xsputn(const char_type* s, std::streamsize n) override;
    int sync() override;
    std::ostream& os;
    log level;
    std::streambuf* old_buf{nullptr};
};

} // namespace scc

#endif /* _SCC_REPORT_H_ */
