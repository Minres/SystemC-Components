/*******************************************************************************
 * Copyright 2021-2022 MINRES Technologies GmbH
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *******************************************************************************/

#ifndef _BUS_AXI_PIN_ACE_TARGET_H_
#define _BUS_AXI_PIN_ACE_TARGET_H_

#include <axi/axi_tlm.h>
#include <axi/fsm/base.h>
#include <axi/fsm/protocol_fsm.h>
#include <axi/signal_if.h>
#include <systemc>
#include <tlm/scc/tlm_mm.h>
#include <util/ities.h>

//! TLM2.0 components modeling AXI
namespace axi {
//! pin level adapters
namespace pin {

using namespace axi::fsm;

template <typename CFG>
struct ace_target : public sc_core::sc_module,
                    public aw_ace<CFG, typename CFG::slave_types>,
                    public wdata_ace<CFG, typename CFG::slave_types>,
                    public b_ace<CFG, typename CFG::slave_types>,
                    public ar_ace<CFG, typename CFG::slave_types>,
                    public rresp_ace<CFG, typename CFG::slave_types>,

                    public ac_ace<CFG, typename CFG::slave_types>,
                    public cr_ace<CFG, typename CFG::slave_types>,
                    public cd_ace<CFG, typename CFG::slave_types>,

                    protected axi::fsm::base,
                    public axi::ace_bw_transport_if<axi::axi_protocol_types> {
    SC_HAS_PROCESS(ace_target);

    using payload_type = axi::axi_protocol_types::tlm_payload_type;
    using phase_type = axi::axi_protocol_types::tlm_phase_type;

    sc_core::sc_in<bool> clk_i{"clk_i"};

    axi::ace_initiator_socket<CFG::BUSWIDTH> isckt{"isckt"};

    ace_target(sc_core::sc_module_name const& nm)
    : sc_core::sc_module(nm)
    // coherent= true
    , base(CFG::BUSWIDTH, true) {
        instance_name = name();
        isckt.bind(*this);
        SC_METHOD(clk_delay);
        sensitive << clk_i.pos();
        dont_initialize();
        SC_THREAD(ar_t);
        SC_THREAD(rresp_t);
        SC_THREAD(aw_t);
        SC_THREAD(wdata_t);
        SC_THREAD(bresp_t);
        SC_THREAD(ac_t);
        SC_THREAD(cr_t);
        SC_THREAD(cd_t);
    }

private:
    tlm::tlm_sync_enum nb_transport_bw(payload_type& trans, phase_type& phase, sc_core::sc_time& t) override;
    void b_snoop(payload_type& trans, sc_core::sc_time& t) override{};

    void invalidate_direct_mem_ptr(sc_dt::uint64 start_range, sc_dt::uint64 end_range) override;

    void end_of_elaboration() override { clk_if = dynamic_cast<sc_core::sc_clock*>(clk_i.get_interface()); }

    axi::fsm::fsm_handle* create_fsm_handle() override { return new fsm_handle(); }

    void setup_callbacks(axi::fsm::fsm_handle*) override;

    /**
     * @brief the default snoop latency between request and response phase. Will be overwritten by the
     * return of the callback function (if registered)
     * @todo this is a hack and should be fixed
     */
    unsigned snoop_latency{1};

    void clk_delay() {
#ifdef DELTA_SYNC
        if(sc_core::sc_delta_count_at_current_time() < 5) {
            clk_self.notify(sc_core::SC_ZERO_TIME);
            next_trigger(clk_self);
        } else
            clk_delayed.notify(sc_core::SC_ZERO_TIME /*clk_if ? clk_if->period() - 1_ps : 1_ps*/);
#else
        clk_delayed.notify(axi::CLK_DELAY);
#endif
    }
    void ar_t();
    void rresp_t();
    void aw_t();
    void wdata_t();
    void bresp_t();
    void ac_t();
    void cr_t();
    void cd_t();
    static typename CFG::data_t get_read_data_for_beat(fsm::fsm_handle* fsm_hndl);
    struct aw_data {
        unsigned id;
        uint64_t addr;
        unsigned prot;
        unsigned size;
        unsigned cache;
        unsigned burst;
        unsigned qos;
        unsigned region;
        unsigned len;
        unsigned domain;
        unsigned snoop;
        unsigned bar;
        unsigned unique;
        bool lock;
        uint64_t user;
    };

    std::deque<axi::fsm::fsm_handle*> snp_resp_queue;

    sc_core::sc_clock* clk_if{nullptr};
    sc_core::sc_event clk_delayed, clk_self, ar_end_req_evt, wdata_end_req_evt, ac_evt, cd_end_req_evt, cr_end_req_evt;
    std::array<fsm_handle*, 3> active_req_beat{nullptr, nullptr, nullptr};
    std::array<fsm_handle*, 4> active_req{nullptr, nullptr, nullptr};
    std::array<fsm_handle*, 3> active_resp_beat{nullptr, nullptr, nullptr};
    scc::peq<aw_data> aw_que;
    scc::peq<std::tuple<uint8_t, fsm_handle*>> rresp_vl;
    scc::peq<std::tuple<uint8_t, fsm_handle*>> wresp_vl;
    scc::peq<std::tuple<uint8_t, fsm_handle*>> cr_vl; // snoop response

    unsigned int SNOOP = 3; // TBD??
    void write_ac(tlm::tlm_generic_payload& trans);
};

} // namespace pin
} // namespace axi

