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
 *  @file     target.h
 *  @brief    target module implementation.
 *            This file declares and implements the functionality of the target.
 *            Few of the parameters of the target sc_module are configured by the
 *            router sc_module
 *  @author   P V S Phaneendra, CircuitSutra Technologies   <pvs@circuitsutra.com>
 *            Parvinder Pal Singh, CircuitSutra Technologies   <parvinder@circuitsutra.com>
 *  @date     5th May, 2011 (Thursday)
 */

#ifndef EXAMPLES_EX09_HIERARCHICAL_OVERRIDE_OF_PARAMETER_VALUES_TARGET_H_
#define EXAMPLES_EX09_HIERARCHICAL_OVERRIDE_OF_PARAMETER_VALUES_TARGET_H_

#include <cci_configuration>
#include <scc/report.h>
#include <string>
#include <tlm>
#include <tlm_utils/simple_target_socket.h>

/**
 *  @class  target
 *  @brief  This module implementas the functionality of a target IP
 */
SC_MODULE(target) {
public:
    tlm_utils::simple_target_socket<target, 32> target_socket;
    sc_core::sc_time read_latency, write_latency;

    SC_CTOR(target)
    : target_socket("target_socket")
    , target_ID("target_ID", "target_default")
    , s_base_addr("s_base_addr", 0)
    , s_size("s_size", 256) {
        SCCINFO(SCMOD) << "[" << target_ID.get_value() << " C_TOR] ------- [TARGET CONSTRUCTOR BEGINS HERE] --------";
        SCCINFO(SCMOD) << "[" << target_ID.get_value() << " C_TOR] : Base Address : " << s_base_addr.get_value();

        // Register b_transport
        target_socket.register_b_transport(this, &target::b_transport);

        write_latency = sc_core::sc_time(3, sc_core::SC_NS);
        read_latency = sc_core::sc_time(5, sc_core::SC_NS);

        mem = new int[s_size.get_value()];

        for(unsigned int i = 0; i < s_size.get_value(); i++)
            mem[i] = 0xAABBCCDD | i;

        // target's SC_THREAD declaration
        SC_THREAD(run_target);
    }

    /**
     *  @fn     void run_target(void)
     *  @brief  The run thread of the modeul (does nothing)
     *  @return void
     */
    void run_target(void) {}

    /**
     *  @fn     void b_transport(tlm::tlm_generic_payload& trans, sc_core::sc_time& delay)
     *  @brief  Implementation of the blocking transport in the target
     *  @param  trans The transaction being sent
     *  @param  delay The annotated delay associated with the transaction
     *  @return void
     */
    void b_transport(tlm::tlm_generic_payload & trans, sc_core::sc_time & delay) {
        tlm::tlm_command cmd = trans.get_command();
        sc_dt::uint64 adr = trans.get_address() - s_base_addr.get_value();
        unsigned char* ptr = trans.get_data_ptr();
        unsigned int len = trans.get_data_length();
        unsigned char* byt = trans.get_byte_enable_ptr();
        unsigned int wid = trans.get_streaming_width();

        SCCINFO(SCMOD) << "[TARGET] : adr ---- " << std::hex << adr;
        SCCINFO(SCMOD) << "[TARGET] : base addr ---- " << std::hex << s_base_addr.get_value();

        // Check for storage address overflow
        if(adr > s_size.get_value()) {
            trans.set_response_status(tlm::TLM_ADDRESS_ERROR_RESPONSE);
            return;
        }

        // Target unable to support byte enable attribute
        if(byt) {
            trans.set_response_status(tlm::TLM_BYTE_ENABLE_ERROR_RESPONSE);
            return;
        }

        // Target unable to support streaming width attribute
        if(wid < len) {
            trans.set_response_status(tlm::TLM_BURST_ERROR_RESPONSE);
            return;
        }

        if(cmd == tlm::TLM_READ_COMMAND) {
            memcpy(ptr, &mem[adr], len);
            delay = delay + read_latency;
        } else if(cmd == tlm::TLM_WRITE_COMMAND) {
            memcpy(&mem[adr], ptr, len);
            delay = delay + write_latency;
        }

        trans.set_response_status(tlm::TLM_OK_RESPONSE);
    }

private:
    cci::cci_param<std::string, cci::CCI_MUTABLE_PARAM>
        target_ID; ///< Elaboration Time Param for assigning target ID (initialized by top_module)

    cci::cci_param<int, cci::CCI_MUTABLE_PARAM> s_base_addr; ///< Mutable time param for setting target's base addr (initialized by router)

    cci::cci_param<unsigned int> s_size; ///< Mutable time parameter for setting target's size (initialized by router);

    /**
     *  @fn     void end_of_elaboration()
     *  @brief  end of elaboration function to lock structural param
     *  @return void
     */
    void end_of_elaboration() {
        target_ID.lock();
        s_base_addr.lock();
    }

    int* mem;
};
    // target

#endif // EXAMPLES_EX09_HIERARCHICAL_OVERRIDE_OF_PARAMETER_VALUES_TARGET_H_
