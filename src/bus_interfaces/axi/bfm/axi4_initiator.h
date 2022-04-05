/*
 * initiator.h
 *
 *  Created on: Dec 20, 2021
 *      Author: eyck
 */

#ifndef _AXI_BFM_INITIATOR_H_
#define _AXI_BFM_INITIATOR_H_

#include <axi/axi_tlm.h>
#include <axi/fsm/base.h>
#include <axi/fsm/protocol_fsm.h>
#include <axi/signal_if.h>
#include <tlm_utils/peq_with_cb_and_phase.h>
#include <systemc>
namespace axi {
namespace bfm {

using namespace axi::fsm;

template<typename CFG>
void write_ar(tlm::tlm_generic_payload& trans, ar_ch<CFG, master_types>& ar);
template<typename CFG>
void write_aw(tlm::tlm_generic_payload& trans, aw_ch<CFG, master_types>& aw);
template<typename CFG>
void write_wdata(tlm::tlm_generic_payload& trans, wdata_ch<CFG, master_types>& wdata, unsigned beat, bool last=false);

template<typename CFG>
struct axi4_initiator: public sc_core::sc_module
, public aw_ch<CFG, master_types>
, public wdata_ch<CFG, master_types>
, public b_ch<CFG, master_types>
, public ar_ch<CFG, master_types>
, public rresp_ch<CFG, master_types>
, protected axi::fsm::base
, public axi::axi_fw_transport_if<axi::axi_protocol_types>
{
    SC_HAS_PROCESS(axi4_initiator);

    using payload_type = axi::axi_protocol_types::tlm_payload_type;
    using phase_type = axi::axi_protocol_types::tlm_phase_type;

    sc_core::sc_in<bool> clk_i{"clk_i"};

    axi::axi_target_socket<CFG::BUSWIDTH> tsckt{"tsckt"};

    axi4_initiator(sc_core::sc_module_name const& nm) :sc_core::sc_module(nm), base(CFG::BUSWIDTH/8){
        instance_name = name();
        tsckt(*this);
        SC_METHOD(clk_delay);
        sensitive<<clk_i.pos();
        SC_THREAD(ar_t);
        SC_THREAD(r_t);
        SC_THREAD(aw_t);
        SC_THREAD(wdata_t);
        SC_THREAD(b_t);
    }

private:
    void b_transport(payload_type& trans, sc_core::sc_time& t) override{
        trans.set_dmi_allowed(false);
        trans.set_response_status(tlm::TLM_OK_RESPONSE);
    }

    tlm::tlm_sync_enum nb_transport_fw(payload_type& trans, phase_type& phase, sc_core::sc_time& t) override {
        sc_core::sc_time delay; //FIXME: calculate delay correctly
        fw_peq.notify(trans, phase, delay);
        return tlm::TLM_ACCEPTED;
    }

    bool get_direct_mem_ptr(payload_type& trans, tlm::tlm_dmi& dmi_data) override {
        trans.set_dmi_allowed(false);
        return false;
    }

    unsigned int transport_dbg(payload_type& trans) override { return 0; }

    void end_of_elaboration() override { clk_if = dynamic_cast<sc_core::sc_clock*>(clk_i.get_interface()); }

    fsm_handle* create_fsm_handle() { return new fsm_handle(); }

    void setup_callbacks(fsm_handle* fsm_hndl);

    void clk_delay() { clk_delayed.notify(clk_if?clk_if->period()-1_ps:1_ps);}
    void ar_t();
    void r_t();
    void aw_t();
    void wdata_t();
    void b_t();
    std::array<unsigned, 3> outstanding_cnt;
    std::array<fsm_handle*, 3> active_req;
    std::array<fsm_handle*, 3> active_resp;
    sc_core::sc_clock* clk_if;
    sc_core::sc_event clk_delayed, r_end_req_evt, aw_evt, ar_evt;
    void nb_fw(payload_type& trans, const phase_type& phase) {
        auto delay=sc_core::SC_ZERO_TIME;
        base::nb_fw(trans, phase, delay);
    }
    tlm_utils::peq_with_cb_and_phase<axi4_initiator> fw_peq{this, &axi4_initiator::nb_fw};
    std::unordered_map<unsigned, std::deque<fsm_handle*>> rd_resp_by_id, wr_resp_by_id;
    sc_core::sc_buffer<uint8_t> wdata_vl;
};

}
}

