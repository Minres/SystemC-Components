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
/*
 * report.cpp
 *
 *  Created on: 19.09.2017
 *      Author: ubuntu
 */

#include <scc/report.h>
#include <spdlog/spdlog.h>
#include <spdlog/async.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <tuple>
#include <array>

using namespace std;
using namespace sc_core;
using namespace logging;

namespace {
struct ExtLogConfig: public scc::LogConfig {
  shared_ptr<spdlog::logger> file_logger;
  shared_ptr<spdlog::logger> console_logger;
  ExtLogConfig& operator=(const scc::LogConfig& o){
    scc::LogConfig::operator=(o);
    return *this;
  }
} log_cfg;

tuple<sc_core::sc_time::value_type, sc_core::sc_time_unit> get_tuple(const sc_core::sc_time& t){
    auto val = t.value();
    auto tr  = (uint64_t)(sc_core::sc_time::from_value(1).to_seconds()*1E15);
    auto scale = 0U;
    while( ( tr % 10 ) == 0 ) {
        tr /= 10;
        scale++;
    }
    sc_assert( tr == 1 );

    auto tu = scale / 3;
    while( tu < SC_SEC && ( val % 10 ) == 0 ) {
        val /= 10;
        scale++;
        tu += ( 0 == ( scale % 3 ) );
    }
    for( scale %= 3; scale != 0 ; scale-- )
        val *= 10;
    return make_tuple(val, static_cast<sc_time_unit>( tu ));
}

string time2string(const sc_core::sc_time &t) {
    const array<const char *, 6> time_units{"fs", "ps", "ns", "us", "ms", "s "};
    const array<uint64_t, 6> multiplier{1ULL,
                                        1000ULL,
                                        1000ULL * 1000,
                                        1000ULL * 1000 * 1000,
                                        1000ULL * 1000 * 1000 * 1000,
                                        1000ULL * 1000 * 1000 * 1000 * 1000};
    ostringstream oss;
    if (!t.value()) {
        oss << "0 s";
    } else {
        const auto tt = get_tuple(t);
        const auto val = get<0>(tt);
        const auto scale = get<1>(tt);
        const auto fs_val = val * multiplier[scale];
        for (int j = multiplier.size() - 1; j >= scale; --j) {
            if (fs_val >= multiplier[j]) {
                const auto i = val / multiplier[j - scale];
                const auto f = val % multiplier[j - scale];
                oss << i << '.' << setw(3 * (j - scale)) << setfill('0') << right << f << ' '
                    << time_units[j];
                break;
            }
        }
    }
    return oss.str();
}

const string compose_message(const sc_report &rep) {
    stringstream os;
    if(log_cfg.print_sim_time) os << "[" << setw(20) << time2string(sc_core::sc_time_stamp()) << "] ";
    if (rep.get_id() >= 0)
        os << "(" << "IWEF"[rep.get_severity()] << rep.get_id() << ") "<<rep.get_msg_type();
    else if(log_cfg.msg_type_field_width)
      os << util::padded(rep.get_msg_type(), log_cfg.msg_type_field_width) << ": ";
    if (*rep.get_msg()) os  << rep.get_msg();
    if (rep.get_severity() > SC_INFO) {
        os << "\n         [FILE:" << rep.get_file_name() << ":" << rep.get_line_number() << "]";
        sc_simcontext *simc = sc_get_curr_simcontext();
        if (simc && sc_is_running()) {
            const char *proc_name = rep.get_process_name();
            if (proc_name) os << "\n         [PROCESS:" << proc_name << "]";
        }
    }
    return os.str();
}

inline log_level verbosity2log(int verb) {
    if (verb >= sc_core::SC_FULL) return TRACE;
    if (verb >= sc_core::SC_HIGH) return DEBUG;
    return INFO;
}

inline void log2logger(spdlog::logger& logger, const sc_report &rep){
  switch(rep.get_severity()){
  case SC_INFO:
    switch(rep.get_verbosity()){
    case sc_core::SC_DEBUG:
    case sc_core::SC_FULL:
      logger.trace(compose_message(rep));
      return;
    case sc_core::SC_HIGH:
      logger.debug(compose_message(rep));
      return;
    default:
      logger.info(compose_message(rep));
    }
    return;
  case SC_WARNING: logger.warn(compose_message(rep));return;
  case SC_ERROR:   logger.error(compose_message(rep));return;
  case SC_FATAL:   logger.critical(compose_message(rep));return;
  }
}
inline void log2logger(spdlog::logger& logger, logging::log_level lvl, const string& msg){
  switch(lvl){
  case logging::DBGTRACE:
  case logging::TRACE:   logger.trace(msg); return;
  case logging::DEBUG:   logger.debug(msg); return;
  case logging::INFO:    logger.info(msg); return;
  case logging::WARNING: logger.warn(msg); return;
  case logging::ERROR:   logger.error(msg); return;
  case logging::FATAL:   logger.critical(msg); return;
  }

}

void report_handler(const sc_report &rep, const sc_actions &actions) {
    if (actions & SC_DISPLAY) log2logger(*log_cfg.console_logger, rep);
    if ((actions & SC_LOG) && log_cfg.file_logger) log2logger(*log_cfg.file_logger, rep);
    if (actions & SC_STOP) sc_stop();
    if (actions & SC_ABORT) abort();
    if (actions & SC_THROW) throw rep;
}
}

