/*******************************************************************************
 * Copyright 2018 MINRES Technologies GmbH
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

#ifndef _TLM_TLM_SIGNAL_CONV_H_
#define _TLM_TLM_SIGNAL_CONV_H_

#include "tlm_signal_gp.h"
#include "tlm_signal_sockets.h"
#include <deque>
#include <scc/peq.h>

//! @brief SystemC TLM
namespace tlm {
//! @brief SCC TLM utilities
namespace scc {

template <typename TYPE>
struct tlm_signal2sc_signal : public sc_core::sc_module,
                              public tlm_signal_fw_transport_if<TYPE, tlm_signal_baseprotocol_types<TYPE>> {

    using protocol_types = tlm_signal_baseprotocol_types<TYPE>;
    using payload_type = typename protocol_types::tlm_payload_type;
    using phase_type = typename protocol_types::tlm_phase_type;

    SC_HAS_PROCESS(tlm_signal2sc_signal); // NOLINT

    tlm_signal_target_socket<TYPE> t_i{"t_i"};

    sc_core::sc_out<TYPE> s_o{"s_o"};

    tlm_signal2sc_signal(sc_core::sc_module_name nm)
    : sc_core::sc_module(nm) {
        t_i.bind(*this);
        SC_METHOD(que_cb);
        sensitive << que.event();
    }

private:
    tlm_sync_enum nb_transport_fw(payload_type& gp, phase_type& phase, sc_core::sc_time& delay) {
        que.notify(gp.get_value(), delay);
        return TLM_COMPLETED;
    }

    void que_cb() {
        while(auto oi = que.get_next())
            s_o.write(oi.get());
    }
    ::scc::peq<TYPE> que;
};

template <typename TYPE>
struct sc_signal2tlm_signal : public sc_core::sc_module,
                              public tlm_signal_bw_transport_if<TYPE, tlm_signal_baseprotocol_types<TYPE>> {

    using protocol_types = tlm_signal_baseprotocol_types<TYPE>;
    using payload_type = typename protocol_types::tlm_payload_type;
    using phase_type = typename protocol_types::tlm_phase_type;

    SC_HAS_PROCESS(sc_signal2tlm_signal); // NOLINT

    sc_core::sc_in<TYPE> s_i{"s_i"};

    tlm_signal_initiator_socket<TYPE> t_o{"t_o"};

    sc_signal2tlm_signal(sc_core::sc_module_name nm)
    : sc_core::sc_module(nm) {
        t_o.bind(*this);
        SC_METHOD(sig_cb);
        sensitive << s_i;
    }

private:
    tlm_sync_enum nb_transport_bw(payload_type& gp, phase_type& phase, sc_core::sc_time& delay) {
        return TLM_COMPLETED;
    }

    void sig_cb() {
        tlm::tlm_phase phase(tlm::BEGIN_REQ);
        sc_core::sc_time delay;
        auto* gp = payload_type::create();
        gp->acquire();
        gp->set_value(s_i.read());
        t_o->nb_transport_fw(*gp, phase, delay);
        gp->release();
    }
};
} // namespace scc
} // namespace tlm
#endif /* _TLM_TLM_SIGNAL_CONV_H_ */
