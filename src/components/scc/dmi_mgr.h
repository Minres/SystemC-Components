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

namespace scc {

template <typename TYPES = tlm::tlm_base_protocol_types>
struct dmi_mgr: public sc_core::sc_object {
    struct tlm_dmi_ext : public tlm::tlm_dmi {
        bool operator==(const tlm_dmi_ext& o) const {
            return this->get_granted_access() == o.get_granted_access() && this->get_start_address() == o.get_start_address() &&
                   this->get_end_address() == o.get_end_address();
        }

        bool operator!=(const tlm_dmi_ext& o) const { return !operator==(o); }
    };

    cci::cci_param<bool> disable_dmi{"disable_dmi", false};

    dmi_mgr(std::string const& name, tlm::tlm_fw_transport_if<TYPES>& fw_if): sc_core::sc_object(name.c_str()), fw_if(fw_if) {
    }

    virtual ~dmi_mgr() = default;

    template <unsigned int BUSWIDTH> bool read_mem(uint64_t addr, unsigned length, uint8_t* const data, bool is_fetch) {
        auto& dmi_lut = is_fetch ? fetch_lut : read_lut;
        auto lut_entry = dmi_lut.getEntry(addr);
        if(lut_entry.get_granted_access() != tlm::tlm_dmi::DMI_ACCESS_NONE && addr + length <= lut_entry.get_end_address() + 1) {
            auto offset = addr - lut_entry.get_start_address();
            std::copy(lut_entry.get_dmi_ptr() + offset, lut_entry.get_dmi_ptr() + offset + length, data);
            if(is_fetch)
                ibus_inc += lut_entry.get_read_latency() / curr_clk;
            else
                dbus_inc += lut_entry.get_read_latency() / curr_clk;
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
                auto incr = (delay - quantum_keeper.get_local_time()) / curr_clk;
                if(is_fetch)
                    ibus_inc += incr;
                else
                    dbus_inc += incr;
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
                tlm_dmi_ext dmi_data;
                if(dbus->get_direct_mem_ptr(gp, dmi_data)) {
                    if(dmi_data.is_read_allowed())
                        dmi_lut.addEntry(dmi_data, dmi_data.get_start_address(), dmi_data.get_end_address() - dmi_data.get_start_address() + 1);
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
            dbus_inc += lut_entry.get_write_latency() / curr_clk;
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
                dbus_inc += (delay - quantum_keeper.get_local_time()) / curr_clk;
            SCCTRACE() << "[local time: " << delay << "]: finish write_mem(0x" << std::hex << addr << ") : 0x"
                       << (length == 4   ? *(uint32_t*)data
                           : length == 2 ? *(uint16_t*)data
                                         : (unsigned)*data);
            if(gp.get_response_status() != tlm::TLM_OK_RESPONSE) {
                return false;
            }
            if(gp.is_dmi_allowed() && !disable_dmi.get_value()) {
                gp.set_command(tlm::TLM_READ_COMMAND);
                gp.set_address(addr);
                tlm_dmi_ext dmi_data;
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
    util::range_lut<tlm_dmi_ext> fetch_lut, read_lut, write_lut;
    std::vector<uint8_t> write_buf;
    tlm_utils::tlm_quantumkeeper quantum_keeper;
    sc_core::sc_signal<sc_core::sc_time> curr_clk;
    uint64_t ibus_inc{0}, dbus_inc{0};
};

} /* namespace scc */

#endif /* SCC_SRC_COMPONENTS_SCC_DMI_MGR_H_ */
