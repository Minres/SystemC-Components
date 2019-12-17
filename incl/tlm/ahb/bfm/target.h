/*
 * ahb_target_bfm.h
 *
 *  Created on: 11.12.2019
 *      Author: developer
 */

#ifndef _TARGET_H_
#define _TARGET_H_

#include <scc/initiator_mixin.h>
#include <tlm>

namespace tlm {
namespace ahb {
namespace bfm {

template<unsigned WIDTH>
class target : sc_core::sc_module{
public:
    sc_core::sc_in<bool>                   HCLK_i{"HCLK_i"};
    sc_core::sc_in<bool>                   HRESETn_i{"HRESETn_i"};
    sc_core::sc_in<sc_dt::sc_uint<32>>     HADDR_i{"HADDR_i"};
    sc_core::sc_in<sc_dt::sc_uint<3>>      HBURST_i{"HBURST_i"};
    sc_core::sc_in<bool>                   HMASTLOCK_i{"HMASTLOCK_i"};
    sc_core::sc_in<sc_dt::sc_uint<4>>      HPROT_i{"HPROT_i"};
    sc_core::sc_in<sc_dt::sc_uint<3>>      HSIZE_i{"HSIZE_i"};
    sc_core::sc_in<sc_dt::sc_uint<2>>      HTRANS_i{"HTRANS_i"};
    sc_core::sc_in<sc_dt::sc_biguint<WIDTH>>  HWDATA_i{"HWDATA_i"};
    sc_core::sc_in<bool>                   HWRITE_i{"HWRITE_i"};
    sc_core::sc_in<bool>                   HSEL_i{"HSEL_i"};
    sc_core::sc_out<sc_dt::sc_biguint<WIDTH>> HRDATA_o{"HRDATA_o"};
    sc_core::sc_out<bool>                  HREADY_o{"HREADY_o"};
    sc_core::sc_out<bool>                  HRESP_o{"HRESP_o"};

    scc::initiator_mixin<tlm::tlm_initiator_socket<WIDTH>> isckt{"isckt"};

    target(const sc_core::sc_module_name& nm);
    virtual ~target();
private:
    void bfm_thread();
    sc_core::sc_fifo<tlm::tlm_generic_payload*> active{"active_tx", 1};
};

} /* namespace bfm */
} /* namespace ahb */
} /* namespace tlm */


#endif /* AHB_TARGET_BFM_H_ */
