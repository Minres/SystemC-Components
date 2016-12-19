#ifndef __INITIATOR_MIXIN_H__
#define __INITIATOR_MIXIN_H__

#include <tlm>
#include <sstream>

namespace scv4tlm {

template<typename BASE_TYPE, typename TYPES = tlm::tlm_base_protocol_types>
class initiator_mixin: public BASE_TYPE {
public:
    typedef typename TYPES::tlm_payload_type transaction_type;
    typedef typename TYPES::tlm_phase_type phase_type;
    typedef tlm::tlm_sync_enum sync_enum_type;
    typedef tlm::tlm_fw_transport_if<TYPES> fw_interface_type;
    typedef tlm::tlm_bw_transport_if<TYPES> bw_interface_type;

public:
    initiator_mixin()
            : BASE_TYPE(sc_core::sc_gen_unique_name("initiator_mixin_socket")), bw_if(this->name()) {
        this->m_export.bind(bw_if);
    }

    explicit initiator_mixin(const char* n)
            : BASE_TYPE(n), bw_if(this->name()) {
        this->m_export.bind(bw_if);
    }

    void register_nb_transport_bw(std::function<sync_enum_type(transaction_type&, phase_type&, sc_core::sc_time&)> cb) {
        bw_if.set_transport_function(cb);
    }

    void register_invalidate_direct_mem_ptr(std::function<void(sc_dt::uint64, sc_dt::uint64)> cb) {
        bw_if.set_invalidate_direct_mem_function(cb);
    }

private:
    class bw_transport_if: public tlm::tlm_bw_transport_if<TYPES> {
    public:
        typedef std::function<sync_enum_type(transaction_type&, phase_type&, sc_core::sc_time&)> transport_fct;
        typedef std::function<void (sc_dt::uint64, sc_dt::uint64)> invalidate_dmi_fct;

        bw_transport_if(const std::string& name)
                : m_name(name), m_transport_ptr(0), m_invalidate_direct_mem_ptr(0) {
        }

        void set_transport_function(transport_fct p) {
            if (m_transport_ptr) {
                std::stringstream s;
                s << m_name << ": non-blocking callback allready registered";
                SC_REPORT_WARNING("/OSCI_TLM-2/simple_socket", s.str().c_str());
            } else {
                m_transport_ptr = p;
            }
        }

        void set_invalidate_direct_mem_function(invalidate_dmi_fct p) {
            if (m_invalidate_direct_mem_ptr) {
                std::stringstream s;
                s << m_name << ": invalidate DMI callback allready registered";
                SC_REPORT_WARNING("/OSCI_TLM-2/simple_socket", s.str().c_str());
            } else {
                m_invalidate_direct_mem_ptr = p;
            }
        }

        sync_enum_type nb_transport_bw(transaction_type& trans, phase_type& phase, sc_core::sc_time& t) {
            if (m_transport_ptr)
                return m_transport_ptr(trans, phase, t);
            std::stringstream s;
            s << m_name << ": no transport callback registered";
            SC_REPORT_ERROR("/OSCI_TLM-2/initiator_mixin", s.str().c_str());
            return tlm::TLM_ACCEPTED;   ///< unreachable code
        }

        void invalidate_direct_mem_ptr(sc_dt::uint64 start_range, sc_dt::uint64 end_range) {
            if (m_invalidate_direct_mem_ptr) // forward call
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

}

#endif