template <typename CFG>
inline tlm::tlm_sync_enum axi::pin::ace_target<CFG>::nb_transport_bw(payload_type& trans, phase_type& phase, sc_core::sc_time& t) {
    auto ret = tlm::TLM_ACCEPTED;
    SCCTRACE(SCMOD) << "nb_transport_bw with " << phase << " with delay= " << t << " of trans " << trans;
    if(phase == END_PARTIAL_REQ || phase == tlm::END_REQ) { // read/write
        schedule(phase == tlm::END_REQ ? EndReqE : EndPartReqE, &trans, t, false);
    } else if(phase == axi::BEGIN_PARTIAL_RESP || phase == tlm::BEGIN_RESP) { // read/write response
        schedule(phase == tlm::BEGIN_RESP ? BegRespE : BegPartRespE, &trans, t, false);
    } else if(phase == tlm::BEGIN_REQ) { // snoop read
        auto fsm_hndl = find_or_create(&trans, true);
        fsm_hndl->is_snoop = true;
        schedule(BegReqE, &trans, t);
    } else if(phase == END_PARTIAL_RESP || phase == tlm::END_RESP) { // snoop read response
        schedule(phase == tlm::END_RESP ? EndRespE : EndPartRespE, &trans, t);
    }
    return ret;
}

template <typename CFG>
inline void axi::pin::ace_target<CFG>::invalidate_direct_mem_ptr(sc_dt::uint64 start_range, sc_dt::uint64 end_range) {}

template <typename CFG> typename CFG::data_t axi::pin::ace_target<CFG>::get_read_data_for_beat(fsm_handle* fsm_hndl) {
    auto beat_count = fsm_hndl->beat_count;
    // SCCTRACE(SCMOD) << " " ;
    auto size = axi::get_burst_size(*fsm_hndl->trans);
    auto byte_offset = beat_count * size;
    auto offset = (fsm_hndl->trans->get_address() + byte_offset) & (CFG::BUSWIDTH / 8 - 1);
    typename CFG::data_t data{0};
    if(offset && (size + offset) > (CFG::BUSWIDTH / 8)) { // un-aligned multi-beat access
        if(beat_count == 0) {
            auto dptr = fsm_hndl->trans->get_data_ptr();
            for(size_t i = offset; i < size; ++i, ++dptr) {
                auto bit_offs = i * 8;
                data(bit_offs + 7, bit_offs) = *dptr;
            }
        } else {
            auto beat_start_idx = byte_offset - offset;
            auto data_len = fsm_hndl->trans->get_data_length();
            auto dptr = fsm_hndl->trans->get_data_ptr() + beat_start_idx;
            for(size_t i = offset; i < size && (beat_start_idx + i) < data_len; ++i, ++dptr) {
                auto bit_offs = i * 8;
                data(bit_offs + 7, bit_offs) = *dptr;
            }
        }
    } else { // aligned or single beat access
        auto dptr = fsm_hndl->trans->get_data_ptr() + byte_offset;
        for(size_t i = 0; i < size; ++i, ++dptr) {
            auto bit_offs = (offset + i) * 8;
            data(bit_offs + 7, bit_offs) = *dptr;
        }
    }
    return data;
}

