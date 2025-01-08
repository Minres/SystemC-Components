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

#ifndef _BUS_AXI_PIN_ACE_INITIATOR_H_
#define _BUS_AXI_PIN_ACE_INITIATOR_H_

#include <axi/axi_tlm.h>
#include <axi/fsm/base.h>
#include <axi/fsm/protocol_fsm.h>
#include <axi/signal_if.h>
#include <cci_configuration>
#include <scc/fifo_w_cb.h>
#include <systemc>
#include <tlm/scc/tlm_mm.h>
#include <tlm_utils/peq_with_cb_and_phase.h>

//! TLM2.0 components modeling AHB
namespace axi {
//! pin level adapters
namespace pin {

using namespace axi::fsm;

template <typename CFG>
struct ace_initiator : public sc_core::sc_module,
                       public aw_ace<CFG, typename CFG::master_types>,
                       public wdata_ace<CFG, typename CFG::master_types>,
                       public b_ace<CFG, typename CFG::master_types>,
                       public ar_ace<CFG, typename CFG::master_types>,
                       public rresp_ace<CFG, typename CFG::master_types>,

                       public ac_ace<CFG, typename CFG::master_types>,
                       public cr_ace<CFG, typename CFG::master_types>,
                       public cd_ace<CFG, typename CFG::master_types>,

