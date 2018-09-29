/*******************************************************************************
 * Copyright 2017 MINRES Technologies GmbH
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

#ifndef _UTIL_LOGGING_H_
#define _UTIL_LOGGING_H_

#include <iomanip>
#include <cassert>
#include <cstdio>
#include <cstring>
#include <iterator>
#include <mutex>
#include <sstream>
#include <string>
#include <vector>
#include <sys/time.h>

#define LEVELS(L) L(NONE) L(FATAL) L(ERROR) L(WARNING) L(INFO) L(DEBUG) L(TRACE)
#define DO_DESCRIPTION(e) #e,
#define DO_ENUM(e) e,

namespace logging {

static std::array<const char*const ,7> buffer = { {LEVELS(DO_DESCRIPTION)} };
enum log_level { LEVELS(DO_ENUM) };

inline log_level as_log_level(int logLevel) {
    assert(logLevel >= NONE && logLevel <= TRACE);
	std::array<const log_level, 7> m = { { NONE, FATAL, ERROR, WARNING, INFO, DEBUG, TRACE } };
    return m[logLevel];
}

inline std::string now_time();

template <typename T> class Log {
public:
    Log() = default;

    Log(const Log &) = delete;

    Log &operator=(const Log &) = delete;

    virtual ~Log() {
        os << std::endl;
        T::output(os.str());
        // TODO: use a more specific exception
        if (get_last_log_level() == FATAL) abort();
    }

    std::ostringstream &get(log_level level = INFO, const char *category = "") {
        if (print_time()) os << "- " << now_time()<< " ";
        if (print_severity()) {
            os  << std::setw(7)<<std::left<<to_string(level);
            if (strlen(category)) os << "[" << category<<"]";
            os << ": ";
        }
        get_last_log_level() = level;
        return os;
    };

    static log_level &reporting_level() {
        static log_level reportingLevel = WARNING;
        return reportingLevel;
    }

    static std::string to_string(log_level level) { return std::string(get_log_level_cstr()[level]); };

    static log_level from_string(const std::string &level) {
        for (unsigned int i = NONE; i <= TRACE; i++)
            if (!strncasecmp(level.c_str(), (const char *)(get_log_level_cstr() + i),
                             strlen((const char *)get_log_level_cstr() + i)))
                return i;
        Log<T>().Get(WARNING) << "Unknown logging level '" << level << "'. Using INFO level as default.";
        return INFO;
    }

    static bool &print_time() {
        static bool flag = true;
        return flag;
    }

    static bool &print_severity() {
        static bool flag = true;
        return flag;
    }

protected:
    log_level &get_last_log_level() {
        static log_level level = TRACE;
        return level;
    }
	static const char * const *get_log_level_cstr() {
		return buffer.data();
	}
	;
    std::ostringstream os;
};

template <typename CATEGORY> class Output2FILE : CATEGORY {
public:
    static FILE *&stream() {
        static FILE *pStream = stdout;
        return pStream;
    }
    static void output(const std::string &msg) {
        static std::mutex mtx;
        std::lock_guard<std::mutex> lock(mtx);
        FILE *pStream = stream();
        if (!pStream) return;
        fprintf(pStream, "%s", msg.c_str());
        fflush(pStream);
    }
};
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
#define FILELOG_MAX_LEVEL logging::TRACE
#endif

#define LOGGER(CATEGORY) logging::Log<logging::Output2FILE<logging::CATEGORY>>
#define LOG_OUTPUT(CATEGORY) logging::Output2FILE<logging::CATEGORY>

#ifndef LOG
#define LOG(LEVEL)                                                                                                     \
    if (logging::LEVEL <= LOGGER(DEFAULT)::reporting_level() && LOG_OUTPUT(DEFAULT)::stream())                         \
    LOGGER(DEFAULT)().get(logging::LEVEL)
#endif
#ifndef CLOG
#define CLOG(LEVEL, CATEGORY)                                                                                          \
    if (logging::LEVEL <= LOGGER(CATEGORY)::reporting_level() && LOG_OUTPUT(CATEGORY)::stream())                       \
    LOGGER(CATEGORY)().get(logging::LEVEL, #CATEGORY)
#endif
#if defined(WIN32)

#include <windows.h>
#include <array>

inline std::string now_time() {
    const int MAX_LEN = 200;
    char buffer[MAX_LEN];
    if (GetTimeFormatA(LOCALE_USER_DEFAULT, 0, 0, "HH':'mm':'ss", buffer, MAX_LEN) == 0) return "Error in now_time()";
    char result[100] = {0};
    static DWORD first = GetTickCount();
    std::sprintf(result, "%s.%03ld", buffer, (long)(GetTickCount() - first) % 1000);
    return result;
}

#else

inline std::string now_time() {
	static std::array<char, 11> buffer;
	static std::array<char, 100> result;
    time_t t;
    time(&t);
    tm r = {0};
	strftime(buffer.data(), buffer.size(), "%X", localtime_r(&t, &r));
    struct timeval tv;
    gettimeofday(&tv, nullptr);
	sprintf(result.data(), "%s.%03ld", buffer.data(), (long) tv.tv_usec / 1000);
	return result.data();
}

#endif // WIN32
// a print function for a vector
template <typename T> std::ostream &operator<<(std::ostream &stream, const std::vector<T> &vector) {
    copy(vector.begin(), vector.end(), std::ostream_iterator<T>(stream, ","));
    return stream;
}

} // namespace
#undef LEVELS
#undef CAT

#ifndef NDEBUG
#define ASSERT(condition, message)                                                                                     \
    do {                                                                                                               \
        if (!(condition)) {                                                                                            \
            logging::Logger().Get(logging::fatal) << "Assertion `" #condition "` failed in " << __FILE__ << " line "   \
                                                  << __LINE__ << ": " << message << std::endl;                         \
            std::terminate();                                                                                          \
        }                                                                                                              \
    } while (false)
#else
#define ASSERT(condition, message)                                                                                     \
    do {                                                                                                               \
    } while (false)
#endif

#define CHECK(condition, message)                                                                                      \
    do {                                                                                                               \
        if (!(condition)) {                                                                                            \
            logging::Logger().Get(logging::fatal) << "Check of `" #condition "` failed in " << __FILE__ << " line "    \
                                                  << __LINE__ << ": " << message << std::endl;                         \
            std::terminate();                                                                                          \
        }                                                                                                              \
    } while (false)

#endif /* _UTIL_LOGGING_H_ */
