#ifndef __SCC_TLM_QUANTUMKEEPER_H__
#define __SCC_TLM_QUANTUMKEEPER_H__

#include "quantum_keeper_st.h"
#if SC_VERSION_MAJOR < 3
#warning "Multithreaded quantum keeper is only supported with SystemC 3.0 and newer"
namespace tlm {
namespace scc {
using quantumkeeper_mt = quantumkeeper_st;
} // namespace scc
} // namespace tlm
#else
#include "quantum_keeper_mt.h"
#define DEBUG_MT_SCHEDULING
#endif
namespace tlm {
namespace scc {
using quantumkeeper = quantumkeeper_st;
} // namespace scc
} // namespace tlm

#endif
