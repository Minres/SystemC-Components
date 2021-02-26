#pragma once

#include <axi/axi_tlm.h>
#include <axi/pe/simple_target.h>
#include <scc/report.h>
#include <scc/peq.h>

#include <systemc>
#include <tlm>
#include <tlm_utils/simple_initiator_socket.h>

#include <memory>
#include <queue>
#include <unordered_map>

#include "axi_pin2tlm_adaptor.h"

namespace axi {
class axi_pinlevel_target_base : public sc_core::sc_module {
public:
    using payload_type = axi::axi_protocol_types::tlm_payload_type;
    using phase_type = axi::axi_protocol_types::tlm_phase_type;

    sc_core::sc_in<bool> clk_i{"clk_i"};
    tlm_utils::simple_initiator_socket<axi_pinlevel_target_base> output_socket{"output_socket"};

    axi_pinlevel_target_base(sc_core::sc_module_name const& nm, axi::pe::axi_target_pe_b& pe)
    : sc_module(nm)
    , pe(pe)
    {
        SC_HAS_PROCESS(axi_pinlevel_target_base);

        SC_THREAD(trans_queue);
    }
    virtual ~axi_pinlevel_target_base(){};


protected:
    unsigned access(tlm::tlm_generic_payload& trans) {
    	peq.notify(&trans);
    	return std::numeric_limits<unsigned>::max();
    }
private:
    axi::pe::axi_target_pe_b& pe;
    scc::peq<tlm::tlm_generic_payload*> peq;

    void trans_queue() {
    	auto delay = sc_core::SC_ZERO_TIME;
    	while(true) {
    		auto trans = peq.get();
    		trans->acquire();
    		output_socket->b_transport(*trans, delay);
    		trans->release();
    		pe.operation_resp(*trans, 0);
    	}
    }
};

template <unsigned int BUSWIDTH = 32>
class axi_pinlevel_target : public axi_pinlevel_target_base {
public:
    axi::axi_target_socket<BUSWIDTH> input_socket;

    axi_pinlevel_target(sc_core::sc_module_name nm)
    : axi_pinlevel_target_base(nm, pe)
    {
    	 pe.clk_i(clk_i);
         pe.set_operation_cb([this](tlm::tlm_generic_payload& trans)->unsigned {return access(trans);});
    };

    virtual ~axi_pinlevel_target(){};
private:
    axi::pe::simple_target<BUSWIDTH> pe{"pe", input_socket};
};

} // namespace axi
