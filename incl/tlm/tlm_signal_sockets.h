/*
 * tlm_signal.h
 *
 *  Created on: 03.07.2018
 *      Author: eyck
 */

#ifndef _TLM_TLM_SIGNAL_SOCKETS_H_
#define _TLM_TLM_SIGNAL_SOCKETS_H_

#include <tlm_core/tlm_2/tlm_sockets/tlm_initiator_socket.h>
#include <tlm_core/tlm_2/tlm_sockets/tlm_target_socket.h>

namespace tlm {

template<typename SIG>
class tlm_signal_gp;

template<typename SIG=bool>
class tlm_signal_fw_transport_if: public virtual sc_core::sc_interface {
  virtual void b_transport(tlm_signal_gp<SIG>&, sc_core::sc_time&) = 0;
  virtual tlm_sync_enum nb_transport_fw(tlm_signal_gp<SIG>&, tlm_phase&, sc_core::sc_time&) = 0;
};

template<typename SIG=bool>
class tlm_signal_bw_transport_if: public virtual sc_core::sc_interface {
    virtual tlm_sync_enum nb_transport_bw(tlm_signal_gp<SIG>&, tlm_phase&, sc_core::sc_time&) = 0;
};

template <typename SIG=bool>
struct tlm_signal_baseprotocol_types {
  typedef SIG tlm_signal_type;
  typedef tlm_signal_gp<tlm_signal_type> tlm_payload_type;
  typedef tlm_phase tlm_phase_type;
};

template <typename SIG=bool,
          typename TYPES = tlm_signal_baseprotocol_types<SIG>,
          int N = 1,
          sc_core::sc_port_policy POL = sc_core::SC_ONE_OR_MORE_BOUND>
struct tlm_signal_initiator_socket :
  public tlm_base_initiator_socket<0,
  tlm_signal_fw_transport_if<TYPES>,
  tlm_signal_bw_transport_if<TYPES>,N, POL>
{

    tlm_signal_initiator_socket() :
      tlm_base_initiator_socket<0,
      tlm_signal_fw_transport_if<TYPES>,
      tlm_signal_bw_transport_if<TYPES>,
      N, POL>()
    {
    }

    explicit tlm_signal_initiator_socket(const char* name) :
      tlm_base_initiator_socket<0,
      tlm_signal_fw_transport_if<TYPES>,
      tlm_signal_bw_transport_if<TYPES>,
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

template<typename SIG=bool,
         typename TYPES=tlm_signal_baseprotocol_types<SIG>,
         int N = 1,
         sc_core::sc_port_policy POL = sc_core::SC_ONE_OR_MORE_BOUND>
struct tlm_signal_target_socket :
  public tlm_base_target_socket<0, tlm_signal_fw_transport_if<TYPES>,
                                tlm_signal_bw_transport_if<TYPES>,N, POL>
{
    tlm_signal_target_socket() :
      tlm_base_target_socket<0,
      tlm_signal_fw_transport_if<TYPES>,
      tlm_signal_bw_transport_if<TYPES>,
      N, POL>()
    {
    }

    explicit tlm_signal_target_socket(const char* name) :
      tlm_base_target_socket<0,
      tlm_signal_fw_transport_if<TYPES>,
      tlm_signal_bw_transport_if<TYPES>,
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

}




#endif /* _TLM_TLM_SIGNAL_SOCKETS_H_ */
