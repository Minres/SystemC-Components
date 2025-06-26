/*******************************************************************************
 * Copyright 2016, 2018 MINRES Technologies GmbH
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

#ifndef _SYSC_MEMORY_H_
#define _SYSC_MEMORY_H_

// Needed for the simple_target_socket
#include <tlm_core/tlm_2/tlm_generic_payload/tlm_gp.h>
#ifndef SC_INCLUDE_DYNAMIC_PROCESSES
#define SC_INCLUDE_DYNAMIC_PROCESSES
#endif

#include "clock_if_mixins.h"
#include <cci_configuration>
#include <cstdint>
#include <numeric>
#include <scc/mt19937_rng.h>
#include <scc/report.h>
#include <scc/signal_opt_ports.h>
#include <scc/utilities.h>
#include <tlm.h>
#include <tlm/scc/target_mixin.h>
#include <util/range_lut.h>
#include <util/sparse_array.h>

namespace scc {
/**
 * @class memory
 * @brief simple TLM2.0 LT memory model
 *
 * This model uses the \ref util::sparse_array as backing store. Therefore it can have an arbitrary size since only
 * pages for accessed addresses are allocated. it is possible to map specific host memory ranges to a target memory range.
 * For this the @ref scc::host_mem_map_extension hs to be used in conjunction with the TLM_IGNORE_COMMAND. The extension
 * carries the pointer to the host memory to be used while the generic payload address and length indicate the size of the memory block
 *
 * TODO: add some more attributes/parameters to configure access time and type (DMI allowed, read only, etc)
 *
 * @tparam SIZE size of the memery
 * @tparam BUSWIDTH bus width of the socket
 */
template <unsigned long long SIZE, unsigned BUSWIDTH = LT> class memory : public sc_core::sc_module {
public:
    //! the target socket to connect to TLM
    tlm::scc::target_mixin<tlm::tlm_target_socket<BUSWIDTH>> target{"ts"};
    //! CCI parameter to configure if DMI is allowd
    cci::cci_param<bool> allow_dmi{"allow_dmi", true, "Allow DMI accesses to this memory if set"};
    /**
     * constructor with explicit instance name
     *
     * @param nm
     */
    memory(const sc_core::sc_module_name& nm);
    /**
     * @fn unsigned long long getSize()const
     * @brief return the size of the array
     *
     */
    constexpr unsigned long long getSize() const { return SIZE; }
    /**
     * @fn void set_operation_callback(std::function<int (memory<SIZE,BUSWIDTH>&, tlm::tlm_generic_payload&)>)
     * @brief allows to register a callback or functor being invoked upon an access to the memory
     *
     * @param cb the callback function or functor
     */
    void set_operation_callback(std::function<int(memory<SIZE, BUSWIDTH>&, tlm::tlm_generic_payload&, sc_core::sc_time& delay)> cb) {
        operation_cb = cb;
    }
    /**
     * @fn void set_dmi_callback(std::function<int (memory<SIZE,BUSWIDTH>&, tlm::tlm_generic_payload&, tlm::tlm_dmi&)>)
     * @brief allows to register a callback or functor being invoked upon a direct memory access (DMI) to the memory
     *
     * @param cb the callback function or functor
     */
    void set_dmi_callback(std::function<int(memory<SIZE, BUSWIDTH>&, tlm::tlm_generic_payload&, tlm::tlm_dmi&)> cb) { dmi_cb = cb; }
    /**
     * read response delay
     */
    cci::cci_param<sc_core::sc_time> rd_resp_delay{"rd_resp_delay", sc_core::SC_ZERO_TIME};
    /**
     * write response delay
     */
    cci::cci_param<sc_core::sc_time> wr_resp_delay{"wr_resp_delay", sc_core::SC_ZERO_TIME};
    /**
     * read response delay in clock cycles
     */
    cci::cci_param<unsigned> rd_resp_clk_delay{"rd_resp_clk_delay", 0};
    /**
     * write response delay in clock cycles
     */
    cci::cci_param<unsigned> wr_resp_clk_delay{"wr_resp_clk_delay", 0};
    /**
     * @fn void map_host_memory(uint64_t, uint64_t, uint8_t*)
     * @brief maps a given memory into the address range of the TLM memory
     *
     * @param base base address where the memory shall be mapped
     * @param size size of the memory range to map
     * @param ptr pointer to the mapped host memory
     */
    void map_host_memory(uint64_t base, uint64_t size, uint8_t* ptr) {
        try {
            host_mem_lut.addEntry(host_map_entry{ptr, base, size}, base, size);
        } catch(std::runtime_error& e) {
            SCCERR(SCMOD) << "Cannot map memory to address=0x" << std::hex << base << " with size=0x" << size << " because: " << e.what();
        }
    }
    /**
     * @fn void unmap_host_memory(uint64_t, uint64_t, uint8_t*)
     * @brief unmpas a host memory mapped into the address range of the TLM memory
     *
     * @param base base address where the memory shall be mapped
     * @param size size of the memory range to map
     */
    void unmap_host_memory(uint64_t base, uint64_t size) {
        if(!host_mem_lut.removeEntry(host_map_entry{nullptr, base, size})) {
            SCCERR(SCMOD) << "Cannot unmap memory at address=0x" << std::hex << base << " with size=0x" << size;
        }
    }

protected:
    //! the real memory structure
    util::sparse_array<uint8_t, SIZE> mem;
    struct host_map_entry {
        uint8_t* ptr;
        uint64_t base;
        uint64_t size;
        bool operator==(host_map_entry const& o) { return base == o.base && size == o.size; }
        bool operator!=(host_map_entry const& o) { return !operator==(o); }
    };
    util::range_lut<host_map_entry> host_mem_lut{host_map_entry{nullptr, 0, 0}};

    void set_clock_period(sc_core::sc_time period) { clk_period = period; }
    sc_core::sc_time clk_period;

public:
    //!! handle the memory operation independent on interface function used
    int handle_operation(tlm::tlm_generic_payload& trans, sc_core::sc_time& delay);
    //! handle the dmi functionality
    bool handle_dmi(tlm::tlm_generic_payload& gp, tlm::tlm_dmi& dmi_data);
    std::function<int(memory<SIZE, BUSWIDTH>&, tlm::tlm_generic_payload&, sc_core::sc_time& delay)> operation_cb;
    std::function<int(memory<SIZE, BUSWIDTH>&, tlm::tlm_generic_payload&, tlm::tlm_dmi&)> dmi_cb;
};

