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
#ifndef __SIGNAL_TARGET_MIXIN_H__
#define __SIGNAL_TARGET_MIXIN_H__

#include "scc/utilities.h"
#include <functional>
#include <sstream>
#include <tlm/scc/tlm_signal.h>

//! @brief SystemC TLM
namespace tlm {
//! @brief SCC TLM utilities
namespace scc {

template <typename BASE_TYPE> class signal_target_mixin : public BASE_TYPE {
public:
    using tlm_signal_type = typename BASE_TYPE::tlm_signal_type;
    using transaction_type = typename BASE_TYPE::transaction_type;
    using phase_type = typename BASE_TYPE::phase_type;
    using sync_enum_type = tlm::tlm_sync_enum;
    using fw_interface_type = tlm_signal_fw_transport_if<tlm_signal_type, typename BASE_TYPE::protocol_types>;
    using bw_interface_type = tlm_signal_bw_transport_if<tlm_signal_type, typename BASE_TYPE::protocol_types>;

public:
    /**
     *
     */
    signal_target_mixin()
    : signal_target_mixin(sc_core::sc_gen_unique_name("signal_target_mixin_socket")) {}
    /**
     *
     * @param n
     */
    explicit signal_target_mixin(const char* n)
    : BASE_TYPE(n)
    , error_if_no_callback(true)
    , fw_if(this) {
        bind(fw_if);
    }

    using BASE_TYPE::bind;

    /**
     *
     * @param cb
     */
    void register_nb_transport(std::function<sync_enum_type(transaction_type&, phase_type&, sc_core::sc_time&)> cb) {
        fw_if.set_nb_transport_ptr(cb);
    }
    /**
     * @fn void register_nb_transport(std::function<sync_enum_type(unsigned int, transaction_type&, phase_type&,
     * sc_core::sc_time&)>, unsigned int)
     * @brief register a functor for nb_transport_fw call
     *
     * @param cb the callback functor
     * @param tag the tag to be used with the functor
     */
    void register_nb_transport(
        std::function<sync_enum_type(unsigned int, transaction_type&, phase_type&, sc_core::sc_time&)> cb,
        unsigned int tag) {
        fw_if.set_nb_transport_ptr(cb, tag);
    }

    bool error_if_no_callback;

private:
    // make call on bw path.
    sync_enum_type bw_nb_transport(transaction_type& trans, phase_type& phase, sc_core::sc_time& t) {
        return BASE_TYPE::operator->()->nb_transport_bw(trans, phase, t);
    }

    class fw_process_if : public fw_interface_type {
    public:
        using transport_fct = std::function<sync_enum_type(transaction_type&, phase_type&, sc_core::sc_time&)>;
        using transport_tagged_fct =
            std::function<sync_enum_type(unsigned int, transaction_type&, phase_type&, sc_core::sc_time&)>;

        fw_process_if(const signal_target_mixin* p_own)
        : m_owner(p_own) {}

        void set_nb_transport_ptr(transport_fct p) {
            if(m_transport_ptr || m_transport_tagged_ptr) {
                std::stringstream s;
                s << m_owner->name() << ": non-blocking callback allready registered";
                SC_REPORT_WARNING("/OSCI_TLM-2/signal_target_mixin", s.str().c_str());
            } else {
                m_transport_ptr = p;
            }
        }

        void set_nb_transport_ptr(transport_tagged_fct p, unsigned int tag) {
            if(m_transport_ptr || m_transport_tagged_ptr) {
                std::stringstream s;
                s << m_owner->name() << ": non-blocking callback allready registered";
                SC_REPORT_WARNING("/OSCI_TLM-2/signal_target_mixin", s.str().c_str());
            } else {
                m_transport_tagged_ptr = p;
                this->tag = tag;
            }
        }

        // Interface implementation
        sync_enum_type nb_transport_fw(transaction_type& trans, phase_type& phase, sc_core::sc_time& t) {
            if(m_transport_ptr) {
                // forward call
                return m_transport_ptr(trans, phase, t);
            } else if(m_transport_tagged_ptr) {
                // forward call
                return m_transport_tagged_ptr(tag, trans, phase, t);
            } else if(m_owner->error_if_no_callback) {
                std::stringstream s;
                s << m_owner->name() << ": no transport callback registered";
                SC_REPORT_ERROR("/OSCI_TLM-2/signal_target_mixin", s.str().c_str());
            }
            return tlm::TLM_COMPLETED; ///< unreachable code
        }

    private:
        const signal_target_mixin* m_owner;
        unsigned int tag = 0;
        transport_fct m_transport_ptr = nullptr;
        transport_tagged_fct m_transport_tagged_ptr = nullptr;
    };

private:
    fw_process_if fw_if;
};
} // namespace scc
} // namespace tlm

#include <sysc/datatypes/bit/sc_logic.h>
namespace tlm {
namespace scc {
using tlm_signal_bool_in = signal_target_mixin<tlm_signal_target_socket<bool>>;
using tlm_signal_logic_in = signal_target_mixin<tlm_signal_target_socket<sc_dt::sc_logic>>;
using tlm_signal_bool_opt_in = signal_target_mixin<tlm_signal_opt_target_socket<bool>>;
using tlm_signal_logic_opt_in = signal_target_mixin<tlm_signal_opt_target_socket<sc_dt::sc_logic>>;
} // namespace scc
} // namespace tlm

#endif //__SIGNAL_TARGET_MIXIN_H__
