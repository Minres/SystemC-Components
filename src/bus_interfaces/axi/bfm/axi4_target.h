/*
 * initiator.h
 *
 *  Created on: Dec 20, 2021
 *      Author: eyck
 */

#ifndef _AXI_BFM_TARGET_H_
#define _AXI_BFM_TARGET_H_

#include <axi/axi_tlm.h>
#include <axi/fsm/base.h>
#include <axi/fsm/protocol_fsm.h>
#include <axi/signal_if.h>
#include <tlm/scc/tlm_mm.h>
#include <systemc>
namespace axi {
namespace bfm {

using namespace axi::fsm;

template<typename CFG>
struct axi4_target: public sc_core::sc_module
, public aw_ch<CFG, slave_types>
, public wdata_ch<CFG, slave_types>
, public b_ch<CFG, slave_types>
, public ar_ch<CFG, slave_types>
, public rresp_ch<CFG, slave_types>
, protected axi::fsm::base
, public axi::axi_bw_transport_if<axi::axi_protocol_types>
{
    SC_HAS_PROCESS(axi4_target);

    using payload_type = axi::axi_protocol_types::tlm_payload_type;
    using phase_type = axi::axi_protocol_types::tlm_phase_type;

    sc_core::sc_in<bool> clk_i{"clk_i"};

    axi::axi_initiator_socket<CFG::BUSWIDTH> isckt{"isckt"};

    axi4_target(sc_core::sc_module_name const& nm):sc_core::sc_module(nm), base(CFG::BUSWIDTH/8){
        instance_name = name();
        isckt.bind(*this);
        SC_METHOD(clk_delay);
        sensitive<<clk_i.pos();
        SC_THREAD(ar_t);
        SC_THREAD(rresp_t);
        SC_THREAD(aw_t);
        SC_THREAD(wdata_t);
        SC_THREAD(bresp_t);
    }

private:
    tlm::tlm_sync_enum nb_transport_bw(payload_type& trans, phase_type& phase, sc_core::sc_time& t);

    void invalidate_direct_mem_ptr(sc_dt::uint64 start_range, sc_dt::uint64 end_range);

    void end_of_elaboration() override { clk_if = dynamic_cast<sc_core::sc_clock*>(clk_i.get_interface()); }

    axi::fsm::fsm_handle* create_fsm_handle() override { return new fsm_handle(); }

    void setup_callbacks(axi::fsm::fsm_handle*) override;

    void clk_delay() { clk_delayed.notify(clk_if?clk_if->period()-1_ps:1_ps);}
    void ar_t();
    void rresp_t();
    void aw_t();
    void wdata_t();
    void bresp_t();
    static typename CFG::data_t  get_read_data_for_beat(fsm::fsm_handle *fsm_hndl);

    sc_core::sc_clock* clk_if;
    sc_core::sc_event clk_delayed, ar_end_req_evt, wdata_end_req_evt, bresp_evt;
    std::array<fsm_handle*, 3> active_req_beat;
    std::array<fsm_handle*, 3> active_req;
    std::array<fsm_handle*, 3> active_resp_beat;
    payload_type* current_wr_tx{nullptr};
    sc_core::sc_buffer<uint8_t> rresp_vl;
};

}
}

template<typename CFG>
inline tlm::tlm_sync_enum axi::bfm::axi4_target<CFG>::nb_transport_bw(payload_type &trans, phase_type &phase, sc_core::sc_time &t) {
    auto ret = tlm::TLM_ACCEPTED;
    sc_core::sc_time delay; //FIXME: calculate correct time
    SCCTRACE(SCMOD) << "nb_transport_bw " << phase << " of trans " << trans;
    if(phase == axi::END_PARTIAL_REQ || phase == tlm::END_REQ) { // read/write
        schedule(phase == tlm::END_REQ ? EndReqE : EndPartReqE, &trans, delay, false);
    } else if(phase == axi::BEGIN_PARTIAL_RESP || phase == tlm::BEGIN_RESP) { // read/write response
        schedule(phase == tlm::BEGIN_RESP ? BegRespE : BegPartRespE, &trans, delay, false);
    } else
        SCCFATAL(SCMOD)<<"Illegal phase received: "<<phase;
    return ret;
}