template <typename CFG> inline void axi::pin::ace_target<CFG>::setup_callbacks(fsm_handle* fsm_hndl) {
    fsm_hndl->fsm->cb[RequestPhaseBeg] = [this, fsm_hndl]() -> void { fsm_hndl->beat_count = 0; };
    fsm_hndl->fsm->cb[BegPartReqE] = [this, fsm_hndl]() -> void {
        sc_assert(fsm_hndl->trans->get_command() == tlm::TLM_WRITE_COMMAND);
        tlm::tlm_phase phase = axi::BEGIN_PARTIAL_REQ;
        sc_core::sc_time t(sc_core::SC_ZERO_TIME);
        auto ret = isckt->nb_transport_fw(*fsm_hndl->trans, phase, t);
        if(ret == tlm::TLM_UPDATED) {
            schedule(EndPartReqE, fsm_hndl->trans, t, true);
        }
    };
    fsm_hndl->fsm->cb[EndPartReqE] = [this, fsm_hndl]() -> void {
        wdata_end_req_evt.notify();
        active_req_beat[tlm::TLM_WRITE_COMMAND] = nullptr;
        fsm_hndl->beat_count++;
    };
    fsm_hndl->fsm->cb[BegReqE] = [this, fsm_hndl]() -> void {
        if(fsm_hndl->is_snoop) {
            SCCTRACE(SCMOD) << "in BegReq of setup_cb, call write_ac() ";
            active_req[SNOOP] = fsm_hndl;
            write_ac(*fsm_hndl->trans);
            ac_evt.notify(sc_core::SC_ZERO_TIME);

        } else {
            tlm::tlm_phase phase = tlm::BEGIN_REQ;
            sc_core::sc_time t(sc_core::SC_ZERO_TIME);
            auto ret = isckt->nb_transport_fw(*fsm_hndl->trans, phase, t);
            if(ret == tlm::TLM_UPDATED) {
                schedule(EndReqE, fsm_hndl->trans, t, true);
            }
        }
    };

    fsm_hndl->fsm->cb[EndReqE] = [this, fsm_hndl]() -> void {
        if(fsm_hndl->is_snoop) {
            SCCTRACE(SCMOD) << "snoop with EndReq evt";
            auto latency = 0;
            snp_resp_queue.push_back(fsm_hndl);
            active_req[SNOOP] = nullptr;
            tlm::tlm_phase phase = tlm::END_REQ;
            //  ?? here t(delay) should be zero or clock cycle??
            // sc_core::sc_time t(clk_if ? ::scc::time_to_next_posedge(clk_if) - 1_ps : sc_core::SC_ZERO_TIME);
            sc_core::sc_time t(sc_core::SC_ZERO_TIME);
            auto ret = isckt->nb_transport_fw(*fsm_hndl->trans, phase, t);
            auto exta = fsm_hndl->trans->get_extension<ace_extension>();
            fsm_hndl->trans->set_response_status(tlm::TLM_OK_RESPONSE);
        } else {
            switch(fsm_hndl->trans->get_command()) {
            case tlm::TLM_READ_COMMAND:
                ar_end_req_evt.notify();
                active_req_beat[tlm::TLM_READ_COMMAND] = nullptr;
                break;
            case tlm::TLM_WRITE_COMMAND:
                wdata_end_req_evt.notify();
                active_req_beat[tlm::TLM_WRITE_COMMAND] = nullptr;
                fsm_hndl->beat_count++;
                break;
            default:
                break;
            }
        }
    };
    fsm_hndl->fsm->cb[BegPartRespE] = [this, fsm_hndl]() -> void {
        if(fsm_hndl->is_snoop) {
            tlm::tlm_phase phase = axi::BEGIN_PARTIAL_RESP;
            sc_core::sc_time t;
            auto ret = isckt->nb_transport_fw(*fsm_hndl->trans, phase, t);

        } else {
            assert(fsm_hndl->trans->is_read());
            active_resp_beat[tlm::TLM_READ_COMMAND] = fsm_hndl;
            rresp_vl.notify({1, fsm_hndl});
        }
    };
    fsm_hndl->fsm->cb[EndPartRespE] = [this, fsm_hndl]() -> void {
        if(fsm_hndl->is_snoop) {
            fsm_hndl->beat_count++;
            cd_end_req_evt.notify();

        } else {
            // scheduling the response
            assert(fsm_hndl->trans->is_read());
            tlm::tlm_phase phase = axi::END_PARTIAL_RESP;
            sc_core::sc_time t(sc_core::SC_ZERO_TIME);
            auto ret = isckt->nb_transport_fw(*fsm_hndl->trans, phase, t);
            active_resp_beat[tlm::TLM_READ_COMMAND] = nullptr;
            fsm_hndl->beat_count++;
        }
    };
    fsm_hndl->fsm->cb[BegRespE] = [this, fsm_hndl]() -> void {
        SCCTRACE(SCMOD) << "processing event BegRespE for trans " << *fsm_hndl->trans;
        if(fsm_hndl->is_snoop) {
            tlm::tlm_phase phase = tlm::BEGIN_RESP;
            sc_core::sc_time t;
            auto ret = isckt->nb_transport_fw(*fsm_hndl->trans, phase, t);
        } else {
            auto size = axi::get_burst_size(*fsm_hndl->trans);
            active_resp_beat[fsm_hndl->trans->get_command()] = fsm_hndl;
            switch(fsm_hndl->trans->get_command()) {
            case tlm::TLM_READ_COMMAND:
                rresp_vl.notify({3, fsm_hndl});
                break;
            case tlm::TLM_WRITE_COMMAND:
                wresp_vl.notify({3, fsm_hndl});
                break;
            default:
                break;
            }
        }
    };
    fsm_hndl->fsm->cb[EndRespE] = [this, fsm_hndl]() -> void {
        if(fsm_hndl->is_snoop) {
            SCCTRACE(SCMOD) << "  in EndRespE  ";
            cd_end_req_evt.notify();
            cr_end_req_evt.notify(); // need to check these two event??
            snp_resp_queue.pop_front();
            fsm_hndl->finish.notify();

        } else {
            // scheduling the response
            tlm::tlm_phase phase = tlm::END_RESP;
            sc_core::sc_time t(sc_core::SC_ZERO_TIME);
            auto ret = isckt->nb_transport_fw(*fsm_hndl->trans, phase, t);
            SCCTRACE(SCMOD) << "EndResp of setup_cb with coherent = " << coherent;
            if(coherent)
                schedule(Ack, fsm_hndl->trans, t); // later can add ack_resp_delay to replace t
            else {
                fsm_hndl->finish.notify();
                active_resp_beat[fsm_hndl->trans->get_command()] = nullptr;
            }
        }
    };
    fsm_hndl->fsm->cb[Ack] = [this, fsm_hndl]() -> void {
        SCCTRACE(SCMOD) << " in Ack of setup_cb";
        sc_core::sc_time t(sc_core::SC_ZERO_TIME);
        tlm::tlm_phase phase = axi::ACK;
        auto ret = isckt->nb_transport_fw(*fsm_hndl->trans, phase, t);
        fsm_hndl->finish.notify();
        active_resp_beat[fsm_hndl->trans->get_command()] = nullptr;
    };
}

