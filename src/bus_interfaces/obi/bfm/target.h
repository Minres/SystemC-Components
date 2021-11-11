#pragma once

#include <obi/obi_tlm.h>
#include <scc/report.h>
#include <tlm/scc/initiator_mixin.h>
#include <tlm/scc/scv/tlm_rec_initiator_socket.h>
#include <tlm/scc/tlm_id.h>
#include <tlm/scc/tlm_mm.h>
#include <tlm/scc/tlm_gp_shared.h>
#include <scc/peq.h>
#include <systemc>
#include <tlm>

#include <memory>
#include <queue>
#include <tuple>
#include <unordered_map>

namespace obi {
namespace bfm {
template <unsigned int DATA_WIDTH = 32, unsigned int ADDR_WIDTH = 32, unsigned int ID_WIDTH = 0,
        unsigned int USER_WIDTH = 0>
class target : public sc_core::sc_module {
public:
    using payload_type = tlm::tlm_base_protocol_types::tlm_payload_type;
    using phase_type = tlm::tlm_base_protocol_types::tlm_phase_type;

    template <unsigned WIDTH = 0, typename TYPE = sc_dt::sc_uint<WIDTH>, int N = 1>
    using sc_in_opt = sc_core::sc_port<sc_core::sc_signal_in_if<TYPE>, N, sc_core::SC_ZERO_OR_MORE_BOUND>;
    template <unsigned WIDTH = 0, typename TYPE = sc_dt::sc_uint<WIDTH>, int N = 1>
    using sc_out_opt = sc_core::sc_port<sc_core::sc_signal_write_if<TYPE>, N, sc_core::SC_ZERO_OR_MORE_BOUND>;

    SC_HAS_PROCESS(target);

    target(sc_core::sc_module_name nm);

    tlm::scc::initiator_mixin<tlm::scc::scv::tlm_rec_initiator_socket<DATA_WIDTH>>  isckt{"isckt"};
    // Global signals
    sc_core::sc_in<bool> clk_i{"clk_i"};
    sc_core::sc_in<bool> resetn_i{"resetn_i"}; // active low reset
    // A Channel signals
    sc_core::sc_in<bool> req_i{"req_i"};
    sc_core::sc_out<bool> gnt_o{"gnt_o"};
    sc_core::sc_in<sc_dt::sc_uint<ADDR_WIDTH>> addr_i{"addr_i"};
    sc_core::sc_in<bool> we_i{"we_i"};
    sc_core::sc_in<sc_dt::sc_uint<DATA_WIDTH/8>> be_i{"be_i"};
    sc_core::sc_in<sc_dt::sc_uint<DATA_WIDTH>> wdata_i{"wdata_i"};
    sc_in_opt<USER_WIDTH> auser_i{"auser_i"};
    sc_in_opt<USER_WIDTH> wuser_i{"wuser_i"};
    sc_in_opt<ID_WIDTH> aid_i{"aid_i"};
    // R Channel signals
    sc_core::sc_out<bool> rvalid_o{"rvalid_o"};
    sc_core::sc_in<bool> rready_i{"rready_i"};
    sc_core::sc_out<sc_dt::sc_uint<DATA_WIDTH>> rdata_o{"rdata_o"};
    sc_core::sc_out<bool> err_o{"err_o"};
    sc_out_opt<USER_WIDTH> ruser_o{"ruser_o"};
    sc_out_opt<ID_WIDTH> r_id_o{"r_id_o"};

    tlm::tlm_sync_enum nb_transport_bw(payload_type& trans, phase_type& phase, sc_core::sc_time& t);

#ifdef HAS_CCI
    cci::cci_param<sc_core::sc_time> sample_delay{"sample_delay", 0_ns};
#else
    sc_core::sc_time sample_delay{0_ns};
#endif
private:
    void clk_cb();
    void achannel_req_t();
    void rchannel_rsp_t();

