/*
 * tlm_signal.h
 *
 *  Created on: 03.07.2018
 *      Author: eyck
 */

#ifndef _TLM_TLM_SIGNAL_SOCKETS_H_
#define _TLM_TLM_SIGNAL_SOCKETS_H_

#include <scc/peq.h>
#include "scc/report.h"

#include <tlm_core/tlm_2/tlm_sockets/tlm_initiator_socket.h>
#include <tlm_core/tlm_2/tlm_sockets/tlm_target_socket.h>
#include <sysc/kernel/sc_object.h>

namespace tlm {

template<typename SIG>
class tlm_signal_gp;

template <typename SIG=bool>
struct tlm_signal_baseprotocol_types {
  using tlm_signal_type = SIG ;
  using tlm_payload_type = tlm_signal_gp<tlm_signal_type>;
  using tlm_phase_type = tlm_phase;
};

template<typename SIG=bool, typename TYPES=tlm_signal_baseprotocol_types<SIG>>
struct tlm_signal_fw_transport_if: public virtual sc_core::sc_interface {
// virtual void b_transport(typename TYPES::tlm_payload_type&, sc_core::sc_time&) = 0;
  virtual tlm_sync_enum nb_transport_fw(typename TYPES::tlm_payload_type&, tlm_phase&, sc_core::sc_time&) = 0;
};

template<typename SIG=bool, typename TYPES=tlm_signal_baseprotocol_types<SIG>>
struct tlm_signal_bw_transport_if: public virtual sc_core::sc_interface {
    virtual tlm_sync_enum nb_transport_bw(typename TYPES::tlm_payload_type&, tlm_phase&, sc_core::sc_time&) = 0;
};

template <typename SIG=bool,
          typename TYPES = tlm_signal_baseprotocol_types<SIG>,
          int N = 1,
          sc_core::sc_port_policy POL = sc_core::SC_ONE_OR_MORE_BOUND>
struct tlm_signal_initiator_socket :
  public tlm_base_initiator_socket<0,
  tlm_signal_fw_transport_if<SIG, TYPES>,
  tlm_signal_bw_transport_if<SIG, TYPES>,N, POL>
{
    using tlm_signal_type = SIG;
    using protocol_types = TYPES;
    using transaction_type = typename TYPES::tlm_payload_type;
    using phase_type = typename TYPES::tlm_phase_type;

    tlm_signal_initiator_socket() :
      tlm_base_initiator_socket<0,
      tlm_signal_fw_transport_if<SIG, TYPES>,
      tlm_signal_bw_transport_if<SIG, TYPES>,
      N, POL>()
    {
    }

    explicit tlm_signal_initiator_socket(const char* name) :
      tlm_base_initiator_socket<0,
      tlm_signal_fw_transport_if<SIG, TYPES>,
      tlm_signal_bw_transport_if<SIG, TYPES>,
      N, POL>(name)
    {
    }

    virtual const char* kind() const {
      return "tlm_signal_initiator_socket";
    }

    virtual sc_core::sc_type_index get_protocol_types() const {
      return typeid(TYPES);
    }
};

template <typename SIG=bool, typename TYPES = tlm_signal_baseprotocol_types<SIG>, int N = 1>
using  tlm_signal_opt_initiator_socket = struct tlm_signal_initiator_socket<SIG, TYPES, N, sc_core::SC_ZERO_OR_MORE_BOUND>;

template<typename SIG=bool,
         typename TYPES=tlm_signal_baseprotocol_types<SIG>,
         int N = 1,
         sc_core::sc_port_policy POL = sc_core::SC_ONE_OR_MORE_BOUND>
struct tlm_signal_target_socket :
  public tlm_base_target_socket<0, tlm_signal_fw_transport_if<SIG, TYPES>,
                                tlm_signal_bw_transport_if<SIG, TYPES>,N, POL>
{
    using tlm_signal_type = SIG;
    using protocol_types = TYPES;
    using transaction_type = typename TYPES::tlm_payload_type;
    using phase_type = typename TYPES::tlm_phase_type;

    tlm_signal_target_socket() :
      tlm_base_target_socket<0,
      tlm_signal_fw_transport_if<TYPES>,
      tlm_signal_bw_transport_if<TYPES>,
      N, POL>()
    {
    }

    explicit tlm_signal_target_socket(const char* name) :
      tlm_base_target_socket<0,
      tlm_signal_fw_transport_if<SIG, TYPES>,
      tlm_signal_bw_transport_if<SIG, TYPES>,
      N, POL>(name)
    {
    }

    virtual const char* kind() const {
      return "tlm_signal_target_socket";
    }

    virtual sc_core::sc_type_index get_protocol_types() const {
      return typeid(TYPES);
    }
};

template <typename SIG=bool, typename TYPES = tlm_signal_baseprotocol_types<SIG>, int N = 1>
using  tlm_signal_opt_target_socket = struct tlm_signal_target_socket<SIG, TYPES, N, sc_core::SC_ZERO_OR_MORE_BOUND>;


template<typename SIG=bool, typename TYPES = tlm_signal_baseprotocol_types<SIG>, int N = 32>
struct tlm_signal:
        public sc_core::sc_module,
        public tlm_signal_fw_transport_if<SIG, TYPES>,
        public tlm_signal_bw_transport_if<SIG, TYPES>
{
    using tlm_signal_type = SIG;
    using protocol_types = TYPES;
    using payload_type = typename TYPES::tlm_payload_type;
    using phase_type = typename TYPES::tlm_phase_type;

    SC_HAS_PROCESS(tlm_signal);

    tlm_signal_opt_target_socket<tlm_signal_type, protocol_types, N> in;

    tlm_signal_opt_initiator_socket<tlm_signal_type, protocol_types, N> out;

    tlm_signal(sc_core::sc_module_name nm)
    : sc_core::sc_module(nm)
    , in(sc_core::sc_gen_unique_name("in"))
    , out(sc_core::sc_gen_unique_name("out"))
    {
        in.bind(*(tlm_signal_fw_transport_if<tlm_signal_type, protocol_types>*)this);
        out.bind(*(tlm_signal_bw_transport_if<tlm_signal_type, protocol_types>*)this);
        SC_METHOD(que_cb);
        sensitive<<que.event();
    }

    void trace( sc_core::sc_trace_file* tf ) const override;

    const char* kind() const override { return "tlm_signal"; }

    tlm_sync_enum nb_transport_fw(payload_type&, phase_type&, sc_core::sc_time&) override;

    tlm_sync_enum nb_transport_bw(payload_type&, phase_type&, sc_core::sc_time&) override;

private:
    void que_cb();
    scc::peq<tlm_signal_type> que;
    tlm_signal_type value;
};

template<typename SIG , typename TYPES, int N>
void tlm_signal<SIG, TYPES, N>::trace(sc_core::sc_trace_file* tf) const {
    sc_trace(tf, value, name());
}


template<typename SIG, typename TYPES, int N>
tlm_sync_enum tlm::tlm_signal<SIG, TYPES, N>::nb_transport_fw(payload_type& gp, phase_type& phase, sc_core::sc_time& delay) {
    que.notify(gp.get_value(), delay);
    auto& p = out.get_base_port();
    for(size_t i=0; i<p.size(); ++i){
        p.get_interface(i)->nb_transport_fw(gp, phase, delay);
    }
}

template<typename SIG, typename TYPES, int N>
tlm_sync_enum tlm::tlm_signal<SIG, TYPES, N>::nb_transport_bw(payload_type& gp, phase_type& phase, sc_core::sc_time& delay) {
    auto& p = in.get_base_port();
    for(size_t i=0; i<p.size(); ++i){
        p.get_interface(i)->nb_transport_bw(gp, phase, delay);
    }
}

template<typename SIG, typename TYPES, int N>
void tlm::tlm_signal<SIG, TYPES, N>::que_cb(){
    while(auto oi = que.get_next())
        value=oi.value();
}

}

#endif /* _TLM_TLM_SIGNAL_SOCKETS_H_ */
