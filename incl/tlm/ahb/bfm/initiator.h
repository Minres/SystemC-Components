/*
 * ahb_target_bfm.h
 *
 *  Created on: 11.12.2019
 *      Author: developer
 */

#ifndef _INITIATOR_H_
#define _INITIATOR_H_

#include <scc/target_mixin.h>
#include <tlm>
#include <tlm_utils/peq_with_get.h>
#include <type_traits>

namespace ahb {
namespace bfm {

template <unsigned WIDTH> class initiator : sc_core::sc_module {
    static constexpr bool is_larger(unsigned x) { return x > 64U; }
    using data_t = typename std::conditional<is_larger(WIDTH), sc_dt::sc_biguint<WIDTH>, sc_dt::sc_uint<WIDTH>>::type;

public:
    sc_core::sc_in<bool> HCLK_i{"HCLK_i"};
    sc_core::sc_in<bool> HRESETn_i{"HRESETn_i"};
    sc_core::sc_out<sc_dt::sc_uint<32>> HADDR_o{"HADDR_o"};
    sc_core::sc_out<sc_dt::sc_uint<3>> HBURST_o{"HBURST_o"};
    sc_core::sc_out<bool> HMASTLOCK_o{"HMASTLOCK_o"};
    sc_core::sc_out<sc_dt::sc_uint<4>> HPROT_o{"HPROT_o"};
    sc_core::sc_out<sc_dt::sc_uint<3>> HSIZE_o{"HSIZE_o"};
    sc_core::sc_out<sc_dt::sc_uint<2>> HTRANS_o{"HTRANS_o"};
    sc_core::sc_out<data_t> HWDATA_o{"HWDATA_o"};
    sc_core::sc_out<bool> HWRITE_o{"HWRITE_o"};
    sc_core::sc_in<data_t> HRDATA_i{"HRDATA_i"};
    sc_core::sc_in<bool> HREADY_i{"HREADY_i"};
    sc_core::sc_in<bool> HRESP_i{"HRESP_i"};

    scc::target_mixin<tlm::tlm_target_socket<WIDTH>> tsckt{"tsckt"};

    initiator(const sc_core::sc_module_name& nm);
    virtual ~initiator() = default;

private:
    struct data_phase_desc {
        tlm::tlm_generic_payload* gp;
        unsigned size;
        unsigned length;
    };
    void bus_task();

    sc_core::sc_mutex addr_phase, data_phase;
    tlm_utils::peq_with_get<tlm::tlm_generic_payload> inqueue{"inqueue"};
};

} /* namespace bfm */
} /* namespace ahb */

#endif /* AHB_TARGET_BFM_H_ */
