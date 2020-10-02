
#include <tlm/axi/axi4_tlm2pin_adaptor.h>

using namespace axi_bfm;
using namespace sc_core;


axi4_tlm2pin_adaptor::axi4_tlm2pin_adaptor(sc_module_name nm)
: sc_module(nm)
{
    input_socket.register_b_transport([this](tlm::tlm_generic_payload& generic_payload, sc_core::sc_time& delay)->void{b_transport(generic_payload, delay);});

    SC_THREAD(write_addr_channel);
    SC_THREAD(write_data_channel);
    SC_THREAD(write_resp_channel);
    SC_THREAD(read_addr_channel);
    SC_THREAD(read_data_channel);
}

void axi4_tlm2pin_adaptor::write_addr_channel() {
    while(true) {
        aw_len_o.write(1); // All AXI4-Lite transactions are of burst length 1
        aw_valid_o.write(false);
        wait(wr_queue.get_event());
        while(auto* trans = wr_queue.get_next_transaction()) {
            aw_addr_o.write(trans->get_address());
            aw_valid_o.write(true);
            wait(clk_i.posedge_event());
            if (!aw_ready_i.read()) {
                wait(aw_ready_i.posedge_event());
            } else {
                wait(clk_i.posedge_event());
            }
            aw_valid_o.write(false);
        }
    }
}

void axi4_tlm2pin_adaptor::write_data_channel() {
    sc_dt::sc_biguint<512> data{0};
    while(true) {
        w_valid_o.write(false);
        wait(wr_queue.get_event());
        while(auto* trans = wr_queue.get_next_transaction()) {
            w_valid_o.write(true);
            for(size_t j = 0, k = 0, offset = 0; k < 32 / 8; j += 8, ++k, ++offset)
                data.range(j + 7, j) = *(uint8_t*)(trans->get_data_ptr() + offset);
            w_data_o.write(data);
            if (!w_ready_i.read()) wait(w_ready_i.posedge_event());

            w_valid_o.write(false);
        }
    }
}

void axi4_tlm2pin_adaptor::write_resp_channel() {
    while(true) {
        wait(wr_queue.get_event());
        while(auto* trans = wr_queue.get_next_transaction()) {
            b_ready_o.write(true);
            // The slave must wait for AWVALID, AWREADY, WVALID, and WREADY to be asserted before asserting BVALID
            if (b_valid_i.negedge()) wait(b_valid_i.posedge_event());
            sc_assert(b_resp_i.read() == OKAY);
            b_ready_o.write(false);
        }
    }
}

void axi4_tlm2pin_adaptor::read_addr_channel() {
    ar_len_o.write(1); // All AXI4-Lite transactions are of burst length 1
    while(true) {
        wait(rd_queue.get_event());
        while(auto* trans = rd_queue.get_next_transaction()) {
            ar_addr_o.write(trans->get_address());
            ar_valid_o.write(true);
            if (ar_ready_i.negedge()) wait(ar_ready_i.posedge_event());
            ar_valid_o.write(false);
        }
    }
}

void axi4_tlm2pin_adaptor::read_data_channel() {
    sc_dt::sc_biguint<512> data{0};
    while(true) {
        wait(rd_queue.get_event());
        while(auto* trans = rd_queue.get_next_transaction()) {
            r_ready_o.write(false);
            if (r_valid_i.negedge()) wait(r_valid_i.posedge_event());
            data = r_data_i.read();
            for(size_t j = 0, k = 0, offset = 0; k < 32 / 8; j += 8, ++k, ++offset)
                *(uint8_t*)(trans->get_data_ptr() + offset) = data.range(j + 7, j).to_uint();
            r_ready_o.write(true);
        }
    }
}

void axi4_tlm2pin_adaptor::b_transport(tlm::tlm_generic_payload& trans, sc_core::sc_time& delay) {
    if(trans.has_mm())
        trans.acquire();

    if (trans.get_command() == tlm::TLM_WRITE_COMMAND) {
        this->wr_queue.notify(trans);
    } else {
        this->rd_queue.notify(trans);
    }
}