                       protected axi::fsm::base,
                       public axi::ace_fw_transport_if<axi::axi_protocol_types> {
    SC_HAS_PROCESS(ace_initiator);

    enum { CACHELINE_SZ = 64 };

    using payload_type = axi::axi_protocol_types::tlm_payload_type;
    using phase_type = axi::axi_protocol_types::tlm_phase_type;

    sc_core::sc_in<bool> clk_i{"clk_i"};

    axi::ace_target_socket<CFG::BUSWIDTH> tsckt{"tsckt"};

    cci::cci_param<bool> pipelined_wrreq{"pipelined_wrreq", false};

    cci::cci_param<bool> mask_axi_id{"mask_axi_id", false};

    ace_initiator(sc_core::sc_module_name const& nm, bool pipelined_wrreq = false)
    : sc_core::sc_module(nm)
    // coherent= true
    , base(CFG::BUSWIDTH, true)
    , pipelined_wrreq("pipelined_wrreq", pipelined_wrreq) {
        instance_name = name();
        tsckt(*this);
        SC_METHOD(clk_delay);
        sensitive << clk_i.pos();
        SC_THREAD(ar_t);
        SC_THREAD(r_t);
        SC_THREAD(aw_t);
        SC_THREAD(wdata_t);
        SC_THREAD(b_t);
        SC_THREAD(ac_t);
        SC_THREAD(cr_resp_t);
        SC_THREAD(cd_t);
        SC_THREAD(rack_t);
        SC_THREAD(wack_t);
    }

private:
    void b_transport(payload_type& trans, sc_core::sc_time& t) override {
        trans.set_dmi_allowed(false);
        trans.set_response_status(tlm::TLM_OK_RESPONSE);
    }

    tlm::tlm_sync_enum nb_transport_fw(payload_type& trans, phase_type& phase, sc_core::sc_time& t) override {
        assert(trans.get_extension<axi::ace_extension>() && "missing ACE extension");
        sc_core::sc_time delay; // FIXME: calculate delay correctly
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

    void clk_delay() { clk_delayed.notify(axi::CLK_DELAY); }

    void ar_t();
    void r_t();
    void aw_t();
    void wdata_t();
    void b_t();
    /**
     * @fn void ac_t()
     * @brief
     *
     */
    void ac_t();
    void cr_resp_t();
    void cd_t();

    void rack_t();
    void wack_t();
    /**
     * @fn CFG::data_t get_cache_data_for_beat(fsm::fsm_handle*)
     * @brief
     *
     * @param fsm_hndl
     * @return
     */
    static typename CFG::data_t get_cache_data_for_beat(fsm::fsm_handle* fsm_hndl);
    unsigned int SNOOP = 3; // TBD??
    scc::peq<std::tuple<uint8_t, fsm_handle*>> cd_vl;
    scc::peq<std::tuple<uint8_t, fsm_handle*>> cr_resp_vl;
    std::array<unsigned, 3> outstanding_cnt{0, 0, 0};
    std::array<fsm_handle*, 3> active_req{nullptr, nullptr, nullptr};
    std::array<fsm_handle*, 4> active_resp_beat{nullptr, nullptr, nullptr};
    sc_core::sc_clock* clk_if{nullptr};
    sc_core::sc_event clk_delayed, clk_self, r_end_resp_evt, w_end_resp_evt, ac_end_req_evt;
    void nb_fw(payload_type& trans, const phase_type& phase) {
        auto t = sc_core::SC_ZERO_TIME;
        base::nb_fw(trans, phase, t);
    }
    tlm_utils::peq_with_cb_and_phase<ace_initiator> fw_peq{this, &ace_initiator::nb_fw};
    std::unordered_map<unsigned, std::deque<fsm_handle*>> rd_resp_by_id, wr_resp_by_id;
    struct fifo_entry {
        tlm::tlm_generic_payload* gp = nullptr;
        bool last = false;
        bool needs_end_req = false;
        size_t beat_num = 0;
        fifo_entry(tlm::tlm_generic_payload* gp, bool last, bool needs_end_req, size_t beat_num)
        : gp(gp)
        , last(last)
        , needs_end_req(needs_end_req)
        , beat_num(beat_num) {
            if(gp->has_mm())
                gp->acquire();
        }
        fifo_entry(tlm::tlm_generic_payload* gp, bool needs_end_req)
        : gp(gp)
        , needs_end_req(needs_end_req) {
            if(gp->has_mm())
                gp->acquire();
        }
        fifo_entry(fifo_entry const& o)
        : gp(o.gp)
        , last(o.last)
        , needs_end_req(o.needs_end_req)
        , beat_num(o.beat_num) {
            if(gp && gp->has_mm())
                gp->acquire();
        }
        fifo_entry& operator=(const fifo_entry& o) {
            gp = o.gp;
            last = o.last;
            needs_end_req = o.needs_end_req;
            beat_num = o.beat_num;
            return *this;
        }
        ~fifo_entry() {
            if(gp && gp->has_mm())
                gp->release();
        }
    };
    scc::fifo_w_cb<fifo_entry> ar_fifo{"ar_fifo"};
    scc::fifo_w_cb<fifo_entry> aw_fifo{"aw_fifo"};
    scc::fifo_w_cb<fifo_entry> wdata_fifo{"wdata_fifo"};
    sc_core::sc_event rack_vl;
    sc_core::sc_event wack_vl;
    void write_ar(tlm::tlm_generic_payload& trans);
    void write_aw(tlm::tlm_generic_payload& trans);
    void write_wdata(tlm::tlm_generic_payload& trans, unsigned beat);
};

} // namespace pin
} // namespace axi

template <typename CFG> inline void axi::pin::ace_initiator<CFG>::write_ar(tlm::tlm_generic_payload& trans) {
    sc_dt::sc_uint<CFG::ADDRWIDTH> addr = trans.get_address();
    this->ar_addr.write(addr);
    if(auto ext = trans.get_extension<axi::ace_extension>()) {
        this->ar_prot.write(ext->get_prot());
        if(!CFG::IS_LITE) {
            auto id = ext->get_id();
            if(!mask_axi_id.get_value() && id >= (1 << CFG::IDWIDTH))
                SCCERR(SCMOD) << "AWID value larger than signal awid with width=" << CFG::IDWIDTH << " can carry";
            this->ar_id->write(sc_dt::sc_uint<CFG::IDWIDTH>(id));
            this->ar_len->write(sc_dt::sc_uint<8>(ext->get_length()));
            this->ar_size->write(sc_dt::sc_uint<3>(ext->get_size()));
            this->ar_burst->write(sc_dt::sc_uint<2>(axi::to_int(ext->get_burst())));
            this->ar_lock->write(ext->is_exclusive());
            this->ar_cache->write(sc_dt::sc_uint<4>(ext->get_cache()));
            this->ar_prot.write(ext->get_prot());
            this->ar_qos->write(ext->get_qos());
            this->ar_region->write(ext->get_region());
            this->ar_domain->write(sc_dt::sc_uint<2>((uint8_t)ext->get_domain()));
            this->ar_snoop->write(sc_dt::sc_uint<4>((uint8_t)ext->get_snoop()));
            this->ar_bar->write(sc_dt::sc_uint<2>((uint8_t)ext->get_barrier()));
            this->ar_user->write(ext->get_user(axi::common::id_type::CTRL));
        }
    }
}
template <typename CFG> inline void axi::pin::ace_initiator<CFG>::write_aw(tlm::tlm_generic_payload& trans) {
    sc_dt::sc_uint<CFG::ADDRWIDTH> addr = trans.get_address();
    this->aw_addr.write(addr);
    if(auto ext = trans.get_extension<axi::ace_extension>()) {
        this->aw_prot.write(ext->get_prot());
        this->aw_lock->write(ext->is_exclusive());
        auto id = ext->get_id();
        if(!mask_axi_id.get_value() && id >= (1 << CFG::IDWIDTH))
            SCCERR(SCMOD) << "AWID value larger than signal awid with width=" << CFG::IDWIDTH << " can carry";
        this->aw_id->write(sc_dt::sc_uint<CFG::IDWIDTH>(id));
        this->aw_len->write(sc_dt::sc_uint<8>(ext->get_length()));
        this->aw_size->write(sc_dt::sc_uint<3>(ext->get_size()));
        this->aw_burst->write(sc_dt::sc_uint<2>(axi::to_int(ext->get_burst())));
        this->aw_cache->write(sc_dt::sc_uint<4>(ext->get_cache()));
        this->aw_qos->write(sc_dt::sc_uint<4>(ext->get_qos()));
        this->aw_region->write(sc_dt::sc_uint<4>(ext->get_region()));
        this->aw_user->write(ext->get_user(axi::common::id_type::CTRL));
        this->aw_domain->write(sc_dt::sc_uint<2>((uint8_t)ext->get_domain()));
        this->aw_snoop->write(sc_dt::sc_uint<CFG::AWSNOOPWIDTH>((uint8_t)ext->get_snoop()));
        this->aw_bar->write(sc_dt::sc_uint<2>((uint8_t)ext->get_barrier()));
        this->aw_unique->write(ext->get_unique());
    }
}

// FIXME: strb not yet correct
template <typename CFG> inline void axi::pin::ace_initiator<CFG>::write_wdata(tlm::tlm_generic_payload& trans, unsigned beat) {
    typename CFG::data_t data{0};
    typename CFG::strb_t strb{0};
    auto ext = trans.get_extension<axi::ace_extension>();
    auto size = 1u << ext->get_size();
    auto byte_offset = beat * size;
    auto offset = (trans.get_address() + byte_offset) & (CFG::BUSWIDTH / 8 - 1);
    auto beptr = trans.get_byte_enable_length() ? trans.get_byte_enable_ptr() + byte_offset : nullptr;
    if(offset && (size + offset) > (CFG::BUSWIDTH / 8)) { // un-aligned multi-beat access
        if(beat == 0) {
            auto dptr = trans.get_data_ptr();
            if(dptr)
                for(size_t i = offset; i < size; ++i, ++dptr) {
                    auto bit_offs = i * 8;
                    data(bit_offs + 7, bit_offs) = *dptr;
                    if(beptr) {
                        strb[i] = *beptr == 0xff;
                        ++beptr;
                    } else
                        strb[i] = true;
                }
        } else {
            auto beat_start_idx = byte_offset - offset;
            auto data_len = trans.get_data_length();
            auto dptr = trans.get_data_ptr() + beat_start_idx;
            if(dptr)
                for(size_t i = 0; i < size && (beat_start_idx + i) < data_len; ++i, ++dptr) {
                    auto bit_offs = i * 8;
                    data(bit_offs + 7, bit_offs) = *dptr;
                    if(beptr) {
                        strb[i] = *beptr == 0xff;
                        ++beptr;
                    } else
                        strb[i] = true;
                }
        }
    } else { // aligned or single beat access
        auto dptr = trans.get_data_ptr() + byte_offset;
        if(dptr)
            for(size_t i = 0; i < size; ++i, ++dptr) {
                auto bit_offs = (offset + i) * 8;
                data(bit_offs + 7, bit_offs) = *dptr;
                if(beptr) {
                    strb[offset + i] = *beptr == 0xff;
                    ++beptr;
                } else
                    strb[offset + i] = true;
            }
    }
    this->w_data.write(data);
    this->w_strb.write(strb);
    if(!CFG::IS_LITE) {
        this->w_id->write(ext->get_id());
        if(this->w_user.get_interface())
            this->w_user->write(ext->get_user(axi::common::id_type::DATA));
    }
}

template <typename CFG> typename CFG::data_t axi::pin::ace_initiator<CFG>::get_cache_data_for_beat(fsm_handle* fsm_hndl) {
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

template <typename CFG> inline void axi::pin::ace_initiator<CFG>::setup_callbacks(fsm_handle* fsm_hndl) {
    fsm_hndl->fsm->cb[RequestPhaseBeg] = [this, fsm_hndl]() -> void {
        if(fsm_hndl->is_snoop) {
            SCCTRACE(SCMOD) << " for snoop in RequestPhaseBeg ";
        } else {
            fsm_hndl->beat_count = 0;
            outstanding_cnt[fsm_hndl->trans->get_command()]++;
            if(CFG::IS_LITE) {
                auto offset = fsm_hndl->trans->get_address() % (CFG::BUSWIDTH / 8);
                if(offset + fsm_hndl->trans->get_data_length() > CFG::BUSWIDTH / 8) {
                    SCCFATAL(SCMOD) << " transaction " << *fsm_hndl->trans << " is not AXI4Lite compliant";
                }
            }
        }
    };
    fsm_hndl->fsm->cb[BegPartReqE] = [this, fsm_hndl]() -> void {
        sc_assert(fsm_hndl->trans->is_write());
        if(fsm_hndl->beat_count == 0) {
            aw_fifo.push_back({fsm_hndl->trans.get(), false});
        }
        wdata_fifo.push_back({fsm_hndl->trans.get(), false, wdata_fifo.num_avail() > 0, fsm_hndl->beat_count});
        if(pipelined_wrreq && !wdata_fifo.num_avail())
            schedule(EndPartReqE, fsm_hndl->trans, sc_core::SC_ZERO_TIME);
    };
    fsm_hndl->fsm->cb[EndPartReqE] = [this, fsm_hndl]() -> void {
        tlm::tlm_phase phase = axi::END_PARTIAL_REQ;
        sc_core::sc_time t(clk_if ? ::scc::time_to_next_posedge(clk_if) - 1_ps : sc_core::SC_ZERO_TIME);
        auto ret = tsckt->nb_transport_bw(*fsm_hndl->trans, phase, t);
        fsm_hndl->beat_count++;
    };
    fsm_hndl->fsm->cb[BegReqE] = [this, fsm_hndl]() -> void {
        if(fsm_hndl->is_snoop) {
            sc_core::sc_time t(sc_core::SC_ZERO_TIME);
            tlm::tlm_phase phase = tlm::BEGIN_REQ;
            auto ret = tsckt->nb_transport_bw(*fsm_hndl->trans, phase, t);
        } else {
            switch(fsm_hndl->trans->get_command()) {
            case tlm::TLM_READ_COMMAND:
                ar_fifo.push_back({fsm_hndl->trans.get(), false});
                break;
            case tlm::TLM_WRITE_COMMAND:
                if(fsm_hndl->beat_count == 0) {
                    aw_fifo.push_back({fsm_hndl->trans.get(), false});
                }
                /* for Evict Trans, only addr on aw_t, response on b_t() */
                if(!axi::is_dataless(fsm_hndl->trans->get_extension<ace_extension>())) {
                    wdata_fifo.push_back({fsm_hndl->trans.get(), true, wdata_fifo.num_avail() > 0, fsm_hndl->beat_count});
                    if(pipelined_wrreq && !wdata_fifo.num_avail())
                        schedule(EndReqE, fsm_hndl->trans, sc_core::SC_ZERO_TIME);
                }
            }
        }
    };
    fsm_hndl->fsm->cb[EndReqE] = [this, fsm_hndl]() -> void {
        if(fsm_hndl->is_snoop) {
            active_req[SNOOP] = nullptr;
            ac_end_req_evt.notify(); // if snoop
        } else {
            auto id = axi::get_axi_id(*fsm_hndl->trans);
            if(mask_axi_id.get_value())
                id &= (1UL << CFG::IDWIDTH) - 1;
            switch(fsm_hndl->trans->get_command()) {
            case tlm::TLM_READ_COMMAND:
                rd_resp_by_id[id].push_back(fsm_hndl);
                break;
            case tlm::TLM_WRITE_COMMAND:
                wr_resp_by_id[id].push_back(fsm_hndl);
                fsm_hndl->beat_count++;
            }
            tlm::tlm_phase phase = tlm::END_REQ;
            sc_core::sc_time t(clk_if ? ::scc::time_to_next_posedge(clk_if) - 1_ps : sc_core::SC_ZERO_TIME);
            auto ret = tsckt->nb_transport_bw(*fsm_hndl->trans, phase, t);
            fsm_hndl->trans->set_response_status(tlm::TLM_OK_RESPONSE);
        }
    };
    fsm_hndl->fsm->cb[BegPartRespE] = [this, fsm_hndl]() -> void {
        if(fsm_hndl->is_snoop) {
            active_resp_beat[SNOOP] = fsm_hndl;
            cd_vl.notify({1, fsm_hndl});

        } else {
            // scheduling the response
            assert(fsm_hndl->trans->is_read());
            tlm::tlm_phase phase = axi::BEGIN_PARTIAL_RESP;
            sc_core::sc_time t(sc_core::SC_ZERO_TIME);
            auto ret = tsckt->nb_transport_bw(*fsm_hndl->trans, phase, t);
        }
    };
    fsm_hndl->fsm->cb[EndPartRespE] = [this, fsm_hndl]() -> void {
        if(fsm_hndl->is_snoop) {
            tlm::tlm_phase phase = axi::END_PARTIAL_RESP;
            sc_core::sc_time t(sc_core::SC_ZERO_TIME);
            auto ret = tsckt->nb_transport_bw(*fsm_hndl->trans, phase, t);
            // why here nullptr??
            active_resp_beat[SNOOP] = nullptr;
            fsm_hndl->beat_count++;
        } else {
            fsm_hndl->beat_count++;
            r_end_resp_evt.notify();
        }
    };
    fsm_hndl->fsm->cb[BegRespE] = [this, fsm_hndl]() -> void {
        if(fsm_hndl->is_snoop) {
            active_resp_beat[SNOOP] = fsm_hndl;
            cd_vl.notify({3, fsm_hndl}); // TBD??
            cr_resp_vl.notify({3, fsm_hndl});

        } else {
            // scheduling the response
            tlm::tlm_phase phase = tlm::BEGIN_RESP;
            sc_core::sc_time t(sc_core::SC_ZERO_TIME);
            auto ret = tsckt->nb_transport_bw(*fsm_hndl->trans, phase, t);
        }
    };
    fsm_hndl->fsm->cb[EndRespE] = [this, fsm_hndl]() -> void {
        if(fsm_hndl->is_snoop) {
            sc_core::sc_time t(sc_core::SC_ZERO_TIME);
            tlm::tlm_phase phase = tlm::END_RESP;
            auto ret = tsckt->nb_transport_bw(*fsm_hndl->trans, phase, t);
            active_resp_beat[SNOOP] = nullptr;
            // here notify cr_evnt
            fsm_hndl->finish.notify();
        } else {
            if(fsm_hndl->trans->is_read()) {
                rd_resp_by_id[axi::get_axi_id(*fsm_hndl->trans)].pop_front();
                r_end_resp_evt.notify();
            } else if(fsm_hndl->trans->is_write()) {
                wr_resp_by_id[axi::get_axi_id(*fsm_hndl->trans)].pop_front();
                w_end_resp_evt.notify();
            }
        }
    };
    fsm_hndl->fsm->cb[Ack] = [this, fsm_hndl]() -> void {
        SCCTRACE(SCMOD) << "in ACK of setup_cb for " << *fsm_hndl->trans;
        if(fsm_hndl->trans->is_read()) {
            rack_vl.notify(sc_core::SC_ZERO_TIME);
        }
        if(fsm_hndl->trans->is_write()) {
            wack_vl.notify(sc_core::SC_ZERO_TIME);
        }
    };
}

template <typename CFG> inline void axi::pin::ace_initiator<CFG>::rack_t() {
    this->r_ack.write(false);
    wait(sc_core::SC_ZERO_TIME);
    while(true) {
        wait(rack_vl);
        this->r_ack.write(true);
        wait(clk_i.posedge_event());
        this->r_ack.write(false);
    }
}

template <typename CFG> inline void axi::pin::ace_initiator<CFG>::wack_t() {
    this->w_ack.write(false);
    wait(sc_core::SC_ZERO_TIME);
    while(true) {
        wait(wack_vl);
        this->w_ack.write(true);
        wait(clk_i.posedge_event());
        this->w_ack.write(false);
    }
}

template <typename CFG> inline void axi::pin::ace_initiator<CFG>::ar_t() {
    this->ar_valid.write(false);
    wait(sc_core::SC_ZERO_TIME);
    while(true) {
        auto val = ar_fifo.read();
        write_ar(*val.gp);
        this->ar_valid.write(true);
        do {
            wait(this->ar_ready.posedge_event() | clk_delayed);
            if(this->ar_ready.read())
                react(axi::fsm::protocol_time_point_e::EndReqE, val.gp);
        } while(!this->ar_ready.read());
        wait(clk_i.posedge_event());
        this->ar_valid.write(false);
    }
}

template <typename CFG> inline void axi::pin::ace_initiator<CFG>::r_t() {
    this->r_ready.write(false);
    wait(sc_core::SC_ZERO_TIME);
    while(true) {
        wait(clk_delayed);
        while(!this->r_valid.read()) {
            wait(this->r_valid.posedge_event());
            wait(CLK_DELAY); // verilator might create spurious events
        }
        auto id = CFG::IS_LITE ? 0U : this->r_id->read().to_uint();
        auto data = this->r_data.read();
        auto resp = this->r_resp.read();
        auto& q = rd_resp_by_id[id];
        sc_assert(q.size() && "No transaction found for received id");
        auto* fsm_hndl = q.front();
        auto beat_count = fsm_hndl->beat_count;
        auto size = axi::get_burst_size(*fsm_hndl->trans);
        auto byte_offset = beat_count * size;
        auto offset = (fsm_hndl->trans->get_address() + byte_offset) & (CFG::BUSWIDTH / 8 - 1);
        if(offset && (size + offset) > (CFG::BUSWIDTH / 8)) { // un-aligned multi-beat access
            if(beat_count == 0) {
                auto dptr = fsm_hndl->trans->get_data_ptr();
                if(dptr)
                    for(size_t i = offset; i < size; ++i, ++dptr) {
                        auto bit_offs = i * 8;
                        *dptr = data(bit_offs + 7, bit_offs).to_uint();
                    }
            } else {
                auto beat_start_idx = beat_count * size - offset;
                auto data_len = fsm_hndl->trans->get_data_length();
                auto dptr = fsm_hndl->trans->get_data_ptr() + beat_start_idx;
                if(dptr)
                    for(size_t i = offset; i < size && (beat_start_idx + i) < data_len; ++i, ++dptr) {
                        auto bit_offs = i * 8;
                        *dptr = data(bit_offs + 7, bit_offs).to_uint();
                    }
            }
        } else { // aligned or single beat access
            auto dptr = fsm_hndl->trans->get_data_ptr() + beat_count * size;
            if(dptr)
                for(size_t i = 0; i < size; ++i, ++dptr) {
                    auto bit_offs = (offset + i) * 8;
                    *dptr = data(bit_offs + 7, bit_offs).to_uint();
                }
        }
        axi::ace_extension* e;
        fsm_hndl->trans->get_extension(e);
        e->set_cresp(resp);
        e->add_to_response_array(*e);
        /* for Make Trans, Clean Trans and Read barrier Trans, no  read data transfer on r_t, only response on r_t
         *  */
        if(axi::is_dataless(e)) {
            SCCTRACE(SCMOD) << " r_t() for Make/Clean/Barrier Trans" << *fsm_hndl->trans;
            react(axi::fsm::protocol_time_point_e::BegRespE, fsm_hndl);
        } else {
            auto tp = CFG::IS_LITE || this->r_last->read() ? axi::fsm::protocol_time_point_e::BegRespE
                                                           : axi::fsm::protocol_time_point_e::BegPartRespE;
            react(tp, fsm_hndl);
        }
        wait(r_end_resp_evt);
        this->r_ready.write(true);
        wait(clk_i.posedge_event());
        this->r_ready.write(false);
    }
}

template <typename CFG> inline void axi::pin::ace_initiator<CFG>::aw_t() {
    this->aw_valid.write(false);
    wait(sc_core::SC_ZERO_TIME);
    while(true) {
        auto val = aw_fifo.read();
        write_aw(*val.gp);
        this->aw_valid.write(true);
        do {
            wait(this->aw_ready.posedge_event() | clk_delayed);
        } while(!this->aw_ready.read());
        if(axi::is_dataless(val.gp->template get_extension<axi::ace_extension>()))
            schedule(axi::fsm::protocol_time_point_e::EndReqE, val.gp, sc_core::SC_ZERO_TIME);
        wait(clk_i.posedge_event());
        this->aw_valid.write(false);
    }
}

template <typename CFG> inline void axi::pin::ace_initiator<CFG>::wdata_t() {
    this->w_valid.write(false);
    wait(sc_core::SC_ZERO_TIME);
    while(true) {
        if(!CFG::IS_LITE)
            this->w_last->write(false);
        if(pipelined_wrreq) {
            while(!wdata_fifo.num_avail()) {
                wait(clk_i.posedge_event());
            }
        } else {
            wait(wdata_fifo.data_written_event());
        }
        auto val = wdata_fifo.front();
        wdata_fifo.pop_front();
        write_wdata(*val.gp, val.beat_num);
        if(pipelined_wrreq && val.needs_end_req) {
            auto evt = CFG::IS_LITE || (val.last) ? axi::fsm::protocol_time_point_e::EndReqE : axi::fsm::protocol_time_point_e::EndPartReqE;
            schedule(evt, val.gp, sc_core::SC_ZERO_TIME);
        }
        this->w_valid.write(true);
        if(!CFG::IS_LITE)
            this->w_last->write(val.last);
        do {
            wait(this->w_ready.posedge_event() | clk_delayed);
            if(!pipelined_wrreq && this->w_ready.read()) {
                auto evt = val.last ? axi::fsm::protocol_time_point_e::EndReqE : axi::fsm::protocol_time_point_e::EndPartReqE;
                schedule(evt, val.gp, sc_core::SC_ZERO_TIME);
            }
        } while(!this->w_ready.read());
        wait(clk_i.posedge_event());
        this->w_valid.write(false);
    }
}

template <typename CFG> inline void axi::pin::ace_initiator<CFG>::b_t() {
    this->b_ready.write(false);
    wait(sc_core::SC_ZERO_TIME);
    while(true) {
        wait(clk_delayed);
        while(!this->b_valid.read()) {
            wait(this->b_valid.posedge_event());
            wait(CLK_DELAY); // verilator might create spurious events
        }
        auto id = !CFG::IS_LITE ? this->b_id->read().to_uint() : 0U;
        auto resp = this->b_resp.read();
        auto& q = wr_resp_by_id[id];
        sc_assert(q.size());
        auto* fsm_hndl = q.front();
        axi::ace_extension* e;
        fsm_hndl->trans->get_extension(e);
        e->set_resp(axi::into<axi::resp_e>(resp));
        react(axi::fsm::protocol_time_point_e::BegRespE, fsm_hndl);
        wait(w_end_resp_evt);
        this->b_ready.write(true);
        wait(clk_i.posedge_event());
        this->b_ready.write(false);
    }
}

template <typename CFG> inline void axi::pin::ace_initiator<CFG>::ac_t() {
    this->ac_ready.write(false);
    wait(sc_core::SC_ZERO_TIME);
    auto arid = 0U;
    // A snoop transaction must be a full cache line in length,
    // here cachelinesize in byte, -1 because last beat in Resp transmitted
    auto arlen = ((CACHELINE_SZ - 1) / CFG::BUSWIDTH / 8);
    auto arsize = util::ilog2(CFG::BUSWIDTH / 8);
    // here +1 because last beat in Resp transmitted
    auto data_len = (1 << arsize) * (arlen + 1);
    while(true) {
        wait(clk_delayed);
        while(!this->ac_valid.read()) {
            wait(this->ac_valid.posedge_event());
            wait(CLK_DELAY); // verilator might create spurious events
        }
        SCCTRACE(SCMOD) << "ACVALID detected, for address 0x" << std::hex << this->ac_addr.read();
        SCCTRACE(SCMOD) << "in ac_t(), create snoop trans with data_len= " << data_len;
        auto gp = tlm::scc::tlm_mm<>::get().allocate<axi::ace_extension>(data_len, true);
        gp->set_address(this->ac_addr.read());
        gp->set_command(tlm::TLM_READ_COMMAND); // snoop command
        gp->set_streaming_width(data_len);
        axi::ace_extension* ext;
        gp->get_extension(ext);
        // if cacheline smaller than buswidth, beat num=1
        if(data_len == (CFG::BUSWIDTH / 8))
            arlen = 1;
        ext->set_length(arlen);
        ext->set_size(arsize);
        ext->set_snoop(axi::into<axi::snoop_e>(this->ac_snoop->read()));
        ext->set_prot(this->ac_prot->read());
        /*snoop transaction of burst length greater than one must be of burst type WRAP.
         * A snoop transaction of burst length one must be of burst type INCR
         */
        ext->set_burst((CACHELINE_SZ * 8) > CFG::BUSWIDTH ? axi::burst_e::WRAP : axi::burst_e::INCR);
        active_req[SNOOP] = find_or_create(gp, true);
        active_req[SNOOP]->is_snoop = true;
        react(axi::fsm::protocol_time_point_e::RequestPhaseBeg, active_req[SNOOP]);
        // ac_end_req_evt notified in EndReqE
        wait(ac_end_req_evt);
        this->ac_ready.write(true);
        wait(clk_i.posedge_event());
        this->ac_ready.write(false);
    }
}

template <typename CFG> inline void axi::pin::ace_initiator<CFG>::cd_t() {
    this->cd_valid.write(false);
    wait(sc_core::SC_ZERO_TIME);
    fsm_handle* fsm_hndl;
    uint8_t val;
    while(true) {
        // cd_vl notified in BEGIN_PARTIAL_REQ ( val=1 ??)or in BEG_RESP(val=3??)
        std::tie(val, fsm_hndl) = cd_vl.get();
        SCCTRACE(SCMOD) << __FUNCTION__ << " val = " << (uint16_t)val << " beat_count = " << fsm_hndl->beat_count;
        SCCTRACE(SCMOD) << __FUNCTION__ << " got snoop beat of trans " << *fsm_hndl->trans;
        // data already packed in Trans in END_REQ via calling operation_cb
        auto ext = fsm_hndl->trans->get_extension<axi::ace_extension>();
        this->cd_data.write(get_cache_data_for_beat(fsm_hndl));
        this->cd_valid.write(val & 0x1);
        SCCTRACE(SCMOD) << __FUNCTION__ << "() write cd_valid high ";
        this->cd_last->write(val & 0x2);
        do {
            wait(this->cd_ready.posedge_event() | clk_delayed);
            if(this->cd_ready.read()) {
                auto evt =
                    CFG::IS_LITE || (val & 0x2) ? axi::fsm::protocol_time_point_e::EndRespE : axi::fsm::protocol_time_point_e::EndPartRespE;

                // here only schedule EndPartResp for cache data because EndResp is scheduled in cr_resp_t() when last beat is transferred
                if(!(val & 0x2)) { // BEGIN_PARTIAL_REQ ( val=1 ) or in BEG_RESP(val=3)
                    SCCTRACE(SCMOD) << __FUNCTION__ << "() receives cd_ready high, schedule evt " << evt2str(evt);
                    react(evt, active_resp_beat[SNOOP]);
                }
            }
        } while(!this->cd_ready.read());
        SCCTRACE(SCMOD) << __FUNCTION__ << " finished snoop beat of trans [" << fsm_hndl->trans << "]";
        wait(clk_i.posedge_event());
        this->cd_valid.write(false);
        if(val & 0x2) // if last beat, after one clock cd_last shouldbe low
            this->cd_last->write(false);
    }
}
template <typename CFG> inline void axi::pin::ace_initiator<CFG>::cr_resp_t() {
    this->cr_valid.write(false);
    wait(sc_core::SC_ZERO_TIME);
    fsm_handle* fsm_hndl;
    uint8_t val;
    while(true) {
        // cr_resp_vl notified in BEG_RESP(val=3??)
        std::tie(val, fsm_hndl) = cr_resp_vl.get();
        SCCTRACE(SCMOD) << __FUNCTION__ << " (), generate snoop response in cr channel, val = " << (uint16_t)val
                        << " total beat_num = " << fsm_hndl->beat_count;
        // data already packed in Trans in END_REQ via bw_o
        auto ext = fsm_hndl->trans->get_extension<axi::ace_extension>();
        this->cr_resp.write((ext->get_cresp()));
        this->cr_valid.write(true);
        do {
            wait(this->cr_ready.posedge_event() | clk_delayed);
            if(this->cr_ready.read()) {
                auto evt = axi::fsm::protocol_time_point_e::EndRespE;
                SCCTRACE(SCMOD) << __FUNCTION__ << "(), schedule EndRespE ";
                react(evt, active_resp_beat[SNOOP]);
            }
        } while(!this->cr_ready.read());
        SCCTRACE(SCMOD) << "finished snoop response ";
        wait(clk_i.posedge_event());
        this->cr_valid.write(false);
    }
}

#endif /* _BUS_AXI_PIN_ACE_INITIATOR_H_ */