template<typename CFG>
inline void axi::bfm::axi4_target<CFG>::invalidate_direct_mem_ptr(sc_dt::uint64 start_range, sc_dt::uint64 end_range) {
}

template<typename CFG>
typename CFG::data_t axi::bfm::axi4_target<CFG>::get_read_data_for_beat(fsm_handle *fsm_hndl) {
    auto beat_count = fsm_hndl->beat_count;
    auto size = axi::get_burst_size(*fsm_hndl->trans);
    auto offset = (fsm_hndl->trans->get_address()+beat_count*size) & (CFG::BUSWIDTH/8-1);
    auto bptr = fsm_hndl->trans->get_data_ptr() + beat_count * size;
    typename CFG::data_t data { 0 };
    for (size_t i = 0; i < size; ++i) {
        auto bit_offs = (offset+i)*8;
        data(bit_offs+ 7, bit_offs) = *(bptr + i);
    }
    return data;
}

template<typename CFG>
inline void axi::bfm::axi4_target<CFG>::setup_callbacks(fsm_handle* fsm_hndl) {
    fsm_hndl->fsm->cb[RequestPhaseBeg] = [this, fsm_hndl]() -> void {
        fsm_hndl->beat_count = 0;
    };
    fsm_hndl->fsm->cb[BegPartReqE] = [this, fsm_hndl]() -> void {
        sc_assert(fsm_hndl->trans->get_command()== tlm::TLM_WRITE_COMMAND);
        tlm::tlm_phase phase = axi::BEGIN_PARTIAL_REQ;
        sc_core::sc_time t(sc_core::SC_ZERO_TIME);
        auto ret = isckt->nb_transport_fw(*fsm_hndl->trans, phase, t);
        if(ret == tlm::TLM_UPDATED) {
            schedule(EndPartReqE, fsm_hndl->trans, t, true);
        }
    };
    fsm_hndl->fsm->cb[EndPartReqE] = [this, fsm_hndl]() -> void {
        fsm_hndl->beat_count++;
        wdata_end_req_evt.notify();
        active_req_beat[tlm::TLM_WRITE_COMMAND]=nullptr;
    };
    fsm_hndl->fsm->cb[BegReqE] = [this, fsm_hndl]() -> void {
        tlm::tlm_phase phase = tlm::BEGIN_REQ;
        sc_core::sc_time t(sc_core::SC_ZERO_TIME);
        auto ret = isckt->nb_transport_fw(*fsm_hndl->trans, phase, t);
        if(ret == tlm::TLM_UPDATED) {
            schedule(EndReqE, fsm_hndl->trans, t, true);
        }
    };
    fsm_hndl->fsm->cb[EndReqE] = [this, fsm_hndl]() -> void {
        switch(fsm_hndl->trans->get_command()){
        case tlm::TLM_READ_COMMAND:
            ar_end_req_evt.notify();
            active_req_beat[tlm::TLM_READ_COMMAND]=nullptr;
            break;
        case tlm::TLM_WRITE_COMMAND:
            wdata_end_req_evt.notify();
            active_req_beat[tlm::TLM_WRITE_COMMAND]=nullptr;
            break;
        }
    };
    fsm_hndl->fsm->cb[BegPartRespE] = [this, fsm_hndl]() -> void {
        assert(fsm_hndl->trans->is_read());
        active_resp_beat[tlm::TLM_READ_COMMAND]=fsm_hndl;
        auto ext = fsm_hndl->trans->get_extension<axi::axi4_extension>();
        this->r_data.write( get_read_data_for_beat(fsm_hndl));
        this->r_id.write(ext->get_id());
        this->r_resp.write(axi::to_int(ext->get_resp()));
        rresp_vl=0x1;
    };
    fsm_hndl->fsm->cb[EndPartRespE] = [this, fsm_hndl]() -> void {
        // scheduling the response
        assert(fsm_hndl->trans->is_read());
        tlm::tlm_phase phase = axi::END_PARTIAL_RESP;
        sc_core::sc_time t(sc_core::SC_ZERO_TIME);
        auto ret = isckt->nb_transport_fw(*fsm_hndl->trans, phase, t);
        active_resp_beat[tlm::TLM_READ_COMMAND]=nullptr;
    };
    fsm_hndl->fsm->cb[BegRespE] = [this, fsm_hndl]() -> void {
        auto size = axi::get_burst_size(*fsm_hndl->trans);
        active_resp_beat[fsm_hndl->trans->get_command()]=fsm_hndl;
        switch(fsm_hndl->trans->get_command()){
        case tlm::TLM_READ_COMMAND: {
            auto ext = fsm_hndl->trans->get_extension<axi::axi4_extension>();
            this->r_data.write(get_read_data_for_beat(fsm_hndl));
            this->r_id.write(ext->get_id());
            this->r_resp.write(axi::to_int(ext->get_resp()));
            rresp_vl=0x3;
        }
        break;
        case tlm::TLM_WRITE_COMMAND: {
            auto ext = fsm_hndl->trans->get_extension<axi::axi4_extension>();
            this->b_id.write(ext->get_id());
            this->b_resp.write(axi::to_int(ext->get_resp()));
            bresp_evt.notify();
        }
        break;
        }

    };
    fsm_hndl->fsm->cb[EndRespE] = [this, fsm_hndl]() -> void {
        // scheduling the response
        tlm::tlm_phase phase = tlm::END_RESP;
        sc_core::sc_time t(sc_core::SC_ZERO_TIME);
        auto ret = isckt->nb_transport_fw(*fsm_hndl->trans, phase, t);
        active_resp_beat[fsm_hndl->trans->get_command()]=nullptr;
    };
}

