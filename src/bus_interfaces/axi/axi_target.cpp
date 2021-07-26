
#include "axi/axi_target.h"
#include "scc/report.h"

using namespace axi;

axi_target_base::axi_target_base(const sc_core::sc_module_name& nm, axi::pe::axi_target_pe_b& pe)
: sc_module(nm)
, pe(pe) {
    SC_HAS_PROCESS(axi_target_base);
    SC_THREAD(trans_queue);
}

unsigned axi_target_base::access(tlm::tlm_generic_payload& trans) {
    peq.notify(&trans);
    return std::numeric_limits<unsigned>::max();
}

void axi_target_base::trans_queue() {
    auto delay = sc_core::SC_ZERO_TIME;
    while(true) {
        auto trans = peq.get();
        trans->acquire();
        output_socket->b_transport(*trans, delay);
        trans->release();
        pe.operation_resp(*trans, 0);
    }
}
