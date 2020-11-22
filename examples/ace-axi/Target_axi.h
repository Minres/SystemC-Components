#pragma once
#include "axi/axi_tlm.h"
#include "common_defs.h"
#include "scc/report.h"
#include <deque>
#include <map>
#include <systemc.h>
#include <tlm.h>

template <unsigned int BUSWIDTH = 32> class Target_axi : public sc_module, public axi::axi_fw_transport_if<axi::axi_protocol_types> {
private:
    SC_HAS_PROCESS(Target_axi);
    bool underReset;
    sc_attribute<unsigned int> wData;

public:
    using payload_type = axi::axi_protocol_types::tlm_payload_type;
    using phase_type = axi::axi_protocol_types::tlm_phase_type;

    Target_axi(sc_module_name);
    virtual ~Target_axi();
    axi::axi_target_socket<BUSWIDTH> axi;

//sc_in<bool> RstN;
    sc_in<bool> Clk;

    // the target fw interfaces functions
    tlm::tlm_sync_enum nb_transport_fw(payload_type&, phase_type&, sc_core::sc_time&);
    // default implementation
    void b_transport(payload_type& trans, sc_core::sc_time& t) { 
      SCCWARN("Target_axi") << " b_transport not implmentated"; 
    };
    bool get_direct_mem_ptr(payload_type& trans, tlm::tlm_dmi& dmi_data) {
        SCCWARN("Target_axi") << " DMI not implmentated";
        return false;
    };
    unsigned int transport_dbg(payload_type& trans) {
        SCCWARN("Target_axi") << " transport_dbg not implmentated";
        return 0;
    };

private:
    void ReadRequestEndSender();
    void WriteRequestEndSender();
    void ReadResponseSender();
    void WriteResponseSender();
    std::deque<TLM2Group> ReadRequestTrans; // ToDo: support reordering/interleaving
    std::deque<TLM2Group> WriteRequestTrans;
    void before_end_of_elaboration();
    void onRstN();
    sc_event ReadRequestReceived;
    sc_event WriteRequestReceived;
    sc_time t;
};