template<typename CFG>
inline void axi::bfm::axi4_target<CFG>::ar_t() {
    this->ar_ready.write(false);
    auto arid=this->ar_id.read();
    auto arlen=this->ar_len.read();
    auto arsize = this->ar_size.read();
    auto data_len = (1<<arsize)*(arlen+1);
    while(true) {
        wait(this->ar_valid.posedge_event() | clk_i.negedge_event());
        if(this->ar_valid.read()) {
            arid=this->ar_id.read();
            arlen=this->ar_len.read();
            arsize = this->ar_size.read();
            data_len = (1<<arsize)*(arlen+1);
            auto gp =  tlm::scc::tlm_mm<>::get().allocate<axi::axi4_extension>(data_len);
            gp->set_address(this->ar_addr.read());
            gp->set_command(tlm::TLM_READ_COMMAND);
            axi::axi4_extension* ext;
            gp->get_extension(ext);
            ext->set_id(arid);
            ext->set_length(arlen);
            ext->set_size(arsize);
            ext->set_burst(axi::into<axi::burst_e>(this->ar_burst.read()));
            active_req_beat[tlm::TLM_READ_COMMAND] = find_or_create(gp);
            react(axi::fsm::protocol_time_point_e::BegReqE, active_req_beat[tlm::TLM_READ_COMMAND]);
            wait(ar_end_req_evt);
            this->ar_ready.write(true);
            wait(clk_i.posedge_event());
            this->ar_ready.write(false);
        }
    }
}

template<typename CFG>
inline void axi::bfm::axi4_target<CFG>::rresp_t() {
    while(true){
        wait(rresp_vl.default_event());
        auto val = rresp_vl.read();
        this->r_valid.write(val&0x1);
        this->r_last.write(val&0x2);
        do{
        wait(this->r_ready.posedge_event() | clk_delayed);
            if(this->r_ready.read()) {
                auto evt = this->r_last.read()?axi::fsm::protocol_time_point_e::EndRespE:axi::fsm::protocol_time_point_e::EndPartRespE;
                react(evt, active_resp_beat[tlm::TLM_READ_COMMAND]);
            }
        } while(!this->r_ready.read());
        wait(clk_i.posedge_event());
        this->r_valid.write(false);
        this->r_last.write(false);
    }
}

