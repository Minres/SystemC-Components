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

#ifndef _SCC_TAGGED_INITIATOR_MIXIN_H__
#define _SCC_TAGGED_INITIATOR_MIXIN_H__

#include <functional>
#include <scc/utilities.h>
#include <sstream>
#include <tlm>

//! @brief SystemC TLM
namespace tlm {
//! @brief SCC TLM utilities
namespace scc {
/**
 *
 */
template <typename BASE_TYPE, typename TYPES = tlm::tlm_base_protocol_types>
class tagged_initiator_mixin : public BASE_TYPE {
public:
    using transaction_type = typename TYPES::tlm_payload_type;
    using phase_type = typename TYPES::tlm_phase_type;
    using sync_enum_type = tlm::tlm_sync_enum;
    using fw_interface_type = tlm::tlm_fw_transport_if<TYPES>;
    using bw_interface_type = tlm::tlm_bw_transport_if<TYPES>;

public:
    /**
     *
     */
    tagged_initiator_mixin()
    : BASE_TYPE(sc_core::sc_gen_unique_name("tagged_initiator_socket"))
    , bw_if(this->name()) {
        this->m_export.bind(bw_if);
    }
    /**
     *
     * @param n
     */
    explicit tagged_initiator_mixin(const char* n)
    : BASE_TYPE(n)
    , bw_if(this->name()) {
        this->m_export.bind(bw_if);
    }
    /**
     *
     * @param cb the callback function
     * @param tag the tag to return upon calling
     */
    void register_nb_transport_bw(
        std::function<sync_enum_type(unsigned int, transaction_type&, phase_type&, sc_core::sc_time&)> cb,
        unsigned int tag) {
        bw_if.set_transport_function(cb, tag);
    }
    /**
     *
     * @param cb the callback function
     * @param tag the tag to return upon calling
     */
    void register_invalidate_direct_mem_ptr(std::function<void(unsigned int, sc_dt::uint64, sc_dt::uint64)> cb,
                                            unsigned int tag) {
        bw_if.set_invalidate_direct_mem_function(cb, tag);
    }

private:
    class bw_transport_if : public tlm::tlm_bw_transport_if<TYPES> {
    public:
        using transport_fct =
            std::function<sync_enum_type(unsigned int, transaction_type&, phase_type&, sc_core::sc_time&)>;
        using invalidate_dmi_fct = std::function<void(unsigned int, sc_dt::uint64, sc_dt::uint64)>;

        bw_transport_if(const std::string& name)
        : m_name(name)
        , m_transport_ptr(0)
        , m_invalidate_direct_mem_ptr(0) {}

        void set_transport_function(transport_fct p, unsigned int tag) {
            if(m_transport_ptr) {
                std::stringstream s;
                s << m_name << ": non-blocking callback allready registered";
                SC_REPORT_WARNING("/OSCI_TLM-2/simple_socket", s.str().c_str());
            } else {
                m_transport_ptr = p;
                tags[0] = tag;
            }
        }

        void set_invalidate_direct_mem_function(invalidate_dmi_fct p, unsigned int tag) {
            if(m_invalidate_direct_mem_ptr) {
                std::stringstream s;
                s << m_name << ": invalidate DMI callback allready registered";
                SC_REPORT_WARNING("/OSCI_TLM-2/simple_socket", s.str().c_str());
            } else {
                m_invalidate_direct_mem_ptr = p;
                tags[1] = tag;
            }
        }

        sync_enum_type nb_transport_bw(transaction_type& trans, phase_type& phase, sc_core::sc_time& t) {
            if(m_transport_ptr)
                return m_transport_ptr(tags[0], trans, phase, t);
            std::stringstream s;
            s << m_name << ": no transport callback registered";
            SC_REPORT_ERROR("/OSCI_TLM-2/tagged_initiator_mixin", s.str().c_str());
            return tlm::TLM_ACCEPTED; ///< unreachable code
        }

        void invalidate_direct_mem_ptr(sc_dt::uint64 start_range, sc_dt::uint64 end_range) {
            if(m_invalidate_direct_mem_ptr) // forward call
                m_invalidate_direct_mem_ptr(tags[1], start_range, end_range);
        }

    private:
        const std::string m_name;
        unsigned int tags[2]; // dbg, dmi
        transport_fct m_transport_ptr;
        invalidate_dmi_fct m_invalidate_direct_mem_ptr;
    };

private:
    bw_transport_if bw_if;
};
} // namespace scc
} // namespace tlm

#endif //_SCC_TAGGED_INITIATOR_MIXIN_H__
