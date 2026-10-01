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

#ifndef _SYSC_MS_ROUTER_H_
#define _SYSC_MS_ROUTER_H_

#include "at_router/nb_router.h"
#include "router_types.h"
#include "scc/at_router/types.h"
#include "scc/signal_opt_ports.h"
#include <limits>
#include <memory>
#include <scc/report.h>
#include <scc/utilities.h>
#include <sstream>
#include <sysc/kernel/sc_object.h>
#include <sysc/kernel/sc_simcontext.h>
#include <sysc/kernel/sc_spawn_options.h>
#include <sysc/utils/sc_vector.h>
#include <tlm/scc/initiator_mixin.h>
#include <tlm/scc/memory_map_collector.h>
#include <tlm/scc/scv/tlm_rec_initiator_socket.h>
#include <tlm/scc/scv/tlm_rec_target_socket.h>
#include <tlm/scc/target_mixin.h>
#include <tlm>
#include <tlm_core/tlm_2/tlm_sockets/tlm_target_socket.h>
#include <unordered_map>
#include <variant>

#include <sysc/kernel/sc_ver.h>

namespace scc {
struct clocked_target_socket {
    // clang-format off
    std::variant<
        std::monostate,
        tlm::scc::target_mixin<tlm::tlm_target_socket<0>>,
        tlm::scc::target_mixin<tlm::tlm_target_socket<32>>,
        tlm::scc::target_mixin<tlm::tlm_target_socket<64>>,
        tlm::scc::target_mixin<tlm::tlm_target_socket<128>>,
        tlm::scc::target_mixin<tlm::tlm_target_socket<256>>,
        tlm::scc::target_mixin<tlm::tlm_target_socket<512>>,
        tlm::scc::target_mixin<tlm::tlm_target_socket<0, tlm::tlm_base_protocol_types, 1, sc_core::SC_ZERO_OR_MORE_BOUND>>,
        tlm::scc::target_mixin<tlm::tlm_target_socket<32, tlm::tlm_base_protocol_types, 1, sc_core::SC_ZERO_OR_MORE_BOUND>>,
        tlm::scc::target_mixin<tlm::tlm_target_socket<64, tlm::tlm_base_protocol_types, 1, sc_core::SC_ZERO_OR_MORE_BOUND>>,
        tlm::scc::target_mixin<tlm::tlm_target_socket<128, tlm::tlm_base_protocol_types, 1, sc_core::SC_ZERO_OR_MORE_BOUND>>,
        tlm::scc::target_mixin<tlm::tlm_target_socket<256, tlm::tlm_base_protocol_types, 1, sc_core::SC_ZERO_OR_MORE_BOUND>>,
        tlm::scc::target_mixin<tlm::tlm_target_socket<512, tlm::tlm_base_protocol_types, 1, sc_core::SC_ZERO_OR_MORE_BOUND>>
        > sckt;
    // clang-format on
    scc::sc_in_opt<sc_core::sc_time> clk;
};
struct clocked_initiator_socket {
    // clang-format off
    std::variant<
        std::monostate,
        tlm::scc::initiator_mixin<tlm::tlm_initiator_socket<0>>,
        tlm::scc::initiator_mixin<tlm::tlm_initiator_socket<32>>,
        tlm::scc::initiator_mixin<tlm::tlm_initiator_socket<64>>,
        tlm::scc::initiator_mixin<tlm::tlm_initiator_socket<128>>,
        tlm::scc::initiator_mixin<tlm::tlm_initiator_socket<256>>,
        tlm::scc::initiator_mixin<tlm::tlm_initiator_socket<512>>,
        tlm::scc::initiator_mixin<tlm::tlm_initiator_socket<0, tlm::tlm_base_protocol_types, 1, sc_core::SC_ZERO_OR_MORE_BOUND>>,
        tlm::scc::initiator_mixin<tlm::tlm_initiator_socket<32, tlm::tlm_base_protocol_types, 1, sc_core::SC_ZERO_OR_MORE_BOUND>>,
        tlm::scc::initiator_mixin<tlm::tlm_initiator_socket<64, tlm::tlm_base_protocol_types, 1, sc_core::SC_ZERO_OR_MORE_BOUND>>,
        tlm::scc::initiator_mixin<tlm::tlm_initiator_socket<128, tlm::tlm_base_protocol_types, 1, sc_core::SC_ZERO_OR_MORE_BOUND>>,
        tlm::scc::initiator_mixin<tlm::tlm_initiator_socket<256, tlm::tlm_base_protocol_types, 1, sc_core::SC_ZERO_OR_MORE_BOUND>>,
        tlm::scc::initiator_mixin<tlm::tlm_initiator_socket<512, tlm::tlm_base_protocol_types, 1, sc_core::SC_ZERO_OR_MORE_BOUND>>
        > sckt;
    // clang-format on
    scc::sc_in_opt<sc_core::sc_time> clk;
};
/**
 * @class ms_router
 * @brief a TLM2.0 non-uniform router for loosly-timed (LT) and approcimately-timed (AT) models
 *
 * Other than \ref scc::router, this router supports the use of tTLM sockets with different widths for target and initiator sockets
 *
 */
struct nu_router : sc_core::sc_module {
    //! the optional clock input
    scc::sc_in_opt<sc_core::sc_time> clk_i{"clk_i"};
    //! \brief the array of target sockets
    std::vector<clocked_target_socket> targets;
    //! \brief  the array of initiator sockets
    std::vector<clocked_initiator_socket> initiators;
    /**
     * @fn  ms_router(const sc_core::sc_module_name&, unsigned=1, unsigned=1)
     * @brief constructs a ms_router
     *
     * @param nm the component name
     * @param slave_cnt number of slaves to be connected
     * @param master_cnt number of masters to be connected
     * @param check_overlap_on_add_target if true this enables validation of overlaps when adding or setting the target range.
     */
    nu_router(const sc_core::sc_module_name& nm, size_t slave_cnt = 1, size_t master_cnt = 1, bool check_overlap_on_add_target = false);

