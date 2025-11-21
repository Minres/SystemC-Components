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

#include "tlm_recorder.h"
#include "tlm/nw/tlm_network_sockets.h"
#include <tlm/scc/scv/tlm_extension_recording_registry.h>
#include <tlm/scc/tlm_id.h>

namespace tlm {
namespace nw {
namespace scv {
void record(SCVNS scv_tr_handle& handle, tlm::nw::tlm_network_payload_base& o) {
    handle.record_attribute("trans.ptr", reinterpret_cast<uintptr_t>(&o));
}
} // namespace scv
} // namespace nw
} // namespace tlm
