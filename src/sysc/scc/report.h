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
#include <cci_configuration>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <sysc/kernel/sc_time.h>
#include <sysc/utils/sc_report.h>
#include <unordered_map>
#include <util/ities.h>

#if defined(_MSC_VER) && defined(ERROR)
#undef ERROR
#endif

/**
## Reporting infrastructure

The reporting infrastructure consist of 2 part: the frontend on to of sc_report and the backend,
replacing SystemC standard handler. Both can be used independently.

## Reporting frontend

The frontend consist of a set of macros which provide a std::ostream to log a message. The execution of the
logging code is dependend on the loglevel thus not impacting performance if the message is not logged.

The macros take an optional argument which becomes the message type. If not provided, the default 'SystemC'
 is being used. The following table outlines how the scc::log level map to the SystemC logging parameter.

| SCC log level | SystemC severity | SystemC verbosity |
|---------------|------------------|-------------------|
| SCCFATAL      | SC_FATAL         | --                |
| SCCERR        | SC_ERROR         | --                |
| SCCWARN       | SC_WARNING       | --                |
| SCCINFO       | SC_INFO          | SC_MEDIUM         |
| SCCDEBUG      | SC_INFO          | SC_HIGH           |
| SCCTRACE      | SC_INFO          | SC_FULL           |
| SCCTRACEALL   | SC_INFO          | SC_DEBUG          |

## Reporting backend

The backend is initialized using the short form:

    scc::init_logging(scc::log::INFO);

or the long form

    scc::init_logging(scc::LogConfig()
        .logLevel(scc::log::DEBUG) // set log level to debug
        .msgTypeFieldWidth(10));   // make the msg type column a bit tighter

which allows more configurability. For detail please check the header file report.h
In both case an alternate report handler is installed which which uses a tabular format and spdlog for
writing. By default spdlog logs asyncronously to keep the performance impact low.
 */
//! the name of the CCI property to attach to modules to control logging of this module
#define SCC_LOG_LEVEL_PARAM_NAME "log_level"
/** \ingroup scc-sysc
 *  @{
 */