template<typename CFG>
inline void axi::bfm::write_ar(tlm::tlm_generic_payload &trans, ar_ch<CFG, master_types> &ar) {
    sc_dt::sc_uint<CFG::ADDRWIDTH> addr = trans.get_address();
    ar.ar_addr.write(addr);
    if(auto ext = trans.get_extension<axi::axi4_extension>()){
        ar.ar_id.write(sc_dt::sc_uint<CFG::IDWIDTH>(ext->get_id()));
        ar.ar_len.write(sc_dt::sc_uint<8>(ext->get_length()));
        ar.ar_size.write(sc_dt::sc_uint<3>(ext->get_size()));
        ar.ar_burst.write(sc_dt::sc_uint<2>(axi::to_int(ext->get_burst())));
        ar.ar_cache.write(sc_dt::sc_uint<4>(ext->get_cache()));
        ar.ar_prot.write(ext->get_prot());
        ar.ar_qos.write(ext->get_qos());
        if(ar.ar_user.get_interface())
            ar.ar_user->write(ext->get_user(axi::common::id_type::CTRL));
    }
}

template<typename CFG>
inline void axi::bfm::write_aw(tlm::tlm_generic_payload &trans, aw_ch<CFG, master_types> &aw) {
    sc_dt::sc_uint<CFG::ADDRWIDTH> addr = trans.get_address();
    aw.aw_addr.write(addr);
    if(auto ext = trans.get_extension<axi::axi4_extension>()){
        aw.aw_id.write(sc_dt::sc_uint<CFG::IDWIDTH>(ext->get_id()));
        aw.aw_len.write(sc_dt::sc_uint<8>(ext->get_length()));
        aw.aw_size.write(sc_dt::sc_uint<3>(ext->get_size()));
        aw.aw_burst.write(sc_dt::sc_uint<2>(axi::to_int(ext->get_burst())));
        aw.aw_cache.write(sc_dt::sc_uint<4>(ext->get_cache()));
        aw.aw_prot.write(ext->get_prot());
        aw.aw_qos.write(ext->get_qos());
        if(aw.aw_user.get_interface())
            aw.aw_user->write(ext->get_user(axi::common::id_type::CTRL));
    }
}

// FIXME: strb not yet correct
template<typename CFG>
inline void axi::bfm::write_wdata(tlm::tlm_generic_payload &trans, wdata_ch<CFG, master_types> &wdata, unsigned beat, bool last) {
    typename CFG::data_t data{0};
    sc_dt::sc_uint<CFG::BUSWIDTH / 8> strb{std::numeric_limits<unsigned>::max()};
    auto ext = trans.get_extension<axi::axi4_extension>();
    auto size = 1u<<ext->get_size();
    auto offset = trans.get_address() & (CFG::BUSWIDTH/8-1);
    auto beptr = trans.get_byte_enable_length()?trans.get_byte_enable_ptr()+beat*size:nullptr;
    if(offset && (size+offset)>(CFG::BUSWIDTH/8)) { // un-aligned multi-beat access
        if(beat==0){
            auto bptr = trans.get_data_ptr();
            for (size_t i = offset; i < size; ++i, ++bptr) {
                auto bit_offs = i*8;
                data(bit_offs+ 7, bit_offs) = *bptr;
                if(beptr){
                    strb[i]=*beptr==0xff;
                    ++beptr;
                }
            }
        } else {
            auto beat_start_idx = beat * size - offset;
            auto data_len = trans.get_data_length();
            auto bptr = trans.get_data_ptr()+ beat_start_idx;
            for (size_t i = offset; i < size && (beat_start_idx+i)<data_len; ++i, ++bptr) {
                auto bit_offs = i*8;
                data(bit_offs+ 7, bit_offs) = *bptr;
                if(beptr){
                    strb[i]=*beptr==0xff;
                    ++beptr;
                }
            }
        }
    } else { // aligned or single beat access
        auto bptr = trans.get_data_ptr() + beat * size;
        for (size_t i = 0; i < size; ++i, ++bptr) {
            auto bit_offs = (offset+i)*8;
            data(bit_offs+ 7, bit_offs) = *bptr;
            if(beptr){
                strb[i]=*beptr==0xff;
                ++beptr;
            }
        }
    }
    wdata.w_data.write(data);
    wdata.w_strb.write(strb);
    wdata.w_id.write(ext->get_id());
    if(wdata.w_user.get_interface())
        wdata.w_user->write(ext->get_user(axi::common::id_type::DATA));
}

