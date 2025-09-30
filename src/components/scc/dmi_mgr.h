/*
 * dmi_mgr.h
 *
 *  Created on: Jun 22, 2025
 *      Author: eyck
 */

#ifndef SCC_SRC_COMPONENTS_SCC_DMI_MGR_H_
#define SCC_SRC_COMPONENTS_SCC_DMI_MGR_H_

#include <scc/report.h>
#include <scv-tr/scv_tr.h>
#include <sysc/communication/sc_port.h>
#include <tlm>
#include <tlm_core/tlm_2/tlm_2_interfaces/tlm_dmi.h>
#include <tlm_utils/tlm_quantumkeeper.h>
#include <util/range_lut.h>

namespace tlm {
inline bool operator==(tlm_dmi const& o1, tlm_dmi const& o2) {
    return o1.get_granted_access() == o2.get_granted_access() && o1.get_start_address() == o2.get_start_address() &&
           o1.get_end_address() == o2.get_end_address();
}
inline bool operator!=(tlm_dmi const& o1, tlm_dmi const& o2) { return !operator==(o1, o2); }
} // namespace tlm

namespace scc {
/**
 * @brief The dmi_status enum represents the status of DMI transactions.
 *
 * The dmi_status enum is used to indicate the success or failure of DMI read and write operations.
 * It provides a clear and concise way to communicate the status of the transactions.
 *
 * @note The dmi_status enum is a part of the SystemC Component (SCC) library.
 */
enum dmi_status { ERROR = 0, OK = 1, DMI_RD = 2, DMI_WR = 4, DMI_ALL = 6 };
inline dmi_status operator|=(dmi_status s1, dmi_status s2) { return static_cast<dmi_status>(s1 | s2); }
/**
 * @brief The dmi_mgr class manages Direct Memory Interface (DMI) transactions.
 *
 * The dmi_mgr class is a template class that provides DMI management functionality.
 * It interacts with the TLM (Transaction Level Modeling) framework to handle DMI transactions.
 *
 * @tparam TYPES The TLM protocol types.
 *
 * @note The dmi_mgr class is a part of the SystemC Component (SCC) library.
 */
template <typename TYPES = tlm::tlm_base_protocol_types> struct dmi_mgr : public sc_core::sc_object {
    /**
     * @brief A CCI parameter to disable DMI transactions.
     *
     * This parameter allows the user to disable DMI transactions if needed.
     * By default, DMI transactions are enabled.
     */
    cci::cci_param<bool> disable_dmi{"disable_dmi", false};
    /**
     * @brief A CCI parameter to specify the clock period for delay calculations.
     *
     * This parameter allows the user to set the clock period for the DMI transactions.
     * By default, the clock period is set to SC_ZERO_TIME.
     */
    cci::cci_param<sc_core::sc_time> clk_period{"clk_period", sc_core::SC_ZERO_TIME};
    /**
     * @brief Constructor for the dmi_mgr class.
     *
     * @param name The name of the dmi_mgr instance.
     * @param fw_if The TLM (Transaction Level Modeling) forward transport interface.
     */

    dmi_mgr(std::string const& name, sc_core::sc_port_b<tlm::tlm_fw_transport_if<TYPES>>& fw_if)
    : sc_core::sc_object(name.c_str())
    , fw_if(fw_if) {}
    /**
     * @brief Virtual destructor for the dmi_mgr class.
     */
    virtual ~dmi_mgr() = default;
    /**
     * @brief Performs a read operation on the DMI interface.
     *
     * @param addr The address to read from.
     * @param length The length of the data to read.
     * @param data A pointer to the buffer where the read data will be stored.
     *
     * @return The status of the read operation.
     */
    dmi_status read(uint64_t addr, unsigned length, uint8_t* const data) {
        auto lut_entry = read_lut.getEntry(addr);
        if(lut_entry.get_granted_access() != tlm::tlm_dmi::DMI_ACCESS_NONE && addr + length <= lut_entry.get_end_address() + 1) {
            auto offset = addr - lut_entry.get_start_address();
            std::copy(lut_entry.get_dmi_ptr() + offset, lut_entry.get_dmi_ptr() + offset + length, data);
            bus_clk_sycles += lut_entry.get_read_latency() / clk_period.get_value();
            return DMI_RD;
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
            SCCTRACE(this->name()) << "[local time: " << delay << "]: finish read(0x" << std::hex << addr << ") : 0x"
                                   << (length == 4   ? *(uint32_t*)data
                                       : length == 2 ? *(uint16_t*)data
                                                     : (unsigned)*data);
            if(gp.get_response_status() != tlm::TLM_OK_RESPONSE) {
                return ERROR;
            }
            if(gp.is_dmi_allowed() && !disable_dmi.get_value()) {
                gp.set_command(tlm::TLM_READ_COMMAND);
                gp.set_address(addr);
                tlm::tlm_dmi dmi_data;
                if(fw_if->get_direct_mem_ptr(gp, dmi_data)) {
                    dmi_status res = ERROR;
                    if(dmi_data.is_read_allowed()) {
                        read_lut.addEntry(dmi_data, dmi_data.get_start_address(),
                                          dmi_data.get_end_address() - dmi_data.get_start_address() + 1);
                        res |= DMI_RD;
                    }
                    if(dmi_data.is_write_allowed()) {
                        write_lut.addEntry(dmi_data, dmi_data.get_start_address(),
                                           dmi_data.get_end_address() - dmi_data.get_start_address() + 1);
                        res |= DMI_WR;
                    }
                    return res;
                }
            }
            return OK;
        }
    }
    /**
     * @brief Performs a write operation on the DMI interface.
     *
     * @param addr The address to write to.
     * @param length The length of the data to write.
     * @param data A pointer to the buffer containing the data to write.
     *
     * @return The status of the write operation.
     */
    dmi_status write(uint64_t addr, unsigned length, const uint8_t* const data) {
        auto lut_entry = write_lut.getEntry(addr);
        if(lut_entry.get_granted_access() != tlm::tlm_dmi::DMI_ACCESS_NONE && addr + length <= lut_entry.get_end_address() + 1) {
            auto offset = addr - lut_entry.get_start_address();
            std::copy(data, data + length, lut_entry.get_dmi_ptr() + offset);
            bus_clk_sycles += lut_entry.get_write_latency() / clk_period.get_value();
            return DMI_WR;
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
            SCCTRACE() << "[local time: " << delay << "]: finish write(0x" << std::hex << addr << ") : 0x"
                       << (length == 4   ? *(uint32_t*)data
                           : length == 2 ? *(uint16_t*)data
                                         : (unsigned)*data);
            if(gp.get_response_status() != tlm::TLM_OK_RESPONSE) {
                return ERROR;
            }
            if(gp.is_dmi_allowed() && !disable_dmi.get_value()) {
                gp.set_command(tlm::TLM_WRITE_COMMAND);
                gp.set_address(addr);
                tlm::tlm_dmi dmi_data;
                if(fw_if->get_direct_mem_ptr(gp, dmi_data)) {
                    dmi_status res = ERROR;
                    if(dmi_data.is_write_allowed()) {
                        write_lut.addEntry(dmi_data, dmi_data.get_start_address(),
                                           dmi_data.get_end_address() - dmi_data.get_start_address() + 1);
                        res |= DMI_WR;
                    }
                    if(dmi_data.is_read_allowed()) {
                        read_lut.addEntry(dmi_data, dmi_data.get_start_address(),
                                          dmi_data.get_end_address() - dmi_data.get_start_address() + 1);
                        res |= DMI_RD;
                    }
                    return res;
                }
            }
            return OK;
        }
    }

private:
    sc_core::sc_port_b<tlm::tlm_fw_transport_if<TYPES>>& fw_if;
    util::range_lut<tlm::tlm_dmi> read_lut{tlm::tlm_dmi{}}, write_lut{tlm::tlm_dmi{}};
    std::vector<uint8_t> write_buf;
    tlm_utils::tlm_quantumkeeper quantum_keeper;
    uint64_t bus_clk_sycles{0};
};

} /* namespace scc */

#endif /* SCC_SRC_COMPONENTS_SCC_DMI_MGR_H_ */
