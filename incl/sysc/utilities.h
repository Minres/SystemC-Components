/*******************************************************************************
 * Copyright 2016 MINRES Technologies GmbH
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

// pragmas to disable the deprecated warnings for SystemC headers
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#include <systemc>
#pragma GCC diagnostic pop

#define NAMED(X,...) X(#X,##__VA_ARGS__)
#define NAMEDD(T,X,...) X(new T(#X,##__VA_ARGS__))

#define TRACE_VAR(F, X)    sc_core::sc_trace(F, X, std::string(this->name())+"." #X)
#define TRACE_ARR(F, X, I) sc_core::sc_trace(F, X[I], (std::string(this->name())+"." #X "("+std::to_string(I)+")").c_str());
#define TRACE_SIG(F, X)    sc_core::sc_trace(F, X,X.name())

namespace sc_core {
// needed to be able to use sc_time as signal value
/**
 *
 * @param
 * @param
 * @param
 */
void sc_trace(sc_trace_file*, const sc_time&, const std::string&);
/**
 *
 * @param
 * @param
 * @param
 */
void sc_trace(sc_trace_file*, const sc_time&, const char*);
/**
 *
 * @param
 * @param
 * @param
 */
template <> void sc_trace(sc_trace_file*, const sc_in<sc_time>&, const std::string&);
/**
 *
 * @param
 * @param
 * @param
 */
template <> void sc_trace(sc_trace_file*, const sc_inout<sc_time>&, const std::string&);
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
inline sc_core::sc_time operator"" _sec ( long double val )       {return sc_core::sc_time(val, sc_core::SC_SEC);}
/**
 *
 * @param val
 * @return
 */
inline sc_core::sc_time operator"" _sec ( unsigned long long val ){return sc_core::sc_time(double(val), sc_core::SC_SEC);}
/**
 *
 * @param val
 * @return
 */
inline sc_core::sc_time operator"" _ms  ( long double val )       {return sc_core::sc_time(val, sc_core::SC_MS);}
/**
 *
 * @param val
 * @return
 */
inline sc_core::sc_time operator"" _ms  ( unsigned long long val ){return sc_core::sc_time(double(val), sc_core::SC_MS);}
/**
 *
 * @param val
 * @return
 */
inline sc_core::sc_time operator"" _us  ( long double val )       {return sc_core::sc_time(val, sc_core::SC_US);}
/**
 *
 * @param val
 * @return
 */
inline sc_core::sc_time operator"" _us  ( unsigned long long val ){return sc_core::sc_time(double(val), sc_core::SC_US);}
/**
 *
 * @param val
 * @return
 */
inline sc_core::sc_time operator"" _ns  ( long double val )       {return sc_core::sc_time(val, sc_core::SC_NS);}
/**
 *
 * @param val
 * @return
 */
inline sc_core::sc_time operator"" _ns  ( unsigned long long val ){return sc_core::sc_time(double(val), sc_core::SC_NS);}
/**
 *
 * @param val
 * @return
 */
inline sc_core::sc_time operator"" _ps  ( long double val )       {return sc_core::sc_time(val, sc_core::SC_PS);}
/**
 *
 * @param val
 * @return
 */
inline sc_core::sc_time operator"" _ps  ( unsigned long long val ){return sc_core::sc_time(double(val), sc_core::SC_PS);}
/**
 *
 * @param val
 * @return
 */
inline sc_core::sc_time operator"" _fs  ( long double val )       {return sc_core::sc_time(val, sc_core::SC_FS);}
/**
 *
 * @param val
 * @return
 */
inline sc_core::sc_time operator"" _fs  ( unsigned long long val ){return sc_core::sc_time(double(val), sc_core::SC_FS);}

#endif /* _SYSC_UTILITIES_H_ */
