/*
 * ahb_target_bfm.cpp
 *
 *  Created on: 11.12.2019
 *      Author: developer
 */

#include <tlm/ahb/bfm/target.h>
#include <tlm/ahb/ahb_tlm.h>
#include <tlm/tlm_mm.h>
#include <scc/utilities.h>

using namespace tlm::ahb::bfm;
using namespace sc_core;

template<unsigned WIDTH>
target<WIDTH>::target(const sc_module_name& nm): sc_module(nm) {
    SC_HAS_PROCESS(target);
    SC_THREAD(bfm_thread);
    sensitive<<HCLK_i.pos();
}

template<unsigned WIDTH>
target<WIDTH>::~target() {
}

template<unsigned WIDTH>
void target<WIDTH>::bfm_thread() {
    tlm::tlm_mm<>& mm = tlm_mm<>::get();
    tlm::tlm_generic_payload* addr_payload=nullptr, *data_payload=nullptr;
    auto const log_width = scc::ilog2(WIDTH/8);
    auto beat_cnt = 0U;
    while(1){
        wait();
        if(!HRESETn_i.read()){
            HREADY_o.write(true);
        } else  {
            if(HSEL_i.read()){
                if(HTRANS_i.read()>0x1){ // HTRANS/BUSY or IDLE check
                    auto* gp=mm.allocate();
                    gp->acquire();
                    gp->set_address(HADDR_i.read());
                    gp->set_command(HWRITE_i.read()?tlm::TLM_WRITE_COMMAND:tlm::TLM_READ_COMMAND);
                    auto* ext = new ahb_extension();
                    gp->set_extension(ext);
                    ext->set_locked(HMASTLOCK_i.read());
                    ext->set_protection(HPROT_i.read());
                    ext->set_burst(HBURST_i.read());
                    size_t size=HSIZE_i.read();
                    sc_assert(size<=log_width);
                    unsigned length = (1<<size)*(1<<ext->get_burst());
                    gp->set_data_length(length);
                    gp->set_streaming_width(length);
                    gp->set_data_ptr(new uint8_t[length]);
                    if(gp->is_read()){
                        sc_time delay;
                        isckt->b_transport(*gp, delay);
                    }
                    if(addr_payload)
                        HREADY_o.write(false);
                    else {
                        HREADY_o.write(true);
                        addr_payload = gp;
                    }
                }
                if(!data_payload){
                    data_payload=addr_payload;
                    addr_payload=nullptr;
                }
                if(data_payload){
                    auto* ext = data_payload->get_extension<ahb_extension>();
                    sc_dt::sc_biguint<WIDTH> data{0};
                    auto offset = WIDTH/8*beat_cnt;
                    if(data_payload->is_write()){
                        data = HWDATA_i.read();
                        for(size_t i=0, j=0;j<WIDTH/8; i+=8, ++j, ++offset)
                            *(uint8_t*)(data_payload->get_data_ptr()+offset)=data.range(i+7,i).to_uint();
                    } else {
                        for(size_t j=0, k=0;k<WIDTH/8; j+=8, ++k, ++offset)
                            data.range(j+7,j)=*(uint8_t*)(data_payload->get_data_ptr()+offset);
                        HRDATA_o.write(data);

                    }
                    if(++beat_cnt==1<<ext->get_burst()){
                        if(data_payload->is_write()){
                            sc_time delay;
                            isckt->b_transport(*data_payload, delay);
                        }
                        beat_cnt=0;
                        data_payload->release();
                        data_payload=nullptr;
                    }
                }
            }
        }
    }
}

template class tlm::ahb::bfm::target<32>;
template class tlm::ahb::bfm::target<64>;
template class tlm::ahb::bfm::target<128>;
template class tlm::ahb::bfm::target<256>;
template class tlm::ahb::bfm::target<512>;
template class tlm::ahb::bfm::target<1024>;
