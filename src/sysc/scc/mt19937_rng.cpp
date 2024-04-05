/*******************************************************************************
 * Copyright 2020-2022 MINRES Technologies GmbH
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

#include "mt19937_rng.h"
#include <cstdlib>
#include <systemc>
#include <unordered_map>

namespace {
struct {
    std::mt19937_64 global;
    std::unordered_map<void*, std::mt19937_64> inst;
    uint64_t seed{std::mt19937_64::default_seed};
    bool global_seed;
} rng;

bool debug_randomization = getenv("SCC_DEBUG_RANDOMIZATION") != nullptr;
}; // namespace

auto scc::MT19937::inst() -> std::mt19937_64& {
#ifndef NCSC
    if(auto* obj = sc_core::sc_get_current_object()) {
        auto sz = rng.inst.size();
        auto& ret = rng.inst[obj];
        if(rng.inst.size() > sz) {
            uint64_t seed{0};
            if(rng.global_seed) {
                seed = reinterpret_cast<uintptr_t>(&rng.inst) ^ rng.seed;
                if(debug_randomization)
                    std::cout << "seeding rng for " << obj->name() << " with global seed " << seed << "\n";
            } else {
                std::string name{obj->name()};
                std::hash<std::string> h;
                seed = (h(name) ^ rng.seed);
                if(debug_randomization)
                    std::cout << "seeding rng for " << obj->name() << " with local seed " << seed << "\n";
            }
            ret.seed(seed);
        }
        if(debug_randomization) {
            std::cout << "retrieving next rnd number for " << obj->name() << "\n";
        }
        return ret;
    }
#endif
    return rng.global;
}

void scc::MT19937::seed(uint64_t new_seed) {
    rng.seed = new_seed;
    rng.global.seed(new_seed);
    for(auto& e : rng.inst)
        e.second.seed(new_seed);
}

void scc::MT19937::enable_global_seed(bool enable) { rng.global_seed = enable; }
