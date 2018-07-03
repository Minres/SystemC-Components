/*******************************************************************************
 * Copyright 2016 MINRES Technologies GmbH
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

#ifndef SC_INCLUDE_DYNAMIC_PROCESSES // needed for sc_spawn
#define SC_INCLUDE_DYNAMIC_PROCESSES
#endif

#include <sstream>
#include "scc/utilities.h"
#include <tlm/tlm_signal.h>
#include <tlm_utils/peq_with_get.h>

namespace scc {

template <typename base_type, typename SIG=bool, typename TYPES = tlm::tlm_signal_baseprotocol_types<SIG>>
class signal_target_mixin : public base_type {
    friend class fw_process;
    friend class bw_process;

public:
    using transaction_type = typename TYPES::tlm_payload_type;
    using phase_type = typename TYPES::tlm_phase_type;
    using sync_enum_type = tlm::tlm_sync_enum;
    using fw_interface_type = tlm::tlm_signal_fw_transport_if<TYPES>;
    using bw_interface_type = tlm::tlm_signal_bw_transport_if<TYPES>;

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
    explicit signal_target_mixin(const char *n)
    : base_type(n)
    , m_fw_process(this)
    , m_bw_process(this)
    , m_current_transaction(nullptr) {
        bind(m_fw_process);
    }

    using base_type::bind;

    // bw transport must come thru us.
    /**
     *
     * @return
     */
    tlm::tlm_signal_bw_transport_if<SIG> *operator->() { return &m_bw_process; }
    // REGISTER_XXX
    /**
     *
     * @param cb
     */
    void
    register_nb_transport_fw(std::function<sync_enum_type(transaction_type &, phase_type &, sc_core::sc_time &)> cb) {
        assert(!sc_core::sc_get_curr_simcontext()->elaboration_done());
        m_fw_process.set_nb_transport_ptr(cb);
    }
    /**
     *
     * @param cb
     */
    void
    register_nb_transport_fw(std::function<sync_enum_type(transaction_type &, phase_type &, sc_core::sc_time &)> cb, unsigned int tag) {
        assert(!sc_core::sc_get_curr_simcontext()->elaboration_done());
        m_fw_process.set_nb_transport_ptr(cb, tag);
    }
    /**
     *
     * @param cb
     */
    void register_b_transport(std::function<void(transaction_type &, sc_core::sc_time &)> cb) {
        assert(!sc_core::sc_get_curr_simcontext()->elaboration_done());
        m_fw_process.set_b_transport_ptr(cb);
    }
    /**
     *
     * @param cb
     */
    void register_b_transport(std::function<void(transaction_type &, sc_core::sc_time &)> cb, unsigned int tag) {
        assert(!sc_core::sc_get_curr_simcontext()->elaboration_done());
        m_fw_process.set_b_transport_ptr(cb, tag);
    }