/**@{*/
//! @brief SCC SystemC utilities
namespace scc {
//! \brief enum defining the log levels
enum class log { NONE, FATAL, ERROR, WARNING, INFO, DEBUG, TRACE, TRACEALL, DBGTRACE = TRACEALL };
namespace {
//! \brief array holding string representations of log levels
static std::array<const char* const, 8> log_level_names = {{"NONE", "FATAL", "ERROR", "WARNING", "INFO", "DEBUG", "TRACE", "TRACEALL"}};
static const std::unordered_map<std::string, scc::log> log_level_lut = {
    {"NONE", scc::log::NONE}, {"FATAL", scc::log::FATAL}, {"ERROR", scc::log::ERROR}, {"WARNING", scc::log::WARNING},
    {"INFO", scc::log::INFO}, {"DEBUG", scc::log::DEBUG}, {"TRACE", scc::log::TRACE}, {"TRACEALL", scc::log::TRACEALL}};
} // namespace
/**
 * @fn log as_log(int)
 * @brief safely convert an integer into a log level
 *
 * @param logLevel the logging level
 * @return the log level
 */
inline log as_log(int logLevel) {
    assert(logLevel >= static_cast<int>(log::NONE) && logLevel <= static_cast<int>(log::TRACEALL));
    std::array<const log, 8> m = {{log::NONE, log::FATAL, log::ERROR, log::WARNING, log::INFO, log::DEBUG, log::TRACE, log::TRACEALL}};
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
    auto it = scc::log_level_lut.find(buf);
    if(it == std::end(scc::log_level_lut))
        throw std::out_of_range(std::string("Illegal log level value: ") + buf);
    val = it->second;
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
    os << log_level_names[static_cast<unsigned>(val)];
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
 * @fn void init_logging()
 * @brief re-initializes the SystemC logging system e.g. after a change in the log configuration via CCI
 *
 */
void reinit_logging();
/**
 * @fn void init_logging(log::WARNING)
 * @brief initializes the SystemC logging system with a particular default logging level
 *
 * @param level the log level
 */
void reinit_logging(log level);
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
    bool instance_based_log_levels{true};
    bool install_handler{true};

    /**
     * set the logging level
     * @param log level
     * @return self
     */
    LogConfig& logLevel(log);
    /**
     * define the width of the message field, 0 to disable, std::numeric_limits<unsigned>::max() for arbitrary width
     * @param width of the message field in the log
     * @return self
     */
    LogConfig& msgTypeFieldWidth(unsigned);
    /**
     * enable/disable printing of system time
     * @param enable
     * @return self
     */
    LogConfig& printSysTime(bool = true);
    /**
     * enable/disable printing of simulation time
     * @param enable
     * @return self
     */
    LogConfig& printSimTime(bool = true);
    /**
     * enable/disable printing delta cycles
     * @param enable
     * @return self
     */
    LogConfig& printDelta(bool = true);
    /**
     * enable/disable printing of severity level
     * @param enable
     * @return self
     */
    LogConfig& printSeverity(bool = true);
    /**
     * enable/disable colored output
     * @param enable
     * @return self
     */
    LogConfig& coloredOutput(bool = true);
    /**
     * set the file name for the log output file
     * @param name of the log file to be generated
     * @return self
     */
    LogConfig& logFileName(std::string&&);
    /**
     * set the file name for the log output file
     * @param name
     * @return self
     */
    LogConfig& logFileName(const std::string&);
    /**
     * set the file name for the log output file
     * @param name
     * @return self
     */
    LogConfig& logFilterRegex(std::string&&);
    /**
     * set the regular expression to filter the output
     * @param string containing the regex
     * @return self
     */
    LogConfig& logFilterRegex(const std::string&);
    /**
     * enable/disable asynchronous output (write to file in separate thread
     * @param enable
     * @return self
     */
    LogConfig& logAsync(bool = true);
    /**
     * disable/enable the automatic creation of a CCI broker
     * @param enable
     * @return self
     */
    LogConfig& dontCreateBroker(bool = true);
    /**
     * disable/enable the suppression of all error messages after the first error
     * @param
     * @return self
     */
    LogConfig& reportOnlyFirstError(bool = true);
    /**
     * disable/enable the suppression of all error messages after the first error
     * @param
     * @return self
     */
    LogConfig& instanceBasedLogLevels(bool = true);
    /**
     * disable/enable the use of the tabular report handler
     * @param
     * @return self
     */
    LogConfig& installHandler(bool = true);
};
/**
 * @fn void init_logging(const LogConfig&)
 * @brief initializes the SystemC logging system with a particular configuration
 *
 * @param log_config the logging configuration
 */
void init_logging(const LogConfig& log_config);
/**
 * @fn log is_logging_initialized()
 * @brief get the state of the SCC logging system
 *
 * @return true if the logging system has been initialized
 */
bool is_logging_initialized();
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
    virtual ~ScLogger() noexcept(false) {
        auto verb = ::sc_core::sc_report_handler::set_verbosity_level( sc_core::SC_FULL);
        ::sc_core::sc_report_handler::report(SEVERITY, t ? t : "SystemC", os.str().c_str(), level, file, line);
        ::sc_core::sc_report_handler::set_verbosity_level(verb);
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
//! macro for log output
#define SCCLOG(lvl, ...) ::scc::ScLogger<::sc_core::SC_INFO>(__FILE__, __LINE__, lvl).type(__VA_ARGS__).get()
//! macro for debug trace level output
#define SCCTRACEALL(...)                                                                                                                   \
    if(::scc::get_log_verbosity(__VA_ARGS__) >= sc_core::SC_DEBUG)                                                                         \
    SCCLOG(sc_core::SC_DEBUG, __VA_ARGS__)
//! macro for trace level output
#define SCCTRACE(...)                                                                                                                      \
    if(::scc::get_log_verbosity(__VA_ARGS__) >= sc_core::SC_FULL)                                                                          \
    SCCLOG(sc_core::SC_FULL, __VA_ARGS__)
//! macro for debug level output
#define SCCDEBUG(...)                                                                                                                      \
    if(::scc::get_log_verbosity(__VA_ARGS__) >= sc_core::SC_HIGH)                                                                          \
    SCCLOG(sc_core::SC_HIGH, __VA_ARGS__)
//! macro for info level output
#define SCCINFO(...)                                                                                                                       \
    if(::scc::get_log_verbosity(__VA_ARGS__) >= sc_core::SC_MEDIUM)                                                                        \
    SCCLOG(sc_core::SC_MEDIUM, __VA_ARGS__)
//! macro for warning level output
#define SCCWARN(...)                                                                                                                       \
    if(::scc::get_log_verbosity(__VA_ARGS__) >= sc_core::SC_LOW)                                                                           \
    ::scc::ScLogger<::sc_core::SC_WARNING>(__FILE__, __LINE__, sc_core::SC_MEDIUM).type(__VA_ARGS__).get()
//! macro for error level output
#define SCCERR(...) ::scc::ScLogger<::sc_core::SC_ERROR>(__FILE__, __LINE__, sc_core::SC_MEDIUM).type(__VA_ARGS__).get()
//! macro for fatal message output
#define SCCFATAL(...) ::scc::ScLogger<::sc_core::SC_FATAL>(__FILE__, __LINE__, sc_core::SC_MEDIUM).type(__VA_ARGS__).get()

#ifdef NDEBUG
#define SCC_ASSERT(expr) ((void)0)
#else
#define SCC_ASSERT(expr) ((void)((expr) ? 0 : (SC_REPORT_FATAL(::sc_core::SC_ID_ASSERTION_FAILED_, #expr), 0)))
#endif
//! get the name of the sc_object/sc_module
#ifndef SCMOD
#define SCMOD this->name()
#endif
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
    stream_redirection(std::ostream& os, log level);

    stream_redirection(stream_redirection const&) = delete;

    stream_redirection& operator=(stream_redirection const&) = delete;

    stream_redirection(stream_redirection&&) = delete;

    stream_redirection& operator=(stream_redirection&&) = delete;
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
/** @} */ // end of scc-sysc
namespace cci {
template <> inline bool cci_value_converter<scc::log>::pack(cci_value::reference dst, scc::log const& src) {
    dst.set_string(scc::log_level_names[static_cast<unsigned>(src)]);
    return true;
}
template <> inline bool cci_value_converter<scc::log>::unpack(scc::log& dst, cci_value::const_reference src) {
    // Highly defensive unpacker; probably could check less
    if(!src.is_string())
        return false;
    auto it = scc::log_level_lut.find(src.get_string());
    if(it != std::end(scc::log_level_lut)) {
        dst = it->second;
        return true;
    }
    return false;
}
} // namespace cci
#endif /* _SCC_REPORT_H_ */
