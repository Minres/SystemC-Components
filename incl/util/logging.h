/*******************************************************************************
 * Copyright (C) 2017, MINRES Technologies GmbH
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 
 * 3. Neither the name of the copyright holder nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 * 
 * Contributors:
 *       eyck@minres.com - initial API and implementation
 ******************************************************************************/

#ifndef _UTIL_LOGGING_H_
#define _UTIL_LOGGING_H_

#include <sstream>
#include <string>
#include <vector>
#include <iterator>
#include <cstring>
#include <mutex>
#include <stdio.h>
#include <sys/time.h>

#define LEVELS(FOO) FOO(none) FOO(fatal) FOO(error) FOO(warning) FOO(info) FOO(debug) FOO(trace)
#define DO_DESCRIPTION(e)  #e,
#define DO_ENUM(e)  e,

namespace logging {

static const char * const buffer[] = { LEVELS(DO_DESCRIPTION)};
enum log_level { LEVELS(DO_ENUM) };

inline std::string now_time();

template <typename T>
class Log{
public:
	Log(){};
    virtual ~Log(){
		os << std::endl;
		T::output(os.str());
		//TODO: use a more specific exception
		if(get_last_log_level() == fatal) throw std::exception();
	};
    std::ostringstream& get(log_level level = info){
		os << "- " << now_time();
		os << " " << to_string(level) << ": ";
		get_last_log_level()=level;
		return os;
	};
public:
    static log_level& reporting_level(){
		static log_level reportingLevel = warning;
		return reportingLevel;
	}
    static std::string to_string(log_level level){
		return std::string(get_log_level_cstr()[level]);
	};
    static log_level from_string(const std::string& level) {
		for(unsigned int i=none; i<=trace; i++)
			if(!strncasecmp(level.c_str(), (const char*)(get_log_level_cstr()+i), strlen((const char*)get_log_level_cstr()+i))) return i;
		Log<T>().Get(warning) << "Unknown logging level '" << level << "'. Using INFO level as default.";
		return info;
	}

protected:
    log_level& get_last_log_level(){
		static log_level level = trace;
		return level;
	}
	static const char* const * get_log_level_cstr(){
		return buffer;
	};
    std::ostringstream os;
private:
    Log(const Log&);
    Log& operator =(const Log&);
};

struct Output2FILE {
    static FILE*& stream(){
		static FILE* pStream = stderr;
		return pStream;
	}
    static void output(const std::string& msg){
        static std::mutex mtx;
        std::lock_guard<std::mutex> lock(mtx);
		FILE* pStream = stream();
		if (!pStream) return;
		fprintf(pStream, "%s", msg.c_str());
		fflush(pStream);
	}
};

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
#   if defined (BUILDING_FILELOG_DLL)
#       define FILELOG_DECLSPEC   __declspec (dllexport)
#   elif defined (USING_FILELOG_DLL)
#       define FILELOG_DECLSPEC   __declspec (dllimport)
#   else
#       define FILELOG_DECLSPEC
#   endif // BUILDING_DBSIMPLE_DLL
#else
#   define FILELOG_DECLSPEC
#endif // _WIN32

class FILELOG_DECLSPEC Logger : public Log<Output2FILE> {};
//typedef Log<Output2FILE> Logger;

#ifndef FILELOG_MAX_LEVEL
#define FILELOG_MAX_LEVEL logging::trace
#endif

#define LOG(level) \
    if (level > FILELOG_MAX_LEVEL) ;\
    else if (level > logging::Logger::reporting_level() || !logging::Output2FILE::stream()) ; \
    else logging::Logger().get(level)

#if defined(WIN32)

#include <windows.h>

inline std::string now_time(){
    const int MAX_LEN = 200;
    char buffer[MAX_LEN];
    if (GetTimeFormatA(LOCALE_USER_DEFAULT, 0, 0, "HH':'mm':'ss", buffer, MAX_LEN) == 0)
        return "Error in now_time()";
    char result[100] = {0};
    static DWORD first = GetTickCount();
    std::sprintf(result, "%s.%03ld", buffer, (long)(GetTickCount() - first) % 1000);
    return result;
}

#else

inline std::string now_time(){
    char buffer[11];
    time_t t;
    time(&t);
    tm r = {0};
    strftime(buffer, sizeof(buffer), "%X", localtime_r(&t, &r));
    struct timeval tv;
    gettimeofday(&tv, 0);
    char result[100] = {0};
    sprintf(result, "%s.%03ld", buffer, (long)tv.tv_usec / 1000);
    return result;
}

#endif //WIN32
// a print function for a vector
template<typename  T>
std::ostream& operator<< (std::ostream& stream, const std::vector<T>& vector)  {
	copy(vector.begin(), vector.end(), std::ostream_iterator<T>(stream, ","));
	return stream;
}

} // namespace
#undef LEVELS
#undef CAT

#ifndef NDEBUG
#   define ASSERT(condition, message) \
    do { \
        if (! (condition)) { \
        	logging::Logger().Get(logging::fatal) << "Assertion `" #condition "` failed in " << __FILE__ << " line " << __LINE__ << ": " << message << std::endl; \
            std::terminate(); \
        } \
    } while (false)
#else
#   define ASSERT(condition, message) do { } while (false)
#endif

#define CHECK(condition, message) \
do { \
	if (! (condition)) { \
		logging::Logger().Get(logging::fatal) << "Check of `" #condition "` failed in " << __FILE__ << " line " << __LINE__ << ": " << message << std::endl; \
		std::terminate(); \
	} \
} while (false)

#endif /* _UTIL_LOGGING_H_ */
