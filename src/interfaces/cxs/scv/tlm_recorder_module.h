/*******************************************************************************
 * Copyright 2016, 2025 MINRES Technologies GmbH
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

#ifndef CXS_SCV_TLM_RECORDER_MODULE_H
#define CXS_SCV_TLM_RECORDER_MODULE_H
#include <cxs/cxs_tlm.h>
#include "tlm/nw/scv/tlm_recorder_module.h"
#include "tlm_recording.h"
//! @brief CXS
namespace cxs {
//! @brief SCC CXS SCV classes and functions
namespace scv {
template <unsigned int FLIT_WITH>
using  csx_recorder_module = typename  tlm::nw::scv::tlm_recorder_module<cxs::CXS_CMD, FLIT_WITH, cxs::cxs_flit_types>;
} // namespace scv
} // namespace cxs

#endif /* TLM_NW_SCV_TLM_RECORDER_MODULE_H */
