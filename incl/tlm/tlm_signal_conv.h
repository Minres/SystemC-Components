/*
 * tlm_signal_conv.h
 *
 *  Created on: 03.10.2018
 *      Author: eyck
 */

#ifndef _TLM_TLM_SIGNAL_CONV_H_
#define _TLM_TLM_SIGNAL_CONV_H_

#include "tlm_signal_sockets.h"
#include "tlm_signal_gp.h"
#include <scc/peq.h>
#include <deque>

namespace tlm {

template<typename TYPE>
class tlm_signal2sc_signal:
        public sc_core::sc_module,
        public tlm_signal_fw_transport_if<TYPE, tlm_signal_baseprotocol_types<TYPE>> {

    using protocol_types = tlm_signal_baseprotocol_types<TYPE>;
    using payload_type = typename protocol_types::tlm_payload_type;
    using phase_type = typename protocol_types::tlm_phase_type;

    SC_HAS_PROCESS(tlm_signal2sc_signal);

    tlm_signal_target_socket<TYPE> t_i;

    sc_core::sc_out<TYPE> s_o;

    tlm_signal2sc_signal(sc_core::sc_module_name nm) : sc_core::sc_module(nm) {
        t_i.bind(*this);
        SC_METHOD(que_cb);
        sensitive<<que.event();
    }

private:
    tlm_sync_enum nb_transport_fw(payload_type& gp, phase_type& phase, sc_core::sc_time& delay) {
        que.notify(gp.get_value(), delay);
        return TLM_COMPLETED;
    }

    void que_cb(){
        while(auto oi = que.get_next())
            s_o.write(oi.get());
    }
    scc::peq<TYPE> que;
};

template<typename TYPE>
class sc_signal2tlm_signal:
        public sc_core::sc_module,
        public tlm_signal_bw_transport_if<TYPE, tlm_signal_baseprotocol_types<TYPE>> {

    using protocol_types = tlm_signal_baseprotocol_types<TYPE>;
    using payload_type = typename protocol_types::tlm_payload_type;
    using phase_type = typename protocol_types::tlm_phase_type;

    SC_HAS_PROCESS(sc_signal2tlm_signal);

    sc_core::sc_in<TYPE> s_i;

    tlm_signal_initiator_socket<TYPE> t_o;

    sc_signal2tlm_signal(sc_core::sc_module_name nm) : sc_core::sc_module(nm) {
        t_o.bind(*this);
        SC_METHOD(sig_cb);
        sensitive<<s_i;
    }

private:

    tlm_sync_enum nb_transport_bw(payload_type& gp, phase_type& phase, sc_core::sc_time& delay) {
        return TLM_COMPLETED;
    }

    void sig_cb(){
        tlm::tlm_phase phase(tlm::BEGIN_REQ);
        sc_core::sc_time delay;
        auto* gp = payload_type::create();
        gp->acquire();
        gp->set_value(s_i.read());
        t_o->nb_transport_fw(*gp, phase, delay);
        gp->release();
    }

};

}
#endif /* _TLM_TLM_SIGNAL_CONV_H_ */
