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

#ifndef _BUS_TLM_PIN_OCP_TARGET_H_
#define _BUS_TLM_PIN_OCP_TARGET_H_

#include <ocp/ocp_tlm.h>
#include <scc/peq.h>
#include <scc/report.h>
#include <scc/utilities.h>
#include <systemc>
#include <tlm/scc/tlm_gp_shared.h>
#include <tlm/scc/tlm_mm.h>
#include <util/ities.h>

//! TLM2.0 components modeling TLM
namespace ocp {
//! pin level adapters
namespace pin {

#define OCP_CLK_DELAY 1_ps

template <unsigned DATA_WIDTH, unsigned ADDR_WIDTH, unsigned BUSWIDTH = DATA_WIDTH>
struct target : public sc_core::sc_module, public tlm::tlm_bw_transport_if<tlm::tlm_base_protocol_types> {
    SC_HAS_PROCESS(target);

    using payload_type = tlm::tlm_base_protocol_types::tlm_payload_type;
    using phase_type = tlm::tlm_base_protocol_types::tlm_phase_type;

    sc_core::sc_in<bool> clk_i{"clk_i"};
    sc_core::sc_in<bool> reset{""};

    sc_core::sc_in<sc_uint<3>> MCmd{"MCmd"};
    sc_core::sc_in<sc_uint<ADDR_WIDTH>> MAddr{"MAddr"};
    sc_core::sc_in<sc_uint<DATA_WIDTH>> MData{"MData"};
    sc_core::sc_in<sc_uint<DATA_WIDTH / 8>> MByteEn{"MByteEn"};
    sc_core::sc_out<bool> SCmdAccept{"SCmdAccept"};

    sc_core::sc_out<sc_uint<2>> SResp{"SResp"};
    sc_core::sc_out<sc_uint<DATA_WIDTH>> SData{"SData"};
    sc_core::sc_in<bool> MRespAccept{"MRespAccept"};

    tlm::tlm_initiator_socket<BUSWIDTH> isckt{"isckt"};

    target(sc_core::sc_module_name const& nm)
    : sc_core::sc_module(nm) {
        isckt.bind(*this);
        SC_METHOD(clk_delay);
        sensitive << clk_i.pos();
        dont_initialize();
        SC_THREAD(req);
        SC_THREAD(resp);
    }

private:
    tlm::tlm_sync_enum nb_transport_bw(payload_type& trans, phase_type& phase, sc_core::sc_time& t) override;

    void invalidate_direct_mem_ptr(sc_dt::uint64 start_range, sc_dt::uint64 end_range) override;

    void end_of_elaboration() override { clk_if = dynamic_cast<sc_core::sc_clock*>(clk_i.get_interface()); }

    void clk_delay() { clk_delayed.notify(OCP_CLK_DELAY); }
    void req();
    void resp();
    sc_core::sc_clock* clk_if{nullptr};
    sc_core::sc_event clk_delayed, end_req_received_evt, wdata_end_req_evt;
    scc::peq<tlm::scc::tlm_gp_shared_ptr> resp_queue;
};

} // namespace pin
} // namespace ocp

template <unsigned DATA_WIDTH, unsigned ADDR_WIDTH, unsigned BUSWIDTH>
inline tlm::tlm_sync_enum ocp::pin::target<DATA_WIDTH, ADDR_WIDTH, BUSWIDTH>::nb_transport_bw(payload_type& trans, phase_type& phase,
                                                                                              sc_core::sc_time& t) {
    if(phase == tlm::END_REQ) {
        end_req_received_evt.notify(sc_core::SC_ZERO_TIME);
    } else if(phase == tlm::BEGIN_RESP) {
        resp_queue.notify(tlm::scc::tlm_gp_shared_ptr(&trans), t);
        t += clk_if->period().value() ? clk_if->period() : sc_core::SC_ZERO_TIME;
    }
    return tlm::TLM_ACCEPTED;
    ;
}

template <unsigned DATA_WIDTH, unsigned ADDR_WIDTH, unsigned BUSWIDTH>
inline void ocp::pin::target<DATA_WIDTH, ADDR_WIDTH, BUSWIDTH>::invalidate_direct_mem_ptr(sc_dt::uint64 start_range,
                                                                                          sc_dt::uint64 end_range) {}