template <typename CFG> inline void axi::pin::ace_target<CFG>::ar_t() {
    this->ar_ready.write(false);
    wait(sc_core::SC_ZERO_TIME);
    auto arid = 0U;
    auto arlen = 0U;
    auto arsize = util::ilog2(CFG::BUSWIDTH / 8);

    auto data_len = (1 << arsize) * (arlen + 1);
    while(true) {
        wait(this->ar_valid.posedge_event() | clk_delayed);
        if(this->ar_valid.read()) {
            SCCTRACE(SCMOD) << "ARVALID detected for 0x" << std::hex << this->ar_addr.read();
            if(!CFG::IS_LITE) {
                arid = this->ar_id->read().to_uint();
                arlen = this->ar_len->read().to_uint();
                arsize = this->ar_size->read().to_uint();
            }
            data_len = (1 << arsize) * (arlen + 1);
            auto gp = tlm::scc::tlm_mm<>::get().allocate<axi::ace_extension>(data_len);
            gp->set_address(this->ar_addr.read());
            gp->set_command(tlm::TLM_READ_COMMAND);
            gp->set_streaming_width(data_len);
            axi::ace_extension* ext;
            gp->get_extension(ext);
            ext->set_id(arid);
            ext->set_length(arlen);
            ext->set_size(arsize);
            if(this->ar_lock->read())
                ext->set_exclusive(true);
            ext->set_domain(axi::into<axi::domain_e>(this->ar_domain->read())); // ace extension
            ext->set_snoop(axi::into<axi::snoop_e>(this->ar_snoop->read()));
            ext->set_barrier(axi::into<axi::bar_e>(this->ar_bar->read()));
            ext->set_burst(CFG::IS_LITE ? axi::burst_e::INCR : axi::into<axi::burst_e>(this->ar_burst->read()));
            ext->set_cache(this->ar_cache->read());
            ext->set_prot(this->ar_prot->read());
            ext->set_qos(this->ar_qos->read());
            ext->set_region(this->ar_region->read());

            active_req_beat[tlm::TLM_READ_COMMAND] = find_or_create(gp);
            react(axi::fsm::protocol_time_point_e::BegReqE, active_req_beat[tlm::TLM_READ_COMMAND]);
            wait(ar_end_req_evt);
            this->ar_ready.write(true);
            wait(clk_i.posedge_event());
            this->ar_ready.write(false);
        }
    }
}

