#pragma once

#define SC_INCLUDE_DYNAMIC_PROCESSES

#include <axi/axi_tlm.h>
#include <axi/pe/simple_target.h>
#include <scc/peq.h>

#include <systemc>
#include <tlm>
#include <tlm_utils/simple_initiator_socket.h>

namespace axi {
/**
 * @brief axi_target class instantiates the AXI Protocol Engine.
 * It accesses the Protocol Engine with access() callback function and
 * forwards the transactions via the output_socket.
 */
class axi_target_base : public sc_core::sc_module {
public:
    sc_core::sc_in<bool> clk_i{"clk_i"};
    tlm_utils::simple_initiator_socket<axi_target_base> output_socket{"output_socket"};

    axi_target_base(const sc_core::sc_module_name& nm, axi::pe::axi_target_pe_b& pe);
    virtual ~axi_target_base(){};

protected:
    unsigned access(tlm::tlm_generic_payload& trans);

private:
    axi::pe::axi_target_pe_b& pe;
    scc::peq<tlm::tlm_generic_payload*> peq;

    void trans_queue();
};

template <unsigned int BUSWIDTH = 32> class axi_target : public axi_target_base {
public:
    axi::axi_target_socket<BUSWIDTH> input_socket;

    axi_target(sc_core::sc_module_name nm)
    : axi_target_base(nm, pe) {
        pe.clk_i(clk_i);
        pe.set_operation_cb([this](tlm::tlm_generic_payload& trans) -> unsigned { return access(trans); });
    };

    virtual ~axi_target(){};

private:
    axi::pe::simple_target<BUSWIDTH> pe{"pe", input_socket};
};

} // namespace axi
