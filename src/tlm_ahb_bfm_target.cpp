/*
 * ahb_target_bfm.cpp
 *
 *  Created on: 11.12.2019
 *      Author: developer
 */

#include <scc/report.h>
#include <scc/utilities.h>
#include <tlm/ahb/ahb_tlm.h>
#include <tlm/ahb/bfm/target.h>
#include <tlm/tlm_mm.h>

using namespace tlm::ahb::bfm;
using namespace sc_core;

template <unsigned WIDTH>
target<WIDTH>::target(const sc_module_name& nm)
: sc_module(nm) {
    SC_HAS_PROCESS(target);
    SC_THREAD(bfm_thread);
    sensitive << HCLK_i.pos();
}

template <unsigned WIDTH> target<WIDTH>::~target() {}

template <unsigned WIDTH> void target<WIDTH>::bfm_thread() {
    tlm::tlm_mm<>& mm = tlm_mm<>::get();
    auto const log_width = scc::ilog2(WIDTH / 8);
    auto beat_cnt = 0U;
    wait(SC_ZERO_TIME);
    while(1) {
        wait();
        if(!HRESETn_i.read()) {
            HREADY_o.write(true);
            data_payload = nullptr;
            addr_payload = nullptr;
        } else {
            if(HSEL_i.read()) {
                tlm::tlm_generic_payload* gp{nullptr};
                if(HTRANS_i.read() > 0x1) { // HTRANS/BUSY or IDLE check
                    gp = mm.allocate();
                    gp->acquire();
                    gp->set_address(HADDR_i.read());
                    if(HWRITE_i.read())
                        gp->set_write();
                    else
                        gp->set_read();
                    // gp->set_command(HWRITE_i.read()?tlm::TLM_WRITE_COMMAND:tlm::TLM_READ_COMMAND);
                    auto* ext = new ahb_extension();
                    gp->set_extension(ext);
                    ext->set_locked(HMASTLOCK_i.read());
                    ext->set_protection(HPROT_i.read());
                    ext->set_burst(HBURST_i.read());
                    size_t size = HSIZE_i.read();
                    if(size > log_width)
                        SCCERR(SCMOD) << "Access size (" << size << ") is larger than bus width (" << log_width << ")!";
                    unsigned length = (1 << size) * (1 << ext->get_burst());
                    gp->set_data_length(length);
                    gp->set_streaming_width(length);
                    gp->set_data_ptr(new uint8_t[length]);
                    if(addr_payload)
                        HREADY_o.write(false);
                    else {
                        HREADY_o.write(true);
                        addr_payload = gp;
                    }
                }
                if(data_payload && data_payload->is_write())
                    handle_data_phase(beat_cnt);
                if(gp && gp->is_read()) {
                    sc_time delay;
                    isckt->b_transport(*gp, delay);
                }
                if(!data_payload) {
                    data_payload = addr_payload;
                    addr_payload = nullptr;
                }
                if(data_payload && data_payload->is_read())
                    handle_data_phase(beat_cnt);
            }
        }
    }
}

template <unsigned WIDTH> void target<WIDTH>::handle_data_phase(unsigned& beat_cnt) {
    auto const width = WIDTH / 8;
    auto* ext = data_payload->get_extension<ahb_extension>();
    auto start_offs = data_payload->get_address() & (width - 1);
    sc_assert((start_offs + data_payload->get_data_length()) <= width);
    data_t data{0};
    auto offset = width * beat_cnt;
    auto len = std::min(data_payload->get_data_length(), width);
    if(data_payload->is_write()) {
        data = HWDATA_i.read();
        for(size_t i = start_offs * 8, j = 0; j < len; i += 8, ++j, ++offset)
            *(uint8_t*)(data_payload->get_data_ptr() + offset) = data.range(i + 7, i).to_uint();
    } else {
        for(size_t i = start_offs * 8, j = 0; j < len; i += 8, ++j, ++offset)
            data.range(i + 7, i) = *(uint8_t*)(data_payload->get_data_ptr() + offset);
        HRDATA_o.write(data);
    }
    if(++beat_cnt == 1 << ext->get_burst()) {
        if(data_payload->is_write()) {
            sc_time delay;
            isckt->b_transport(*data_payload, delay);
        }
        beat_cnt = 0;
        data_payload->release();
        data_payload = nullptr;
    }
}
template class tlm::ahb::bfm::target<32>;
template class tlm::ahb::bfm::target<64>;
template class tlm::ahb::bfm::target<128>;
template class tlm::ahb::bfm::target<256>;
template class tlm::ahb::bfm::target<512>;
template class tlm::ahb::bfm::target<1024>;
