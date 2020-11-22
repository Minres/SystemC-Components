#include "Target_axi.h"
#include "AXIreport.h"
#include "TLM2report.h"
#include "scc/report.h"

template <unsigned int BUSWIDTH>
Target_axi<BUSWIDTH>::Target_axi(sc_module_name mod)
: sc_module(mod)
, axi("axi")
, Clk("Clk")
  //, RstN("RstN")
, underReset(false)
, wData("wData", BUSWIDTH / 8) {
    SC_THREAD(ReadResponseSender);
    SC_THREAD(WriteResponseSender);
    SC_METHOD(ReadRequestEndSender);
    sensitive << ReadRequestReceived;
    dont_initialize();
    SC_METHOD(WriteRequestEndSender);
    sensitive << WriteRequestReceived;
    dont_initialize();
    //SC_METHOD(onRstN);
    //sensitive << RstN;
    axi.bind(*this);

    cout << name() << " construction of module finished" << endl;
}

template <unsigned int BUSWIDTH> Target_axi<BUSWIDTH>::~Target_axi() {}

template <unsigned int BUSWIDTH> void Target_axi<BUSWIDTH>::before_end_of_elaboration() {
    sc_interface* i_f = Clk.get_interface();
    sc_clock* clk = dynamic_cast<sc_clock*>(i_f);
    t = clk->period(); // Call method of clock object to which port is bound
}

//=============================================================================
// nb_transport_fw implementation calls from initiators
//
//=============================================================================
template <unsigned int BUSWIDTH>
tlm::tlm_sync_enum                        // synchronization state
    Target_axi<BUSWIDTH>::nb_transport_fw // non-blocking transport call through Bus
    (payload_type& my_axi_payload         // generic payoad pointer
     ,
     phase_type& phase // transaction phase
     ,
     sc_core::sc_time& delay_time) // time it should take for transport
{
    tlm::tlm_sync_enum return_status = tlm::TLM_ACCEPTED;
    TLM2Group tmp;
    tmp.trans = &my_axi_payload;
    tmp.phase = phase;
    tmp.delay = delay_time;
    SCCTRACE("Target_axi") << sc_time_stamp() << " " << name() << " Target receiving : " << report::print(my_axi_payload) << " phase "
                          << report::printAXI(phase);

    if(phase == tlm::BEGIN_REQ) {
        if(my_axi_payload.get_command() == tlm::TLM_WRITE_COMMAND) {
            WriteRequestTrans.push_back(tmp);
            WriteRequestReceived.notify(t);
        } else {
            ReadRequestTrans.push_back(tmp);
            ReadRequestReceived.notify(t);
        };
    }; // end BEGIN_REQ
    if(phase == axi::BEGIN_PARTIAL_REQ) {
        if(my_axi_payload.get_command() != tlm::TLM_WRITE_COMMAND) {
            SCCWARN() << "unexpected partial request for none write commands";
        };
        WriteRequestTrans.push_back(tmp);
        WriteRequestReceived.notify(t);
    };
    if(phase == axi::END_PARTIAL_RESP || phase == tlm::END_RESP) {
        // nothing here to do
    };
    if(phase == tlm::END_REQ || phase == tlm::BEGIN_RESP) {
        SCCERR("Target_axi") << sc_time_stamp() << " " << name() << " Illegal phase received by target " << report::print(phase);
    };
    return return_status;
} // end nb_transport_fw

template <unsigned int BUSWIDTH> void Target_axi<BUSWIDTH>::onRstN() {
  //underReset = !bool(RstN.read());
  underReset = false;
    if(underReset)
        SCCINFO("Target_axi") << sc_time_stamp() << " " << name() << " activate reset";
    else
        SCCINFO("Target_axi") << sc_time_stamp() << " " << name() << " coming out of reset";
}