template <typename CFG> inline void axi::pin::ace_target<CFG>::rresp_t() {
    this->r_valid.write(false);
    wait(sc_core::SC_ZERO_TIME);
    fsm_handle* fsm_hndl;
    uint8_t val;
    while(true) {
        // rresp_vl notified in BEGIN_PARTIAL_REQ ( val=1 ??)or in BEG_RESP(val=3??)
        std::tie(val, fsm_hndl) = rresp_vl.get();
        SCCTRACE(SCMOD) << __FUNCTION__ << " val = " << (uint16_t)val << " beat count = " << fsm_hndl->beat_count;
        SCCTRACE(SCMOD) << __FUNCTION__ << " got read response beat of trans " << *fsm_hndl->trans;
        auto ext = fsm_hndl->trans->get_extension<axi::ace_extension>();
        this->r_data.write(get_read_data_for_beat(fsm_hndl));
        this->r_resp.write(ext->get_cresp());
        this->r_valid.write(val & 0x1);
        if(!CFG::IS_LITE) {
            this->r_id->write(ext->get_id());
            this->r_last->write(val & 0x2);
        }
        do {
            wait(this->r_ready.posedge_event() | clk_delayed);
            if(this->r_ready.read()) {
                auto evt =
                    CFG::IS_LITE || (val & 0x2) ? axi::fsm::protocol_time_point_e::EndRespE : axi::fsm::protocol_time_point_e::EndPartRespE;
                react(evt, active_resp_beat[tlm::TLM_READ_COMMAND]);
            }
        } while(!this->r_ready.read());
        SCCTRACE(SCMOD) << "finished read response beat of trans [" << fsm_hndl->trans << "]";
        wait(clk_i.posedge_event());
        this->r_valid.write(false);
        if(!CFG::IS_LITE)
            this->r_last->write(false);
    }
}

template <typename CFG> inline void axi::pin::ace_target<CFG>::aw_t() {
    this->aw_ready.write(false);
    wait(sc_core::SC_ZERO_TIME);
    const auto awsize = util::ilog2(CFG::BUSWIDTH / 8);
    while(true) {
        wait(this->aw_valid.posedge_event() | clk_delayed);
        if(this->aw_valid.event() || (!active_req_beat[tlm::TLM_IGNORE_COMMAND] && this->aw_valid.read())) {
            SCCTRACE(SCMOD) << "AWVALID detected for 0x" << std::hex << this->aw_addr.read();
            // clang-format off
            aw_data awd = {CFG::IS_LITE ? 0U : this->aw_id->read().to_uint(),
                    this->aw_addr.read().to_uint64(),
                    this->aw_prot.read().to_uint(),
                    CFG::IS_LITE ? awsize : this->aw_size->read().to_uint(),
                    CFG::IS_LITE ? 0U : this->aw_cache->read().to_uint(),
                    CFG::IS_LITE ? 0U : this->aw_burst->read().to_uint(),
                    CFG::IS_LITE ? 0U : this->aw_qos->read().to_uint(),
                    CFG::IS_LITE ? 0U : this->aw_region->read().to_uint(),
                    CFG::IS_LITE ? 0U : this->aw_len->read().to_uint(),
                    CFG::IS_LITE ? 0U : this->aw_domain->read().to_uint(),
                    CFG::IS_LITE ? 0U : this->aw_snoop->read().to_uint(),
                    CFG::IS_LITE ? 0U : this->aw_bar->read().to_uint(),
                    CFG::IS_LITE ? 0U : this->aw_unique->read(),
                    CFG::IS_LITE ? false : this->aw_lock->read(),
                    0};
            // clang-format on
            aw_que.notify(awd);
            this->aw_ready.write(true);
            wait(clk_i.posedge_event());
            this->aw_ready.write(false);
        }
    }
}