template<typename CFG>
inline void axi::bfm::axi4_target<CFG>::aw_t() {
    this->aw_ready.write(false);
    auto arid=this->aw_id.read();
    auto arlen=this->aw_len.read();
    auto arsize = this->aw_size.read();
    auto data_len = (1<<arsize)*(arlen+1);
    while(true) {
        wait(this->aw_valid.posedge_event() | clk_i.negedge_event());
        if(this->aw_valid.event() || (!active_req_beat[tlm::TLM_IGNORE_COMMAND] && this->aw_valid.read())) {
            arid=this->aw_id.read();
            arlen=this->aw_len.read();
            arsize = this->aw_size.read();
            data_len = (1<<arsize)*(arlen+1);
            auto need_reallocate = current_wr_tx!=nullptr;
            if(!current_wr_tx) {
                current_wr_tx =  tlm::scc::tlm_mm<>::get().allocate<axi::axi4_extension>(data_len);
            }
            active_req_beat[tlm::TLM_IGNORE_COMMAND] = find_or_create(current_wr_tx);
            auto& gp = active_req_beat[tlm::TLM_IGNORE_COMMAND]->trans;
            gp->set_address(this->aw_addr.read());
            gp->set_command(tlm::TLM_WRITE_COMMAND);
            if(need_reallocate) {
                auto ext = tlm::scc::tlm_gp_mm::create(data_len);
                ext->copy_from(*gp->set_auto_extension(ext));
                gp->set_data_ptr(ext->data_ptr);
                gp->set_data_length(data_len);
            }
            axi::axi4_extension* ext;
            gp->get_extension(ext);
            ext->set_id(arid);
            ext->set_length(arlen);
            ext->set_size(arsize);
            ext->set_burst(axi::into<axi::burst_e>(this->aw_burst.read()));
            this->aw_ready.write(true);
            wait(clk_i.posedge_event());
            this->aw_ready.write(false);
            active_req_beat[tlm::TLM_IGNORE_COMMAND]=nullptr;
        }
    }
}

template<typename CFG>
inline void axi::bfm::axi4_target<CFG>::wdata_t() {
    this->w_ready.write(false);
    while(true) {
        wait(this->w_valid.posedge_event() | clk_i.negedge_event());
        if(this->w_valid.event() || (!active_req_beat[tlm::TLM_WRITE_COMMAND] && this->w_valid.read())) {
            if(!active_req[tlm::TLM_WRITE_COMMAND]) {
                if(active_req_beat[tlm::TLM_IGNORE_COMMAND]) {
                    active_req_beat[tlm::TLM_WRITE_COMMAND]= active_req_beat[tlm::TLM_IGNORE_COMMAND];
                    auto& gp = active_req_beat[tlm::TLM_IGNORE_COMMAND]->trans;
                } else {
                    auto gp =  tlm::scc::tlm_mm<>::get().allocate<axi::axi4_extension>(CFG::BUSWIDTH / 8);
                    active_req_beat[tlm::TLM_WRITE_COMMAND] = find_or_create(gp);
                }
                active_req[tlm::TLM_WRITE_COMMAND]=active_req_beat[tlm::TLM_WRITE_COMMAND];
            }
            auto* fsm_hndl = active_req[tlm::TLM_WRITE_COMMAND];
            auto& gp = fsm_hndl->trans;
            auto id = this->w_id.read();
            auto data = this->w_data.read();
            auto strb = this->w_strb.read();
            auto last = this->w_last.read();
            auto size = axi::get_burst_size(*fsm_hndl->trans);
            auto bptr=fsm_hndl->trans->get_data_ptr()+fsm_hndl->beat_count*size;
            for(size_t i=0; i<CFG::BUSWIDTH/8; ++i) {
                if(strb[i]) {
                    *bptr=data(i*8+7, i*8).to_uint();
                    ++bptr;
                }
            }
            auto tp = this->w_last.read()?axi::fsm::protocol_time_point_e::BegReqE:axi::fsm::protocol_time_point_e::BegPartReqE;
            react(tp, fsm_hndl);
            wait(wdata_end_req_evt);
            this->w_ready.write(true);
            wait(clk_i.posedge_event());
            this->w_ready.write(false);
            if(last) active_req[tlm::TLM_WRITE_COMMAND]=nullptr;
        }
    }
}

template<typename CFG>
inline void axi::bfm::axi4_target<CFG>::bresp_t() {
    while(true){
        wait(bresp_evt);
        this->b_valid.write(true);
        do{
            wait(this->b_ready.posedge_event() | clk_delayed);
            if(this->b_ready.read()) {
                react(axi::fsm::protocol_time_point_e::EndRespE, active_resp_beat[tlm::TLM_WRITE_COMMAND]);
            }
        } while(!this->b_ready.read());
        wait(clk_i.posedge_event());
        this->b_valid.write(false);
    }
}

#endif /* _AXI_BFM_TARGET_H_ */
