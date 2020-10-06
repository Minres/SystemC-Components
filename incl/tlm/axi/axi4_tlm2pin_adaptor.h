
#ifndef AXI_BFM__INITIATOR_H_
#define AXI_BFM__INITIATOR_H_

#include "scv4tlm/tlm_rec_target_socket.h"
#include <scc/target_mixin.h>

#include <systemc>
#include <tlm>


namespace axi_bfm {

class axi4_tlm2pin_adaptor: public sc_core::sc_module {
public:
    SC_HAS_PROCESS(axi4_tlm2pin_adaptor);

    axi4_tlm2pin_adaptor(sc_core::sc_module_name nm);

    scc::target_mixin<scv4tlm::tlm_rec_target_socket<32>> input_socket{"input_socket"};

    sc_core::sc_in<bool> clk_i{"clk_i"};
    sc_core::sc_in<bool> resetn_i{"resetn_i"};

    // Write address channel signals
    sc_core::sc_out<bool>                   aw_id_o       {"aw_id_o"};
    sc_core::sc_out<sc_dt::sc_uint<32> >    aw_addr_o     {"aw_addr_o"};
    sc_core::sc_in<bool>                    aw_ready_i    {"aw_ready_i"};
    sc_core::sc_out<bool>                   aw_lock_o     {"aw_lock_o"};
    sc_core::sc_out<bool>                   aw_valid_o    {"aw_valid_o"};
    sc_core::sc_out<sc_dt::sc_uint<3> >     aw_prot_o     {"aw_prot_o"};
    sc_core::sc_out<sc_dt::sc_uint<3> >     aw_size_o     {"aw_size_o"};
    sc_core::sc_out<sc_dt::sc_uint<4> >     aw_cache_o    {"aw_cache_o"};
    sc_core::sc_out<sc_dt::sc_uint<2> >     aw_burst_o    {"aw_burst_o"};
    sc_core::sc_out<sc_dt::sc_uint<4> >     aw_qos_o      {"aw_qos_o"};
    sc_core::sc_out<sc_dt::sc_uint<4> >     aw_region_o   {"aw_region_o"};
    sc_core::sc_out<sc_dt::sc_uint<8> >     aw_len_o      {"aw_len_o"};

    // write data channel signals
    sc_core::sc_out<sc_dt::sc_uint<32> >    w_data_o      {"w_data_o"};
    sc_core::sc_out<sc_dt::sc_uint<4> >     w_strb_o      {"w_strb_o"};
    sc_core::sc_out<bool>                   w_last_o      {"w_last_o"};
    sc_core::sc_out<bool>                   w_valid_o     {"w_valid_o"};
    sc_core::sc_in<bool>                    w_ready_i     {"w_ready_i"};

    // write response channel signals
    sc_core::sc_in<bool>                    b_valid_i     {"b_valid_i"};
    sc_core::sc_out<bool>                   b_ready_o     {"b_ready_o"};
    sc_core::sc_in<bool>                    b_id_i        {"b_id_i"};
    sc_core::sc_in<sc_dt::sc_uint<2> >      b_resp_i      {"b_resp_i"};

    // read address channel signals
    sc_core::sc_out<bool>                   ar_id_o       {"ar_id_o"};
    sc_core::sc_out<sc_dt::sc_uint<32> >    ar_addr_o     {"ar_addr_o"};
    sc_core::sc_out<sc_dt::sc_uint<8> >     ar_len_o      {"ar_len_o"};
    sc_core::sc_out<sc_dt::sc_uint<3> >     ar_size_o     {"ar_size_o"};
    sc_core::sc_out<sc_dt::sc_uint<2> >     ar_burst_o    {"ar_burst_o"};
    sc_core::sc_out<bool>                   ar_lock_o     {"ar_lock_o"};
    sc_core::sc_out<sc_dt::sc_uint<4> >     ar_cache_o    {"ar_cache_o"};
    sc_core::sc_out<sc_dt::sc_uint<3> >     ar_prot_o     {"ar_prot_o"};
    sc_core::sc_out<sc_dt::sc_uint<4> >     ar_qos_o      {"ar_qos_o"};
    sc_core::sc_out<sc_dt::sc_uint<4> >     ar_region_o   {"ar_region_o"};
    sc_core::sc_out<bool>                   ar_valid_o    {"ar_valid_o"};
    sc_core::sc_in<bool>                    ar_ready_i    {"ar_ready_i"};

    // Read data channel signals
    sc_core::sc_in<bool>                    r_id_i        {"r_id_i"};
    sc_core::sc_in<sc_dt::sc_uint<32> >     r_data_i      {"r_data_i"};
    sc_core::sc_in<sc_dt::sc_uint<2> >      r_resp_i      {"r_resp_i"};
    sc_core::sc_in<bool>                    r_last_i      {"r_last_i"};
    sc_core::sc_in<bool>                    r_valid_i     {"r_valid_i"};
    sc_core::sc_out<bool>                   r_ready_o     {"r_ready_o"};

    void b_transport(tlm::tlm_generic_payload& trans, sc_core::sc_time& delay);

private:
    void reset();
    void write_addr_channel(const tlm::tlm_generic_payload& trans);
    void write_data_channel(const tlm::tlm_generic_payload& trans);
    void write_resp_channel();
    void read_addr_channel(const tlm::tlm_generic_payload& trans);
    void read_data_channel(tlm::tlm_generic_payload& trans);

    enum { OKAY = 0x0, EXOKAY = 0x1, SLVERR = 0x2, DECERR = 0x3 };

};

} // namespace axi_bfm

#endif /* AXI_BFM__INITIATOR_H_ */
