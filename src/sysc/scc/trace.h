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

#ifndef SCC_TRACE_H
#define SCC_TRACE_H

#include "observer.h"
#include <functional>
#include <sysc/tracing/sc_trace.h>

/** \ingroup scc-sysc
 *  @{
 */
/**@{*/
//! @brief SCC SystemC utilities
namespace scc {
//! create VCD file which uses pull mechanism
sc_core::sc_trace_file* create_vcd_pull_trace_file(const char* name,
                                                   std::function<bool()> enable = std::function<bool()>());
//! close the VCD file
void close_vcd_pull_trace_file(sc_core::sc_trace_file* tf);

//! create VCD file which uses push mechanism
sc_core::sc_trace_file* create_vcd_push_trace_file(const char* name,
                                                   std::function<bool()> enable = std::function<bool()>());
//! close the VCD file
void close_vcd_push_trace_file(sc_core::sc_trace_file* tf);

//! create compressed VCD file which uses push mechanism and multithreading
sc_core::sc_trace_file* create_vcd_mt_trace_file(const char* name,
                                                 std::function<bool()> enable = std::function<bool()>());
//! close the VCD file
void close_vcd_mt_trace_file(sc_core::sc_trace_file* tf);

//! create FST file which uses pull mechanism
sc_core::sc_trace_file* create_fst_trace_file(const char* name, std::function<bool()> enable = std::function<bool()>());
//! close the FST file
void close_fst_trace_file(sc_core::sc_trace_file* tf);
} // namespace scc
/** @} */ // end of scc-sysc
#endif // SCC_SC_VCD_TRACE_H
