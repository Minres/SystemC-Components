/*******************************************************************************
 * Copyright 2021 MINRES Technologies GmbH
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *******************************************************************************/

#include "axi_initiator.h"
#include <scc/report.h>
#include <tlm/scc/tlm_id.h>
#include <tlm/scc/tlm_mm.h>
#include <tlm/scc/tlm_gp_shared.h>

using namespace axi;

axi_initiator_base::axi_initiator_base(const sc_core::sc_module_name& nm, axi::pe::simple_initiator_b& pe,
                                       uint32_t width)
: sc_module(nm)
, pe(pe)
, buswidth(width) {
    SC_HAS_PROCESS(axi_initiator_base);
    // Register callback for incoming b_transport interface method call
    b_tsck.register_b_transport(this, &axi_initiator_base::b_transport);
    setup_cb = [this](tlm::tlm_generic_payload& p) {
        auto len = p.get_data_length();
        auto* ext = p.get_extension<axi::axi4_extension>();
        ext->set_size(scc::ilog2(std::min<size_t>(len, buswidth / 8)));
        sc_assert(len < (buswidth / 8) || len % (buswidth / 8) == 0);
        ext->set_length((len * 8 - 1) / buswidth);
        ext->set_burst(axi::burst_e::INCR);
        ext->set_id(id);
    };
}

void axi_initiator_base::b_transport(tlm::tlm_generic_payload& trans, sc_core::sc_time& delay) {
    tlm::scc::tlm_gp_shared_ptr payload = create_axi_trans(trans);
    pe.transport(*payload, false);
    trans.update_original_from(*payload);
}

tlm::tlm_generic_payload* axi_initiator_base::create_axi_trans(tlm::tlm_generic_payload& p) {
    tlm::tlm_generic_payload* trans = nullptr;
    if(p.has_mm()) {
        trans = &p;
        auto* ext = new axi::axi4_extension;
        trans->set_auto_extension(ext);
    } else {
        trans = tlm::scc::tlm_mm<>::get().allocate<axi::axi4_extension>();
        trans->deep_copy_from(p);
        tlm::scc::tlm_gp_mm::add_data_ptr(trans->get_data_length(), trans);
        std::copy(p.get_data_ptr(), p.get_data_ptr() + p.get_data_length(), trans->get_data_ptr());
    }
    tlm::scc::setId(*trans, id++);
    setup_cb(*trans);
    return trans;
}
