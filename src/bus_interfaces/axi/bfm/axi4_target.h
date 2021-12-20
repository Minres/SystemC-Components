/*
 * initiator.h
 *
 *  Created on: Dec 20, 2021
 *      Author: eyck
 */

#ifndef _AXI_BFM_TARGET_H_
#define _AXI_BFM_TARGET_H_

#include <axi/axi_tlm.h>
#include <axi/bfm/axi_signal_if.h>
#include <systemc>
namespace axi {
namespace bfm {

template<typename CFG>
struct axi4_target: public sc_core::sc_module
, public aw_ch<CFG, slave_types>
, public wdata_ch<CFG, slave_types>
, public b_ch<CFG, slave_types>
, public ar_ch<CFG, slave_types>
, public rresp_ch<CFG, slave_types>
{

    axi::axi_initiator_socket<CFG::BUSWIDTH> isckt{"isckt"};

    axi4_target(sc_core::sc_module_name const& nm){}
};

}
}
#endif /* _AXI_BFM_TARGET_H_ */
