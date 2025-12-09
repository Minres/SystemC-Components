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
 *  @file     router.h
 *  @brief    Definition of the router module.
 *            This file declares and implements the functionality of the router.
 *            Few of the parameters of the target and initiator sc_module(s) are
 *            configured by the router sc_module
 *  @authors  P V S Phaneendra, CircuitSutra Technologies   <pvs@circuitsutra.com>
 *  @date     29th April, 2011 (Friday)
 */

#ifndef EXAMPLES_EX09_HIERARCHICAL_OVERRIDE_OF_PARAMETER_VALUES_ROUTER_H_
#define EXAMPLES_EX09_HIERARCHICAL_OVERRIDE_OF_PARAMETER_VALUES_ROUTER_H_

#ifdef _MSC_VER
#define snprintf _snprintf
#endif

#include <cci_configuration>
#include <iomanip>
#include <sstream>
#include <tlm>
#include <vector>

#include <tlm_utils/multi_passthrough_initiator_socket.h>
#include <tlm_utils/multi_passthrough_target_socket.h>

/**
 *  @class  router
 *  @brief  Thes module implements a router functionality
 */
SC_MODULE(router) {
public:
    // Declare tlm multi-passthrough sockets for target and initiator modules
    tlm_utils::multi_passthrough_target_socket<router, 32> Router_target;
    tlm_utils::multi_passthrough_initiator_socket<router, 32> Router_initiator;

    /**
     *  @fn     router
     *  @brief  The class constructor
     *  @return void
     */
    SC_CTOR(router)
    : Router_target("Router_target")
    , Router_initiator("Router_initiator")
    , r_initiators("r_initiators", 0)
    , r_targets("r_targets", 0)
    , addr_limit("addr_max", 64)
    , m_broker(cci::cci_get_broker())
    , addrSize(0) {
        SCCINFO(SCMOD) << "[ROUTER C_TOR] ----- [ROUTER CONSTRUCTOR BEGINS HERE] ------";

        // Register b_transport
        Router_target.register_b_transport(this, &router::b_transport);
    }

    /**
     *  @fn     void before_end_of_elaboration(void)
     *  @brief  The router table information is filled in during this function
     *  @return void
     */
    void before_end_of_elaboration(void) {
        SCCINFO(SCMOD) << "[ROUTER in beoe] : Number of initiator(s) : " << r_initiators.get_cci_value().to_json();
        SCCINFO(SCMOD) << "[ROUTER in beoe] : Number of target(s) : " << r_targets.get_value();
        SCCINFO(SCMOD) << "[ROUTER in beoe] : Maximum Addressable Limit of the router : " << addr_limit.get_value();

        char targetName[10]; ///< Holds router table's fields' names
        addrSize = (unsigned int)(addr_limit.get_value() / r_targets);

        // Printing the Router Table contents
        SCCINFO(SCMOD) << "============= ROUTER TABLE INFORMATION ==============";
        SCCINFO(SCMOD) << "-----------------------------------------------------";
        SCCINFO(SCMOD) << "| Target ID  | Start Addr |  End Addr  | Base Addr  |";
        SCCINFO(SCMOD) << "-----------------------------------------------------";

        // Sets the contents of the routing table with (default) values
        // calculated within 'beoe' phase
        for(int i = 0; i < r_targets; i++) {
            snprintf(targetName, sizeof(targetName), "r_index_%d", i);
            r_target_index.push_back(new cci::cci_param<unsigned int, cci::CCI_IMMUTABLE_PARAM>(targetName, i));

            snprintf(targetName, sizeof(targetName), "r_sa_%d", i);
            r_addr_start.push_back(new cci::cci_param<unsigned int, cci::CCI_IMMUTABLE_PARAM>(targetName, (i * addrSize)));

            snprintf(targetName, sizeof(targetName), "r_ea_%d", i);
            r_addr_end.push_back(new cci::cci_param<unsigned int, cci::CCI_IMMUTABLE_PARAM>(targetName, ((i + 1) * addrSize - 1)));
        }

        for(int i = 0; i < r_targets; i++) {
            snprintf(stringName, sizeof(stringName), "top_module_inst.target_%d.s_base_addr", i);

            base_handle = m_broker.get_param_handle(stringName);
            if(!base_handle.is_valid()) {
                sc_assert(!"target Base Address Handle returned is NULL");
            }
            std::stringstream row_ss;
            row_ss << "| " << std::setw(10) << r_target_index[i]->get_value() << " | " << std::setw(10) << std::hex << std::showbase
                   << r_addr_start[i]->get_value() << " | " << std::setw(10) << r_addr_end[i]->get_value() << " | " << std::setw(10)
                   << base_handle.get_cci_value().to_json() << " |";
            SCCINFO(SCMOD) << row_ss.str().c_str();
            SCCINFO(SCMOD) << "-----------------------------------------------------";
        }
    }

    // Blocking transport implementation of the router
    void b_transport(int i_, tlm::tlm_generic_payload& trans, sc_core::sc_time& delay) {
        wait(delay);

        delay = sc_core::SC_ZERO_TIME;

        sc_dt::uint64 addr = trans.get_address();

        if(addr >= static_cast<sc_dt::uint64>(addr_limit.get_value())) {
            trans.set_response_status(tlm::TLM_ADDRESS_ERROR_RESPONSE);
            return;
        }

        for(unsigned int i = 0; i < r_target_index.size(); i++) {
            if((addr >= (r_addr_start[i]->get_value())) && (addr <= (r_addr_end[i]->get_value()))) {
                SCCINFO(SCMOD) << "[Router in 'b_transport' layer]";
                SCCINFO(SCMOD) << "Address       = " << std::hex << addr;
                SCCINFO(SCMOD) << "Index         = " << (r_target_index[i])->get_value();
                SCCINFO(SCMOD) << "Start addres  = " << std::hex << (r_addr_start[i]->get_value());
                SCCINFO(SCMOD) << "End   Address = " << std::hex << (r_addr_end[i]->get_value());
                Router_initiator[(r_target_index[i])->get_value()]->b_transport(trans, delay);
                break;
            }
        }
    }

private:
    /// Demonstrates Model-to-Model Configuration (UC12)
    /// Elaboration Time Parameters for setting up the model hierarcy;
    cci::cci_param<int, cci::CCI_MUTABLE_PARAM> r_initiators;        ///< initiator ID assigned by the top_module upon instantiation
    cci::cci_param<int, cci::CCI_MUTABLE_PARAM> r_targets;           ///< target ID assigned by the top_module upon instantiation
    cci::cci_param<unsigned int, cci::CCI_MUTABLE_PARAM> addr_limit; ///< Router Addressing Range
    cci::cci_broker_handle m_broker;                                 ///< CCI configuration broker handle

    /// Router Table contents holding targets related information
    std::vector<cci::cci_param<unsigned int, cci::CCI_IMMUTABLE_PARAM>*> r_target_index; ///< Router table target index
    std::vector<cci::cci_param<unsigned int, cci::CCI_IMMUTABLE_PARAM>*> r_addr_start;   ///< Router table start address
    std::vector<cci::cci_param<unsigned int, cci::CCI_IMMUTABLE_PARAM>*> r_addr_end;     ///< Router table end address

    cci::cci_param_handle base_handle; ///< CCI base parameter handle for target base address

    /**
     *  @fn     void end_of_elaboration()
     *  @brief  end of elaboration function to lock structural param
     *  @return void
     */
    void end_of_elaboration() {
        r_initiators.lock();
        r_targets.lock();
    }

    int addrSize;
    char stringName[50];
};
    // router

#endif // EXAMPLES_EX09_HIERARCHICAL_OVERRIDE_OF_PARAMETER_VALUES_ROUTER_H_
