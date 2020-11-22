// ------------------------------------------------------------------------------
//
//         Project
//          Module          Adapter
//
// ------------------------------------------------------------------------------
#include <Adapter.h>
using namespace std;

namespace TLM2_COMMON {
template <unsigned int INPUT_BUSWIDTH, unsigned int OUTPUT_BUSWIDTH, typename INPT, typename OUTPT>
AdapterImpl<INPUT_BUSWIDTH, OUTPUT_BUSWIDTH, INPT, OUTPT>::AdapterImpl(sc_core::sc_module_name name)
: sc_module(name)
, target_socket("target_socket")
, initiator_socket("initiator_socket")
, _doJournalTracing(false)
, _convertLT2DBG(false)
, _convertDBG2LT(false)
, _convertLT2AT(false)
, _max_outstanding_trans(0)
, _traceEnable(false) {
    target_socket.bind(*this);
    initiator_socket.bind(*this);

    //    setAddressOffset(addressOffset);
    //_downConvertion      = INPUT_BUSWIDTH/OUTPUT_BUSWIDTH;
    //_upConvertion        = OUTPUT_BUSWIDTH/INPUT_BUSWIDTH;
    //_ID                  = id;
    //_checkRepeatCommands = false;
    //_doWidthConversion   = false;
    //_doPTTracing         = false; // ToDo; unused
    _b_transport_count = 0;
    _transport_dbg_count = 0;
    _nb_transport_count = 0;
    _outstanding_trans_counter = 0;
    _dmi_count = 0;
    // m_data 	       	= new unsigned char[64];
    // m_gp_transaction 	= new tlm::tlm_generic_payload();
    // m_gp_transaction->set_mm(&my_mm);
    // m_gp_transaction->set_data_ptr(m_data);
    std::cout << "Adapter construction for " << name << " Journal tracing " << _doJournalTracing << std::endl;
    // sim_start_time = boost::posix_time::microsec_clock::universal_time();

    if(_doJournalTracing) {
        _journal_file.open(string(name) + ".txt");
        cout << "opening file " << name << ".txt" << endl;
    };
};

template <unsigned int INPUT_BUSWIDTH, unsigned int OUTPUT_BUSWIDTH, typename INPT, typename OUTPT>
AdapterImpl<INPUT_BUSWIDTH, OUTPUT_BUSWIDTH, INPT, OUTPT>::~AdapterImpl() {
    print_counters();
};

template <unsigned int INPUT_BUSWIDTH, unsigned int OUTPUT_BUSWIDTH, typename INPT, typename OUTPT>
tlm::tlm_sync_enum AdapterImpl<INPUT_BUSWIDTH, OUTPUT_BUSWIDTH, INPT, OUTPT>::nb_transport_fw(payload_type& trans, phase_type& phase,
                                                                                              sc_core::sc_time& delay) {
    tlm::tlm_sync_enum status = tlm::TLM_ACCEPTED;

    // Addition of Address Offset on the forward path.
    // trans.set_address( trans.get_address() + _addressOffset );
    if(phase == tlm::BEGIN_REQ) {
        _outstanding_trans_counter++;
        _nb_transport_count++;
    } else {
        _outstanding_trans_counter--;
    };
    status = initiator_socket->nb_transport_fw(trans, phase, delay);
    SCCDEBUG("Adapter") << name() << " phase " // << report::printAXI(phase)
                       << " status " << report::print(status) << " transaction " << report::print(trans) << " got now "
                       << _outstanding_trans_counter << " outstanding transactions ";
    if(_doJournalTracing)
        journalTrace("nb_transport_fw", trans, phase);
    return status;
};

template <unsigned int INPUT_BUSWIDTH, unsigned int OUTPUT_BUSWIDTH, typename INPT, typename OUTPT>
tlm::tlm_sync_enum AdapterImpl<INPUT_BUSWIDTH, OUTPUT_BUSWIDTH, INPT, OUTPT>::nb_transport_bw(payload_type& trans, phase_type& phase,
                                                                                              sc_core::sc_time& delay) {
    tlm::tlm_sync_enum status = tlm::TLM_ACCEPTED;
    // according to the TLM spec all downstream components are allowed to change the address so that the initiator cannot
    // rely anymore on it. If it does so it violates the spec. So no need of address processing for backward path.
    status = target_socket->nb_transport_bw(trans, phase, delay);
    if(status == tlm::TLM_COMPLETED) {
        _outstanding_trans_counter--;
    };
    SCCDEBUG("Adapter") << name() << " phase " // << report::printAXI(phase)
                       << " status " << report::print(status) << " transaction " << report::print(trans) << " got now "
                       << _outstanding_trans_counter << " outstanding transactions";
    if(_doJournalTracing)
        journalTrace("nb_transport_bw", trans, phase);
    return status;
};

template <unsigned int INPUT_BUSWIDTH, unsigned int OUTPUT_BUSWIDTH, typename INPT, typename OUTPT>
void AdapterImpl<INPUT_BUSWIDTH, OUTPUT_BUSWIDTH, INPT, OUTPT>::b_transport(payload_type& trans, sc_core::sc_time& delay) {
    tlm::tlm_phase phase = tlm::BEGIN_REQ;
    tlm::tlm_sync_enum status = tlm::TLM_ACCEPTED;
    // Increase the number of counts
    _b_transport_count++;

    // sc_dt::uint64 address_prev =  trans.get_address();
    // sc_dt::uint64 address_tmp  =  address_prev + _addressOffset;

    // if (_doWidthConversion)
    //{
    //    do_width_conversion("do_width_conversion", address_tmp, trans, delay);
    //}
    // else
    //{
    // Addition of Address Offset on the forward path.
    //  trans.set_address( address_tmp );

    // start_time = sc_simulation_time();

    bool added_extension = add_extension(trans);

    //       	if (_traceEnable)
    //	trace_transaction(trans, phase, status, true);

    //  if (_fixeStreamingWidth) {
    //	if (trans.get_data_length()!=trans.get_streaming_width()) {
    //	  SCWARN ("Adapter") << name() << " found a differ streaming and data length; seems to be an error; will make them equal "
    //				<< trans.get_data_length() << "/"
    //				<< trans.get_streaming_width();
    //	  trans.set_streaming_width(trans.get_data_length());
    //	};
    //};

    // if (_checkRepeatCommands) { // ToDo: do we need this?
    // do_check_for_repeat(trans, ref_payload);
    //  ref_payload.deep_copy_from(trans);
    //};

    if(_convertLT2DBG) {
        initiator_socket->transport_dbg(trans);
    } else {
        initiator_socket->b_transport(trans, delay);
    };

    if(added_extension)
        remove_extension(trans);

    //	end_time = sc_simulation_time();

    //       	if (_traceEnable)
    // trace_transaction(trans, phase, status, false);

    // trans.set_address( address_prev );
    SCCDEBUG("Adapter") << " got " << report::print(trans);
    //<< " duration " << (end_time-start_time);
    if(_doJournalTracing)
        journalTrace("b_transport", trans);
    //   }
};

template <unsigned int INPUT_BUSWIDTH, unsigned int OUTPUT_BUSWIDTH, typename INPT, typename OUTPT>
bool AdapterImpl<INPUT_BUSWIDTH, OUTPUT_BUSWIDTH, INPT, OUTPT>::get_direct_mem_ptr(payload_type& trans, tlm::tlm_dmi& dmi_data) {
    _dmi_count++;

    // sc_dt::uint64 address_prev =  trans.get_address();
    // sc_dt::uint64 address_tmp  =  address_prev + _addressOffset;
    // Addition of address Offset on the forward path.
    // trans.set_address( address_tmp );
    bool status = initiator_socket->get_direct_mem_ptr(trans, dmi_data);
    // Substraction of Address Offset on the Backward path.
    // dmi_data.set_start_address( dmi_data.get_start_address() - _addressOffset);
    // dmi_data.set_end_address  ( dmi_data.get_end_address()   - _addressOffset);

    // trans.set_address( address_prev );
    SCCDEBUG("Adapter") << " got DMI access " << report::print(trans);
    if(_doJournalTracing)
        journalTrace("DMI", trans);
    return status;
}

template <unsigned int INPUT_BUSWIDTH, unsigned int OUTPUT_BUSWIDTH, typename INPT, typename OUTPT>
void AdapterImpl<INPUT_BUSWIDTH, OUTPUT_BUSWIDTH, INPT, OUTPT>::invalidate_direct_mem_ptr(sc_dt::uint64 start_addr,
                                                                                          sc_dt::uint64 end_addr) {
    target_socket->invalidate_direct_mem_ptr(start_addr, end_addr);
}

template <unsigned int INPUT_BUSWIDTH, unsigned int OUTPUT_BUSWIDTH, typename INPT, typename OUTPT>
unsigned int AdapterImpl<INPUT_BUSWIDTH, OUTPUT_BUSWIDTH, INPT, OUTPT>::transport_dbg(payload_type& trans) {
    tlm::tlm_phase phase = tlm::BEGIN_REQ;
    tlm::tlm_sync_enum status = tlm::TLM_ACCEPTED;
    unsigned int count = 0;
    // Increase the number of counts
    _transport_dbg_count++;

    // sc_dt::uint64 address_prev =  trans.get_address();
    // sc_dt::uint64 address_tmp  =  address_prev + _addressOffset;
    // Addition of Address Offset on the forward path.
    // trans.set_address( address_tmp );

    //  if (_traceEnable)
    //  trace_transaction(trans, phase, status, true);

    // if (_checkRepeatCommands) {
    // do_check_for_repeat(trans, ref_payload);
    //  ref_payload.deep_copy_from(trans);
    //};

    if(_convertDBG2LT) {
        sc_core::sc_time delay(0, SC_NS);
        b_transport(trans, delay);
        count = trans.get_data_length();
    } else {
        count = initiator_socket->transport_dbg(trans);
    };

    // trans.set_address( address_prev );
    SCCDEBUG("Adapter") << "got " << report::print(trans);
    if(_doJournalTracing)
        journalTrace("transport_dbg", trans);
    return count;
}

// template<unsigned int INPUT_BUSWIDTH, unsigned int OUTPUT_BUSWIDTH, typename INPT, typename OUTPT>
// void
// AdapterImpl<INPUT_BUSWIDTH, OUTPUT_BUSWIDTH, INPT, OUTPT>
//::do_width_conversion(
//  std::string               transfer_type,
//  sc_dt::sc_uint<64>        address,
//  payload_type& trans,
//  sc_core::sc_time&         delay
//)
//{
// SCWARN("Adapter")<< "bit width conversion is currently not supported";
//}

template <unsigned int INPUT_BUSWIDTH, unsigned int OUTPUT_BUSWIDTH, typename INPT, typename OUTPT>
bool AdapterImpl<INPUT_BUSWIDTH, OUTPUT_BUSWIDTH, INPT, OUTPT>::add_extension(payload_type& trans) {
    SCCWARN("Adapter") << "default dummy implementation for add_extensions";
    return false;
};

template <unsigned int INPUT_BUSWIDTH, unsigned int OUTPUT_BUSWIDTH, typename INPT, typename OUTPT>
bool AdapterImpl<INPUT_BUSWIDTH, OUTPUT_BUSWIDTH, INPT, OUTPT>::remove_extension(payload_type& trans) {
    return false;
};

template <unsigned int INPUT_BUSWIDTH, unsigned int OUTPUT_BUSWIDTH, typename INPT, typename OUTPT>
bool AdapterImpl<INPUT_BUSWIDTH, OUTPUT_BUSWIDTH, INPT, OUTPT>::journalTrace(const char* my, payload_type& trans, const phase_type& phase){
    // axi::axi4_extension* ext = trans.get_extension<axi::axi4_extension>();
    // if (_journal_file.is_open())
    //  _journal_file << sc_time_stamp() << ": " << my << " " << report::print(trans) << " " << report::print(*ext) << " phase " <<
    //  report::printAXI(phase) << endl;
};

template <unsigned int INPUT_BUSWIDTH, unsigned int OUTPUT_BUSWIDTH>
CLASSNAME<INPUT_BUSWIDTH, OUTPUT_BUSWIDTH>::CLASSNAME(sc_core::sc_module_name name)
: sc_module(name)
, target_socket("target_socket")
, initiator_socket("initiator_socket") {
    std::cout << "construction of general template implementation for " << basename() << std::endl;
    const char* myname = name;
    adapter1 = new AdapterImpl<INPUT_BUSWIDTH, OUTPUT_BUSWIDTH>(sc_gen_unique_name(myname));
    target_socket.bind(adapter1->target_socket);
    adapter1->initiator_socket.bind(initiator_socket);
};

// prototype definitions in order to get the explicit compile model to work
template class CLASSNAME<32, 32>;
template class CLASSNAME<64, 32>;
template class CLASSNAME<128, 32>;
template class CLASSNAME<256, 32>;
template class CLASSNAME<32, 64>;
template class CLASSNAME<64, 64>;
template class CLASSNAME<128, 64>;
template class CLASSNAME<256, 64>;
template class CLASSNAME<32, 128>;
template class CLASSNAME<64, 128>;
template class CLASSNAME<128, 128>;
template class CLASSNAME<256, 128>;

}; // namespace TLM2_COMMON