template <typename CFG> inline void axi::pin::ace_target<CFG>::wdata_t() {
    this->w_ready.write(false);
    wait(sc_core::SC_ZERO_TIME);
    while(true) {
        wait(this->w_valid.posedge_event() | clk_delayed);
        this->w_ready.write(false);
        if(this->w_valid.event() || (!active_req_beat[tlm::TLM_WRITE_COMMAND] && this->w_valid.read())) {
            if(!active_req[tlm::TLM_WRITE_COMMAND]) {
                if(!aw_que.has_next())
                    wait(aw_que.event());
                auto awd = aw_que.get();
                auto data_len = (1 << awd.size) * (awd.len + 1);
                auto gp = tlm::scc::tlm_mm<>::get().allocate<axi::ace_extension>(data_len, true);
                gp->set_address(awd.addr);
                gp->set_command(tlm::TLM_WRITE_COMMAND);
                axi::ace_extension* ext;
                gp->get_extension(ext);
                ext->set_id(awd.id);
                ext->set_length(awd.len);
                ext->set_size(awd.size);
                ext->set_burst(axi::into<axi::burst_e>(awd.burst));
                ext->set_prot(awd.prot);
                ext->set_qos(awd.qos);
                ext->set_cache(awd.cache);
                ext->set_region(awd.region);
                ext->set_snoop(axi::into<axi::snoop_e>(awd.snoop));
                ext->set_barrier(axi::into<axi::bar_e>(awd.bar));
                ext->set_unique(awd.unique);
                ext->set_exclusive(awd.lock);
                if(CFG::USERWIDTH)
                    ext->set_user(axi::common::id_type::CTRL, awd.user);

                active_req_beat[tlm::TLM_WRITE_COMMAND] = find_or_create(gp);
                active_req[tlm::TLM_WRITE_COMMAND] = active_req_beat[tlm::TLM_WRITE_COMMAND];
            }
            auto* fsm_hndl = active_req[tlm::TLM_WRITE_COMMAND];
            SCCTRACE(SCMOD) << "WDATA detected for 0x" << std::hex << this->ar_addr.read();
            auto& gp = fsm_hndl->trans;
            auto data = this->w_data.read();
            auto strb = this->w_strb.read();
            auto last = CFG::IS_LITE ? true : this->w_last->read();
            auto beat_count = fsm_hndl->beat_count;
            auto size = axi::get_burst_size(*fsm_hndl->trans);
            auto byte_offset = beat_count * size;
            auto offset = (fsm_hndl->trans->get_address() + byte_offset) & (CFG::BUSWIDTH / 8 - 1);
            if(offset && (size + offset) > (CFG::BUSWIDTH / 8)) { // un-aligned multi-beat access
                if(beat_count == 0) {
                    auto dptr = fsm_hndl->trans->get_data_ptr();
                    auto beptr = fsm_hndl->trans->get_byte_enable_ptr();
                    for(size_t i = offset; i < size; ++i, ++dptr, ++beptr) {
                        auto bit_offs = i * 8;
                        *dptr = data(bit_offs + 7, bit_offs).to_uint();
                        *beptr = strb[i] ? 0xff : 0;
                    }
                } else {
                    auto beat_start_idx = byte_offset - offset;
                    auto data_len = fsm_hndl->trans->get_data_length();
                    auto dptr = fsm_hndl->trans->get_data_ptr() + beat_start_idx;
                    auto beptr = fsm_hndl->trans->get_byte_enable_ptr() + beat_start_idx;
                    for(size_t i = 0; i < size && (beat_start_idx + i) < data_len; ++i, ++dptr, ++beptr) {
                        auto bit_offs = i * 8;
                        *dptr = data(bit_offs + 7, bit_offs).to_uint();
                        *beptr = strb[i] ? 0xff : 0;
                    }
                }
            } else { // aligned or single beat access
                auto dptr = fsm_hndl->trans->get_data_ptr() + byte_offset;
                auto beptr = fsm_hndl->trans->get_byte_enable_ptr() + byte_offset;
                for(size_t i = 0; i < size; ++i, ++dptr, ++beptr) {
                    auto bit_offs = (offset + i) * 8;
                    *dptr = data(bit_offs + 7, bit_offs).to_uint();
                    *beptr = strb[offset + i] ? 0xff : 0;
                }
            }
            // TODO: assuming consecutive write (not scattered)
            auto strobe = strb.to_uint();
            if(last) {
                auto act_data_len = CFG::IS_LITE ? util::bit_count(strobe) : (beat_count + 1) * size;
                //            if(CFG::IS_LITE && act_data_len<CFG::BUSWIDTH/8) {
                //                std::fill(gp->get_byte_enable_ptr(), gp->get_byte_enable_ptr() + act_data_len, 0xff);
                //                std::fill(gp->get_byte_enable_ptr() + act_data_len, gp->get_byte_enable_ptr() +
                //                gp->get_byte_enable_length(), 0x0);
                //            }
                gp->set_data_length(act_data_len);
                gp->set_byte_enable_length(act_data_len);
                gp->set_streaming_width(act_data_len);
            }
            auto tp = CFG::IS_LITE || this->w_last->read() ? axi::fsm::protocol_time_point_e::BegReqE
                                                           : axi::fsm::protocol_time_point_e::BegPartReqE;
            react(tp, fsm_hndl);
            // notifed in EndPartReqE/EndReq
            wait(wdata_end_req_evt);
            this->w_ready.write(true);
            wait(clk_i.posedge_event());
            this->w_ready.write(false);
            if(last)
                active_req[tlm::TLM_WRITE_COMMAND] = nullptr;
        }
    }
}

