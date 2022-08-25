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

#ifndef SCC_INCL_TLM_PE_INTOR_IF_H_
#define SCC_INCL_TLM_PE_INTOR_IF_H_

#include <tlm>

//! @brief SystemC TLM
namespace tlm {
//! @brief SCC TLM utilities
namespace scc {
//! @brief SCC protocol engines
namespace pe {
/**
 * enum to express expectations and capabilities. NB means no backpressure (aka non-blocking)
 *
 */
enum class type { NB, BL };

template <type TYPE> struct intor_fw : public sc_core::sc_interface {
    /**
     * execute the transport of the payload. Independent of the underlying layer this function is blocking
     *
     * @param payload object with (optional) extensions
     * @param lt_transport use b_transport instead of nb_transport*
     */
    virtual void transport(tlm::tlm_generic_payload& payload, bool lt_transport = false) = 0;
    /**
     * send a response to a backward transaction if not immediately answered
     *
     * @param payload object with (optional) extensions
     * @param sync if true send with next rising clock edge of the pe otherwise send it immediately
     */
    virtual void snoop_resp(tlm::tlm_generic_payload& payload, bool sync = false) = 0;
};

template <type TYPE> struct intor_bw : public sc_core::sc_interface {
    /**
     * callback from the pe top if there is a backward transaction e.g. a snoop
     *
     * @param payload object with (optional) extensions
     * return the latency until reponse is sent by the protocol engine
     */
    virtual unsigned transport(tlm::tlm_generic_payload& payload) = 0;
};

struct intor_fw_b : public intor_fw<type::BL> {};
struct intor_fw_nb : public intor_fw<type::NB> {};
struct intor_bw_b : public intor_bw<type::BL> {};
struct intor_bw_nb : public intor_bw<type::NB> {};
} // namespace pe
} // namespace scc
} // namespace tlm

#endif /* SCC_INCL_TLM_PE_INTOR_IF_H_ */