template <unsigned long long SIZE, unsigned BUSWIDTH = LT> using memory_tl = tickless_clock<memory<SIZE, BUSWIDTH>>;
template <unsigned long long SIZE, unsigned BUSWIDTH = LT> using memory_tc = ticking_clock<memory<SIZE, BUSWIDTH>>;

template <unsigned long long SIZE, unsigned BUSWIDTH = LT>
int handle_operation(memory<SIZE, BUSWIDTH>&, tlm::tlm_generic_payload&, sc_core::sc_time& delay);

struct host_mem_map_extension : public tlm::tlm_extension<host_mem_map_extension> {
    tlm_extension_base* clone() const override { return nullptr; }
    void copy_from(tlm_extension_base const& ext) override {}
    host_mem_map_extension(uint8_t* ptr)
    : ptr(ptr) {}
    host_mem_map_extension() = default;
    ~host_mem_map_extension() {}
    uint8_t* ptr{nullptr};
};

///////////////////////////////////////////////////////////////////////////////
/// implementation details
/// ///////////////////////////////////////////////////////////////////////////
template <unsigned long long SIZE, unsigned BUSWIDTH>
memory<SIZE, BUSWIDTH>::memory(const sc_core::sc_module_name& nm)
: sc_module(nm) {
    // Register callback for incoming b_transport interface method call
    target.register_b_transport([this](tlm::tlm_generic_payload& gp, sc_core::sc_time& delay) -> void {
        operation_cb ? operation_cb(*this, gp, delay) : handle_operation(gp, delay);
    });
    target.register_transport_dbg([this](tlm::tlm_generic_payload& gp) -> unsigned {
        sc_core::sc_time z = sc_core::SC_ZERO_TIME;
        if(auto ext = gp.get_extension<host_mem_map_extension>()) {
            if(ext->ptr)
                map_host_memory(gp.get_address(), gp.get_data_length(), ext->ptr);
            else
                unmap_host_memory(gp.get_address(), gp.get_data_length());
            if(gp.get_command() == tlm::TLM_IGNORE_COMMAND)
                return 0;
        }
        return operation_cb ? operation_cb(*this, gp, z) : handle_operation(gp, z);
    });
    target.register_get_direct_mem_ptr([this](tlm::tlm_generic_payload& gp, tlm::tlm_dmi& dmi_data) -> bool {
        return dmi_cb ? dmi_cb(*this, gp, dmi_data) : handle_dmi(gp, dmi_data);
    });
}

