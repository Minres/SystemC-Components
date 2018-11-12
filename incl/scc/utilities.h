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
/*
 * utilities.h
 *
 *  Created on: Nov 5, 2016
 *      Author: eyck
 */

#ifndef _SYSC_UTILITIES_H_
#define _SYSC_UTILITIES_H_

#include "traceable.h"
#include <memory>

// pragmas to disable the deprecated warnings for SystemC headers
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#include <systemc>
#pragma GCC diagnostic pop

#include <locale>

#if __cplusplus < 201402L
namespace std {
template <typename T, typename... Args> std::unique_ptr<T> make_unique(Args &&... args) {
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}
}
#endif

#define NAMED(X, ...) X(#X, ##__VA_ARGS__)
#define NAMEDD(X, T, ...) X(std::make_unique<T>(#X, ##__VA_ARGS__))
#define NAMEDC(X, T, I, ...) X(T::create<I>(#X, ##__VA_ARGS__))

#define TRACE_VAR(F, X) sc_core::sc_trace(F, X, std::string(this->name()) + "." #X)
#define TRACE_ARR(F, X, I)                                                                                             \
    sc_core::sc_trace(F, X[I], (std::string(this->name()) + "." #X "(" + std::to_string(I) + ")").c_str());
#define TRACE_SIG(F, X) sc_core::sc_trace(F, X, X.name())

namespace sc_core {
// needed to be able to use sc_time as signal value
#if SC_VERSION_MAJOR <= 2 && SC_VERSION_MINOR <= 3 && SC_VERSION_PATCH < 2
/**
 *
 * @param
 * @param
 * @param
 */
void sc_trace(sc_trace_file *, const sc_time &, const std::string &);
/**
 *
 * @param
 * @param
 * @param
 */
void sc_trace(sc_trace_file *, const sc_time &, const char *);
#endif
/**
 *
 * @param
 * @param
 * @param
 */
template <> void sc_trace(sc_trace_file *, const sc_in<sc_time> &, const std::string &);
/**
 *
 * @param
 * @param
 * @param
 */
template <> void sc_trace(sc_trace_file *, const sc_inout<sc_time> &, const std::string &);
/**
 *
 * @param val
 * @return
 */
}
// user-define literals for easy time creation
/**
 *
 * @param val
 * @return
 */
inline sc_core::sc_time operator"" _sec(long double val) { return sc_core::sc_time(val, sc_core::SC_SEC); }
/**
 *
 * @param val
 * @return
 */
inline sc_core::sc_time operator"" _sec(unsigned long long val) {
    return sc_core::sc_time(double(val), sc_core::SC_SEC);
}
/**
 *
 * @param val
 * @return
 */
inline sc_core::sc_time operator"" _ms(long double val) { return sc_core::sc_time(val, sc_core::SC_MS); }
/**
 *
 * @param val
 * @return
 */
inline sc_core::sc_time operator"" _ms(unsigned long long val) { return sc_core::sc_time(double(val), sc_core::SC_MS); }
/**
 *
 * @param val
 * @return
 */
inline sc_core::sc_time operator"" _us(long double val) { return sc_core::sc_time(val, sc_core::SC_US); }
/**
 *
 * @param val
 * @return
 */
inline sc_core::sc_time operator"" _us(unsigned long long val) { return sc_core::sc_time(double(val), sc_core::SC_US); }
/**
 *
 * @param val
 * @return
 */
inline sc_core::sc_time operator"" _ns(long double val) { return sc_core::sc_time(val, sc_core::SC_NS); }
/**
 *
 * @param val
 * @return
 */
inline sc_core::sc_time operator"" _ns(unsigned long long val) { return sc_core::sc_time(double(val), sc_core::SC_NS); }
/**
 *
 * @param val
 * @return
 */
inline sc_core::sc_time operator"" _ps(long double val) { return sc_core::sc_time(val, sc_core::SC_PS); }
/**
 *
 * @param val
 * @return
 */
inline sc_core::sc_time operator"" _ps(unsigned long long val) { return sc_core::sc_time(double(val), sc_core::SC_PS); }
/**
 *
 * @param val
 * @return
 */
inline sc_core::sc_time operator"" _fs(long double val) { return sc_core::sc_time(val, sc_core::SC_FS); }
/**
 *
 * @param val
 * @return
 */
inline sc_core::sc_time operator"" _fs(unsigned long long val) { return sc_core::sc_time(double(val), sc_core::SC_FS); }

inline constexpr uint64_t operator"" _kB(unsigned long long val) { return val * 1 << 10; }
inline constexpr uint64_t operator"" _MB(unsigned long long val) { return val * 1 << 20; }
inline constexpr uint64_t operator"" _GB(unsigned long long val) { return val * 1 << 30; }

namespace scc {

inline bool icompare(std::string const &a, std::string const &b) {
    if (a.length() == b.length()) {
        return std::equal(b.begin(), b.end(), a.begin(),
                          [](unsigned char a, unsigned char b) -> bool { return std::tolower(a) == std::tolower(b); });
    } else {
        return false;
    }
}

inline sc_core::sc_time parse_from_string(std::string value, std::string unit) noexcept {
    std::string::size_type offs{0};
    double t_val = std::stod(value, &offs);
    if (offs > 0) {
        if (icompare(unit, "fs")) return t_val * 1_fs;
        if (icompare(unit, "ps")) return t_val * 1_ps;
        if (icompare(unit, "ns")) return t_val * 1_ns;
        if (icompare(unit, "us")) return t_val * 1_us;
        if (icompare(unit, "ms")) return t_val * 1_ms;
        if (icompare(unit, "s")) return t_val * 1_sec;
    }
    return sc_core::SC_ZERO_TIME;
}

inline sc_core::sc_time parse_from_string(std::string value) noexcept {
    std::string::size_type offs{0};
    double t_val = std::stod(value, &offs);
    if (offs > 0) {
        std::string unit = value.substr(offs);
        if (icompare(unit, "fs")) return t_val * 1_fs;
        if (icompare(unit, "ps")) return t_val * 1_ps;
        if (icompare(unit, "ns")) return t_val * 1_ns;
        if (icompare(unit, "us")) return t_val * 1_us;
        if (icompare(unit, "ms")) return t_val * 1_ms;
        if (icompare(unit, "s")) return t_val * 1_sec;
    }
    return sc_core::SC_ZERO_TIME;
}
}

#endif /* _SYSC_UTILITIES_H_ */
