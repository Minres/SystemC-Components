/*******************************************************************************
 * Copyright 2021 MINRES Technologies GmbH
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

#ifndef SCC_SC_VCD_TRACE_H
#define SCC_SC_VCD_TRACE_H

#include "trace.h"

/** \ingroup scc-sysc
 *  @{
 */
/**@{*/
//! @brief SCC SystemC utilities
namespace scc {
//! keep backward compatibility
inline sc_core::sc_trace_file* scc_create_vcd_trace_file(const char* name,
                                                         std::function<bool()> enable = std::function<bool()>()) {
    return create_vcd_pull_trace_file(name, enable);
}
inline void scc_close_vcd_trace_file(sc_core::sc_trace_file* tf) { close_vcd_pull_trace_file(tf); }
} // namespace scc
/** @} */ // end of scc-sysc
#endif // SCC_SC_VCD_TRACE_H
