/*****************************************************************************

 Licensed to Accellera Systems Initiative Inc. (Accellera) under one or
 more contributor license agreements.  See the NOTICE file distributed
 with this work for additional information regarding copyright ownership.
 Accellera licenses this file to you under the Apache License, Version 2.0
 (the "License"); you may not use this file except in compliance with the
 License.  You may obtain a copy of the License at

 http://www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
 implied.  See the License for the specific language governing
 permissions and limitations under the License.

 ****************************************************************************/

/**
 *  @file     initiator.h
 *  @brief    initiator module implementation.
 *            This file declares and implements the functionality of the initiator.
 *            Few of the parameters of the initiator sc_module are configured by the
 *            router sc_module
 *  @author   P V S Phaneendra, CircuitSutra Technologies   <pvs@circuitsutra.com>
 *            Parvinder Pal Singh, CircuitSutra Technologies   <parvinder@circuitsutra.com>
 *  @date     29th April, 2011 (Friday)
 */

#ifndef EXAMPLES_EX09_HIERARCHICAL_OVERRIDE_OF_PARAMETER_VALUES_INITIATOR_H_
#define EXAMPLES_EX09_HIERARCHICAL_OVERRIDE_OF_PARAMETER_VALUES_INITIATOR_H_

#include <cci_configuration>
#include <scc/report.h>
#include <string>
#include <tlm>
#include <tlm_utils/simple_initiator_socket.h>

/**
 *  @class  initiator
 *  @brief  The implementation of the initiator module with tlm2 socket for communication
 *  @return void
 */
SC_MODULE(initiator) {
public:
    int data;

    tlm_utils::simple_initiator_socket<initiator, 32> initiator_socket; ///< Instance of TLM2 simple initiator socket

    /**
     *  @fn     initiator
     *  @brief  The class constructor
     *  @return void
     */
    SC_CTOR(initiator)
    : data(0)
    , initiator_socket("initiator_socket")
    , initiator_ID("initiator_ID", "initiator_default") {
        SCCINFO(SCMOD) << "[" << initiator_ID.get_value() << " C_TOR] ------- [INITIATOR CONSTRUCTOR BEGINS HERE] --------";

        // initiator's SC_THREAD declaration
        SC_THREAD(run_initiator);
    }

    /**
     *  @fn     void run_initiator(void)
     *  @brief  Main function to send transactions
     *  @return void
     */
    void run_initiator(void) {
        tlm::tlm_generic_payload* trans = new tlm::tlm_generic_payload;

        int i = 0;

        static tlm::tlm_command cmds[8] = {tlm::TLM_WRITE_COMMAND, tlm::TLM_READ_COMMAND, tlm::TLM_WRITE_COMMAND, tlm::TLM_READ_COMMAND,
                                           tlm::TLM_READ_COMMAND,  tlm::TLM_READ_COMMAND, tlm::TLM_WRITE_COMMAND, tlm::TLM_WRITE_COMMAND};
        while(1) {
            tlm::tlm_command cmd = cmds[(i >> 2) % 8];
            // static_cast<tlm::tlm_command>(cmd_dist(rng));

            if(cmd == tlm::TLM_WRITE_COMMAND)
                data = 0xFF000000 | i;

            trans->set_command(cmd);
            trans->set_address(i);
            trans->set_data_ptr(reinterpret_cast<unsigned char*>(&data));
            trans->set_data_length(4);
            trans->set_streaming_width(4);
            trans->set_byte_enable_ptr(0);
            trans->set_dmi_allowed(false);
            trans->set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);
            sc_core::sc_time delay = sc_core::sc_time(0, sc_core::SC_NS);

            if(cmd == tlm::TLM_WRITE_COMMAND) {
                SCCINFO(SCMOD) << "[Initiators Message]=>At address " << std::hex << i << " sending transaction with command = Write"
                               << ", data=" << std::hex << data << " at time " << sc_core::sc_time_stamp();
            } else {
                SCCINFO(SCMOD) << "[Initiators Message]=>At address " << std::hex << i << " sending transaction with command= Read "
                               << " at time " << sc_core::sc_time_stamp();
            }

            initiator_socket->b_transport(*trans, delay);

            if(trans->is_response_error())
                SCCERR(SCMOD) << "TLM_2" << trans->get_response_string().c_str();

            if(delay.to_double() != 0)
                wait(delay);

            if(cmd == tlm::TLM_WRITE_COMMAND) {
                SCCINFO(SCMOD) << "[Initiators Message]=>At address " << std::hex << i << " received response of Write transaction "
                               << " at time " << sc_core::sc_time_stamp();
            } else {
                SCCINFO(SCMOD) << "[Initiators Message]=>At address " << std::hex << i << " received response of Read transaction "
                               << " data " << data << " at time " << sc_core::sc_time_stamp();
            }

            SCCINFO(SCMOD) << "--------------------------------------------------------";

            wait(5.0, sc_core::SC_NS);

            i = i + 4;
        }
    }

private:
    cci::cci_param<std::string, cci::CCI_MUTABLE_PARAM>
        initiator_ID; ///< Elab Time Param for assigning initiator ID (initialized by top_module)
    /**
     *  @fn     void end_of_elaboration()
     *  @brief  end of elaboration function to lock structural param
     *  @return void
     */
    void end_of_elaboration() { initiator_ID.lock(); }
};
    // initiator

#endif // EXAMPLES_EX09_HIERARCHICAL_OVERRIDE_OF_PARAMETER_VALUES_INITIATOR_H_
