/*
 * pe_fw.h
 *
 *  Created on: Dec 16, 2020
 *      Author: eyck
 */

#ifndef SCC_INCL_TLM_PE_INTOR_IF_H_
#define SCC_INCL_TLM_PE_INTOR_IF_H_

#include <tlm>

namespace tlm {
namespace pe {
struct intor_fw: public sc_core::sc_interface {
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

struct intor_bw: public sc_core::sc_interface {
    /**
     * callback from the pe top if there is a backward transaction e.g. a snoop
     *
     * @param payload object with (optional) extensions
     * return the latency until reponse is sent by the protocol engine
     */
    virtual unsigned transport(tlm::tlm_generic_payload& payload) = 0;

};
}
}



#endif /* SCC_INCL_TLM_PE_INTOR_IF_H_ */
