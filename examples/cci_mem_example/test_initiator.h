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
#ifndef _TEST_INITIATOR_H_
#define _TEST_INITIATOR_H_

#include <scc/utilities.h>
#include <tlm_utils/simple_initiator_socket.h>

namespace sysc {

class test_initiator : public sc_core::sc_module {
public:
    SC_HAS_PROCESS(test_initiator);
    tlm_utils::simple_initiator_socket<test_initiator, 32> intor{"intor"};
    sc_core::sc_in<bool> rst_i{"rst_i"};
    test_initiator(sc_core::sc_module_name nm);

    void run();
    void write_bus(std::uint32_t adr, std::uint32_t dat);
    std::uint32_t read_bus(std::uint32_t adr);
    void reg_check(std::uint32_t adr, std::uint32_t exp);
};

} /* namespace sysc */

#endif /* _TEST_INITIATOR_H_ */