template <unsigned long long SIZE, unsigned BUSWIDTH>
int memory<SIZE, BUSWIDTH>::handle_operation(tlm::tlm_generic_payload& trans, sc_core::sc_time& delay) {
    uint64_t adr = trans.get_address();
    uint8_t* ptr = trans.get_data_ptr();
    unsigned len = trans.get_data_length();
    uint8_t* byt = trans.get_byte_enable_ptr();
    unsigned wid = trans.get_streaming_width();
    // check address range and check for unsupported features,
    //   i.e. byte enables, streaming, and bursts
    // Can ignore DMI hint and extensions
    if(adr + len > ::sc_dt::uint64(SIZE)) {
        SC_REPORT_ERROR("TLM-2", "generic payload transaction exceeeds memory size");
        trans.set_response_status(tlm::TLM_ADDRESS_ERROR_RESPONSE);
        return 0;
    }
    if(wid < len) {
        SCCERR(SCMOD) << "Streaming width: " << wid << ", data length: " << len;
        SC_REPORT_ERROR("TLM-2", "generic payload transaction not supported");
        trans.set_response_status(tlm::TLM_GENERIC_ERROR_RESPONSE);
        return 0;
    }
    if(byt) {
        auto res = std::accumulate(byt, byt + trans.get_byte_enable_length(), 0xff, [](uint8_t a, uint8_t b) { return a | b; });
        if(trans.get_byte_enable_length() != len || res != 0xff) {
            SC_REPORT_ERROR("TLM-2", "generic payload transaction with scattered byte enable not supported");
            trans.set_response_status(tlm::TLM_GENERIC_ERROR_RESPONSE);
            return 0;
        }
    }
    tlm::tlm_command cmd = trans.get_command();
    SCCTRACE(SCMOD) << (cmd == tlm::TLM_READ_COMMAND ? "read" : "write") << " access to addr 0x" << std::hex << adr;
    trans.set_response_status(tlm::TLM_OK_RESPONSE);
    auto hm_entry = host_mem_lut.getEntry(adr);
    if(cmd == tlm::TLM_READ_COMMAND) {
        delay += clk_period.value() ? clk_period * rd_resp_clk_delay : rd_resp_delay;
        if(hm_entry.ptr) {
            auto hm_start_offs = adr - hm_entry.base;
            auto hm_end_offs = hm_start_offs + len;
            auto hm_ptr = hm_entry.ptr + hm_start_offs;
            if(hm_end_offs < hm_entry.size) {
                std::copy(hm_ptr, hm_ptr + len, ptr);
            } else {
                auto transfer_length = hm_end_offs - hm_start_offs;
                std::copy(hm_ptr, hm_ptr + transfer_length, ptr);
                for(size_t i = transfer_length; i < len; i++)
                    ptr[i] = scc::MT19937::uniform() % 256;
            }
        } else {
            if(mem.is_allocated(adr)) {
                const auto& p = mem(adr / mem.page_size);
                auto offs = adr & mem.page_addr_mask;
                if((offs + len) > mem.page_size) {
                    auto first_part = mem.page_size - offs;
                    std::copy(p.data() + offs, p.data() + offs + first_part, ptr);
                    const auto& p2 = mem((adr / mem.page_size) + 1);
                    std::copy(p2.data(), p2.data() + len - first_part, ptr + first_part);
                } else {
                    std::copy(p.data() + offs, p.data() + offs + len, ptr);
                }
            } else {
                // no allocated page so return randomized data
                for(size_t i = 0; i < len; i++)
                    ptr[i] = scc::MT19937::uniform() % 256;
            }
        }
    } else if(cmd == tlm::TLM_WRITE_COMMAND) {
        delay += clk_period.value() ? clk_period * wr_resp_clk_delay : wr_resp_delay;
        if(hm_entry.ptr) {
            auto hm_start_offs = adr - hm_entry.base;
            auto hm_end_offs = adr + len - hm_entry.base;
            auto hm_ptr = hm_entry.ptr + hm_start_offs;
            auto transfer_length = hm_end_offs < hm_entry.size ? len : hm_end_offs - hm_start_offs;
            std::copy(ptr, ptr + transfer_length, hm_ptr);
        } else {
            auto& p = mem(adr / mem.page_size);
            auto offs = adr & mem.page_addr_mask;
            if((offs + len) > mem.page_size) {
                auto first_part = mem.page_size - offs;
                std::copy(ptr, ptr + first_part, p.data() + offs);
                auto& p2 = mem((adr / mem.page_size) + 1);
                std::copy(ptr + first_part, ptr + len, p2.data());
            } else {
                std::copy(ptr, ptr + len, p.data() + offs);
            }
        }
    }
    trans.set_dmi_allowed(allow_dmi.get_value());
    return len;
}

template <unsigned long long SIZE, unsigned BUSWIDTH>
inline bool memory<SIZE, BUSWIDTH>::handle_dmi(tlm::tlm_generic_payload& gp, tlm::tlm_dmi& dmi_data) {
    if(allow_dmi.get_value()) {
        auto hm_entry = host_mem_lut.getEntry(gp.get_address());
        if(hm_entry.ptr) {
            dmi_data.set_start_address(hm_entry.base);
            dmi_data.set_end_address(hm_entry.base + hm_entry.size - 1);
            dmi_data.set_dmi_ptr(hm_entry.ptr);
        } else {
            auto& p = mem(gp.get_address() / mem.page_size);
            auto size = mem.page_size > SIZE ? SIZE : mem.page_size;
            dmi_data.set_start_address(gp.get_address() & ~mem.page_addr_mask);
            dmi_data.set_end_address(dmi_data.get_start_address() + size - 1);
            dmi_data.set_dmi_ptr(p.data());
        }
        dmi_data.set_granted_access(tlm::tlm_dmi::DMI_ACCESS_READ_WRITE);
        dmi_data.set_read_latency(clk_period.value() ? clk_period * rd_resp_clk_delay : rd_resp_delay);
        dmi_data.set_write_latency(clk_period.value() ? clk_period * wr_resp_clk_delay : wr_resp_delay);
    }
    return allow_dmi.get_value();
}

} // namespace scc

#endif /* _SYSC_MEMORY_H_ */
