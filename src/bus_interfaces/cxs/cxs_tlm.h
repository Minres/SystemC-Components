/*******************************************************************************
 * Copyright 2024 MINRES Technologies GmbH
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

#ifndef _CXS_CXS_TLM_H_
#define _CXS_CXS_TLM_H_

#include <cci_configuration>
#include <cstdint>
#include <scc/fifo_w_cb.h>
#include <scc/peq.h>
#include <scc/report.h>
#include <tlm/nw/tlm_network_gp.h>
#include <tlm/nw/tlm_network_sockets.h>
#include <tlm/scc/tlm_gp_shared.h>
#include <tlm/scc/tlm_mm.h>

//! @brief CXS TLM utilities
namespace cxs {
enum class CXS_CMD { FLIT, CREDIT, CRDRTN };

struct cxs_flit_payload : public tlm::nw::tlm_network_payload<CXS_CMD> {
    cxs_flit_payload() = default;

    explicit cxs_flit_payload(tlm::nw::tlm_base_mm_interface* mm)
    : tlm::nw::tlm_network_payload<CXS_CMD>(mm) {}

    std::array<uint8_t, 8> start_ptr;
    std::array<uint8_t, 8> end_ptr;
    uint8_t start{0};
    uint8_t end{0};
    uint8_t end_error{0};
    bool last{false};
};

struct cxs_flit_types {
    using tlm_payload_type = cxs_flit_payload;
    using tlm_phase_type = ::tlm::tlm_phase;
};

enum class CXS_PKT { DATA };
struct cxs_packet_payload : public tlm::nw::tlm_network_payload<CXS_PKT> {
    cxs_packet_payload() = default;

    explicit cxs_packet_payload(tlm::nw::tlm_base_mm_interface* mm)
    : tlm::nw::tlm_network_payload<CXS_PKT>(mm) {}
};

struct cxs_packet_types {
    using tlm_payload_type = cxs_packet_payload;
    using tlm_phase_type = ::tlm::tlm_phase;
};

} // namespace cxs

namespace tlm {
namespace scc {
// provide needed info for SCC memory manager
template <> struct tlm_mm_traits<cxs::cxs_flit_types> {
    using mm_if_type = tlm::nw::tlm_base_mm_interface;
    using payload_base = tlm::nw::tlm_network_payload_base;
};
template <> struct tlm_mm_traits<cxs::cxs_packet_types> {
    using mm_if_type = tlm::nw::tlm_base_mm_interface;
    using payload_base = tlm::nw::tlm_network_payload_base;
};
} // namespace scc
} // namespace tlm

namespace cxs {
template <unsigned PHITWIDTH = 256, int N = 1>
using cxs_flit_initiator_socket = tlm::nw::tlm_network_initiator_socket<PHITWIDTH, CXS_CMD, cxs_flit_types, N>;
template <unsigned PHITWIDTH = 256, int N = 1>
using cxs_flit_target_socket = tlm::nw::tlm_network_target_socket<PHITWIDTH, CXS_CMD, cxs_flit_types, N>;
using cxs_flit_shared_ptr = tlm::scc::tlm_payload_shared_ptr<cxs_flit_payload>;
using cxs_flit_mm = tlm::scc::tlm_mm<cxs_flit_types, false>;

template <int N = 1> using cxs_pkt_initiator_socket = tlm::nw::tlm_network_initiator_socket<8, CXS_PKT, cxs_packet_types, N>;
template <int N = 1> using cxs_pkt_target_socket = tlm::nw::tlm_network_target_socket<8, CXS_PKT, cxs_packet_types, N>;
using cxs_pkt_shared_ptr = tlm::scc::tlm_payload_shared_ptr<cxs_packet_payload>;
using cxs_pkt_mm = tlm::scc::tlm_mm<cxs_packet_types, false>;

struct orig_pkt_extension : public tlm::tlm_extension<orig_pkt_extension> {
    virtual tlm_extension_base* clone() const override {
        auto ret = new orig_pkt_extension();
        *ret = *this;
        return ret;
    }
    void copy_from(tlm_extension_base const& ext) override { *this = dynamic_cast<orig_pkt_extension const&>(ext); }
    virtual ~orig_pkt_extension() = default;

    std::vector<cxs_pkt_shared_ptr> orig_ext;
};

template <unsigned PHITWIDTH = 256, unsigned CXSMAXPKTPERFLIT = 2>
struct cxs_transmitter : public sc_core::sc_module,
                         public tlm::nw::tlm_network_fw_transport_if<cxs_packet_types>,
                         public tlm::nw::tlm_network_bw_transport_if<cxs_flit_types> {
    using flit_tx_type = cxs_flit_types::tlm_payload_type;
    using flit_phase_type = cxs_flit_types::tlm_phase_type;

    using pkt_tx_type = cxs_packet_types::tlm_payload_type;
    using pkt_phase_type = cxs_packet_types::tlm_phase_type;

    static constexpr unsigned PHIT_BYTE_WIDTH = PHITWIDTH / 8;
    static constexpr unsigned BUCKET_SIZE = PHITWIDTH / 8 / CXSMAXPKTPERFLIT;

    sc_core::sc_in<bool> clk_i{"clk_i"};

    cxs_pkt_target_socket<> tsck{"tsck"};

    cxs_flit_initiator_socket<PHITWIDTH> isck{"isck"};

    cci::cci_param<sc_core::sc_time> clock_period{"clock_period", sc_core::SC_ZERO_TIME, "clock period of the CXS transmitter"};

    cci::cci_param<unsigned> burst_len{"burst_len", 1, "minimum amount of credits to start transmitting flits"};

    cxs_transmitter(sc_core::sc_module_name const& nm)
    : sc_core::sc_module(nm) {
        tsck(*this);
        isck(*this);
        SC_HAS_PROCESS(cxs_transmitter);
        SC_METHOD(clock);
        sensitive << clk_i.pos();
    }

private:
    void start_of_simulation() override {
        if(clock_period.get_value() == sc_core::SC_ZERO_TIME)
            if(auto clk_if = dynamic_cast<sc_core::sc_clock*>(clk_i.get_interface()))
                clock_period.set_value(clk_if->period());
    }

    void b_transport(pkt_tx_type& trans, sc_core::sc_time& t) override {
        flit_tx_type tx;
        auto ext = new orig_pkt_extension();
        tx.set_extension(ext);
        ext->orig_ext.emplace_back(&trans);
        isck->b_transport(tx, t);
    }

    tlm::tlm_sync_enum nb_transport_fw(pkt_tx_type& trans, pkt_phase_type& phase, sc_core::sc_time& t) override {
        SCCTRACE(SCMOD) << "Received non-blocking transaction in fw path with phase " << phase.get_name();
        if(phase == tlm::nw::REQUEST) {
            pkt_peq.notify(cxs_pkt_shared_ptr(&trans));
            phase = tlm::nw::CONFIRM;
            return tlm::TLM_UPDATED;
        }
        throw std::runtime_error("illegal request in forward path");
    }

    unsigned int transport_dbg(pkt_tx_type& trans) override { return 0; }

    tlm::tlm_sync_enum nb_transport_bw(flit_tx_type& trans, flit_phase_type& phase, sc_core::sc_time& t) override {
        SCCTRACE(SCMOD) << "Received non-blocking transaction in bw path with phase " << phase.get_name();
        if(phase == tlm::nw::REQUEST) {
            credits += trans.get_data()[0];
            SCCDEBUG(SCMOD) << "Received " << static_cast<unsigned>(trans.get_data()[0]) << " credit(s), " << credits
                            << " credit(s) in total";
            phase = tlm::nw::CONFIRM;
            return tlm::TLM_UPDATED;
        }
        throw std::runtime_error("illegal request in backward path");
    }

    void clock() {
        auto flit_fill = 0U;
        unsigned next_bucket = 0;
        if((!pending_pkt && !pkt_peq.has_next()) ||             // there are no packets to send
           (credits < burst_len.get_value() && !burst_credits)) // we do not have enough credits to send them as burst
            return;
        auto* ptr = cxs_flit_mm::get().allocate();
        auto ext = ptr->get_extension<orig_pkt_extension>();
        if(!ext) {
            ext = new orig_pkt_extension();
            ptr->set_auto_extension(ext);
        }
        for(unsigned i = 0; i < CXSMAXPKTPERFLIT & pkt_peq.has_next(); ++i) {
            if(i == 0 && pending_pkt) {
                auto size = pending_pkt->get_data().size();
                auto bucketed_size = (size + 1 / BUCKET_SIZE);
                auto bytes_to_put = size - pending_byte_pos;
                if(bytes_to_put > PHIT_BYTE_WIDTH) {
                    pending_byte_pos += PHIT_BYTE_WIDTH;
                    fw_peq.notify(ptr);
                } else {
                    ptr->end |= 1u << i;
                    ptr->end_ptr[i] = (next_bucket * BUCKET_SIZE + size + 1) / 4; // end pointer is 4 byte aligned
                    ptr->last = true;
                    next_bucket += bucketed_size;
                    ext->orig_ext.push_back(pending_pkt);
                }
            } else {
                auto trans = pkt_peq.get();
                auto size = trans->get_data().size();
                auto bucketed_size = (size + 1) / BUCKET_SIZE;
                ptr->start |= 1u << i;
                ptr->start_ptr[i] = next_bucket;
                auto available_bytes = (CXSMAXPKTPERFLIT - i) * BUCKET_SIZE;
                if(bucketed_size <= (CXSMAXPKTPERFLIT - i)) {
                    ptr->end |= 1u << i;
                    ptr->end_ptr[i] = (next_bucket * BUCKET_SIZE + size + 1) / 4 - 1; // end pointer is 4 byte aligned
                    ptr->last = true;
                    next_bucket += bucketed_size;
                    ext->orig_ext.push_back(trans);
                }
            }
        }
        sc_core::sc_time t;
        auto phase = tlm::nw::REQUEST;
        isck->nb_transport_fw(*ptr, phase, t);
        if(!burst_credits) {
            burst_credits = burst_len.get_value();
            credits -= burst_credits;
        }
        burst_credits--;
    }

    scc::peq<cxs_pkt_shared_ptr> pkt_peq;
    cxs_pkt_shared_ptr pending_pkt;
    unsigned pending_byte_pos{0};
    scc::peq<cxs_flit_shared_ptr> fw_peq;

    unsigned credits{0};
    unsigned burst_credits{0};
};

template <unsigned PHITWIDTH = 64, unsigned CXSMAXPKTPERFLIT = 2>
struct cxs_receiver : public sc_core::sc_module,
                      public tlm::nw::tlm_network_fw_transport_if<cxs_flit_types>,
                      public tlm::nw::tlm_network_bw_transport_if<cxs_packet_types> {
    using flit_tx_type = cxs_flit_types::tlm_payload_type;
    using flit_phase_type = cxs_flit_types::tlm_phase_type;

    using pkt_tx_type = cxs_packet_types::tlm_payload_type;
    using pkt_phase_type = cxs_packet_types::tlm_phase_type;

    static constexpr unsigned PHIT_BYTE_WIDTH = PHITWIDTH / 8;
    static constexpr unsigned BUCKET_SIZE = PHITWIDTH / 8 / CXSMAXPKTPERFLIT;

    sc_core::sc_in<bool> clk_i{"clk_i"};

    sc_core::sc_in<bool> rst_i{"rst_i"};

    cxs_flit_target_socket<PHITWIDTH> tsck{"tsck"};

    cxs_pkt_initiator_socket<> isck{"isck"};

    cci::cci_param<sc_core::sc_time> clock_period{"clock_period", sc_core::SC_ZERO_TIME, "clock period of the CXS receiver"};

    cci::cci_param<unsigned> max_credit{"max_credits", 1, "CXS_MAX_CREDIT property"};

    cxs_receiver(sc_core::sc_module_name const& nm)
    : sc_core::sc_module(nm) {
        tsck(*this);
        isck(*this);
        SC_HAS_PROCESS(cxs_receiver);
        SC_METHOD(reset);
        sensitive << rst_i.neg();
        dont_initialize();
        SC_METHOD(send_credit);
        sensitive << credit_returned.data_written_event();
        dont_initialize();
    }

private:
    void start_of_simulation() override {
        if(clock_period.get_value() == sc_core::SC_ZERO_TIME)
            if(auto clk_if = dynamic_cast<sc_core::sc_clock*>(clk_i.get_interface()))
                clock_period.set_value(clk_if->period());
    }

    void b_transport(flit_tx_type& trans, sc_core::sc_time& t) override {
        auto ext = trans.get_extension<orig_pkt_extension>();
        sc_assert(ext);
        auto tx = ext->orig_ext.front();
        isck->b_transport(*tx, t);
    }

    tlm::tlm_sync_enum nb_transport_fw(flit_tx_type& trans, flit_phase_type& phase, sc_core::sc_time& t) override {
        SCCTRACE(SCMOD) << "Received non-blocking transaction in fw path with phase " << phase.get_name();
        credit_returned.push_back(1);
        auto* ptr = cxs_pkt_mm::get().allocate();
        auto ph = tlm::nw::REQUEST;
        auto d = sc_core::SC_ZERO_TIME;
        auto status = isck->nb_transport_fw(*ptr, ph, t);
        sc_assert(status == tlm::TLM_UPDATED);
        phase = tlm::nw::RESPONSE;
        if(clock_period.get_value() != sc_core::SC_ZERO_TIME)
            t += clock_period.get_value() - 1_ps;
        return tlm::TLM_UPDATED;
    }

    unsigned int transport_dbg(flit_tx_type& trans) override { return 0; }

    tlm::tlm_sync_enum nb_transport_bw(pkt_tx_type& trans, flit_phase_type& phase, sc_core::sc_time& t) override {
        SCCTRACE(SCMOD) << "Received non-blocking transaction in bw path with phase " << phase.get_name();
        if(phase == tlm::nw::CONFIRM)
            return tlm::TLM_ACCEPTED;
        throw std::runtime_error("illegal request in backward path");
    }

    void reset() {
        if(!rst_i.read()) {
            credit_returned.push_back(max_credit.get_value());
        }
    }

    void send_credit() {
        auto amount = credit_returned.front();
        credit_returned.pop_front();
        given_credits += amount;
        auto* ptr = cxs_flit_mm::get().allocate();
        ptr->set_command(cxs::CXS_CMD::CREDIT);
        ptr->set_data({amount});
        auto ph = tlm::nw::REQUEST;
        auto t = sc_core::SC_ZERO_TIME;
        auto status = tsck->nb_transport_bw(*ptr, ph, t);
        sc_assert(status == tlm::TLM_UPDATED);
    }

    unsigned given_credits{0};
    scc::fifo_w_cb<unsigned char> credit_returned;
};

template <unsigned PHITWIDTH = 64>
struct cxs_channel : public sc_core::sc_module,
                     public tlm::nw::tlm_network_fw_transport_if<cxs_flit_types>,
                     public tlm::nw::tlm_network_bw_transport_if<cxs_flit_types> {

    using transaction_type = cxs_flit_types::tlm_payload_type;
    using phase_type = cxs_flit_types::tlm_phase_type;

    cxs_flit_target_socket<PHITWIDTH> tsck{"tsck"};

    cxs_flit_initiator_socket<PHITWIDTH> isck{"isck"};

    cci::cci_param<sc_core::sc_time> channel_delay{"channel_delay", sc_core::SC_ZERO_TIME, "delay of the CXS channel"};

    cci::cci_param<sc_core::sc_time> tx_clock_period{"tx_clock_period", sc_core::SC_ZERO_TIME,
                                                     "receiver side clock period of the CXS channel"};

    cci::cci_param<sc_core::sc_time> rx_clock_period{"rx_clock_period", sc_core::SC_ZERO_TIME,
                                                     "receiver side clock period of the CXS channel"};

    cxs_channel(sc_core::sc_module_name const& nm)
    : sc_core::sc_module(nm) {
        isck(*this);
        tsck(*this);
        SC_HAS_PROCESS(cxs_channel);
        SC_METHOD(fw);
        SC_METHOD(bw);
    }

    void b_transport(transaction_type& trans, sc_core::sc_time& t) override {
        t += channel_delay;
        isck->b_transport(trans, t);
    }

    tlm::tlm_sync_enum nb_transport_fw(transaction_type& trans, phase_type& phase, sc_core::sc_time& t) override {
        SCCTRACE(SCMOD) << "Received non-blocking transaction in fw path with phase " << phase.get_name();
        if(phase == tlm::nw::REQUEST) {
            if(trans.get_data().size() > PHITWIDTH / 8) {
                SCCERR(SCMOD) << "A CXS flit can be maximal " << PHITWIDTH / 8 << " bytes long, current data length is "
                              << trans.get_data().size() << " bytes";
            }
            fw_peq.notify(cxs_flit_shared_ptr(&trans), channel_delay.get_value());
            if(rx_clock_period.get_value() > sc_core::SC_ZERO_TIME)
                t += rx_clock_period - 1_ps;
            phase = tlm::nw::CONFIRM;
            return tlm::TLM_UPDATED;
        } else if(phase == tlm::nw::RESPONSE) { // a credit response
            bw_resp.notify(sc_core::SC_ZERO_TIME);
            return tlm::TLM_ACCEPTED;
        }
        throw std::runtime_error("illegal request in forward path");
    }

    tlm::tlm_sync_enum nb_transport_bw(transaction_type& trans, phase_type& phase, sc_core::sc_time& t) override {
        SCCTRACE(SCMOD) << "Received non-blocking transaction in bw path with phase " << phase.get_name();
        if(phase == tlm::nw::REQUEST) { // this is a credit
            SCCDEBUG(SCMOD) << "Forwarding " << static_cast<unsigned>(trans.get_data()[0]) << " credit(s)";
            bw_peq.notify(cxs_flit_shared_ptr(&trans), channel_delay.get_value());
            if(tx_clock_period.get_value() > sc_core::SC_ZERO_TIME)
                t += tx_clock_period - 1_ps;
            phase = tlm::nw::CONFIRM;
            return tlm::TLM_UPDATED;
        } else if(phase == tlm::nw::RESPONSE) { // a data transfer completion
            fw_resp.notify(sc_core::SC_ZERO_TIME);
            return tlm::TLM_ACCEPTED;
        }
        throw std::runtime_error("illegal response in backward path");
    }

    unsigned int transport_dbg(transaction_type& trans) override { return isck->transport_dbg(trans); }

private:
    void fw() {
        while(fw_peq.has_next()) {
            auto ptr = fw_peq.get();
            auto phase = tlm::nw::INDICATION;
            auto t = sc_core::SC_ZERO_TIME;
            auto sync = isck->nb_transport_fw(*ptr, phase, t);
            if(sync == tlm::TLM_ACCEPTED) {
                next_trigger(fw_resp);
                return;
            } else {
                sc_assert(sync == tlm::TLM_UPDATED || sync == tlm::TLM_COMPLETED);
            }
        }
        next_trigger(fw_peq.event());
    }

    void bw() {
        while(bw_peq.has_next()) {
            auto ptr = bw_peq.get();
            auto ph{tlm::nw::REQUEST};
            sc_core::sc_time d;
            auto sync = tsck->nb_transport_bw(*ptr, ph, d);
            if(sync == tlm::TLM_ACCEPTED) {
                next_trigger(bw_resp);
                return;
            } else {
                sc_assert(sync == tlm::TLM_UPDATED || sync == tlm::TLM_COMPLETED);
            }
        }
        next_trigger(bw_peq.event());
    }

    scc::peq<cxs_flit_shared_ptr> fw_peq;
    scc::peq<cxs_flit_shared_ptr> bw_peq;
    sc_core::sc_event fw_resp, bw_resp;
};
} // namespace cxs
#endif // _CXS_CXS_TLM_H_
