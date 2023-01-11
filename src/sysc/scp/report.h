/*******************************************************************************
 * Copyright 2016-2022 MINRES Technologies GmbH
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

#ifndef _SCP_REPORT_H_
#define _SCP_REPORT_H_

#include <scc/report.h>

//! the name of the CCI property to attach to modules to control logging of
//! this module
#define SCP_LOG_LEVEL_PARAM_NAME "log_level"

namespace scp {
	using log = ::scc::log;
	using LogConfig=::scc::LogConfig;
	void init_logging(log level = log::WARNING, unsigned type_field_width = 24, bool print_time = false) {
		scc::init_logging(level, type_field_width, print_time);
	}
	void reinit_logging(log level){ scc::reinit_logging(level);}
	void init_logging(const LogConfig& log_config) { scc::init_logging(log_config);}
	void set_logging_level(log level){scc::set_logging_level(level);}
	log get_logging_level(){return scc::get_logging_level();}
	void set_cycle_base(sc_core::sc_time period){scc::set_cycle_base(period);}
	inline sc_core::sc_verbosity get_log_verbosity() {
	    return static_cast<sc_core::sc_verbosity>(::sc_core::sc_report_handler::get_verbosity_level());
	}
	sc_core::sc_verbosity get_log_verbosity(char const* t) { return scc::get_log_verbosity(t); }
	inline sc_core::sc_verbosity get_log_verbosity(std::string const& t) { return scc::get_log_verbosity(t.c_str()); }
}

/**
 * logging macros
 */
//! macro for log output
#define SCP_LOG(lvl, ...)                                             \
    ::scc::ScLogger<::sc_core::SC_INFO>(__FILE__, __LINE__, lvl / 10) \
        .type(__VA_ARGS__)                                            \
        .get()
//! macro for debug trace level output
#define SCP_TRACEALL(...)                                           \
    if (::scc::get_log_verbosity(__VA_ARGS__) >= sc_core::SC_DEBUG) \
    SCP_LOG(sc_core::SC_DEBUG, __VA_ARGS__)
//! macro for trace level output
#define SCP_TRACE(...)                                             \
    if (::scc::get_log_verbosity(__VA_ARGS__) >= sc_core::SC_FULL) \
    SCP_LOG(sc_core::SC_FULL, __VA_ARGS__)
//! macro for debug level output
#define SCP_DEBUG(...)                                             \
    if (::scc::get_log_verbosity(__VA_ARGS__) >= sc_core::SC_HIGH) \
    SCP_LOG(sc_core::SC_HIGH, __VA_ARGS__)
//! macro for info level output
#define SCP_INFO(...)                                                \
    if (::scc::get_log_verbosity(__VA_ARGS__) >= sc_core::SC_MEDIUM) \
    SCP_LOG(sc_core::SC_MEDIUM, __VA_ARGS__)
//! macro for warning level output
#define SCP_WARN(...)                                          \
    ::scc::ScLogger<::sc_core::SC_WARNING>(__FILE__, __LINE__, \
                                           sc_core::SC_MEDIUM) \
        .type(__VA_ARGS__)                                     \
        .get()
//! macro for error level output
#define SCP_ERR(...)                                         \
    ::scc::ScLogger<::sc_core::SC_ERROR>(__FILE__, __LINE__, \
                                         sc_core::SC_MEDIUM) \
        .type(__VA_ARGS__)                                   \
        .get()
//! macro for fatal message output
#define SCP_FATAL(...)                                       \
    ::scc::ScLogger<::sc_core::SC_FATAL>(__FILE__, __LINE__, \
                                         sc_core::SC_MEDIUM) \
        .type(__VA_ARGS__)                                   \
        .get()

#ifdef NDEBUG
#define SCP_ASSERT(expr) ((void)0)
#else
#define SCP_ASSERT(expr)                                                  \
    ((void)((expr) ? 0                                                    \
                   : (SC_REPORT_FATAL(::sc_core::SC_ID_ASSERTION_FAILED_, \
                                      #expr),                             \
                      0)))
#endif
/** @} */ // end of scp-report
#endif    /* _SCP_REPORT_H_ */
