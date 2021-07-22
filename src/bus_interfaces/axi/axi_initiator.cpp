
#include "axi/axi_initiator.h"
#include "scc/report.h"
#include "tlm/scc/tlm_id.h"
#include "tlm/scc/tlm_mm.h"

using namespace axi;

axi_initiator_base::axi_initiator_base(const sc_core::sc_module_name& nm, axi::pe::simple_initiator_b& pe,
                                       uint32_t width)
: sc_module(nm)
, pe(pe)
, buswidth(width) {
    SC_HAS_PROCESS(axi_initiator_base);
    // Register callback for incoming b_transport interface method call
    input_socket.register_b_transport(this, &axi_initiator_base::b_transport);
}

void axi_initiator_base::b_transport(tlm::tlm_generic_payload& trans, sc_core::sc_time& delay) {
    auto payload = create_axi_trans(trans);
    pe.transport(*payload, false);
    trans.update_original_from(*payload);
    payload->release();
}

tlm::tlm_generic_payload* axi_initiator_base::create_axi_trans(tlm::tlm_generic_payload& p) {
    tlm::tlm_generic_payload* trans = nullptr;
    uint8_t* data_buf = nullptr;
    axi::axi4_extension* ext = new axi::axi4_extension;
    if(p.has_mm()) {
        p.acquire();
        trans = &p;
    } else {
        trans = tlm::scc::tlm_mm<>::get().allocate();
        trans->deep_copy_from(p);
        data_buf = new uint8_t[trans->get_data_length()];
        std::copy(p.get_data_ptr(), p.get_data_ptr() + p.get_data_length(), data_buf);
        trans->set_data_ptr(data_buf);
    }
    tlm::scc::setId(*trans, id++);
    trans->set_extension(ext);
    auto len = trans->get_data_length();
    ext->set_size(scc::ilog2(std::min<size_t>(len, buswidth / 8)));
    sc_assert(len < (buswidth / 8) || len % (buswidth / 8) == 0);
    ext->set_length((len * 8 - 1) / buswidth);
    ext->set_burst(axi::burst_e::INCR);
    ext->set_id(id);
    return trans;
}
