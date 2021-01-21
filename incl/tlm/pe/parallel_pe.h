/*
 * parallel_pe.h
 *
 *  Created on: Dec 16, 2020
 *      Author: eyck
 */

#ifndef SCC_INCL_TLM_PE_PARALLEL_PE_H_
#define SCC_INCL_TLM_PE_PARALLEL_PE_H_

#include "intor_if.h"
#include <tlm>
#include <deque>
namespace tlm {
namespace pe {

class parallel_pe: public sc_core::sc_module, public intor_fw {
    struct thread_unit {
        sc_core::sc_event evt;
        tlm::tlm_generic_payload* gp{nullptr};
        bool lt_transport{false};
        sc_core::sc_process_handle hndl{};
        thread_unit(const thread_unit& o):gp(o.gp), lt_transport(o.lt_transport), hndl(o.hndl){}
        thread_unit() = default;
    };
public:

    sc_core::sc_export<intor_fw> fw_i{"fw_i"};

    sc_core::sc_port<intor_fw> fw_o{"fw_o"};

    sc_core::sc_port<intor_bw> bw_o{"bw_o"};

    parallel_pe(sc_core::sc_module_name const& nm);

    virtual ~parallel_pe();

private:
    void transport(tlm::tlm_generic_payload& payload, bool lt_transport = false) override;

    void snoop_resp(tlm::tlm_generic_payload& payload, bool sync) override {
        fw_o->snoop_resp(payload, sync);
    }

    std::deque<unsigned> waiting_ids;
    std::vector<thread_unit>threads;
};

} /* namespace pe */
} /* namespace tlm */

#endif /* SCC_INCL_TLM_PE_PARALLEL_PE_H_ */
