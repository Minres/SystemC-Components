#ifndef AXI_PIN2TLM_ADAPTOR_H_
#define AXI_PIN2TLM_ADAPTOR_H_

#include "tlm_utils/simple_initiator_socket.h"
#include <systemc>
#include <tlm>


namespace axi_bfm {

class axi_pin2tlm_adaptor: public sc_core::sc_module {
public:
    SC_HAS_PROCESS(axi_pin2tlm_adaptor);

    axi_pin2tlm_adaptor(sc_core::sc_module_name nm);

    tlm_utils::simple_initiator_socket<axi_pin2tlm_adaptor> output_socket{"output_socket"};

    sc_core::sc_in<bool> clk_i{"clk_i"};
    sc_core::sc_in<bool> resetn_i{"resetn_i"};

    // Write address channel signals
    sc_core::sc_in<bool>                   aw_id_i       {"aw_id_i"};
    sc_core::sc_in<sc_dt::sc_uint<32> >    aw_addr_i     {"aw_addr_i"};
    sc_core::sc_out<bool>                  aw_ready_o    {"aw_ready_o"};
    sc_core::sc_in<bool>                   aw_lock_i     {"aw_lock_i"};
    sc_core::sc_in<bool>                   aw_valid_i    {"aw_valid_i"};
    sc_core::sc_in<sc_dt::sc_uint<3> >     aw_prot_i     {"aw_prot_i"};
    sc_core::sc_in<sc_dt::sc_uint<3> >     aw_size_i     {"aw_size_i"};
    sc_core::sc_in<sc_dt::sc_uint<4> >     aw_cache_i    {"aw_cache_i"};
    sc_core::sc_in<sc_dt::sc_uint<2> >     aw_burst_i    {"aw_burst_i"};
    sc_core::sc_in<sc_dt::sc_uint<4> >     aw_qos_i      {"Aaw_qos_i"};
    sc_core::sc_in<sc_dt::sc_uint<4> >     aw_region_i   {"aw_region_i"};
    sc_core::sc_in<sc_dt::sc_uint<8> >     aw_len_i      {"aw_len_i"};

    // write data channel signals
    sc_core::sc_in<sc_dt::sc_biguint<512> >w_data_i      {"w_data_i"};
    sc_core::sc_in<sc_dt::sc_uint<64> >    w_strb_i      {"w_strb_i"};
    sc_core::sc_in<bool>                   w_last_i      {"w_last_i"};
    sc_core::sc_in<bool>                   w_valid_i     {"w_valid_i"};
    sc_core::sc_out<bool>                  w_ready_o     {"w_ready_o"};

    // write response channel signals
    sc_core::sc_out<bool>                  b_valid_o     {"b_valid_o"};
    sc_core::sc_in<bool>                   b_ready_i     {"b_ready_i"};
    sc_core::sc_out<bool>                  b_id_o        {"b_id_o"};
    sc_core::sc_out<sc_dt::sc_uint<2> >    b_resp_o      {"b_resp_o"};

    // read address channel signals
    sc_core::sc_in<bool>                   ar_id_i       {"ar_id_i"};
    sc_core::sc_in<sc_dt::sc_uint<32> >    ar_addr_i     {"ar_addr_i"};
    sc_core::sc_in<sc_dt::sc_uint<8> >     ar_len_i      {"ar_len_i"};
    sc_core::sc_in<sc_dt::sc_uint<3> >     ar_size_i     {"ar_size_i"};
    sc_core::sc_in<sc_dt::sc_uint<2> >     ar_burst_i    {"ar_burst_i"};
    sc_core::sc_in<bool>                   ar_lock_i     {"ar_lock_i"};
    sc_core::sc_in<sc_dt::sc_uint<4> >     ar_cache_i    {"ar_cache_i"};
    sc_core::sc_in<sc_dt::sc_uint<3> >     ar_prot_i     {"ar_prot_i"};
    sc_core::sc_in<sc_dt::sc_uint<4> >     ar_qos_i      {"ar_qos_i"};
    sc_core::sc_in<sc_dt::sc_uint<4> >     ar_region_i   {"ar_region_i"};
    sc_core::sc_in<bool>                   ar_valid_i    {"ar_valid_i"};
    sc_core::sc_out<bool>                  ar_ready_o    {"ar_ready_o"};

    // Read data channel signals
    sc_core::sc_out<bool>                    r_id_o      {"r_id_o"};
    sc_core::sc_out<sc_dt::sc_biguint<512> > r_data_o    {"r_data_o"};
    sc_core::sc_out<sc_dt::sc_uint<2> >      r_resp_o    {"r_resp_o"};
    sc_core::sc_out<bool>                    r_last_o    {"r_last_o"};
    sc_core::sc_out<bool>                    r_valid_o   {"r_valid_o"};
    sc_core::sc_in<bool>                     r_ready_i   {"r_ready_i"};

private:
    void bus_thread();

    enum { OKAY = 0x0, EXOKAY = 0x1, SLVERR = 0x2, DECERR = 0x3 };

};

} // namespace axi_bfm

#endif /* AXI_PIN2TLM_ADAPTOR_H_ */
