/*******************************************************************************
 * Copyright 2023 MINRES Technologies GmbH
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
#ifndef SIMPLESYSTEM_H_
#define SIMPLESYSTEM_H_

#include "test_initiator.h"
#include <array>
#include <sysc/kernel/sc_module.h>
#include <scc/router.h>
#include <scc/memory.h>
#include "periph.h"

namespace sysc {

class simple_system : public sc_core::sc_module {
public:
    SC_HAS_PROCESS(simple_system);

    test_initiator master{"master"};
    scc::router<> router{"router", 2};
    scc::memory<4_kB> mem{"mem"};
    sysc::periph uart{"uart"};
    sc_core::sc_signal<sc_core::sc_time> clk{"clk"};
    sc_core::sc_signal<bool> rst{"rst"};

    simple_system(sc_core::sc_module_name nm);

protected:
    void gen_reset();
};

} /* namespace sysc */

#endif /* SIMPLESYSTEM_H_ */