template <unsigned DATA_WIDTH, unsigned ADDR_WIDTH, unsigned BUSWIDTH>
inline void ocp::pin::target<DATA_WIDTH, ADDR_WIDTH, BUSWIDTH>::req() {
    this->SCmdAccept.write(false);
    wait(sc_core::SC_ZERO_TIME);
    while(true) {
        wait(clk_delayed);
        this->SCmdAccept.write(false);
        while(!this->MCmd.read()) {
            wait(this->MCmd.default_event());
        }
        auto cmd = static_cast<ocp::cmd_e>(this->MCmd.read().to_uint());
        auto gp = tlm::scc::tlm_mm<>::get().allocate<ocp::ocp_extension>(DATA_WIDTH / 8, true);
        gp->set_streaming_width(DATA_WIDTH / 8);
        gp->set_data_length(DATA_WIDTH / 8);
        gp->set_byte_enable_length(DATA_WIDTH / 8);
        auto addr = this->MAddr.read().to_uint64();
        gp->set_address(addr);
        switch(cmd) {
        case cmd_e::WRITE:
        case cmd_e::WRITE_CONDITIONAL:
        case cmd_e::WRITE_NON_POSTED: {
            gp->set_command(tlm::TLM_WRITE_COMMAND);
            auto data = this->MData.read();
            auto strb = this->MByteEn.read();
            auto dptr = gp->get_data_ptr();
            auto beptr = gp->get_byte_enable_ptr();
            for(size_t i = 0; i < DATA_WIDTH / 8; ++i, ++dptr, ++beptr) {
                *dptr = data((i << 3) + 7, i << 3).to_uint();
                *beptr = strb[i] ? 0xff : 0;
            }
        } break;
        case cmd_e::READ:
        case cmd_e::READEX:
        case cmd_e::READ_LINKED:
            gp->set_command(tlm::TLM_READ_COMMAND);
            std::fill(gp->get_byte_enable_ptr(), gp->get_byte_enable_ptr() + DATA_WIDTH / 8, 0xff);
            break;
        default:
            SCCFATAL(SCMOD) << "not supported";
        }
        auto ext = gp->get_extension<ocp::ocp_extension>();
        ext->set_mcmd(cmd);
        SCCDEBUG(SCMOD) << "received OCP bus transaction req " << *gp;
        tlm::tlm_phase ph = tlm::BEGIN_REQ;
        auto t{sc_core::SC_ZERO_TIME};
        auto status = isckt->nb_transport_fw(*gp, ph, t);
        if(status == tlm::TLM_ACCEPTED) {
            wait(end_req_received_evt);
        }
        this->SCmdAccept.write(true);
        wait(clk_i.posedge_event());
    }
}

template <unsigned DATA_WIDTH, unsigned ADDR_WIDTH, unsigned BUSWIDTH>
inline void ocp::pin::target<DATA_WIDTH, ADDR_WIDTH, BUSWIDTH>::resp() {
    this->SResp.write(static_cast<unsigned>(ocp::resp_e::NULL_));
    wait(sc_core::SC_ZERO_TIME);
    uint8_t val;
    while(true) {
        wait(clk_i.posedge_event());
        this->SResp.write(static_cast<unsigned>(ocp::resp_e::NULL_));
        if(resp_queue.has_next()) {
            auto gp = resp_queue.get();
            SCCDEBUG(SCMOD) << "received OCP bus transaction resp " << *gp;
            this->SResp.write(static_cast<unsigned>(ocp::resp_e::DVA));
            if(gp->is_read()) {
                auto d = gp->get_data_ptr();
                this->SData.write(bit_comb<uint32_t>(d[0], d[1], d[2], d[3]));
            }
            do {
                wait(this->MRespAccept.default_event() | clk_delayed);
            } while(!this->MRespAccept.read());
            tlm::tlm_phase ph = tlm::END_RESP;
            auto t{sc_core::SC_ZERO_TIME};
            auto status = isckt->nb_transport_fw(*gp, ph, t);
        }
    }
}

#endif /* _BUS_TLM_PIN_OCP_TARGET_H_ */
