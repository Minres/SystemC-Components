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
#include <sstream>
namespace tlm {
namespace scc {
namespace tcp {

initiator::initiator(sc_core::sc_module_name mn)
: sc_module(mn) {
    SC_THREAD(main_thread);
};

initiator::~initiator(){};

void initiator::main_thread() {
    unsigned int NrOfTrans = 5;
    unsigned int StartAddress = 0;
    unsigned int DataLength = 4;
    std::stringstream buf;
    tlm::tlm_generic_payload trans;
    trans.set_address(StartAddress);
    trans.set_data_length(DataLength);
    unsigned char* Data = new unsigned char[DataLength];
    trans.set_data_ptr(&Data[0]);
    // trans.set_command          ( tlm::TLM_WRITE_COMMAND );
    trans.set_command(tlm::TLM_READ_COMMAND);
    sc_core::sc_time delay;
    wait(1.0, sc_core::SC_US);

    for(unsigned int i = 0; i < NrOfTrans; ++i) {
        buf << "Info: start sending transfer on address " << trans.get_address() << " command type is " << trans.get_command()
            << " at time " << sc_core::sc_time_stamp() << std::endl;
        SC_REPORT_INFO("slave", buf.str().c_str());
        isckt->b_transport(trans, delay);
        wait(1, sc_core::SC_US);
        trans.set_address(0x4 * i);
    };
    wait(1, sc_core::SC_US);
    sc_core::sc_stop();
};
} // namespace tcp
} // namespace scc
} // namespace tlm