template <typename CFG> inline void axi::pin::ace_target<CFG>::bresp_t() {
    this->b_valid.write(false);
    wait(sc_core::SC_ZERO_TIME);
    fsm_handle* fsm_hndl;
    uint8_t val;
    while(true) {
        std::tie(val, fsm_hndl) = wresp_vl.get();
        SCCTRACE(SCMOD) << "got write response of trans " << *fsm_hndl->trans;
        auto ext = fsm_hndl->trans->get_extension<axi::ace_extension>();
        this->b_resp.write(axi::to_int(ext->get_resp()));
        this->b_valid.write(true);
        if(!CFG::IS_LITE)
            this->b_id->write(ext->get_id());
        SCCTRACE(SCMOD) << "got write response";
        do {
            wait(this->b_ready.posedge_event() | clk_delayed);
            if(this->b_ready.read()) {
                react(axi::fsm::protocol_time_point_e::EndRespE, active_resp_beat[tlm::TLM_WRITE_COMMAND]);
            }
        } while(!this->b_ready.read());
        SCCTRACE(SCMOD) << "finished write response of trans [" << fsm_hndl->trans << "]";
        wait(clk_i.posedge_event());
        this->b_valid.write(false);
    }
}

// write snoop address
template <typename CFG> inline void axi::pin::ace_target<CFG>::write_ac(tlm::tlm_generic_payload& trans) {
    sc_dt::sc_uint<CFG::ADDRWIDTH> addr = trans.get_address();
    this->ac_addr.write(addr);
    auto ext = trans.get_extension<ace_extension>();
    sc_assert(ext && "No ACE extension found for snoop access");
    this->ac_prot.write(ext->get_prot());
    this->ac_snoop->write(sc_dt::sc_uint<4>((uint8_t)ext->get_snoop()));
}

template <typename CFG> inline void axi::pin::ace_target<CFG>::ac_t() {
    this->ac_valid.write(false);
    wait(sc_core::SC_ZERO_TIME);
    while(true) {
        wait(ac_evt);
        this->ac_valid.write(true);
        do {
            wait(this->ac_ready.posedge_event() | clk_delayed);
            if(this->ac_ready.read()) {
                SCCTRACE(SCMOD) << "in ac_t() detect ac_ready high , schedule EndReq";
                react(axi::fsm::protocol_time_point_e::EndReqE, active_req[SNOOP]);
            }
        } while(!this->ac_ready.read());
        wait(clk_i.posedge_event());
        this->ac_valid.write(false);
    }
}