template<typename CFG>
inline void axi::bfm::axi4_initiator<CFG>::setup_callbacks(fsm_handle* fsm_hndl) {
    fsm_hndl->fsm->cb[RequestPhaseBeg] = [this, fsm_hndl]() -> void {
        fsm_hndl->beat_count = 0;
        outstanding_cnt[fsm_hndl->trans->get_command()]++;
    };
    fsm_hndl->fsm->cb[BegPartReqE] = [this, fsm_hndl]() -> void {
        sc_assert(fsm_hndl->trans->is_write());
        if(fsm_hndl->beat_count == 0){
            write_aw(*fsm_hndl->trans, *this);
            aw_evt.notify();
        }
        write_wdata(*fsm_hndl->trans, *this, fsm_hndl->beat_count);
        active_req[tlm::TLM_WRITE_COMMAND]=fsm_hndl;
        wdata_vl.write(0x1);
    };
    fsm_hndl->fsm->cb[EndPartReqE] = [this, fsm_hndl]() -> void {
        active_req[tlm::TLM_WRITE_COMMAND]=nullptr;
        this->aw_valid.write(false);
        tlm::tlm_phase phase = axi::END_PARTIAL_REQ;
        sc_core::sc_time t;//(clk_if?clk_if->period()-1_ps:sc_core::SC_ZERO_TIME);
        fsm_hndl->beat_count++;
        auto ret = tsckt->nb_transport_bw(*fsm_hndl->trans, phase, t);
    };
    fsm_hndl->fsm->cb[BegReqE] = [this, fsm_hndl]() -> void {
        switch(fsm_hndl->trans->get_command()){
        case tlm::TLM_READ_COMMAND:
            active_req[tlm::TLM_READ_COMMAND]=fsm_hndl;
            write_ar(*fsm_hndl->trans, *this);
            ar_evt.notify();
        break;
        case tlm::TLM_WRITE_COMMAND:
            active_req[tlm::TLM_WRITE_COMMAND]=fsm_hndl;
            if(fsm_hndl->beat_count == 0){
                write_aw(*fsm_hndl->trans, *this);
                aw_evt.notify();
            }
            write_wdata(*fsm_hndl->trans, *this, fsm_hndl->beat_count, true);
            wdata_vl.write(0x3);
        }
    };
    fsm_hndl->fsm->cb[EndReqE] = [this, fsm_hndl]() -> void {
        switch(fsm_hndl->trans->get_command()){
        case tlm::TLM_READ_COMMAND:
            rd_resp_by_id[axi::get_axi_id(*fsm_hndl->trans)].push_back(active_req[tlm::TLM_READ_COMMAND]);
            active_req[tlm::TLM_READ_COMMAND]=nullptr;
        break;
        case tlm::TLM_WRITE_COMMAND:
            wr_resp_by_id[axi::get_axi_id(*fsm_hndl->trans)].push_back(active_req[tlm::TLM_WRITE_COMMAND]);
            active_req[tlm::TLM_WRITE_COMMAND]=nullptr;
        }
        tlm::tlm_phase phase = tlm::END_REQ;
        sc_core::sc_time t(sc_core::SC_ZERO_TIME);
        auto ret = tsckt->nb_transport_bw(*fsm_hndl->trans, phase, t);
        fsm_hndl->trans->set_response_status(tlm::TLM_OK_RESPONSE);
        if(auto ext3 = fsm_hndl->trans->get_extension<axi3_extension>()) {
            ext3->set_resp(resp_e::OKAY);
        } else if(auto ext4 = fsm_hndl->trans->get_extension<axi4_extension>()) {
            ext4->set_resp(resp_e::OKAY);
        } else if(auto exta = fsm_hndl->trans->get_extension<ace_extension>()) {
            exta->set_resp(resp_e::OKAY);
        } else
            sc_assert(false && "No valid AXITLM extension found!");
    };
    fsm_hndl->fsm->cb[BegPartRespE] = [this, fsm_hndl]() -> void {
        // scheduling the response
        assert(fsm_hndl->trans->is_read());
        tlm::tlm_phase phase = axi::BEGIN_PARTIAL_RESP;
        sc_core::sc_time t(sc_core::SC_ZERO_TIME);
        auto ret = tsckt->nb_transport_bw(*fsm_hndl->trans, phase, t);
    };
    fsm_hndl->fsm->cb[EndPartRespE] = [this, fsm_hndl]() -> void {
        fsm_hndl->beat_count++;
        r_end_req_evt.notify();
    };
    fsm_hndl->fsm->cb[BegRespE] = [this, fsm_hndl]() -> void {
        // scheduling the response
        tlm::tlm_phase phase = tlm::BEGIN_RESP;
        sc_core::sc_time t(sc_core::SC_ZERO_TIME);
        auto ret = tsckt->nb_transport_bw(*fsm_hndl->trans, phase, t);
    };
    fsm_hndl->fsm->cb[EndRespE] = [this, fsm_hndl]() -> void {
        r_end_req_evt.notify();
        if(fsm_hndl->trans->is_read())  rd_resp_by_id[axi::get_axi_id(*fsm_hndl->trans)].pop_front();
        if(fsm_hndl->trans->is_write()) wr_resp_by_id[axi::get_axi_id(*fsm_hndl->trans)].pop_front();
    };
}

