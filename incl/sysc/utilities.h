/*
 * utilities.h
 *
 *  Created on: Nov 5, 2016
 *      Author: eyck
 */

#ifndef SYSC_UTILITIES_H_
#define SYSC_UTILITIES_H_

#include "traceable.h"

// pragmas to disable the deprecated warnings for SystemC headers
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#include <systemc>
#pragma GCC diagnostic pop

#define NAMED(X,...) X(#X,##__VA_ARGS__)
#define MOD(X) X(#X)
#define MOD_A(X,...) X(#X,__VA_ARGS__)
#define ATTR(X) X(#X)
#define SCPIN(X) X(#X)
#define SCPIN_A(X,...) X(#X,__VA_ARGS__)
#define SIG(X) X(#X)
#define SIG_A(X,...) X(#X,__VA_ARGS__)
#define SOCKET(X) X(#X)

#define TRACE_VAR(F, X)    sc_core::sc_trace(F, X, std::string(this->name())+"." #X)
#define TRACE_ARR(F, X, I) sc_core::sc_trace(F, X[I], (std::string(this->name())+"." #X "("+std::to_string(I)+")").c_str());
#define TRACE_SIG(F, X)    sc_core::sc_trace(F, X,X.name())

namespace sc_core {
// needed to be able to use sc_time as signal value
void sc_trace(sc_trace_file*, const sc_time&, const std::string&);
void sc_trace(sc_trace_file*, const sc_time&, const char*);
template <> void sc_trace(sc_trace_file*, const sc_in<sc_time>&, const std::string&);
template <> void sc_trace(sc_trace_file*, const sc_inout<sc_time>&, const std::string&);
}
// user-define literals for easy time creation
inline sc_core::sc_time operator"" _sec ( long double val )       {return sc_core::sc_time(val, sc_core::SC_SEC);}
inline sc_core::sc_time operator"" _sec ( unsigned long long val ){return sc_core::sc_time(double(val), sc_core::SC_SEC);}
inline sc_core::sc_time operator"" _ms  ( long double val )       {return sc_core::sc_time(val, sc_core::SC_MS);}
inline sc_core::sc_time operator"" _ms  ( unsigned long long val ){return sc_core::sc_time(double(val), sc_core::SC_MS);}
inline sc_core::sc_time operator"" _us  ( long double val )       {return sc_core::sc_time(val, sc_core::SC_US);}
inline sc_core::sc_time operator"" _us  ( unsigned long long val ){return sc_core::sc_time(double(val), sc_core::SC_US);}
inline sc_core::sc_time operator"" _ns  ( long double val )       {return sc_core::sc_time(val, sc_core::SC_NS);}
inline sc_core::sc_time operator"" _ns  ( unsigned long long val ){return sc_core::sc_time(double(val), sc_core::SC_NS);}
inline sc_core::sc_time operator"" _ps  ( long double val )       {return sc_core::sc_time(val, sc_core::SC_PS);}
inline sc_core::sc_time operator"" _ps  ( unsigned long long val ){return sc_core::sc_time(double(val), sc_core::SC_PS);}
inline sc_core::sc_time operator"" _fs  ( long double val )       {return sc_core::sc_time(val, sc_core::SC_FS);}
inline sc_core::sc_time operator"" _fs  ( unsigned long long val ){return sc_core::sc_time(double(val), sc_core::SC_FS);}

#endif /* SYSC_UTILITIES_H_ */