template <unsigned int BUSWIDTH> void Target_axi<BUSWIDTH>::ReadRequestEndSender() {
    TLM2Group tmp = ReadRequestTrans.back();
    tlm::tlm_phase phase;
    if(tmp.phase == axi::BEGIN_PARTIAL_REQ)
        phase = axi::END_PARTIAL_REQ;
    else
        phase = tlm::END_REQ;
    tlm::tlm_sync_enum status = axi->nb_transport_bw(*tmp.trans, phase, tmp.delay);
};
template <unsigned int BUSWIDTH> void Target_axi<BUSWIDTH>::WriteRequestEndSender() {
    TLM2Group tmp = WriteRequestTrans.back();
    tlm::tlm_phase phase;
    if(tmp.phase == axi::BEGIN_PARTIAL_REQ)
        phase = axi::END_PARTIAL_REQ;
    else
        phase = tlm::END_REQ;
    tlm::tlm_sync_enum status = axi->nb_transport_bw(*tmp.trans, phase, tmp.delay);
};

template <unsigned int BUSWIDTH>
void Target_axi<BUSWIDTH>::ReadResponseSender() // ToDo support delay
{
    while(underReset)
        wait(t);
    while(true) {
        while(ReadRequestTrans.empty())
            wait(ReadRequestReceived);
        SCCTRACE("Target_axi") << sc_time_stamp() << " " << name() << " Target seeing a read request and responding";
        TLM2Group tmp = ReadRequestTrans.front();
        payload_type* tr = tmp.trans;
        if(tr) {
            sc_time delay = SC_ZERO_TIME;
            for(unsigned int i = 1; i < tr->get_data_length() / wData.value; ++i) {
                phase_type phase_data = axi::BEGIN_PARTIAL_RESP;
                tr->set_response_status(tlm::TLM_OK_RESPONSE);
                SCCTRACE("Target_axi") << sc_time_stamp() << " " << name()
                                      << " Target sending partial read response : " << report::print(*tr) << " phase "
                                      << report::print(phase_data);
                tlm::tlm_sync_enum status = axi->nb_transport_bw(*tr, phase_data, delay);
                wait(t);
            };
            tlm::tlm_phase phase = tlm::BEGIN_RESP;
            SCCTRACE("Target_axi") << sc_time_stamp() << " " << name() << " Target sending read response : " << report::print(*tr)
                                  << " phase " << report::print(phase);
            tlm::tlm_sync_enum status = axi->nb_transport_bw(*tr, phase, delay);
        } else
            SCCWARN("Target_axi") << sc_time_stamp() << " " << name() << " Target didn't find response pointer??? ";
        ReadRequestTrans.pop_front();
        wait(t);
    }
};

template <unsigned int BUSWIDTH>
void Target_axi<BUSWIDTH>::WriteResponseSender() // ToDo support delay
{
    while(underReset)
        wait(t);
    while(true) {
        while(WriteRequestTrans.empty())
            wait(WriteRequestReceived);
        SCCTRACE("Target_axi") << sc_time_stamp() << " " << name() << " Target seeing a write request and responding";
        TLM2Group tmp = WriteRequestTrans.front();
        payload_type* tr = tmp.trans;
        phase_type phase = tmp.phase;
        wait(t);
        if(phase == tlm::BEGIN_REQ) {
            sc_time delay = SC_ZERO_TIME;
            tlm::tlm_phase phase = tlm::BEGIN_RESP;
            if(tr) {
                SCCTRACE("Target_axi") << sc_time_stamp() << " " << name() << " Target sending : " << report::print(*tr) << " phase "
                                      << report::print(phase);
                tr->set_response_status(tlm::TLM_OK_RESPONSE);
                tlm::tlm_sync_enum status = axi->nb_transport_bw(*tr, phase, delay);
            } else
                SCCWARN("Target_axi") << sc_time_stamp() << " " << name() << " Target didn't find response pointer??? ";
        }
        WriteRequestTrans.pop_front();
    }
};
template class Target_axi<32>;
template class Target_axi<64>;
template class Target_axi<128>;
