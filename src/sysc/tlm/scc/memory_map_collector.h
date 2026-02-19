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

#ifndef _TLM_SCC_MEMORY_MAP_COLLECTOR_H_
#define _TLM_SCC_MEMORY_MAP_COLLECTOR_H_

#include <cstdint>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <tlm>
#include <tlm_core/tlm_2/tlm_2_interfaces/tlm_fw_bw_ifs.h>

//! @brief SystemC TLM
namespace tlm {
//! @brief SCC TLM utilities
namespace scc {
struct memory_node {
    uint64_t start{0};
    uint64_t end{std::numeric_limits<uint64_t>::max()};
    std::string name;
    std::vector<memory_node> elemets;
    memory_node(uint64_t start, uint64_t end)
    : start(start)
    , end(end) {}
    memory_node() = default;
    std::string to_string(const std::string &prefix = "") const;
};

struct memory_map_extension : tlm::tlm_extension<memory_map_extension> {
    tlm_extension_base* clone() const override { return new memory_map_extension(*this); }
    void copy_from(tlm_extension_base const& from) override { throw std::runtime_error("copying of memory_map_extension not allowed"); }

    memory_map_extension(memory_node& node)
    : node(node) {}
    ~memory_map_extension() {}

    memory_node& node;
    uint64_t offset{0};
};

template <typename TYPES = tlm::tlm_base_protocol_types>
memory_node gather_memory(sc_core::sc_port_b<tlm::tlm_fw_transport_if<TYPES>>& port) {
    memory_node root;
    auto ext = memory_map_extension(root);
    /*TYPES::tlm_payload_type*/ tlm::tlm_generic_payload trans;
    trans.set_extension(&ext);
    port->transport_dbg(trans);
    trans.set_extension<memory_map_extension>(nullptr);
    return std::move(root);
}

} // namespace scc
} // namespace tlm
#endif // _TLM_SCC_MEMORY_MAP_COLLECTOR_H_
