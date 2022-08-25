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

#ifndef _TLM_TLM_SIGNAL_H_
#define _TLM_TLM_SIGNAL_H_

#include "tlm_signal_gp.h"
#include "tlm_signal_sockets.h"
#include <scc/peq.h>

//! @brief SystemC TLM
namespace tlm {
//! @brief SCC TLM utilities
namespace scc {

template <typename SIG = bool, typename TYPES = tlm_signal_baseprotocol_types<SIG>, int N = 32>
struct tlm_signal : public sc_core::sc_module,
                    public tlm_signal_fw_transport_if<SIG, TYPES>,
                    public tlm_signal_bw_transport_if<SIG, TYPES>,
                    sc_core::sc_signal_in_if<SIG> {
    using tlm_signal_type = SIG;
    using protocol_types = TYPES;
    using payload_type = typename TYPES::tlm_payload_type;
    using phase_type = typename TYPES::tlm_phase_type;

    SC_HAS_PROCESS(tlm_signal); // NOLINT

    tlm_signal_opt_target_socket<tlm_signal_type, protocol_types, N> in;

    tlm_signal_opt_initiator_socket<tlm_signal_type, protocol_types, N> out;

    tlm_signal(sc_core::sc_module_name nm)
    : sc_core::sc_module(nm)
    , in(sc_core::sc_gen_unique_name("in"))
    , out(sc_core::sc_gen_unique_name("out")) {
        in.bind(*(tlm_signal_fw_transport_if<tlm_signal_type, protocol_types>*)this);
        out.bind(*(tlm_signal_bw_transport_if<tlm_signal_type, protocol_types>*)this);
        SC_METHOD(que_cb);
        sensitive << que.event();
    }

    void trace(sc_core::sc_trace_file* tf) const override;

    const char* kind() const override { return "tlm_signal"; }

    tlm_sync_enum nb_transport_fw(payload_type&, phase_type&, sc_core::sc_time&) override;

    tlm_sync_enum nb_transport_bw(payload_type&, phase_type&, sc_core::sc_time&) override;

    // get the value changed event
    const sc_core::sc_event& value_changed_event() const override { return value.value_changed_event(); }
    // read the current value
    const SIG& read() const override { return value.read(); }
    // get a reference to the current value (for tracing)
    const SIG& get_data_ref() const override { return value.get_data_ref(); }
    // was there a value changed event?
    bool event() const override { return false; }

    const sc_core::sc_event& default_event() const override { return value.default_event(); }

    const sc_core::sc_event& posedge_event() const override { return value.posedge_event(); }

    const sc_core::sc_event& negedge_event() const override { return value.negedge_event(); }

    bool posedge() const override { return value.posedge(); }

    bool negedge() const override { return value.posedge(); };

private:
    void que_cb();
    ::scc::peq<tlm_signal_type> que;
    sc_core::sc_signal<tlm_signal_type> value;
};

template <typename SIG, typename TYPES, int N> void tlm_signal<SIG, TYPES, N>::trace(sc_core::sc_trace_file* tf) const {
    sc_trace(tf, value, name());
}

template <typename SIG, typename TYPES, int N>
tlm_sync_enum tlm_signal<SIG, TYPES, N>::nb_transport_fw(payload_type& gp, phase_type& phase, sc_core::sc_time& delay) {
    que.notify(gp.get_value(), delay);
    auto& p = out.get_base_port();
    for(size_t i = 0; i < p.size(); ++i) {
        p.get_interface(i)->nb_transport_fw(gp, phase, delay);
    }
    return TLM_COMPLETED;
}

template <typename SIG, typename TYPES, int N>
tlm_sync_enum tlm_signal<SIG, TYPES, N>::nb_transport_bw(payload_type& gp, phase_type& phase, sc_core::sc_time& delay) {
    auto& p = in.get_base_port();
    for(size_t i = 0; i < p.size(); ++i) {
        p.get_interface(i)->nb_transport_bw(gp, phase, delay);
    }
    return TLM_COMPLETED;
}

template <typename SIG, typename TYPES, int N> void tlm_signal<SIG, TYPES, N>::que_cb() {
    while(auto oi = que.get_next())
        value.write(oi.get());
}
} // namespace scc
} // namespace tlm
#endif /* _TLM_TLM_SIGNAL_H_ */
