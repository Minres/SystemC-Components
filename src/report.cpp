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
#include <tuple>
#include <array>

using namespace sc_core;
using namespace logging;

namespace {
std::tuple<sc_core::sc_time::value_type, sc_core::sc_time_unit> get_tuple(const sc_core::sc_time& t){
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
    return std::make_tuple(val, static_cast<sc_time_unit>( tu ));
}

std::string time2string(const sc_core::sc_time &t) {
    const std::array<const char *, 6> time_units{"fs", "ps", "ns", "us", "ms", "s "};
    const std::array<uint64_t, 6> multiplier{1ULL,
                                             1000ULL,
                                             1000ULL * 1000,
                                             1000ULL * 1000 * 1000,
                                             1000ULL * 1000 * 1000 * 1000,
                                             1000ULL * 1000 * 1000 * 1000 * 1000};
    std::ostringstream oss;
    if (!t.value()) {
        oss << "0 s";
    } else {
        const auto tt = get_tuple(t);
        const auto val = std::get<0>(tt);
        const auto scale = std::get<1>(tt);
        const auto fs_val = val * multiplier[scale];
        for (int j = multiplier.size() - 1; j >= scale; --j) {
            if (fs_val >= multiplier[j]) {
                const auto i = val / multiplier[j - scale];
                const auto f = val % multiplier[j - scale];
                oss << i << '.' << std::setw(3 * (j - scale)) << std::setfill('0') << std::left << f << ' '
                    << time_units[j];
                break;
            }
        }
    }
    return oss.str();
}

const std::string compose_message(const sc_report &rep) {
    std::stringstream os;
    os << "[" << std::setw(20) << time2string(sc_core::sc_time_stamp()) << "] ";
    if (rep.get_id() >= 0)
        os << "("
           << "IWEF"[rep.get_severity()] << rep.get_id() << ") ";
    os << rep.get_msg_type();
    if (*rep.get_msg()) os << ": " << rep.get_msg();
    if (rep.get_severity() > SC_INFO) {
        std::array<char, 16> line_number_str;
        os << " [FILE:" << rep.get_file_name() << ":" << rep.get_line_number() << "]";
        sc_simcontext *simc = sc_get_curr_simcontext();
        if (simc && sc_is_running()) {
            const char *proc_name = rep.get_process_name();
            if (proc_name) os << "[PROCESS:" << proc_name << "]";
        }
    }
    return os.str();
}

inline log_level verbosity2log(int verb) {
    if (verb >= sc_core::SC_FULL) return TRACE;
    if (verb >= sc_core::SC_HIGH) return DEBUG;
    return INFO;
}

void report_handler(const sc_report &rep, const sc_actions &actions) {
    std::array<const log_level, 4> map = {{INFO, WARNING, ERROR, FATAL}};
    if (actions & SC_DISPLAY) {
        auto level =
            rep.get_severity() > sc_core::SC_INFO ? map[rep.get_severity()] : verbosity2log(rep.get_verbosity());
        if (level <= Log<Output2FILE<SystemC>>::reporting_level() && Output2FILE<SystemC>::stream())
            Log<Output2FILE<SystemC>>().get(level, "") << compose_message(rep);
    }
    if (actions & SC_STOP) sc_stop();
    if (actions & SC_ABORT) abort();
    if (actions & SC_THROW) throw rep;
}
}

scc::stream_redirection::stream_redirection(std::ostream& os, logging::log_level level):os(os), level(level){
    old_buf=os.rdbuf(this); //save and redirect
}

scc::stream_redirection::~stream_redirection(){
    os.rdbuf(old_buf); // restore
}

void scc::stream_redirection::reset(){
    os.rdbuf(old_buf); // restore
    old_buf=nullptr;
}

std::streamsize scc::stream_redirection::xsputn(const char_type* s, std::streamsize n) {
    auto sz = std::stringbuf::xsputn(s, n);
    if (s[n-1]=='\n'){
        if(logging::INFO <= Log<Output2FILE<STDIO>>::reporting_level() && Output2FILE<STDIO>::stream()){
            auto timestr = time2string(sc_core::sc_time_stamp());
            std::istringstream buf(str());
            std::string line;
            while(getline(buf, line)) {
                Log<Output2FILE<STDIO>>().get(logging::INFO, "") << "[" << std::setw(20) << timestr << "] "<<line;
            }
        }
        str(std::string(""));
    }
    return sz;
}

void scc::init_logging(logging::log_level level, bool print_time) {
    const std::array<int, 8> verbosity = {SC_NONE,   // Logging::NONE
                                          SC_LOW,    // Logging::FATAL
                                          SC_LOW,    // Logging::ERROR
                                          SC_LOW,    // Logging::WARNING
                                          SC_MEDIUM, // Logging::INFO
                                          SC_HIGH,   // logging::DEBUG
                                          SC_FULL,   // logging::TRACE
                                          SC_DEBUG}; // logging::DBGTRACE
    LOGGER(DEFAULT)::reporting_level() = level;
    LOGGER(SystemC)::reporting_level() = level;
    LOGGER(SystemC)::print_time() = print_time;
    LOGGER(SystemC)::abort_on_fatal() = false;
    sc_report_handler::set_actions(SC_ERROR, SC_DEFAULT_ERROR_ACTIONS | SC_DISPLAY);
    sc_report_handler::set_actions(SC_FATAL, SC_DEFAULT_FATAL_ACTIONS);
    sc_report_handler::set_verbosity_level(verbosity[level]);
    sc_report_handler::set_handler(report_handler);
    LOGGER(STDIO)::reporting_level() = level;
    LOGGER(STDIO)::print_time() = print_time;
    LOGGER(STDIO)::abort_on_fatal() = false;
}