    scc::peq<tlm::scc::tlm_gp_shared_ptr> achannel_rsp;
    tlm::scc::tlm_gp_shared_ptr achannel_active_tx;
    bool achannel_tx_finished{false};
    scc::peq<tlm::scc::tlm_gp_shared_ptr> rchannel_pending_rsp;
    scc::peq<tlm::scc::tlm_gp_shared_ptr> rchannel_rsp;
    //std::pair<tlm::scc::tlm_gp_shared_ptr, tlm::tlm_phase> rchannel_req;
    std::vector<std::deque<std::tuple<tlm::scc::tlm_gp_shared_ptr, tlm::tlm_phase, uint64_t>>> pending_tx;
    uint64_t clk_cnt{0};
};

/////////////////////////////////////////////////////////////////////////////////////////
// Class definition
/////////////////////////////////////////////////////////////////////////////////////////
template <unsigned int DATA_WIDTH, unsigned int ADDR_WIDTH, unsigned int ID_WIDTH, unsigned int USER_WIDTH>
inline target<DATA_WIDTH, ADDR_WIDTH, ID_WIDTH, USER_WIDTH>::target::target(
        sc_core::sc_module_name nm)
: sc_module(nm) {
    isckt.register_nb_transport_bw(
            [this](payload_type& trans, phase_type& phase, sc_core::sc_time& t) -> tlm::tlm_sync_enum {
        return nb_transport_bw(trans, phase, t);
    });
    SC_METHOD(clk_cb)
    sensitive << clk_i.pos() << resetn_i.neg();
    SC_THREAD(achannel_req_t)
    SC_THREAD(rchannel_rsp_t);
}

template <unsigned int DATA_WIDTH, unsigned int ADDR_WIDTH, unsigned int ID_WIDTH, unsigned int USER_WIDTH>
inline void target<DATA_WIDTH, ADDR_WIDTH, ID_WIDTH, USER_WIDTH>::target::clk_cb() {
    sc_core::sc_time delay = sc_core::SC_ZERO_TIME;
    sc_dt::sc_biguint<DATA_WIDTH> write_data{0};
    if(clk_i.event()) {
        clk_cnt++;
        if(rchannel_pending_rsp.has_next())
            rchannel_rsp.notify(rchannel_pending_rsp.get());
    }
}

template <unsigned int DATA_WIDTH, unsigned int ADDR_WIDTH, unsigned int ID_WIDTH, unsigned int USER_WIDTH>
inline tlm::tlm_sync_enum
target<DATA_WIDTH, ADDR_WIDTH, ID_WIDTH, USER_WIDTH>::target::nb_transport_bw(
        payload_type& trans, phase_type& phase, sc_core::sc_time& t) {
    auto id = obi::get_obi_id(trans);
    auto* ext = trans.get_extension<obi::obi_extension>();
    sc_assert(ext && "obi_extension missing");
    auto& q = pending_tx[ext->get_id()].front();
    switch(phase){
    case tlm::END_REQ:{
        if(achannel_active_tx.get()!=&trans)
            SCCFATAL(SCMOD)<<"Illegal response from TLM target";
        achannel_rsp.notify(&trans, t);
        return tlm::TLM_ACCEPTED;
    }
    case tlm::BEGIN_RESP:{
        if(achannel_active_tx.get()==&trans) {
            if(!achannel_tx_finished) achannel_rsp.notify(&trans, t);
            rchannel_pending_rsp.notify(&trans);
        } else {
            rchannel_rsp.notify(&trans, t);
        }
        return tlm::TLM_ACCEPTED;
    }
    default:
        SCCWARN(SCMOD) << phase << " is unsupported phase transaction combination";
        return tlm::TLM_ACCEPTED;
    }
}

template<unsigned int DATA_WIDTH, unsigned int ADDR_WIDTH, unsigned int ID_WIDTH, unsigned int USER_WIDTH>
inline void target<DATA_WIDTH, ADDR_WIDTH, ID_WIDTH, USER_WIDTH>::achannel_req_t() {
    while(true) {
        gnt_o.write(false);
        if(!req_i.read())
            wait(req_i.posedge_event());
        achannel_active_tx = tlm::scc::tlm_mm<>::get().allocate<obi::obi_extension>();
        achannel_active_tx->set_address(addr_i.read());
        achannel_active_tx->set_command(we_i.read() ? tlm::TLM_WRITE_COMMAND : tlm::TLM_READ_COMMAND);
        auto be = static_cast<unsigned>(be_i.read());
        auto cnt = util::bit_count(be);
        achannel_active_tx->set_streaming_width(cnt);
        achannel_active_tx->set_data_length(cnt);
        achannel_active_tx->set_data_ptr(new uint8_t[cnt]);
        if(we_i.read()) {
            auto bus_data = wdata_i.read();
            auto tx_data{sc_dt::sc_uint<DATA_WIDTH>(0)};
            auto tx_byte_idx=0;
            for(auto i = 0U, be_idx=0U; i<DATA_WIDTH; i+=8, ++be_idx){
                if(be&(1U<<be_idx)){
                    *(achannel_active_tx->get_data_ptr()+tx_byte_idx)=bus_data.range(i+7, i).to_uint();
                    tx_byte_idx++;
                }
            }
            SCCTRACE(SCMOD)<<"Got write request to address 0x"<<std::hex<<achannel_active_tx->get_address();
        } else
            SCCTRACE(SCMOD)<<"Got read request to address 0x"<<std::hex<<achannel_active_tx->get_address();
        auto ext = achannel_active_tx->get_extension<obi::obi_extension>();
        if(ID_WIDTH && aid_i.get_interface())
            ext->set_id(aid_i->read());
        if(USER_WIDTH){
            if(auser_i.get_interface())
                ext->set_auser(auser_i->read());
            if(wuser_i.get_interface() && we_i.read())
                ext->set_duser(wuser_i->read());
        }
        phase_type phase = tlm::BEGIN_REQ;
        auto delay = sc_core::SC_ZERO_TIME;
        achannel_tx_finished=false;
        auto ret = isckt->nb_transport_fw(*achannel_active_tx, phase, delay);
        auto id = ext->get_id();
        if(pending_tx.size()<=id) pending_tx.resize(id+1);
        pending_tx[id].emplace_back(std::make_tuple(achannel_active_tx, phase, clk_cnt));
        if (ret == tlm::TLM_UPDATED) {
            achannel_tx_finished=true;
            if (phase == tlm::BEGIN_RESP) {
                rchannel_pending_rsp.notify(achannel_active_tx);
            } else if(phase != tlm::END_REQ){
                SCCFATAL(SCMOD) << "Bummer: nyi";
            }
        } else {
            SCCTRACE(SCMOD)<<"waiting for TLM reponse";
            auto resp=achannel_rsp.get();
            sc_assert(resp.get()==achannel_active_tx.get());
            achannel_tx_finished=true;
        }
        gnt_o.write(true);
        wait(clk_i.posedge_event());
        achannel_active_tx=nullptr;
        // wait until RTL output has settled
        wait(sample_delay);
    }
}

template<unsigned int DATA_WIDTH, unsigned int ADDR_WIDTH, unsigned int ID_WIDTH, unsigned int USER_WIDTH>
inline void target<DATA_WIDTH, ADDR_WIDTH, ID_WIDTH, USER_WIDTH>::rchannel_rsp_t() {
    rdata_o.write(0);
    err_o.write(false);
    if(ID_WIDTH && r_id_o.get_interface())
        r_id_o->write(0);
    if(USER_WIDTH && ruser_o.get_interface())
        ruser_o->write(0);
    while(true) {
        rvalid_o.write(false);
        tlm::scc::tlm_gp_shared_ptr tx=rchannel_rsp.get();
        if(tx->get_command()== tlm::TLM_READ_COMMAND){
            auto offset = tx->get_address()%(DATA_WIDTH/8);
            auto rx_data{sc_dt::sc_uint<DATA_WIDTH>(0)};
            for(unsigned i=0U, j=offset*8; i<tx->get_data_length(); ++i, j+=8){
                rx_data.range(j+7, j)= *(tx->get_data_ptr()+i);
            }
            rdata_o.write(rx_data);
            SCCTRACE(SCMOD)<<"responding to read request to address 0x"<<std::hex<<tx->get_address();
        } else
            SCCTRACE(SCMOD)<<"responding to write request to address 0x"<<std::hex<<tx->get_address();
        err_o.write(tx->get_response_status()!=tlm::TLM_OK_RESPONSE);
        if(ID_WIDTH || USER_WIDTH){
            auto ext = tx->get_extension<obi::obi_extension>();
            if(ID_WIDTH && r_id_o.get_interface())
                r_id_o->write(ext->get_id());
            if(USER_WIDTH && ruser_o.get_interface())
                ruser_o->write(ext->get_duser());
        }
        rvalid_o.write(true);
        wait(sample_delay); // let rready settle
        while(!rready_i.read()) wait(rready_i.value_changed_event());
        phase_type phase = tlm::END_RESP;
        auto delay = sc_core::SC_ZERO_TIME;
        auto ret = isckt->nb_transport_fw(*tx, phase, delay);
        wait(clk_i.posedge_event());
    }
}
} // namespace bfm
} // namespace axi