scc::stream_redirection::stream_redirection(ostream& os, logging::log_level level):os(os), level(level){
    old_buf=os.rdbuf(this); //save and redirect
}

scc::stream_redirection::~stream_redirection(){
    os.rdbuf(old_buf); // restore
}

void scc::stream_redirection::reset(){
    os.rdbuf(old_buf); // restore
    old_buf=nullptr;
}

streamsize scc::stream_redirection::xsputn(const char_type* s, streamsize n) {
    auto sz = stringbuf::xsputn(s, n);
    if (s[n-1]=='\n'){
      sync();
    }
    return sz;
}

int scc::stream_redirection::sync(){
  if(level <= log_cfg.level){
      auto timestr = time2string(sc_core::sc_time_stamp());
      istringstream buf(str());
      string line;
      while(getline(buf, line)) {
        stringstream os;
        if(log_cfg.print_sim_time) os << "[" << setw(20) << time2string(sc_core::sc_time_stamp()) << "] ";
        os  << line;
        log2logger(*log_cfg.console_logger, level, os.str());
        if (log_cfg.file_logger) log2logger(*log_cfg.file_logger, level, os.str());
      }
      str(string(""));
  }
  return 0; //Success
}

static void configure_logging() {
    const array<int, 8> verbosity = { SC_NONE,   // Logging::NONE
                                      SC_LOW,    // Logging::FATAL
                                      SC_LOW,    // Logging::ERROR
                                      SC_LOW,    // Logging::WARNING
                                      SC_MEDIUM, // Logging::INFO
                                      SC_HIGH,   // logging::DEBUG
                                      SC_FULL,   // logging::TRACE
                                      SC_DEBUG}; // logging::DBGTRACE
    sc_report_handler::set_actions(SC_ERROR, SC_DEFAULT_ERROR_ACTIONS | SC_DISPLAY);
    sc_report_handler::set_actions(SC_FATAL, SC_DEFAULT_FATAL_ACTIONS);
    sc_report_handler::set_verbosity_level(verbosity[log_cfg.level]);
    sc_report_handler::set_handler(report_handler);
    spdlog::init_thread_pool(8192U, 1U); // queue with 8k items and 1 backing thread.
    log_cfg.console_logger = spdlog::stdout_color_mt<spdlog::async_factory>("console_logger");
    log_cfg.console_logger->set_pattern("[%L] %v");
    log_cfg.console_logger->set_level(
        static_cast<spdlog::level::level_enum>(SPDLOG_LEVEL_OFF - min<int>(SPDLOG_LEVEL_OFF, log_cfg.level)));
    if(log_cfg.log_file_name.size()){
      {
        ofstream ofs;
        ofs.open (log_cfg.log_file_name, ios::out | ios::trunc);
      }
      log_cfg.file_logger = spdlog::basic_logger_mt<spdlog::async_factory>("file_logger", log_cfg.log_file_name);
      log_cfg.file_logger->set_pattern("[%8l] %v");
      log_cfg.file_logger->set_level(
          static_cast<spdlog::level::level_enum>(SPDLOG_LEVEL_OFF - min<int>(SPDLOG_LEVEL_OFF, log_cfg.level)));
    }
}

void scc::init_logging(logging::log_level level, unsigned type_field_width, bool print_time) {
  log_cfg.msg_type_field_width = type_field_width;
  log_cfg.print_sys_time = print_time;
  log_cfg.level = level;
  configure_logging();
}

void scc::init_logging(const scc::LogConfig& log_config){
  log_cfg=log_config;
  configure_logging();
}

scc::LogConfig& scc::LogConfig::setLogLevel(logging::log_level log_level) {
  this->level=log_level;
  return *this;
}

scc::LogConfig& scc::LogConfig::setFieldWidth(unsigned width) {
  this->msg_type_field_width=width;
  return *this;
}

scc::LogConfig& scc::LogConfig::setPrintSysTime(bool enable) {
  this->print_sys_time=enable;
  return *this;
}

scc::LogConfig& scc::LogConfig::setPrintSimTime(bool enable) {
  this->print_sim_time=enable;
  return *this;
}

scc::LogConfig& scc::LogConfig::setPrintSeverity(bool enable) {
  this->print_severity=enable;
  return *this;
}

scc::LogConfig& scc::LogConfig::setLogFileName(string&& name) {
  this->log_file_name=name;
  return *this;
}
