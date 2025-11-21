/*******************************************************************************
 * Copyright 2016-2025 MINRES Technologies GmbH
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

#ifndef _CXS_SCV_RECORDER_H_
#define _CXS_SCV_RECORDER_H_

#include <cxs/cxs_tlm.h>
#include <tlm/nw/scv/tlm_recorder.h>
#include <tlm/nw/scv/tlm_recorder_module.h>
#ifdef HAS_SCV
#include <scv.h>
#else
#include <scv-tr.h>
#ifndef SCVNS
#define SCVNS ::scv_tr::
#endif
#endif
//! @brief SystemC TLM
namespace tlm {
//! @brief SCC TLM utilities
namespace nw {
//! @brief SCC SCV4TLM classes and functions
namespace scv {

inline void record(SCVNS scv_tr_handle& handle, cxs::cxs_flit_payload const& o) {
    static const std::array<std::string, 3> cmd2char{{"FLIT", "CREDIT", "CRDRTN"}};
    handle.record_attribute("flit.ptr", reinterpret_cast<uintptr_t>(&o));
    handle.record_attribute("flit.type", cmd2char.at(static_cast<unsigned>(o.get_command())));
    // std::array<uint8_t, 8> start_ptr;
    // std::array<uint8_t, 8> end_ptr;
    handle.record_attribute("flit.start", static_cast<unsigned>(o.start));
    handle.record_attribute("flit.end", static_cast<unsigned>(o.end));
    handle.record_attribute("flit.end_error", o.end_error);
    handle.record_attribute("flit.last", o.last);
}

inline void record(SCVNS scv_tr_handle& handle, ::cxs::cxs_packet_payload const& o) {
}
} // namespace scv
} // namespace nw
} // namespace tlm

#endif /* _CXS_SCV_RECORDER_H_ */
