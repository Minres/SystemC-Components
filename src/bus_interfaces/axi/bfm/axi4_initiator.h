/*
 * initiator.h
 *
 *  Created on: Dec 20, 2021
 *      Author: eyck
 */

#ifndef _AXI_BFM_INITIATOR_H_
#define _AXI_BFM_INITIATOR_H_

#include <axi/axi_tlm.h>
#include <axi/bfm/axi_signal_if.h>
#include <systemc>
namespace axi {
namespace bfm {


template<typename CFG>
struct axi4_initiator: public sc_core::sc_module
, public aw_ch<CFG, master_types>
, public wdata_ch<CFG, master_types>
, public b_ch<CFG, master_types>
, public ar_ch<CFG, master_types>
, public rresp_ch<CFG, master_types>
{

    axi::axi_target_socket<CFG::BUSWIDTH> tsckt{"tsckt"};

    axi4_initiator(sc_core::sc_module_name const& nm){}
};

}
}
#endif /* _AXI_BFM_INITIATOR_H_ */
