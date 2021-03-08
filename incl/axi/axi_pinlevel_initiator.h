#pragma once

#include <axi/axi_tlm.h>
#include <axi/pe/simple_initiator.h>
#include <scc/report.h>

#include <systemc>
#include <tlm>
#include <tlm_utils/simple_target_socket.h>

namespace axi {
/**
 * @brief axi_pinlevel_initiator class provides an input_socket for incomming TLM transactions.
 * It attaches AXI extension to the tlm_generic_payload and forwards it to the AXI Protocol Engine.
 */
class axi_pinlevel_initiator_base : public sc_core::sc_module {
public:
    sc_core::sc_in<bool> clk_i{"clk_i"};
    tlm_utils::simple_target_socket<axi_pinlevel_initiator_base> input_socket{"input_socket"};

    /**
     * Create and attach AXI extension.
     * AXI protocol engine expects the incoming transactions to be controlled by a memory manager.
     * If transaction does not have an associated memory manager, copy the payload into a new one with mm.
     */
    tlm::tlm_generic_payload* create_axi_trans(tlm::tlm_generic_payload& p) {
    	tlm::tlm_generic_payload* trans = nullptr;
    	axi::axi4_extension* ext = new axi::axi4_extension;
    	if(p.has_mm()) {
    		p.acquire();
        	trans = &p;
    	} else {
            trans = tlm::tlm_mm<>::get().allocate();
            uint8_t* data_buf = new uint8_t[trans->get_data_length()];
    		trans->set_data_ptr(data_buf);
            trans->deep_copy_from(p);
    	}
        setId(*trans, id++);
    	trans->set_extension(ext);
        auto len = trans->get_data_length();
        ext->set_size(scc::ilog2(std::min<size_t>(len, buswidth / 8)));
        sc_assert(len < (buswidth / 8) || len % (buswidth / 8) == 0);
        ext->set_length((len * 8 - 1) / buswidth);
        ext->set_burst(axi::burst_e::INCR);
        ext->set_id(id);
        return trans;
    }

    void b_transport(tlm::tlm_generic_payload& trans, sc_core::sc_time& delay) {
    	auto payload = create_axi_trans(trans);
    	pe.transport(*payload, false);
    	trans.update_original_from(*payload);
    	payload->release();
    }

	axi_pinlevel_initiator_base(const sc_core::sc_module_name &nm, axi::pe::simple_initiator_b &pe, uint32_t width);
    virtual ~axi_pinlevel_initiator_base(){};
private:
    axi::pe::simple_initiator_b& pe;
    uint32_t buswidth{0};
    unsigned id{0};
};

inline axi_pinlevel_initiator_base::axi_pinlevel_initiator_base(const sc_core::sc_module_name &nm, axi::pe::simple_initiator_b &pe, uint32_t width)
:sc_module(nm)
, pe(pe)
, buswidth(width)
{
	SC_HAS_PROCESS(axi_pinlevel_initiator_base);
    // Register callback for incoming b_transport interface method call
    input_socket.register_b_transport(this, &axi_pinlevel_initiator_base::b_transport);
}


template <unsigned int BUSWIDTH = 32>
class axi_pinlevel_initiator : public axi_pinlevel_initiator_base {
public:
    axi::axi_initiator_socket<BUSWIDTH> intor{"intor"};

    axi_pinlevel_initiator(sc_core::sc_module_name nm)
    : axi_pinlevel_initiator_base(nm, intor_pe, BUSWIDTH)
    {
    	intor_pe.clk_i(clk_i);
    };

    virtual ~axi_pinlevel_initiator(){};

private:
    axi::pe::simple_axi_initiator<BUSWIDTH> intor_pe{"intor_pe", intor};
};

} // namespace axi
