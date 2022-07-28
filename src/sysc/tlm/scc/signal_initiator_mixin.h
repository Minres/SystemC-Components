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

#ifndef __SIGNAL_INITIATOR_MIXIN_H__
#define __SIGNAL_INITIATOR_MIXIN_H__

#include "scc/utilities.h"
#include <functional>
#include <sstream>
#include <tlm/scc/tlm_signal.h>

//! @brief SystemC TLM
namespace tlm {
//! @brief SCC TLM utilities
namespace scc {

template <typename BASE_TYPE> class signal_initiator_mixin : public BASE_TYPE {
public:
    using tlm_signal_type = typename BASE_TYPE::tlm_signal_type;
    using transaction_type = typename BASE_TYPE::transaction_type;
    using phase_type = typename BASE_TYPE::phase_type;
    using sync_enum_type = tlm::tlm_sync_enum;
    using fw_interface_type = tlm_signal_fw_transport_if<tlm_signal_type, typename BASE_TYPE::protocol_types>;
    using bw_interface_type = tlm_signal_bw_transport_if<tlm_signal_type, typename BASE_TYPE::protocol_types>;

public:
    signal_initiator_mixin()
    : signal_initiator_mixin(sc_core::sc_gen_unique_name("signal_initiator_mixinn_socket")) {}

    explicit signal_initiator_mixin(const char* n)
    : BASE_TYPE(n)
    , error_if_no_callback(false)
    , bw_if(this) {
        bind(bw_if);
    }

    using BASE_TYPE::bind;

    void write_now(tlm_signal_type value) {
        auto* gp = tlm_signal_gp<tlm_signal_type>::create();
        gp->set_command(tlm::TLM_WRITE_COMMAND);
        gp->set_value(value);
        gp->acquire();
        tlm::tlm_phase phase{tlm::BEGIN_REQ};
        sc_core::sc_time delay{sc_core::SC_ZERO_TIME};
        (*this)->nb_transport_fw(*gp, phase, delay);
        gp->release();
    }

    template <typename EXT_TYPE> void write_now(tlm_signal_type value, EXT_TYPE* ext) {
        auto* gp = tlm_signal_gp<tlm_signal_type>::create();
        gp->set_command(tlm::TLM_WRITE_COMMAND);
        gp->set_value(value);
        if(ext)
            gp->set_extension(ext);
        gp->acquire();
        tlm::tlm_phase phase{tlm::BEGIN_REQ};
        sc_core::sc_time delay{sc_core::SC_ZERO_TIME};
        (*this)->nb_transport_fw(*gp, phase, delay);
        gp->release();
    }

    /**
     *
     * @param cb the callback function
     */
    void register_nb_transport(std::function<sync_enum_type(transaction_type&, phase_type&, sc_core::sc_time&)> cb) {
        bw_if.set_nb_transport_ptr(cb);
    }
    /**
     * @fn void register_nb_transport(std::function<sync_enum_type(unsigned int, transaction_type&, phase_type&,
     * sc_core::sc_time&)>, unsigned int)
     * @brief register a functor for nb_transport_bw call
     *
     * @param cb  the callback function
     * @param tag the tag to be used in the callback
     */
    void register_nb_transport(
        std::function<sync_enum_type(unsigned int, transaction_type&, phase_type&, sc_core::sc_time&)> cb,
        unsigned int tag) {
        bw_if.set_nb_transport_ptr(cb);
    }

    bool error_if_no_callback;

private:
    class bw_transport_if : public bw_interface_type {
    public:
        using transport_fct = std::function<sync_enum_type(transaction_type&, phase_type&, sc_core::sc_time&)>;
        using transport_tagged_fct =
            std::function<sync_enum_type(unsigned int, transaction_type&, phase_type&, sc_core::sc_time&)>;

        bw_transport_if(const signal_initiator_mixin* owner)
        : m_owner(owner) {}

        void set_nb_transport_ptr(transport_fct p) {
            if(m_transport_ptr || m_transport_tagged_ptr) {
                std::stringstream s;
                s << m_owner->name() << ": non-blocking callback allready registered";
                SC_REPORT_WARNING("/OSCI_TLM-2/signal_initiator_mixin", s.str().c_str());
            } else {
                m_transport_ptr = p;
            }
        }

        void set_nb_transport_ptr(transport_fct p, unsigned int tag) {
            if(m_transport_ptr || m_transport_tagged_ptr) {
                std::stringstream s;
                s << m_owner->name() << ": non-blocking callback allready registered";
                SC_REPORT_WARNING("/OSCI_TLM-2/signal_initiator_mixin", s.str().c_str());
            } else {
                m_transport_tagged_ptr = p;
                this->tag = tag;
            }
        }

        sync_enum_type nb_transport_bw(transaction_type& trans, phase_type& phase, sc_core::sc_time& t) {
            if(m_transport_ptr)
                return m_transport_ptr(trans, phase, t);
            else if(m_transport_tagged_ptr)
                return m_transport_tagged_ptr(tag, trans, phase, t);
            else if(m_owner->error_if_no_callback) {
                std::stringstream s;
                s << m_owner->name() << ": no transport callback registered";
                SC_REPORT_ERROR("/OSCI_TLM-2/signal_initiator_mixin", s.str().c_str());
            }
            return tlm::TLM_COMPLETED;
        }

    private:
        const signal_initiator_mixin* m_owner;
        unsigned int tag = 0;
        transport_fct m_transport_ptr = nullptr;
        transport_tagged_fct m_transport_tagged_ptr = nullptr;
    };

private:
    bw_transport_if bw_if;
};
} // namespace scc
} // namespace tlm

#include <sysc/datatypes/bit/sc_logic.h>
namespace tlm {
namespace scc {
using tlm_signal_bool_out = signal_initiator_mixin<tlm_signal_initiator_socket<bool>>;
using tlm_signal_logic_out = signal_initiator_mixin<tlm_signal_initiator_socket<sc_dt::sc_logic>>;
using tlm_signal_bool_opt_out = signal_initiator_mixin<tlm_signal_opt_initiator_socket<bool>>;
using tlm_signal_logic_opt_out = signal_initiator_mixin<tlm_signal_opt_initiator_socket<sc_dt::sc_logic>>;
} // namespace scc
} // namespace tlm
#endif //__SIGNAL_INITIATOR_MIXIN_H__