    ~nu_router() = default;

    template <unsigned BUSWIDTH> tlm::tlm_target_socket<BUSWIDTH>& target_socket(size_t idx) {
        if(std::holds_alternative<std::monostate>(targets[idx].sckt)) {
            std::ostringstream os;
            os << "tsckt_" << idx;
            auto& sckt = targets[idx].sckt.template emplace<tlm::scc::target_mixin<tlm::tlm_target_socket<BUSWIDTH>>>(
                sc_core::sc_gen_unique_name(os.str().c_str(), false));
            sckt.register_b_transport(
                [this, idx](tlm::tlm_generic_payload& trans, sc_core::sc_time& delay) -> void { this->b_transport(idx, trans, delay); });
            sckt.register_get_direct_mem_ptr([this, idx](tlm::tlm_generic_payload& trans, tlm::tlm_dmi& dmi_data) -> bool {
                return this->get_direct_mem_ptr(idx, trans, dmi_data);
            });
            sckt.register_transport_dbg(
                [this, idx](tlm::tlm_generic_payload& trans) -> unsigned { return this->transport_dbg(idx, trans); });
        }
        return std::get<tlm::scc::target_mixin<tlm::tlm_target_socket<BUSWIDTH>>>(targets[idx].sckt);
    }
    template <unsigned BUSWIDTH> tlm::tlm_initiator_socket<BUSWIDTH>& initiator_socket(size_t idx) {
        if(std::holds_alternative<std::monostate>(initiators[idx].sckt)) {
            std::ostringstream os;
            os << "tsckt_" << idx;
            auto& sckt = initiators[idx].sckt.template emplace<tlm::scc::initiator_mixin<tlm::tlm_initiator_socket<BUSWIDTH>>>(
                sc_core::sc_gen_unique_name(os.str().c_str(), false));
            sckt.register_invalidate_direct_mem_ptr([this, idx](::sc_dt::uint64 start_range, ::sc_dt::uint64 end_range) -> void {
                this->invalidate_direct_mem_ptr(idx, start_range, end_range);
            });
        }
        return std::get<tlm::scc::initiator_mixin<tlm::tlm_initiator_socket<BUSWIDTH>>>(initiators[idx].sckt);
    }
    /**
     * @fn void bind_target(TYPE&, size_t, uint64_t, uint64_t, bool=true)
     * @brief bind the initiator socket of the ms_router to some target giving a base and size
     *
     * @tparam TYPE the socket type to bind
     * @param socket the target socket to bind
     * @param idx number of the target
     * @param base base address of the target
     * @param size size of the address range occupied by the target
     * @param remap if true address will be rewritten in accesses to be 0-based at the target
     */
    template <unsigned BUSWIDTH>
    void bind(tlm::tlm_target_socket<BUSWIDTH>& socket, size_t idx, uint64_t base, uint64_t size, bool remap = true) {
        set_target_range(idx, base, size, remap);
        initiator_socket<BUSWIDTH>(idx).bind(socket);
    }
    template <unsigned BUSWIDTH>
    void bind(tlm::tlm_initiator_socket<BUSWIDTH>& socket, size_t idx, uint64_t base, uint64_t size, bool remap = true) {
        set_target_range(idx, base, size, remap);
        initiator_socket<BUSWIDTH>(idx).bind(socket);
    }
    template <unsigned BUSWIDTH>
    void bind(tlm::tlm_target_socket<BUSWIDTH, tlm::tlm_base_protocol_types, 1, sc_core::SC_ZERO_OR_MORE_BOUND>& socket, size_t idx,
              uint64_t base, uint64_t size, bool remap = true) {
        set_target_range(idx, base, size, remap);
        initiator_socket<BUSWIDTH>(idx).bind(socket);
    }
    template <unsigned BUSWIDTH>
    void bind(tlm::tlm_initiator_socket<BUSWIDTH, tlm::tlm_base_protocol_types, 1, sc_core::SC_ZERO_OR_MORE_BOUND>& socket, size_t idx,
              uint64_t base, uint64_t size, bool remap = true) {
        set_target_range(idx, base, size, remap);
        initiator_socket<BUSWIDTH>(idx).bind(socket);
    }
    template <unsigned BUSWIDTH> void bind(tlm::tlm_target_socket<BUSWIDTH>& socket, size_t idx) {
        initiator_socket<BUSWIDTH>(idx).bind(socket);
    }
    template <unsigned BUSWIDTH> void bind(tlm::tlm_initiator_socket<BUSWIDTH>& socket, size_t idx) {
        initiator_socket<BUSWIDTH>(idx).bind(socket);
    }
    template <unsigned BUSWIDTH>
    void bind(tlm::tlm_target_socket<BUSWIDTH, tlm::tlm_base_protocol_types, 1, sc_core::SC_ZERO_OR_MORE_BOUND>& socket, size_t idx) {
        initiator_socket<BUSWIDTH>(idx).bind(socket);
    }
    template <unsigned BUSWIDTH>
    void bind(tlm::tlm_initiator_socket<BUSWIDTH, tlm::tlm_base_protocol_types, 1, sc_core::SC_ZERO_OR_MORE_BOUND>& socket, size_t idx) {
        initiator_socket<BUSWIDTH>(idx).bind(socket);
    }
    /**
     * @fn void set_initiator_base(size_t, uint64_t)
     * @brief define a base address of a socket
     *
     * This will be added to the address of each access coming thru this socket
     *
     * @param idx
     * @param base
     */
    void set_initiator_base(size_t idx, uint64_t base) { ibases[idx] = base; }
    /**
     * @fn void set_default_target(size_t)
     * @brief define the default target socket
     *
     * If no target address range is hit the access is routed to this socket.
     * If this is not defined a address error response is generated
     *
     * @param idx the default target
     */
    void set_default_target(size_t idx) { default_idx = idx; }
    /**
     * @fn void set_target_range(size_t, uint64_t, uint64_t, bool=true)
     * @brief establish a mapping between a socket and a target address range
     *
     * @param idx
     * @param base base address of the target
     * @param size size of the address range occupied by the target
     * @param remap if true address will be rewritten in accesses to be 0-based at the target
     */
    void set_target_range(size_t idx, uint64_t base, uint64_t size, bool remap = true);
    /**
     * @fn void set_warn_on_address_error(bool)
     * @brief enable warning message on address not found error
     *
     * @param enable if true enable warning message
     */
    void set_warn_on_address_error(bool enable) { warn_on_address_error = enable; }
    /**
     * @fn void b_transport(int, tlm::tlm_generic_payload&, sc_core::sc_time&)
     * @brief tagged blocking transport method
     *
     * @param i the tag
     * @param trans the incoming transaction
     * @param delay the annotated delay
     */
    void b_transport(int i, tlm::tlm_generic_payload& trans, sc_core::sc_time& delay);
    /**
     * @fn bool get_direct_mem_ptr(int, tlm::tlm_generic_payload&, tlm::tlm_dmi&)
     * @brief tagged forward DMI method
     *
     * @param i the tag
     * @param trans the incoming transaction
     * @param dmi_data
     * @return
     */
    bool get_direct_mem_ptr(int i, tlm::tlm_generic_payload& trans, tlm::tlm_dmi& dmi_data);
    /**
     * @fn unsigned transport_dbg(int, tlm::tlm_generic_payload&)
     * @brief tagged debug transaction method
     *
     * @param i the tag
     * @param trans the incoming transaction
     */
    unsigned transport_dbg(int i, tlm::tlm_generic_payload& trans);
    /**
     * @fn void invalidate_direct_mem_ptr(int, ::sc_dt::uint64, ::sc_dt::uint64)
     * @brief tagged backward DMI method
     *
     * @param id the tag
     * @param start_range address range start address
     * @param end_range address range end address
     */
    void invalidate_direct_mem_ptr(int id, ::sc_dt::uint64 start_range, ::sc_dt::uint64 end_range);

