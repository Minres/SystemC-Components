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

#include <functional>
#include <ostream>

namespace sc_core {
inline std::ostream& operator<<(std::ostream& os, std::function<void()>&) { return os; }
} // namespace sc_core

#define SC_INCLUDE_DYNAMIC_PROCESSES
#include "sc_thread_pool.h"

namespace scc {

sc_thread_pool::sc_thread_pool()
: sc_core::sc_object(sc_core::sc_gen_unique_name("pool")) {}

sc_thread_pool::~sc_thread_pool() = default;

void sc_thread_pool::execute(std::function<void(void)> fct) {
    sc_core::sc_spawn_options opts;
    opts.set_stack_size(0x10000);
    if(thread_avail == 0 && thread_active < max_concurrent_threads.get_value())
        sc_core::sc_spawn(
            [this]() {
                thread_active++;
                while(true) {
                    thread_avail++;
                    auto fct = dispatch_queue.read();
                    sc_assert(thread_avail > 0);
                    thread_avail--;
                    fct();
                }
            },
            nullptr, &opts);
    dispatch_queue.write(fct);
}

} /* namespace scc */
