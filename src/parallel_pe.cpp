/*
 * parallel_pe.cpp
 *
 *  Created on: Dec 16, 2020
 *      Author: eyck
 */

#define SC_INCLUDE_DYNAMIC_PROCESSES
#include <tlm/pe/parallel_pe.h>

namespace tlm {
namespace pe {
using namespace sc_core;

parallel_pe::parallel_pe(sc_core::sc_module_name const& nm): sc_module(nm) {
    fw_i.bind(*this);
}

parallel_pe::~parallel_pe() {
}

void parallel_pe::transport(tlm::tlm_generic_payload &payload, bool lt_transport) {
    if(!waiting_ids.size()) {
        auto id = threads.size();
        threads.resize(threads.size()+1);
        thread_unit& tu = threads.back();
        tu.hndl = sc_core::sc_spawn([this, id]() -> void {
            auto& tu = threads[id];
            while(true){
                fw_o->transport(*tu.gp, tu.lt_transport);
                bw_o->transport(*tu.gp);
                if(tu.gp->has_mm()) tu.gp->release();
                tu.gp=nullptr;
                waiting_ids.push_back(id);
                wait(tu.evt);
            }
        },
        sc_core::sc_gen_unique_name("execute"));
        tu.gp=&payload;
    } else {
        auto& tu = threads[waiting_ids.front()];
        waiting_ids.pop_front();
        tu.gp=&payload;
        tu.lt_transport= lt_transport;
        tu.evt.notify();
    }
    if(payload.has_mm())
        payload.acquire();
}

} /* namespace pe */
} /* namespace tlm */
