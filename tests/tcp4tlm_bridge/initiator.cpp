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

#include "initiator.h"
#include <scc/report.h>
#include <sstream>

namespace tcp4tlm_bridge {

SC_HAS_PROCESS(initiator);
initiator::initiator(sc_core::sc_module_name mn)
: sc_module(mn) {
    SC_THREAD(main_thread);
};

initiator::~initiator() {};

void initiator::main_thread() {
    unsigned int nr_of_trans = 5;
    unsigned int start_address = 0;
    unsigned int data_length = 4;
    std::stringstream buf;
    tlm::tlm_generic_payload trans;
    trans.set_address(start_address);
    trans.set_data_length(data_length);
    unsigned char* data = new unsigned char[data_length];
    trans.set_data_ptr(&data[0]);
    // trans.set_command          ( tlm::TLM_WRITE_COMMAND );
    trans.set_command(tlm::TLM_READ_COMMAND);
    sc_core::sc_time delay;
    wait(1.0, sc_core::SC_US);

    for(unsigned int i = 0; i < nr_of_trans; ++i) {
        SCCINFO(SCMOD) << "Info: start sending transfer on address " << trans.get_address() << " command type is " << trans.get_command()
                       << " at time " << sc_core::sc_time_stamp();
        isckt->b_transport(trans, delay);
        wait(1, sc_core::SC_US);
        trans.set_address(0x4 * i);
    };
    wait(1, sc_core::SC_US);
    sc_core::sc_stop();
};
} // namespace tcp4tlm_bridge