template<typename CFG>
inline void axi::bfm::axi4_initiator<CFG>::ar_t() {
    while(true){
        this->ar_valid.write(false);
        wait(ar_evt);
        this->ar_valid.write(true);
        do{
            wait(this->ar_ready.posedge_event() | clk_delayed);
            if(this->ar_ready.read())
                react(axi::fsm::protocol_time_point_e::EndReqE, active_req[tlm::TLM_READ_COMMAND]);
        } while(!this->ar_ready.read());
        wait(clk_i.posedge_event());
    }
}

template<typename CFG>
inline void axi::bfm::axi4_initiator<CFG>::r_t() {
    this->r_ready.write(false);
    while(true) {
        wait(this->r_valid.posedge_event() | clk_i.negedge_event());
        if(this->r_valid.event() || (!active_resp[tlm::TLM_READ_COMMAND] && this->r_valid.read())) {
            auto id = this->r_id.read();
            auto data = this->r_data.read();
            auto resp = this->r_resp.read();
            auto& q = rd_resp_by_id[id];
            sc_assert(q.size());
            auto* fsm_hndl = q.front();
            auto size = axi::get_burst_size(*fsm_hndl->trans);
            auto bptr=fsm_hndl->trans->get_data_ptr()+fsm_hndl->beat_count*size;
            for(size_t i=0; i<size; ++i) {
                 *(bptr+i)=data(i*8+7, i*8).to_uint();
            }
            axi::axi4_extension* e;
            fsm_hndl->trans->get_extension(e);
            e->set_resp(axi::into<axi::resp_e>(resp));
            e->add_to_response_array(*e);
            auto tp = this->r_last.read()?axi::fsm::protocol_time_point_e::BegRespE:axi::fsm::protocol_time_point_e::BegPartRespE;
            react(tp, fsm_hndl);
            wait(r_end_req_evt);
            this->r_ready.write(true);
            wait(clk_i.posedge_event());
            this->r_ready.write(false);
        }
    }
}

template<typename CFG>
inline void axi::bfm::axi4_initiator<CFG>::aw_t() {
    while(true){
        this->aw_valid.write(false);
        wait(aw_evt);
        this->aw_valid.write(true);
        do{
            wait(this->aw_ready.posedge_event() | clk_delayed);
        } while(!this->aw_ready.read());
        wait(clk_i.posedge_event());
    }
}

template<typename CFG>
inline void axi::bfm::axi4_initiator<CFG>::wdata_t() {
    while(true){
        this->w_valid.write(false);
        this->w_last.write(false);
        wait(wdata_vl.default_event());
        auto val = wdata_vl.read();
        this->w_valid.write(val&0x1);
        this->w_last.write(val&0x2);
        do{
        wait(this->w_ready.posedge_event() | clk_delayed);
            if(this->w_ready.read()) {
                auto evt = this->w_last.read()?axi::fsm::protocol_time_point_e::EndReqE:axi::fsm::protocol_time_point_e::EndPartReqE;
                react(evt, active_req[tlm::TLM_WRITE_COMMAND]);
            }
        } while(!this->w_ready.read());
        wait(clk_i.posedge_event());
    }
}


template<typename CFG>
inline void axi::bfm::axi4_initiator<CFG>::b_t() {
    this->r_ready.write(false);
    while(true) {
        wait(this->b_valid.posedge_event() | clk_i.negedge_event());
        if(this->b_valid.event() || (!active_resp[tlm::TLM_WRITE_COMMAND] && this->b_valid.read())) {
            auto id = this->b_id.read();
            auto resp = this->b_resp.read();
            auto& q = wr_resp_by_id[id];
            sc_assert(q.size());
            auto* fsm_hndl = q.front();
            axi::axi4_extension* e;
            fsm_hndl->trans->get_extension(e);
            e->set_resp(axi::into<axi::resp_e>(resp));
            react(axi::fsm::protocol_time_point_e::BegRespE, fsm_hndl);
            wait(r_end_req_evt);
            this->b_ready.write(true);
            wait(clk_i.posedge_event());
            this->b_ready.write(false);
        }
    }
}

#endif /* _AXI_BFM_INITIATOR_H_ */
