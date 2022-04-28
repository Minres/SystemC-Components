/*******************************************************************************
 * Copyright 2017, 2018 MINRES Technologies GmbH
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

#pragma once

#include <atomic>
#include <cassert>
#include <cstdio>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <mutex>
#include <sstream>
#include <string>
#ifdef __unix__
#include <sys/time.h>
#endif
#include <array>
#include <vector>

//! log level definitions
#define LEVELS(L) L(NONE) L(FATAL) L(ERR) L(WARN) L(INFO) L(DEBUG) L(TRACE) L(TRACEALL)
#define DO_DESCRIPTION(e) #e,
#define DO_ENUM(e) e,

/**
 * \ingroup scc-common
 */
/**@{*/
//! @brief SCC C++ logging
namespace logging {
//! array holding string representations of log levels
static const std::array<const char* const, 8> buffer = {{LEVELS(DO_DESCRIPTION)}};
//! enum defining the log levels
enum log_level { LEVELS(DO_ENUM) DBGTRACE = TRACEALL };
/**
 * safely convert an integer into a log level
 * @param logLevel the integer
 * @return the log level
 */
inline log_level as_log_level(int logLevel) {
    assert(logLevel >= NONE && logLevel <= TRACEALL);
    std::array<const log_level, 8> m = {{NONE, FATAL, ERR, WARN, INFO, DEBUG, TRACE, TRACEALL}};
    return m[logLevel];
}
/**
 * read a log level from input stream e.g. used by boost::lexical_cast
 * @param is input stream holding the string representation
 * @param val the value holding the resulting value
 * @return the input stream
 */
inline std::istream& operator>>(std::istream& is, log_level& val) {
    std::string buf;
    is >> buf;
    for(auto i = 0U; i <= log_level::TRACEALL; ++i) {
        if(strcmp(buf.c_str(), buffer[i]) == 0) {
            val = as_log_level(i);
            return is;
        }
    }
    return is;
}
/**
 * get the current host time as string
 * @return
 */
inline std::string now_time();
/**
 * the logger class
 */
template <typename T> class Log {
public:
    Log() = default;

    Log(const Log&) = delete;

    Log& operator=(const Log&) = delete;
    /**
     * @fn  ~Log()
     * @brief the destructor
     *
     */
    virtual ~Log() {
        os << std::endl;
        T::output(os.str());
        // TODO: use a more specific exception
        if(get_last_log_level() == FATAL && abort_on_fatal())
            abort();
    }
    /**
     * get the underlying ostringstream for a certain log level and category
     *
     * @param level the log level
     * @param category the category string
     * @return the underlying output stream
     */
    std::ostream& get(log_level level = INFO, const char* category = "") {
        if(print_time())
            os << "- " << now_time() << " ";
        if(print_severity()) {
            os << std::setw(7) << std::left << to_string(level);
            if(strlen(category))
                os << "[" << category << "]";
            os << ": " << std::internal;
        }
        get_last_log_level() = level;
        return os;
    };
    /**
     * get a reference to the configured logging level
     *
     * @return the logging level
     */
    static std::atomic<log_level>& reporting_level() {
        static std::atomic<log_level> reportingLevel{WARN};
        return reportingLevel;
    }
    /**
     * get a reference to the configured logging level
     *
     * @return the logging level
     */
    static log_level get_reporting_level() { return reporting_level().load(std::memory_order_relaxed); }
    /**
     * get a reference to the abort on fatal flag
     *
     * @return the logging level
     */
    static std::atomic<bool>& abort_on_fatal() {
        static std::atomic<bool> flag{false};
        return flag;
    }
    /**
     * translate a lg level to a string
     *
     * @param level the log level
     * @return the string representing the log level
     */
    static std::string to_string(log_level level) { return std::string(get_log_level_cstr()[level]); };
    /**
     * parse a log level from a string
     *
     * @param level the string representing the log level
     * @return the log level
     */
    static log_level from_string(const std::string& level) {
        for(unsigned int i = NONE; i <= TRACE; i++)
            if(!strncasecmp(level.c_str(), (const char*)(get_log_level_cstr() + i),
                            strlen((const char*)get_log_level_cstr() + i)))
                return static_cast<log_level>(i);
        Log<T>().Get(WARN) << "Unknown logging level '" << level << "'. Using INFO level as default.";
        return INFO;
    }
    /**
     * get the reference to the flag indicating if the current host time should be part of the log
     *
     * @return the print time flag
     */
    static std::atomic<bool>& print_time() {
        static std::atomic<bool> flag{true};
        return flag;
    }
    /**
     * get the reference to the flag indicating if severity should be part of the log
     *
     * @return the print severity flag
     */
    static std::atomic<bool>& print_severity() {
        static std::atomic<bool> flag{true};
        return flag;
    }

protected:
    std::atomic<log_level>& get_last_log_level() {
        static std::atomic<log_level> level{TRACE};
        return level;
    }
    static const char* const* get_log_level_cstr() { return buffer.data(); };
    std::ostringstream os;
};
/**
 * the output writer
 */
template <typename CATEGORY> class Output2FILE : CATEGORY {
public:
    /**
     * get the file handle of the underlying output file (or stdout)
     *
     * @return the file handle
     */
    static std::atomic<FILE*>& stream() {
        static std::atomic<FILE*> pStream{stdout};
        return pStream;
    }

    static std::atomic<std::ostream*>& ostream() {
        static std::atomic<std::ostream*> oStream{nullptr};
        return oStream;
    }
    /**
     * write an output string to the file
     *
     * @param msg the string to write
     */
    static void output(const std::string& msg) {
        static std::mutex mtx;
        std::lock_guard<std::mutex> lock(mtx);
        std::ostream* ostr = ostream();
        if(ostr) {
            *ostr << msg;
        } else {
            FILE* pStream = stream();
            if(pStream) {
                fprintf(pStream, "%s", msg.c_str());
                fflush(pStream);
            }
        }
    }
};
//! the default logging category
class DEFAULT {};

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
#if defined(BUILDING_FILELOG_DLL)
#define FILELOG_DECLSPEC __declspec(dllexport)
#elif defined(USING_FILELOG_DLL)
#define FILELOG_DECLSPEC __declspec(dllimport)
#else
#define FILELOG_DECLSPEC
#endif // BUILDING_DBSIMPLE_DLL
#else
#define FILELOG_DECLSPEC
#endif // _WIN32

#ifndef FILELOG_MAX_LEVEL
#define FILELOG_MAX_LEVEL ::logging::TRACE
#endif

#define LOGGER(CATEGORY) ::logging::Log<::logging::Output2FILE<::logging::CATEGORY>>
#define LOG_OUTPUT(CATEGORY) ::logging::Output2FILE<::logging::CATEGORY>

#ifndef LOG
#define LOG(LEVEL)                                                                                                     \
    if(::logging::LEVEL <= LOGGER(DEFAULT)::get_reporting_level() && LOG_OUTPUT(DEFAULT)::stream())                    \
    LOGGER(DEFAULT)().get(::logging::LEVEL)
#endif
#ifndef CLOG
#define CLOG(LEVEL, CATEGORY)                                                                                          \
    if(::logging::LEVEL <= LOGGER(CATEGORY)::get_reporting_level() && LOG_OUTPUT(CATEGORY)::stream())                  \
    LOGGER(CATEGORY)().get(::logging::LEVEL, #CATEGORY)
#endif
#if defined(WIN32)

#include <array>
#include <windows.h>

inline std::string now_time() {
    const int MAX_LEN = 200;
    char buffer[MAX_LEN];
    if(GetTimeFormatA(LOCALE_USER_DEFAULT, 0, 0, "HH':'mm':'ss", buffer, MAX_LEN) == 0)
        return "Error in now_time()";
    char result[100] = {0};
    static DWORD first = GetTickCount();
    std::sprintf(result, "%s.%03ld", buffer, (long)(GetTickCount() - first) % 1000);
    return result;
}

#else

inline std::string now_time() {
    static std::mutex mtx;
    static std::array<char, 11> buffer;
    static std::array<char, 100> result;
    std::lock_guard<std::mutex> lck(mtx);
    time_t t;
    time(&t);
    tm r{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, nullptr};
    strftime(buffer.data(), buffer.size(), "%X", localtime_r(&t, &r));
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    memset(result.data(), 100, 1);
    sprintf(result.data(), "%s.%03ld", buffer.data(), (long)tv.tv_usec / 1000);
    return result.data();
}

#endif // WIN32
//! a print function for a vector
template <typename T> std::ostream& operator<<(std::ostream& stream, const std::vector<T>& vector) {
    copy(vector.begin(), vector.end(), std::ostream_iterator<T>(stream, ","));
    return stream;
}
} // namespace logging
/** @} */
#undef LEVELS
#undef CAT

#ifndef NDEBUG
//! check only in debug mode
#define SCCASSERT(condition, message)                                                                                  \
    do {                                                                                                               \
        if(!(condition)) {                                                                                             \
            logging::Logger().Get(logging::fatal) << "Assertion `" #condition "` failed in " << __FILE__ << " line "   \
                                                  << __LINE__ << ": " << message << std::endl;                         \
            std::terminate();                                                                                          \
        }                                                                                                              \
    } while(false)
#else
#define SCCASSERT(condition, message)                                                                                  \
    do {                                                                                                               \
    } while(false)
#endif
//! check always
#define CHECK(condition, message)                                                                                      \
    do {                                                                                                               \
        if(!(condition)) {                                                                                             \
            logging::Logger().Get(logging::fatal) << "Check of `" #condition "` failed in " << __FILE__ << " line "    \
                                                  << __LINE__ << ": " << message << std::endl;                         \
            std::terminate();                                                                                          \
        }                                                                                                              \
    } while(false)
