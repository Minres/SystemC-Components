#include <scc/report.h>
#include "tlm/axi/axi4_pin2tlm_adaptor.h"

using namespace axi_bfm;
using namespace sc_core;

axi_pin2tlm_adaptor::axi_pin2tlm_adaptor(sc_module_name nm)
: sc_module(nm) {
    SC_THREAD(bus_thread)
    sensitive << clk_i.pos();
}

void axi_pin2tlm_adaptor::bus_thread() {
    tlm::tlm_generic_payload payload;
    sc_time delay = SC_ZERO_TIME;
    sc_dt::sc_biguint<512> read_data{0};
    sc_dt::sc_biguint<512> write_data{0};
    uint8_t r_data_buf[64]{0};
    uint64_t output_data[8]{0};
    while(true) {
        wait();
        if(reset_i.read()) {
            r_valid_o.write(false);
            b_valid_o.write(false);
            ar_ready_o.write(true);
        } else {
            // Handle read request
            if(ar_valid_i.read()) {
                ar_ready_o.write(false);
                uint32_t length = ar_len_i.read();
                uint32_t addr = ar_addr_i.read();
                payload.set_command(tlm::TLM_READ_COMMAND);
                for (int l = 0; l <= length; l++) {
                    payload.set_address(addr);
                    payload.set_data_length(64);
                    payload.set_streaming_width(64);
                    payload.set_data_ptr(r_data_buf);
                    output_socket->b_transport(payload, delay);

                    for(size_t i = 0, j = 0; j < 64; i += 8, j++) {
                        read_data.range(i + 7, i) = *(uint8_t*)(payload.get_data_ptr() + j);
                    }
                    r_data_o.write(read_data);
                    r_valid_o.write(true);
                    wait(clk_i.posedge());
                    addr += 64;
                }
                ar_ready_o.write(true);
                r_valid_o.write(false);
            }

            // Handle write request
            if(aw_valid_i.read() && w_valid_i.read()) {
                aw_ready_o.write(true);
                w_ready_o.write(true);
                b_valid_o.write(false);
                wait(clk_i.posedge());

                auto length = aw_len_i.read();
                auto addr = aw_addr_i.read();
                auto num_bytes = 1 << aw_size_i.read();

                payload.set_data_length(num_bytes);
                payload.set_streaming_width(num_bytes);
                payload.set_command(tlm::TLM_WRITE_COMMAND);
                payload.set_address(addr);

                for (int l = 0; l <= length; l++) {
                    write_data = w_data_i.read();
                    for (size_t i=0, j = 0; i < 8; j += num_bytes, i++) {
                    	output_data[i] = write_data.range(j + num_bytes - 1, j).to_uint64();
                    	SCCDEBUG(SCMOD) << "Addr: 0x" << std::hex << payload.get_address() << " Value: 0x" << output_data[i];
                    }
                    payload.set_data_ptr((unsigned char *)&output_data);

                    output_socket->b_transport(payload, delay);
                    addr += num_bytes;
                    payload.set_address(addr);

                    if (w_last_i.read()) {
                        aw_ready_o.write(false);
                        w_ready_o.write(false);
                        b_resp_o.write(OKAY);
                        b_valid_o.write(true);
                    }

                    wait(clk_i.posedge());
                }
            }
        }
    }
}
