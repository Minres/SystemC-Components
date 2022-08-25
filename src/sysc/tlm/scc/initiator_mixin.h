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

#ifndef _TLM_SCC_INITIATOR_MIXIN_H__
#define _TLM_SCC_INITIATOR_MIXIN_H__

#include "scc/utilities.h"
#include <functional>
#include <sstream>
#include <tlm>

//! @brief SystemC TLM
namespace tlm {
//! @brief SCC TLM utilities
namespace scc {
/**
 * @class initiator_mixin
 * @brief initiator socket mixin
 *
 * an initiator socket mixin adding default implementation of callback functions similar to tlm::simple_initiator_socket
 *
 * @tparam BASE_TYPE
 * @tparam TYPES
 */
template <typename BASE_TYPE, typename TYPES = tlm::tlm_base_protocol_types> class initiator_mixin : public BASE_TYPE {
public:
    using transaction_type = typename TYPES::tlm_payload_type;
    using phase_type = typename TYPES::tlm_phase_type;
    using sync_enum_type = tlm::tlm_sync_enum;
    using fw_interface_type = tlm::tlm_fw_transport_if<TYPES>;
    using bw_interface_type = tlm::tlm_bw_transport_if<TYPES>;

public:
    /**
     * the default constructor automatically generating a name
     */
    initiator_mixin()
    : initiator_mixin(sc_core::sc_gen_unique_name("initiator_mixin_socket")) {}
    /**
     * constructor with explicit instance name
     *
     * @param name the instance name
     */
    explicit initiator_mixin(const sc_core::sc_module_name& name)
    : BASE_TYPE(name)
    , bw_if(this->name()) {
        this->m_export.bind(bw_if);
    }
    /**
     * register a non-blocking backward path callback function
     *
     * @param cb the callback function
     */
    void register_nb_transport_bw(std::function<sync_enum_type(transaction_type&, phase_type&, sc_core::sc_time&)> cb) {
        bw_if.set_transport_function(cb);
    }
    /**
     * register an invalidate DMI callback function
     *
     * @param cb the callback function
     */
    void register_invalidate_direct_mem_ptr(std::function<void(sc_dt::uint64, sc_dt::uint64)> cb) {
        bw_if.set_invalidate_direct_mem_function(cb);
    }

private:
    class bw_transport_if : public tlm::tlm_bw_transport_if<TYPES> {
    public:
        using transport_fct = std::function<sync_enum_type(transaction_type&, phase_type&, sc_core::sc_time&)>;
        using invalidate_dmi_fct = std::function<void(sc_dt::uint64, sc_dt::uint64)>;

        bw_transport_if(const std::string& name)
        : m_name(name)
        , m_transport_ptr(0)
        , m_invalidate_direct_mem_ptr(0) {}

        void set_transport_function(transport_fct p) {
            if(m_transport_ptr) {
                std::stringstream s;
                s << m_name << ": non-blocking callback allready registered";
                SC_REPORT_WARNING("/OSCI_TLM-2/simple_socket", s.str().c_str());
            } else {
                m_transport_ptr = p;
            }
        }

        void set_invalidate_direct_mem_function(invalidate_dmi_fct p) {
            if(m_invalidate_direct_mem_ptr) {
                std::stringstream s;
                s << m_name << ": invalidate DMI callback allready registered";
                SC_REPORT_WARNING("/OSCI_TLM-2/simple_socket", s.str().c_str());
            } else {
                m_invalidate_direct_mem_ptr = p;
            }
        }

        sync_enum_type nb_transport_bw(transaction_type& trans, phase_type& phase, sc_core::sc_time& t) {
            if(m_transport_ptr)
                return m_transport_ptr(trans, phase, t);
            std::stringstream s;
            s << m_name << ": no transport callback registered";
            SC_REPORT_ERROR("/OSCI_TLM-2/initiator_mixin", s.str().c_str());
            return tlm::TLM_ACCEPTED; ///< unreachable code
        }

        void invalidate_direct_mem_ptr(sc_dt::uint64 start_range, sc_dt::uint64 end_range) {
            if(m_invalidate_direct_mem_ptr) // forward call
                m_invalidate_direct_mem_ptr(start_range, end_range);
        }

    private:
        const std::string m_name;
        transport_fct m_transport_ptr;
        invalidate_dmi_fct m_invalidate_direct_mem_ptr;
    };

private:
    bw_transport_if bw_if;
};
} // namespace scc
} // namespace tlm

#endif //_TLM_SCC_INITIATOR_MIXIN_H__