    /**
     * @fn void before_end_of_elaboration()
     * @brief tagged end of construction callback.
     */
    void before_end_of_elaboration() override;

    /**
     * @fn void end_of_elaboration()
     * @brief tagged end of elaboration callback.
     */
    void end_of_elaboration() override;

    util::range_lut<unsigned> const& get_address_decoder() { return addr_decoder; }

    void set_at_architecture(at_router::creator_fct<> creator) { this->creator = creator; }

protected:
    util::range_lut<unsigned> addr_decoder;
    std::vector<uint64_t> ibases;
    std::vector<range_entry> tranges;
    std::vector<sc_core::sc_mutex> mutexes;
    std::unordered_map<std::string, size_t> target_name_lut;
    size_t default_idx = std::numeric_limits<size_t>::max();
    bool check_overlap_on_add_target;
    bool warn_on_address_error{false};
    at_router::creator_fct<> creator;
    std::deque<at_router::t_port_bw_adapter<tlm::tlm_base_protocol_types>> t_port_bw_adapt;
    std::deque<at_router::i_port_fw_adapter<tlm::tlm_base_protocol_types>> i_port_bw_adapt;
    std::unique_ptr<at_router::nb_router<>> rt;
    sc_core::sc_time clk_period;
};

} // namespace scc

#endif /* _SYSC_MS_ROUTER_H_ */
