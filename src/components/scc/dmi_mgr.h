/*
 * dmi_mgr.h
 *
 *  Created on: Jun 22, 2025
 *      Author: eyck
 */

#ifndef SCC_SRC_COMPONENTS_SCC_DMI_MGR_H_
#define SCC_SRC_COMPONENTS_SCC_DMI_MGR_H_

#include <tlm>
#include <util/range_lut.h>
#include <tlm_utils/tlm_quantumkeeper.h>
#include <scc/report.h>
#include <scv-tr/scv_tr.h>

namespace tlm {
bool operator==(tlm_dmi const& o1, tlm_dmi const& o1) const {
    return o1.get_granted_access() == o2.get_granted_access() && o1.get_start_address() == o2.get_start_address() &&
           o1.get_end_address() == o2.get_end_address();
}
bool operator!=(tlm_dmi const& o1, tlm_dmi const& o2) const { return !operator==(o1, o2); }
}
namespace scc {

template <typename TYPES = tlm::tlm_base_protocol_types>
struct dmi_mgr: public sc_core::sc_object {

    cci::cci_param<bool> disable_dmi{"disable_dmi", false};

    cci::cci_param<sc_core::sc_time> clk_period{"clk_period", sc_core::SC_ZERO_TIME};

    dmi_mgr(std::string const& name, tlm::tlm_fw_transport_if<TYPES>& fw_if): sc_core::sc_object(name.c_str()), fw_if(fw_if) { }

    virtual ~dmi_mgr() = default;

    template <unsigned int BUSWIDTH> bool read_mem(uint64_t addr, unsigned length, uint8_t* const data) {
        auto lut_entry = read_lut.getEntry(addr);
        if(lut_entry.get_granted_access() != tlm::tlm_dmi::DMI_ACCESS_NONE && addr + length <= lut_entry.get_end_address() + 1) {
            auto offset = addr - lut_entry.get_start_address();
            std::copy(lut_entry.get_dmi_ptr() + offset, lut_entry.get_dmi_ptr() + offset + length, data);
            bus_clk_sycles += lut_entry.get_read_latency() / clk_period.get_value();
            return true;
        } else {
            tlm::tlm_generic_payload gp;
            gp.set_command(tlm::TLM_READ_COMMAND);
            gp.set_address(addr);
            gp.set_data_ptr(data);
            gp.set_data_length(length);
            gp.set_streaming_width(length);
            sc_core::sc_time delay = quantum_keeper.get_local_time();
            auto pre_delay = delay;
            fw_if->b_transport(gp, delay);
            if(pre_delay > delay) {
                quantum_keeper.reset();
            } else {
                auto incr = (delay - quantum_keeper.get_local_time()) / clk_period.get_value();
                bus_clk_sycles += incr;
            }
            SCCTRACE(this->name()) << "[local time: " << delay << "]: finish read_mem(0x" << std::hex << addr << ") : 0x"
                                   << (length == 4   ? *(uint32_t*)data
                                       : length == 2 ? *(uint16_t*)data
                                                     : (unsigned)*data);
            if(gp.get_response_status() != tlm::TLM_OK_RESPONSE) {
                return false;
            }
            if(gp.is_dmi_allowed() && !disable_dmi.get_value()) {
                gp.set_command(tlm::TLM_READ_COMMAND);
                gp.set_address(addr);
                tlm::tlm_dmi dmi_data;
                if(fw_if->get_direct_mem_ptr(gp, dmi_data)) {
                    if(dmi_data.is_read_allowed())
                        read_lut.addEntry(dmi_data, dmi_data.get_start_address(), dmi_data.get_end_address() - dmi_data.get_start_address() + 1);
                }
            }
            return true;
        }
    }

    template <unsigned int BUSWIDTH> bool write_mem(uint64_t addr, unsigned length, const uint8_t* const data) {
        auto lut_entry = write_lut.getEntry(addr);
        if(lut_entry.get_granted_access() != tlm::tlm_dmi::DMI_ACCESS_NONE && addr + length <= lut_entry.get_end_address() + 1) {
            auto offset = addr - lut_entry.get_start_address();
            std::copy(data, data + length, lut_entry.get_dmi_ptr() + offset);
            bus_clk_sycles += lut_entry.get_write_latency() / clk_period.get_value();
            return true;
        } else {
            write_buf.resize(length);
            std::copy(data, data + length, write_buf.begin()); // need to copy as TLM does not guarantee data integrity
            tlm::tlm_generic_payload gp;
            gp.set_command(tlm::TLM_WRITE_COMMAND);
            gp.set_address(addr);
            gp.set_data_ptr(write_buf.data());
            gp.set_data_length(length);
            gp.set_streaming_width(length);
            sc_core::sc_time delay = quantum_keeper.get_local_time();
            auto pre_delay = delay;
            fw_if->b_transport(gp, delay);
            if(pre_delay > delay)
                quantum_keeper.reset();
            else
                bus_clk_sycles += (delay - quantum_keeper.get_local_time()) / clk_period.get_value();
            SCCTRACE() << "[local time: " << delay << "]: finish write_mem(0x" << std::hex << addr << ") : 0x"
                       << (length == 4   ? *(uint32_t*)data
                           : length == 2 ? *(uint16_t*)data
                                         : (unsigned)*data);
            if(gp.get_response_status() != tlm::TLM_OK_RESPONSE) {
                return false;
            }
            if(gp.is_dmi_allowed() && !disable_dmi.get_value()) {
                gp.set_command(tlm::TLM_WRITE_COMMAND);
                gp.set_address(addr);
                tlm::tlm_dmi dmi_data;
                if(fw_if->get_direct_mem_ptr(gp, dmi_data)) {
                    if(dmi_data.is_write_allowed())
                        write_lut.addEntry(dmi_data, dmi_data.get_start_address(),
                                           dmi_data.get_end_address() - dmi_data.get_start_address() + 1);
                }
            }
            return true;
        }
    }
private:
    tlm::tlm_fw_transport_if<TYPES>& fw_if;
    util::range_lut<tlm::tlm_dmi> fetch_lut, read_lut, write_lut;
    std::vector<uint8_t> write_buf;
    tlm_utils::tlm_quantumkeeper quantum_keeper;
    uint64_t bus_clk_sycles{0};
};

} /* namespace scc */

#endif /* SCC_SRC_COMPONENTS_SCC_DMI_MGR_H_ */