private:
    // make call on bw path.
    sync_enum_type bw_nb_transport(transaction_type &trans, phase_type &phase, sc_core::sc_time &t) {
        return base_type::operator->()->nb_transport_bw(trans, phase, t);
    }

    // Helper class to handle bw path calls
    // Needed to detect transaction end when called from b_transport.
    class bw_process : public tlm::tlm_signal_bw_transport_if<SIG> {
    public:
        bw_process(signal_target_mixin *p_own)
        : m_owner(p_own) {}

        sync_enum_type nb_transport_bw(transaction_type &trans, phase_type &phase, sc_core::sc_time &t) {
            typename std::map<transaction_type *, sc_core::sc_event *>::iterator it;

            it = m_owner->m_pending_trans.find(&trans);
            if (it == m_owner->m_pending_trans.end()) {
                // Not a blocking call, forward.
                return m_owner->bw_nb_transport(trans, phase, t);
            } else {
                if (phase == tlm::END_REQ) {
                    m_owner->m_end_request.notify(sc_core::SC_ZERO_TIME);
                    return tlm::TLM_ACCEPTED;
                } else {
                    assert(false && "Illegal phase encountered");
                    exit(1);
                }
            }
        }
    private:
        signal_target_mixin *m_owner;
    };

    class fw_process : public tlm::tlm_signal_fw_transport_if<SIG>, public tlm::tlm_mm_interface {
    public:
        using NBTransportPtr = std::function<sync_enum_type(transaction_type &, phase_type &, sc_core::sc_time &)>;
        using BTransportPtr = std::function<void(transaction_type &, sc_core::sc_time &)>;
        using NBTransportTaggedPtr = std::function<sync_enum_type(unsigned int, transaction_type &, phase_type &, sc_core::sc_time &)>;
        using BTransportTaggedPtr = std::function<void(unsigned int,transaction_type &, sc_core::sc_time &)>;

        fw_process(signal_target_mixin *p_own)
        : m_name(p_own->name())
        , m_owner(p_own)
        , m_nb_transport_ptr(0)
        , m_b_transport_ptr(0)
        , m_peq(sc_core::sc_gen_unique_name("m_peq"))
        , m_response_in_progress(false) {
            sc_core::sc_spawn_options opts;
            opts.set_sensitivity(&m_peq.get_event());
            sc_core::sc_spawn(sc_bind(&fw_process::b2nb_thread, this), sc_core::sc_gen_unique_name("b2nb_thread"),
                              &opts);
        }

        void set_nb_transport_ptr(NBTransportPtr p) {
            if (m_nb_transport_ptr || m_nb_transport_tagged_ptr) {
                std::stringstream s;
                s << m_name << ": non-blocking callback allready registered";
                SC_REPORT_WARNING("/OSCI_TLM-2/simple_socket", s.str().c_str());
            } else {
                m_nb_transport_ptr = p;
            }
        }

        void set_nb_transport_ptr(NBTransportPtr p, unsigned int tag) {
            if (m_nb_transport_ptr || m_nb_transport_tagged_ptr) {
                std::stringstream s;
                s << m_name << ": non-blocking callback allready registered";
                SC_REPORT_WARNING("/OSCI_TLM-2/simple_socket", s.str().c_str());
            } else {
                m_nb_transport_tagged_ptr = p;
                tags[1]=tag;
            }
        }

        void set_b_transport_ptr(BTransportPtr p) {
            if (m_b_transport_ptr || m_b_transport_tagged_ptr) {
                std::stringstream s;
                s << m_name << ": blocking callback allready registered";
                SC_REPORT_WARNING("/OSCI_TLM-2/simple_socket", s.str().c_str());
            } else {
                m_b_transport_ptr = p;
            }
        }

        void set_b_transport_ptr(BTransportPtr p, unsigned int tag) {
            if (m_b_transport_ptr || m_b_transport_tagged_ptr) {
                std::stringstream s;
                s << m_name << ": blocking callback allready registered";
                SC_REPORT_WARNING("/OSCI_TLM-2/simple_socket", s.str().c_str());
            } else {
                m_b_transport_tagged_ptr = p;
                tags[1]=tag;
            }
        }

        // Interface implementation
        sync_enum_type nb_transport_fw(transaction_type &trans, phase_type &phase, sc_core::sc_time &t) {
            if (m_nb_transport_ptr) {
                // forward call
                return m_nb_transport_ptr(trans, phase, t);
            } else if (m_nb_transport_tagged_ptr) {
                // forward call
                return m_nb_transport_ptr(tags[1], trans, phase, t);
            } else if (m_b_transport_ptr) {
                if (phase == tlm::BEGIN_REQ) {
                    // prepare thread to do blocking call
                    process_handle_class *ph = m_process_handle.get_handle(&trans);

                    if (!ph) { // create new dynamic process
                        ph = new process_handle_class(&trans);
                        m_process_handle.put_handle(ph);

                        sc_core::sc_spawn_options opts;
                        opts.dont_initialize();
                        opts.set_sensitivity(&ph->m_e);
                        sc_core::sc_spawn(sc_bind(&fw_process::nb2b_thread, this, ph),
                                          sc_core::sc_gen_unique_name("nb2b_thread"), &opts);
                    }

                    ph->m_e.notify(t);
                    return tlm::TLM_ACCEPTED;
                } else if (phase == tlm::END_RESP) {
                    m_response_in_progress = false;
                    m_end_response.notify(t);
                    return tlm::TLM_COMPLETED;
                } else {
                    assert(false);
                    exit(1);
                    //          return tlm::TLM_COMPLETED;   ///< unreachable code
                }

            } else {
                std::stringstream s;
                s << m_name << ": no non-blocking transport callback registered";
                SC_REPORT_ERROR("/OSCI_TLM-2/simple_socket", s.str().c_str());
            }
            return tlm::TLM_ACCEPTED; ///< unreachable code
        }

        void b_transport(transaction_type &trans, sc_core::sc_time &t) {
            if (m_b_transport_ptr) {
                // forward call
                m_b_transport_ptr(trans, t);
                return;
            } else if (m_b_transport_tagged_ptr) {
                // forward call
                m_b_transport_tagged_ptr(tags[0], trans, t);
                return;
            } else if (m_nb_transport_ptr || m_nb_transport_tagged_ptr) {
                m_peq.notify(trans, t);
                t = sc_core::SC_ZERO_TIME;

                mm_end_event_ext mm_ext;
                const bool mm_added = !trans.has_mm();

                if (mm_added) {
                    trans.set_mm(this);
                    trans.set_auto_extension(&mm_ext);
                    trans.acquire();
                }
                // wait until transaction is finished
                sc_core::sc_event end_event;
                m_owner->m_pending_trans[&trans] = &end_event;
                sc_core::wait(end_event);
                if (mm_added) {
                    // release will not delete the transaction, it will notify mm_ext.done
                    trans.release();
                    if (trans.get_ref_count()) {
                        sc_core::wait(mm_ext.done);
                    }
                    trans.set_mm(0);
                }
            } else {
                std::stringstream s;
                s << m_name << ": no blocking transport callback registered";
                SC_REPORT_ERROR("/OSCI_TLM-2/simple_socket", s.str().c_str());
            }
        }

    private:
        // dynamic process handler for nb2b conversion

        class process_handle_class {
        public:
            explicit process_handle_class(transaction_type *trans)
            : m_trans(trans)
            , m_suspend(false) {}

            transaction_type *m_trans;
            sc_core::sc_event m_e;
            bool m_suspend;
        };

        class process_handle_list {
        public:
            process_handle_list() = default;

            ~process_handle_list() {
                for (typename std::vector<process_handle_class *>::iterator it = v.begin(), end = v.end(); it != end;
                     ++it)
                    delete *it;
            }

            process_handle_class *get_handle(transaction_type *trans) {
                typename std::vector<process_handle_class *>::iterator it;

                for (it = v.begin(); it != v.end(); it++) {
                    if ((*it)->m_suspend) {     // found suspended dynamic process, re-use it
                        (*it)->m_trans = trans; // replace to new one
                        (*it)->m_suspend = false;
                        return *it;
                    }
                }
                return NULL; // no suspended process
            }

            void put_handle(process_handle_class *ph) { v.push_back(ph); }

        private:
            std::vector<process_handle_class *> v;
        };

        process_handle_list m_process_handle;

        void nb2b_thread(process_handle_class *h) {

            while (true) {
                transaction_type *trans = h->m_trans;
                sc_core::sc_time t = sc_core::SC_ZERO_TIME;

                // forward call
                m_b_transport_ptr?m_b_transport_ptr(*trans, t):m_b_transport_tagged_ptr(tags[0], *trans, t);

                sc_core::wait(t);

                // return path
                while (m_response_in_progress) {
                    sc_core::wait(m_end_response);
                }
                t = sc_core::SC_ZERO_TIME;
                phase_type phase = tlm::BEGIN_RESP;
                sync_enum_type sync = m_owner->bw_nb_transport(*trans, phase, t);
                if (!(sync == tlm::TLM_COMPLETED || (sync == tlm::TLM_UPDATED && phase == tlm::END_RESP))) {
                    m_response_in_progress = true;
                }

                // suspend until next transaction
                h->m_suspend = true;
                sc_core::wait();
            }
        }

        void b2nb_thread() {
            while (true) {
                sc_core::wait(m_peq.get_event());

                transaction_type *trans;
                while ((trans = m_peq.get_next_transaction()) != 0) {
                    assert(m_nb_transport_ptr || m_nb_transport_tagged_ptr);
                    phase_type phase = tlm::BEGIN_REQ;
                    sc_core::sc_time t = sc_core::SC_ZERO_TIME;
                    auto sync = m_nb_transport_ptr?m_nb_transport_ptr(*trans, phase, t):m_nb_transport_tagged_ptr(tags[1], *trans, phase, t);
                    switch (sync) {
                    case tlm::TLM_COMPLETED: {
                        // notify transaction is finished
                        typename std::map<transaction_type *, sc_core::sc_event *>::iterator it =
                            m_owner->m_pending_trans.find(trans);
                        assert(it != m_owner->m_pending_trans.end());
                        it->second->notify(t);
                        m_owner->m_pending_trans.erase(it);
                        break;
                    }

                    case tlm::TLM_ACCEPTED:
                        switch (phase) {
                        case tlm::BEGIN_REQ:
                            m_owner->m_current_transaction = trans;
                            sc_core::wait(m_owner->m_end_request);
                            m_owner->m_current_transaction = 0;
                            break;
                        default:
                            assert(false && "Illegal phase encountered");
                         }
                        break;

                    default:
                        assert(false && "Illegal sync enum encountered");
                        exit(1);
                    };
                }
            }
        }

        void free(tlm::tlm_generic_payload *trans) {
            mm_end_event_ext *ext = trans->template get_extension<mm_end_event_ext>();
            assert(ext);
            // notif event first before freeing extensions (reset)
            ext->done.notify();
            trans->reset();
        }

    private:
        class mm_end_event_ext : public tlm::tlm_extension<mm_end_event_ext> {
        public:
            tlm::tlm_extension_base *clone() const { return NULL; }
            void free() {}
            void copy_from(tlm::tlm_extension_base const &) {}
            sc_core::sc_event done;
        };

    private:
        const std::string m_name;
        signal_target_mixin *m_owner;
        NBTransportPtr m_nb_transport_ptr;
        NBTransportTaggedPtr m_nb_transport_tagged_ptr;
        BTransportPtr m_b_transport_ptr;
        BTransportTaggedPtr m_b_transport_tagged_ptr;
        tlm_utils::peq_with_get<transaction_type> m_peq;
        bool m_response_in_progress;
        sc_core::sc_event m_end_response;
    };

private:
    fw_process m_fw_process;
    bw_process m_bw_process;
    std::map<transaction_type *, sc_core::sc_event *> m_pending_trans;
    sc_core::sc_event m_end_request;
    transaction_type *m_current_transaction;
};
}

#endif //__SIGNAL_TARGET_MIXIN_H__