template <typename CFG> inline void axi::pin::ace_target<CFG>::cd_t() {
    this->cd_ready.write(false);
    wait(sc_core::SC_ZERO_TIME);
    while(true) {
        wait(this->cd_valid.posedge_event() | clk_delayed);
        if(this->cd_valid.read()) {
            SCCTRACE(SCMOD) << "in cd_t(), received cd_valid high ";
            wait(sc_core::SC_ZERO_TIME);
            auto data = this->cd_data.read();
            if(snp_resp_queue.empty())
                sc_assert(" snp_resp_queue empty");
            auto* fsm_hndl = snp_resp_queue.front();
            auto beat_count = fsm_hndl->beat_count;
            SCCTRACE(SCMOD) << "in cd_t(), received beau_count = " << fsm_hndl->beat_count;
            auto size = axi::get_burst_size(*fsm_hndl->trans);
            auto byte_offset = beat_count * size;
            auto offset = (fsm_hndl->trans->get_address() + byte_offset) & (CFG::BUSWIDTH / 8 - 1);
            if(offset && (size + offset) > (CFG::BUSWIDTH / 8)) { // un-aligned multi-beat access
                if(beat_count == 0) {
                    auto dptr = fsm_hndl->trans->get_data_ptr();
                    for(size_t i = offset; i < size; ++i, ++dptr) {
                        auto bit_offs = i * 8;
                        *dptr = data(bit_offs + 7, bit_offs).to_uint();
                    }
                } else {
                    auto beat_start_idx = beat_count * size - offset;
                    auto data_len = fsm_hndl->trans->get_data_length();
                    auto dptr = fsm_hndl->trans->get_data_ptr() + beat_start_idx;
                    for(size_t i = offset; i < size && (beat_start_idx + i) < data_len; ++i, ++dptr) {
                        auto bit_offs = i * 8;
                        *dptr = data(bit_offs + 7, bit_offs).to_uint();
                    }
                }
            } else { // aligned or single beat access
                auto dptr = fsm_hndl->trans->get_data_ptr() + beat_count * size;
                for(size_t i = 0; i < size; ++i, ++dptr) {
                    auto bit_offs = (offset + i) * 8;
                    *dptr = data(bit_offs + 7, bit_offs).to_uint();
                }
            }
            /*
            axi::ace_extension* e;
            fsm_hndl->trans->get_extension(e);
            e->set_resp(axi::into<axi::resp_e>(resp));
            e->add_to_response_array(*e);
            */
            auto tp = CFG::IS_LITE || this->cd_last->read() ? axi::fsm::protocol_time_point_e::BegRespE
                                                            : axi::fsm::protocol_time_point_e::BegPartRespE;
            if(!this->cd_last->read()) // only react BegPartRespE
                react(tp, fsm_hndl);
            // cd_end_req_evt notified in EndPartRespE or EndResp
            wait(cd_end_req_evt);
            this->cd_ready.write(true);
            wait(clk_i.posedge_event());
            this->cd_ready.write(false);
        }
    }
}

template <typename CFG> inline void axi::pin::ace_target<CFG>::cr_t() {
    this->cr_ready.write(false);
    wait(sc_core::SC_ZERO_TIME);
    while(true) {
        wait(this->cr_valid.posedge_event() | clk_delayed);
        if(this->cr_valid.read()) {
            SCCTRACE(SCMOD) << "in cr_t()  received cr_valid high ";
            wait(sc_core::SC_ZERO_TIME);

            auto* fsm_hndl = snp_resp_queue.front();
            auto crresp = this->cr_resp.read();
            axi::ace_extension* e;
            fsm_hndl->trans->get_extension(e);
            e->set_cresp(crresp);

            SCCTRACE(SCMOD) << " in cr_t()  react() with BegRespE ";
            // hongyu TBD?? schedule BegResp??
            react(axi::fsm::protocol_time_point_e::BegRespE, fsm_hndl);
            wait(cr_end_req_evt); // notify in EndResp
            this->cr_ready.write(true);
            wait(clk_i.posedge_event());
            this->cr_ready.write(false);
        }
    }
}

#endif /* _BUS_AXI_PIN_ACE_TARGET_H_ */
