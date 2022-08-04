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

#ifndef _SYSC_UTILITIES_H_
#define _SYSC_UTILITIES_H_

#include <array>
#include <limits>
#include <memory>
#include <cctype>

#if defined(__GNUG__)
// pragmas to disable the deprecated warnings for SystemC headers
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif
#include <systemc>
#if defined(__GNUG__)
#pragma GCC diagnostic pop
#endif

#ifdef HAS_CCI
#include <cci_cfg/cci_param_typed.h>
#endif

#include <locale>

/** \ingroup scc-sysc
 *  @{
 */
/**@{*/
//! @brief SCC SystemC utilities
namespace scc {
template <typename T, typename... Args> std::unique_ptr<T> make_unique(Args&&... args) {
#if __cplusplus < 201402L
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
#else
    return std::make_unique<T>(std::forward<Args>(args)...);
#endif
}
} // namespace scc
//! macros to simplify constructor lists
#define NAMED(X, ...) X(#X, ##__VA_ARGS__)
#define NAMEDD(X, T, ...) X(scc::make_unique<T>(#X, ##__VA_ARGS__))
#define NAMEDC(X, T, I, ...) X(T::create<I>(#X, ##__VA_ARGS__))
//! macros to simplify declaration of members to trace
#define TRACE_VAR(F, X) sc_core::sc_trace(F, X, std::string(this->name()) + "." #X)
#define TRACE_ARR(F, X, I)                                                                                             \
    sc_core::sc_trace(F, X[I], (std::string(this->name()) + "." #X "(" + std::to_string(I) + ")").c_str());
#define TRACE_SIG(F, X) sc_core::sc_trace(F, X, X.name())

namespace sc_core {
// needed to be able to use sc_time as signal value
#if SC_VERSION_MAJOR <= 2 && SC_VERSION_MINOR <= 3 && SC_VERSION_PATCH < 2
#define HAS_NO_TIME_TRACE
#endif
#ifdef NCSC
#define HAS_NO_TIME_TRACE
#endif

#ifdef HAS_NO_TIME_TRACE

void sc_trace(sc_trace_file*, const sc_time&, const std::string&);

/**
 * compatibility for SC2.3.1
 *
 */
inline void sc_trace(sc_core::sc_trace_file*&, const sc_core::sc_event&, const char*) {}

#endif
/**
 * @brief trace function for sc_time
 *
 * @param tf the trace file
 * @param value the data to trace
 * @param name the hierarchical name of the data
 */
template <> void sc_trace(sc_trace_file* tf, const sc_in<sc_time>& value, const std::string& name);
/**
 * trace function for sc_time
 *
 * @param tf the trace file
 * @param value the port carrying the data to trace
 * @param name the hierarchical name of the data
 */
template <> void sc_trace(sc_trace_file* tf, const sc_inout<sc_time>& value, const std::string& name);

} // namespace sc_core
// user-defined literals for easy time creation
/**
 * UDL for second
 *
 * @param val
 * @return
 */
inline sc_core::sc_time operator"" _sec(long double val) { return sc_core::sc_time(val, sc_core::SC_SEC); }
/**
 * UDL for second
 *
 * @param val
 * @return
 */
inline sc_core::sc_time operator"" _sec(unsigned long long val) {
    return sc_core::sc_time(double(val), sc_core::SC_SEC);
}
/**
 * UDL for millisecond
 *
 * @param val
 * @return
 */
inline sc_core::sc_time operator"" _ms(long double val) { return sc_core::sc_time(val, sc_core::SC_MS); }
/**
 * UDL for millisecond
 *
 * @param val
 * @return
 */
inline sc_core::sc_time operator"" _ms(unsigned long long val) { return sc_core::sc_time(double(val), sc_core::SC_MS); }
/**
 * UDL for microsecond
 *
 * @param val
 * @return
 */
inline sc_core::sc_time operator"" _us(long double val) { return sc_core::sc_time(val, sc_core::SC_US); }
/**
 * UDL for microsecond
 *
 * @param val
 * @return
 */
inline sc_core::sc_time operator"" _us(unsigned long long val) { return sc_core::sc_time(double(val), sc_core::SC_US); }
/**
 * UDL for nanosecond
 *
 * @param val
 * @return
 */
inline sc_core::sc_time operator"" _ns(long double val) { return sc_core::sc_time(val, sc_core::SC_NS); }
/**
 * UDL for nanosecond
 *
 * @param val
 * @return
 */
inline sc_core::sc_time operator"" _ns(unsigned long long val) { return sc_core::sc_time(double(val), sc_core::SC_NS); }
/**
 * UDL for picosecond
 *
 * @param val
 * @return
 */
inline sc_core::sc_time operator"" _ps(long double val) { return sc_core::sc_time(val, sc_core::SC_PS); }
/**
 * UDL for picosecond
 *
 * @param val
 * @return
 */
inline sc_core::sc_time operator"" _ps(unsigned long long val) { return sc_core::sc_time(double(val), sc_core::SC_PS); }
/**
 * UDL for femtosecond
 *
 * @param val
 * @return
 */
inline sc_core::sc_time operator"" _fs(long double val) { return sc_core::sc_time(val, sc_core::SC_FS); }
/**
 * UDL for femtosecond
 *
 * @param val
 * @return
 */
inline sc_core::sc_time operator"" _fs(unsigned long long val) { return sc_core::sc_time(double(val), sc_core::SC_FS); }
//! UDL for kilobyte
inline constexpr uint64_t operator"" _kB(unsigned long long val) { return val * 1 << 10; }
//! UDL for megabyte
inline constexpr uint64_t operator"" _MB(unsigned long long val) { return val * 1 << 20; }
//! UDL for gigabyte
inline constexpr uint64_t operator"" _GB(unsigned long long val) { return val * 1 << 30; }

namespace scc {
inline char* legalize_name(char* const name) {
    char* ptr = name;
    while(*ptr) {
        if(*ptr == '.' || std::isspace(*ptr)) {
            *ptr = '_';
        }
        ptr++;
    }
    return name;
}
inline std::string legalize_name(std::string const& name) {
    std::string ret;
    for(auto c : name) {
        ret += (c == '.' || std::isspace(c)) ? '_' : c;
    }
    return ret;
}
/**
 * case-insensitive string compare
 *
 * @param a string a
 * @param b string b
 * @return result of std::equal
 */
inline bool icompare(std::string const& a, std::string const& b) {
    if(a.length() == b.length()) {
        return std::equal(b.begin(), b.end(), a.begin(),
                          [](unsigned char a, unsigned char b) -> bool { return std::tolower(a) == std::tolower(b); });
    } else {
        return false;
    }
}
/**
 * parse a time value from given strings
 *
 * @param value the string to parse
 * @param unit the unit string
 * @return the parsed sc_core::sc_time value
 */
inline sc_core::sc_time parse_from_string(std::string value, std::string unit) noexcept {
    std::string::size_type offs{0};
    double t_val = std::stod(value, &offs);
    if(offs > 0) {
        if(icompare(unit, "fs"))
            return t_val * 1_fs;
        if(icompare(unit, "ps"))
            return t_val * 1_ps;
        if(icompare(unit, "ns"))
            return t_val * 1_ns;
        if(icompare(unit, "us"))
            return t_val * 1_us;
        if(icompare(unit, "ms"))
            return t_val * 1_ms;
        if(icompare(unit, "s"))
            return t_val * 1_sec;
    }
    return sc_core::SC_ZERO_TIME;
}
/**
 * parse a time value from a given string
 *
 * @param value the string to parse
 * @return the parsed sc_core::sc_time value
 */
inline sc_core::sc_time parse_from_string(std::string value) noexcept {
    std::string::size_type offs{0};
    double t_val = std::stod(value, &offs);
    if(offs > 0) {
        std::string unit = value.substr(offs);
        if(icompare(unit, "fs"))
            return t_val * 1_fs;
        if(icompare(unit, "ps"))
            return t_val * 1_ps;
        if(icompare(unit, "ns"))
            return t_val * 1_ns;
        if(icompare(unit, "us"))
            return t_val * 1_us;
        if(icompare(unit, "ms"))
            return t_val * 1_ms;
        if(icompare(unit, "s"))
            return t_val * 1_sec;
    }
    return sc_core::SC_ZERO_TIME;
}

inline sc_core::sc_time time_to_next_posedge(sc_core::sc_clock const* clk) {
    auto period = clk->period();
    auto m_posedge_time = period - period * clk->duty_cycle();
    auto clk_run_time = sc_core::sc_time_stamp() - clk->start_time();
    auto clk_period_point = clk_run_time % period;
    if(clk_period_point == sc_core::SC_ZERO_TIME)
        clk_period_point = period;
    if(clk->posedge_first())
        return clk_period_point;
    else if(clk_period_point < m_posedge_time) {
        return m_posedge_time - clk_period_point;
    } else {
        return period + m_posedge_time - clk_period_point;
    }
}

#if __cplusplus < 201402L
inline unsigned ilog2(uint32_t val) {
#else
inline constexpr unsigned ilog2(uint32_t val) {
#endif
#ifdef __GNUG__
    return sizeof(uint32_t) * 8 - 1 - __builtin_clz(static_cast<unsigned>(val));
#else

    if(val == 0)
        return std::numeric_limits<uint32_t>::max();
    if(val == 1)
        return 0;
    auto ret = 0U;
    while(val > 1) {
        val >>= 1;
        ++ret;
    }
    return ret;
#endif
}

template <typename T> inline T get_value(sc_core::sc_attribute<T>& a) { return a.value; }

template <typename T> inline void set_value(sc_core::sc_attribute<T>& a, T&& value) { a.value = value; }

#ifdef HAS_CCI
template <typename T> inline T get_value(cci::cci_param_typed<T>& a) { return a.get_value(); }

template <typename T> inline void set_value(cci::cci_param_typed<T>& a, T&& value) { a.set_value(value); }
#endif
} // namespace scc
/** @} */ // end of scc-sysc

#define declare_method_process_cl(handle, name, host_tag, func)                                                        \
    {                                                                                                                  \
        ::sc_core::sc_spawn_options opt;                                                                               \
        opt.dont_initialize();                                                                                         \
        opt.spawn_method();                                                                                            \
        ::sc_core::sc_process_handle handle = ::sc_core::sc_spawn(func, name, &opt);                                   \
        this->sensitive << handle;                                                                                     \
        this->sensitive_pos << handle;                                                                                 \
        this->sensitive_neg << handle;                                                                                 \
    }

#define declare_thread_process_cl(handle, name, host_tag, func)                                                        \
    {                                                                                                                  \
        ::sc_core::sc_spawn_options opt;                                                                               \
        ::sc_core::sc_process_handle handle = ::sc_core::sc_spawn(func, name, &opt);                                   \
        this->sensitive << handle;                                                                                     \
        this->sensitive_pos << handle;                                                                                 \
    }

#define SC_METHOD_CL(name, func) declare_method_process_cl(name##_handle, #name, SC_CURRENT_USER_MODULE, func)

#define SC_THREAD_CL(name, func) declare_thread_process_cl(name##_handle, #name, SC_CURRENT_USER_MODULE, func)

#endif /* _SYSC_UTILITIES_H_ */
