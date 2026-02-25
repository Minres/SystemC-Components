/*******************************************************************************
 * Copyright 2026 MINRES Technologies GmbH
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

#include "memory_map_collector.h"
#include <fmt/format.h>

namespace tlm {
namespace scc {
std::string memory_node::to_string(const std::string& prefix) const {
    std::ostringstream os;
    std::string filler(16 - prefix.length(), ' ');
    os << fmt::format("{} 0x{:016x}:0x{:016x}{}{}\n", prefix, start, end, filler, name);
    for(auto e : elemets)
        os << e.to_string(prefix + "  ");
    return os.str();
}

} // namespace scc
} // namespace tlm
