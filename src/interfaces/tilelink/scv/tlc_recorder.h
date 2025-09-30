/*
 * Copyright 2020, 2021 Arteris IP
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
 */

#pragma once

#include <tlm_core/tlm_2/tlm_generic_payload/tlm_gp.h>
#ifndef SC_INCLUDE_DYNAMIC_PROCESSES
#define SC_INCLUDE_DYNAMIC_PROCESSES
#endif

#include <array>
#include <string>
#include <tilelink/tl_tlm.h>
#include <tlm/scc/scv/tlm_recorder.h>
#include <tlm/scc/scv/tlm_recording_extension.h>
#include <tlm_utils/peq_with_cb_and_phase.h>
#include <unordered_map>
#include <vector>

//! SCV components for AXI/ACE
namespace tilelink {
namespace scv {

bool register_extensions();

namespace impl {
//! \brief the class to hold the information to be recorded on the timed
//! streams
template <typename TYPES = tilelink::tl_protocol_types> class tlc_recording_payload : public TYPES::tlm_payload_type {
public:
    SCVNS scv_tr_handle parent;
    uint64_t id{0};
    bool is_snoop{false};
    tlc_recording_payload& operator=(const typename TYPES::tlm_payload_type& x) {
        id = (uint64_t)&x;
        this->set_command(x.get_command());
        this->set_address(x.get_address());
        this->set_data_ptr(nullptr);
        this->set_data_length(x.get_data_length());
        this->set_response_status(x.get_response_status());
        this->set_byte_enable_ptr(nullptr);
        this->set_byte_enable_length(x.get_byte_enable_length());
        this->set_streaming_width(x.get_streaming_width());
        return (*this);
    }
    explicit tlc_recording_payload(tlm::tlm_mm_interface* mm)
    : TYPES::tlm_payload_type(mm)
    , parent() {}
};

template <typename TYPES = tilelink::tl_protocol_types> struct tlc_recording_types {
    using tlm_payload_type = tlc_recording_payload<TYPES>;
    using tlm_phase_type = typename TYPES::tlm_phase_type;
};

} // namespace impl
/*! \brief The TLM2 transaction recorder
 *
 * This module records all TLM transaction to a SCV transaction stream for
 * further viewing and analysis.
 * The handle of the created transaction is storee in an tlm_extension so that
 * another instance of the tlc_recorder
 * e.g. further down the path can link to it.
 */
template <typename TYPES = tilelink::tl_protocol_types>
class tlc_recorder : public virtual tilelink::tlc_fw_transport_if<TYPES>, public virtual tilelink::tlc_bw_transport_if<TYPES> {
public:
    template <unsigned int BUSWIDTH = 32, int N = 1, sc_core::sc_port_policy POL = sc_core::SC_ONE_OR_MORE_BOUND>
    using initiator_socket_type = tilelink::tlc_initiator_socket<BUSWIDTH, TYPES, N, POL>;

    template <unsigned int BUSWIDTH = 32, int N = 1, sc_core::sc_port_policy POL = sc_core::SC_ONE_OR_MORE_BOUND>
    using target_socket_type = tilelink::tlc_target_socket<BUSWIDTH, TYPES, N, POL>;

    using recording_types = impl::tlc_recording_types<TYPES>;
    using mm = tlm::scc::tlm_mm<recording_types>;
    using tlm_recording_payload = impl::tlc_recording_payload<TYPES>;

    SC_HAS_PROCESS(tlc_recorder<TYPES>); // NOLINT

    //! \brief the attribute to selectively enable/disable recording of blocking protocol tx
    sc_core::sc_attribute<bool> enableBlTracing;

    //! \brief the attribute to selectively enable/disable recording of non-blocking protocol tx
    sc_core::sc_attribute<bool> enableNbTracing;

    //! \brief the attribute to selectively enable/disable timed recording
    sc_core::sc_attribute<bool> enableTimedTracing{"enableTimedTracing", true};

    //! \brief the attribute to selectively enable/disable DMI recording
    sc_core::sc_attribute<bool> enableDmiTracing{"enableDmiTracing", false};

    //! \brief the attribute to selectively enable/disable transport dbg recording
    sc_core::sc_attribute<bool> enableTrDbgTracing{"enableTrDbgTracing", false};

    //! \brief the attribute to  enable/disable protocol checking
    sc_core::sc_attribute<bool> enableProtocolChecker{"enableProtocolChecker", false};

    sc_core::sc_attribute<unsigned> rd_response_timeout{"rd_response_timeout", 0};

    sc_core::sc_attribute<unsigned> wr_response_timeout{"wr_response_timeout", 0};

    //! \brief the port where fw accesses are forwarded to
    virtual tilelink::tlc_fw_transport_if<TYPES>* get_fw_if() = 0;

    //! \brief the port where bw accesses are forwarded to
    virtual tilelink::tlc_bw_transport_if<TYPES>* get_bw_if() = 0;

    /*! \brief The constructor of the component
     *
     * \param name is the SystemC module name of the recorder
     * \param tr_db is a pointer to a transaction recording database. If none is
     * provided the default one is retrieved.
     *        If this database is not initialized (e.g. by not calling
     * scv_tr_db::set_default_db() ) recording is disabled.
     */
    tlc_recorder(const char* name, unsigned bus_width, bool recording_enabled = true,
                 SCVNS scv_tr_db* tr_db = SCVNS scv_tr_db::get_default_db())
    : enableBlTracing("enableBlTracing", recording_enabled)
    , enableNbTracing("enableNbTracing", recording_enabled)
    , b_timed_peq(this, &tlc_recorder::btx_cb)
    , nb_timed_peq(this, &tlc_recorder::nbtx_cb)
    , m_db(tr_db)
    , fixed_basename(name) {
        register_extensions();
    }

    virtual ~tlc_recorder() override {
        btx_handle_map.clear();
        nbtx_req_handle_map.clear();
        nbtx_last_req_handle_map.clear();
        nbtx_resp_handle_map.clear();
        nbtx_last_resp_handle_map.clear();
        delete b_streamHandle;
        for(auto* p : b_trHandle)
            delete p; // NOLINT
        delete b_streamHandleTimed;
        for(auto* p : b_trTimedHandle)
            delete p; // NOLINT
        delete nb_streamHandle;
        for(auto* p : nb_trHandle)
            delete p; // NOLINT
        delete nb_streamHandleTimed;
        for(auto* p : nb_trTimedHandle)
            delete p; // NOLINT
        delete dmi_streamHandle;
        delete dmi_trGetHandle;
        delete dmi_trInvalidateHandle;
    }

    // TLM-2.0 interface methods for initiator and target sockets, surrounded with
    // Tx Recording
    /*! \brief The non-blocking forward transport function
     *
     * This type of transaction is forwarded and recorded to a transaction stream
     * named "nb_fw" with current timestamps.
     * \param trans is the generic payload of the transaction
     * \param phase is the current phase of the transaction
     * \param delay is the annotated delay
     * \return the sync state of the transaction
     */
    tlm::tlm_sync_enum nb_transport_fw(typename TYPES::tlm_payload_type& trans, typename TYPES::tlm_phase_type& phase,
                                       sc_core::sc_time& delay) override;
    /*! \brief The non-blocking backward transport function
     *
     * This type of transaction is forwarded and recorded to a transaction stream
     * named "nb_bw" with current timestamps.
     * \param trans is the generic payload of the transaction
     * \param phase is the current phase of the transaction
     * \param delay is the annotated delay
     * \return the sync state of the transaction
     */
    tlm::tlm_sync_enum nb_transport_bw(typename TYPES::tlm_payload_type& trans, typename TYPES::tlm_phase_type& phase,
                                       sc_core::sc_time& delay) override;
    /*! \brief The blocking transport function
     *
     * This type of transaction is forwarded and recorded to a transaction stream
     * named "b_tx" with current timestamps. Additionally a "b_tx_timed"
     * is been created recording the transactions at their annotated delay
     * \param trans is the generic payload of the transaction
     * \param delay is the annotated delay
     */
    void b_transport(typename TYPES::tlm_payload_type& trans, sc_core::sc_time& delay) override;
    /*! \brief The blocking snoop function
     *
     * This type of transaction is forwarded and recorded to a transaction stream
     * named "b_tx" with current timestamps. Additionally a "b_tx_timed"
     * is been created recording the transactions at their annotated delay
     * \param trans is the generic payload of the transaction
     * \param delay is the annotated delay
     */
    void b_snoop(typename TYPES::tlm_payload_type& trans, sc_core::sc_time& delay) override;
    /*! \brief The direct memory interface forward function
     *
     * This type of transaction is just forwarded and not recorded.
     * \param trans is the generic payload of the transaction
     * \param dmi_data is the structure holding the dmi information
     * \return if the dmi structure is valid
     */
    bool get_direct_mem_ptr(typename TYPES::tlm_payload_type& trans, tlm::tlm_dmi& dmi_data) override;
    /*! \brief The direct memory interface backward function
     *
     * This type of transaction is just forwarded and not recorded.
     * \param start_addr is the start address of the memory area being invalid
     * \param end_addr is the end address of the memory area being invalid
     */
    void invalidate_direct_mem_ptr(sc_dt::uint64 start_addr, sc_dt::uint64 end_addr) override;
    /*! \brief The debug transportfunction
     *
     * This type of transaction is just forwarded and not recorded.
     * \param trans is the generic payload of the transaction
     * \return the sync state of the transaction
     */
    unsigned int transport_dbg(typename TYPES::tlm_payload_type& trans) override;
    /*! \brief get the current state of transaction recording
     *
     * \return if true transaction recording is enabled otherwise transaction
     * recording is bypassed
     */
    inline bool isRecordingBlockingTxEnabled() const { return m_db && enableBlTracing.value; }
    /*! \brief get the current state of transaction recording
     *
     * \return if true transaction recording is enabled otherwise transaction
     * recording is bypassed
     */
    inline bool isRecordingNonBlockingTxEnabled() const { return m_db && enableNbTracing.value; }

private:
    //! event queue to hold time points of blocking transactions
    tlm_utils::peq_with_cb_and_phase<tlc_recorder, recording_types> b_timed_peq;
    //! event queue to hold time points of non-blocking transactions
    tlm_utils::peq_with_cb_and_phase<tlc_recorder, recording_types> nb_timed_peq;
    /*! \brief The thread processing the blocking accesses with their annotated
     * times
     *  to generate the timed view of blocking tx
     */
    void btx_cb(tlm_recording_payload& rec_parts, const typename TYPES::tlm_phase_type& phase);
    /*! \brief The thread processing the non-blocking requests with their
     * annotated times
     * to generate the timed view of non-blocking tx
     */
    void nbtx_cb(tlm_recording_payload& rec_parts, const typename TYPES::tlm_phase_type& phase);
    //! transaction recording database
    SCVNS scv_tr_db* m_db{nullptr};
    //! blocking transaction recording stream handle
    SCVNS scv_tr_stream* b_streamHandle{nullptr};
    //! transaction generator handle for blocking transactions
    std::array<SCVNS scv_tr_generator<sc_dt::uint64, sc_dt::uint64>*, 3> b_trHandle{{nullptr, nullptr, nullptr}};
    //! timed blocking transaction recording stream handle
    SCVNS scv_tr_stream* b_streamHandleTimed{nullptr};
    //! transaction generator handle for blocking transactions with annotated
    //! delays
    std::array<SCVNS scv_tr_generator<>*, 3> b_trTimedHandle{{nullptr, nullptr, nullptr}};
    std::unordered_map<uint64_t, SCVNS scv_tr_handle> btx_handle_map;

    enum DIR { FW, BW, REQ = FW, RESP = BW, ACK };
    //! non-blocking transaction recording stream handle
    SCVNS scv_tr_stream* nb_streamHandle{nullptr};
    //! non-blocking transaction recording stream handle with timing
    SCVNS scv_tr_stream* nb_streamHandleTimed{nullptr};
    //! transaction generator handle for non-blocking transactions
    std::array<SCVNS scv_tr_generator<std::string, std::string>*, 2> nb_trHandle{{nullptr, nullptr}};
    //! transaction generator handle for non-blocking transactions with annotated delays
    std::array<SCVNS scv_tr_generator<>*, 3> nb_trTimedHandle{{nullptr, nullptr, nullptr}};
    std::unordered_map<uint64_t, SCVNS scv_tr_handle> nbtx_req_handle_map;
    std::unordered_map<uint64_t, SCVNS scv_tr_handle> nbtx_last_req_handle_map;
    std::unordered_map<uint64_t, SCVNS scv_tr_handle> nbtx_resp_handle_map;
    std::unordered_map<uint64_t, SCVNS scv_tr_handle> nbtx_last_resp_handle_map;
    //! dmi transaction recording stream handle
    SCVNS scv_tr_stream* dmi_streamHandle{nullptr};
    //! transaction generator handle for DMI transactions
    SCVNS scv_tr_generator<>* dmi_trGetHandle{nullptr};
    SCVNS scv_tr_generator<sc_dt::uint64, sc_dt::uint64>* dmi_trInvalidateHandle{nullptr};

protected:
    void initialize_streams() {
        if(isRecordingBlockingTxEnabled()) {
            b_streamHandle = new SCVNS scv_tr_stream((fixed_basename + "_bl").c_str(), "[TLM][ace][b]", m_db);
            b_trHandle[tlm::TLM_READ_COMMAND] =
                new SCVNS scv_tr_generator<sc_dt::uint64, sc_dt::uint64>("read", *b_streamHandle, "start_delay", "end_delay");
            b_trHandle[tlm::TLM_WRITE_COMMAND] =
                new SCVNS scv_tr_generator<sc_dt::uint64, sc_dt::uint64>("write", *b_streamHandle, "start_delay", "end_delay");
            b_trHandle[tlm::TLM_IGNORE_COMMAND] =
                new SCVNS scv_tr_generator<sc_dt::uint64, sc_dt::uint64>("ignore", *b_streamHandle, "start_delay", "end_delay");
            if(enableTimedTracing.value) {
                b_streamHandleTimed = new SCVNS scv_tr_stream((fixed_basename + "_bl_timed").c_str(), "[TLM][ace][b][timed]", m_db);
                b_trTimedHandle[tlm::TLM_READ_COMMAND] = new SCVNS scv_tr_generator<>("read", *b_streamHandleTimed);
                b_trTimedHandle[tlm::TLM_WRITE_COMMAND] = new SCVNS scv_tr_generator<>("write", *b_streamHandleTimed);
                b_trTimedHandle[tlm::TLM_IGNORE_COMMAND] = new SCVNS scv_tr_generator<>("ignore", *b_streamHandleTimed);
            }
        }
        if(isRecordingNonBlockingTxEnabled() && !nb_streamHandle) {
            nb_streamHandle = new SCVNS scv_tr_stream((fixed_basename + "_nb").c_str(), "[TLM][ace][nb]", m_db);
            nb_trHandle[FW] =
                new SCVNS scv_tr_generator<std::string, std::string>("fw", *nb_streamHandle, "tlm_phase", "tlm_phase[return_path]");
            nb_trHandle[BW] =
                new SCVNS scv_tr_generator<std::string, std::string>("bw", *nb_streamHandle, "tlm_phase", "tlm_phase[return_path]");
            if(enableTimedTracing.value) {
                nb_streamHandleTimed = new SCVNS scv_tr_stream((fixed_basename + "_nb_timed").c_str(), "[TLM][ace][nb][timed]", m_db);
                nb_trTimedHandle[FW] = new SCVNS scv_tr_generator<>("request", *nb_streamHandleTimed);
                nb_trTimedHandle[BW] = new SCVNS scv_tr_generator<>("response", *nb_streamHandleTimed);
                nb_trTimedHandle[ACK] = new SCVNS scv_tr_generator<>("ack", *nb_streamHandleTimed);
            }
        }
        if(m_db && enableDmiTracing.value && !dmi_streamHandle) {
            dmi_streamHandle = new SCVNS scv_tr_stream((fixed_basename + "_dmi").c_str(), "[TLM][ace][dmi]", m_db);
            dmi_trGetHandle = new SCVNS scv_tr_generator<>("get", *dmi_streamHandle);
            dmi_trInvalidateHandle =
                new SCVNS scv_tr_generator<sc_dt::uint64, sc_dt::uint64>("invalidate", *dmi_streamHandle, "start_addr", "end_addr");
        }
        //        if(enableProtocolChecker.value) {
        //            checker=new tilelink::checker::tlc_protocol(fixed_basename, bus_width/8, rd_response_timeout.value,
        //            wr_response_timeout.value);
        //        }
    }

private:
    const std::string fixed_basename;
};

/*! \brief The TLM2 transaction recorder
 *
 * This module records all TLM transaction to a SCV transaction stream for
 * further viewing and analysis.
 * The handle of the created transaction is storee in an tlm_extension so that
 * another instance of the scv_tlm2_recorder
 * e.g. further down the opath can link to it.
 * The transaction recorder is simply bound between an existing pair of
 * initiator and target sockets
 */
template <unsigned int BUSWIDTH, typename TYPES = tilelink::tl_protocol_types>
class tlc_recorder_module : public sc_core::sc_module, public tlc_recorder<TYPES> {
public:
    using BASE = tlc_recorder<TYPES>;
    SC_HAS_PROCESS(axitlm_recorder_module); // NOLINT
    //! The target socket of the recorder to be bound to the initiator
    typename BASE::template target_socket_type<BUSWIDTH> ts{"ts"};
    //! The initiator to be bound to the target socket
    typename BASE::template initiator_socket_type<BUSWIDTH> is{"is"};

    /*! \brief The constructor of the component
     *
     * \param name is the SystemC module name of the recorder
     * \param tr_db is a pointer to a transaction recording database. If none is
     * provided the default one is retrieved.
     *        If this database is not initialized (e.g. by not calling
     * scv_tr_db::set_default_db() ) recording is disabled.
     */
    tlc_recorder_module(sc_core::sc_module_name name, bool recording_enabled = true,
                        SCVNS scv_tr_db* tr_db = SCVNS scv_tr_db::get_default_db())
    : sc_module(name)
    , BASE(this->name(), BUSWIDTH, recording_enabled, tr_db) {
        // bind the sockets to the module
        ts.bind(*this);
        is.bind(*this);
        add_attribute(BASE::enableBlTracing);
        add_attribute(BASE::enableNbTracing);
        add_attribute(BASE::enableTimedTracing);
        add_attribute(BASE::enableDmiTracing);
        add_attribute(BASE::enableTrDbgTracing);
        add_attribute(BASE::enableProtocolChecker);
        add_attribute(BASE::rd_response_timeout);
        add_attribute(BASE::wr_response_timeout);
    }

    ~tlc_recorder_module() {}

    typename BASE::template target_socket_type<BUSWIDTH>::base_type::fw_interface_type* get_fw_if() override {
        return is.get_base_port().operator->();
    }

    typename BASE::template target_socket_type<BUSWIDTH>::base_type::bw_interface_type* get_bw_if() override {
        return ts.get_base_port().operator->();
    }

private:
    void start_of_simulation() override { BASE::initialize_streams(); }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// implementations of functions
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template <typename TYPES> void tlc_recorder<TYPES>::b_transport(typename TYPES::tlm_payload_type& trans, sc_core::sc_time& delay) {
    tlm_recording_payload* req{nullptr};
    if(!isRecordingBlockingTxEnabled()) {
        get_fw_if()->b_transport(trans, delay);
        return;
    }
    // Get a handle for the new transaction
    SCVNS scv_tr_handle h = b_trHandle[trans.get_command()]->begin_transaction(delay.value(), sc_core::sc_time_stamp());
    /*************************************************************************
     * do the timed notification
     *************************************************************************/
    if(b_streamHandleTimed) {
        req = mm::get().allocate();
        req->acquire();
        (*req) = trans;
        req->parent = h;
        req->id = h.get_id();
        b_timed_peq.notify(*req, tlm::BEGIN_REQ, delay);
    }

    auto addr = trans.get_address();
    for(auto& ext : tlm::scc::scv::tlm_extension_recording_registry<TYPES>::inst().get())
        if(ext)
            ext->recordBeginTx(h, trans);
    tlm::scc::scv::tlm_recording_extension* preExt = nullptr;

    trans.get_extension(preExt);
    if(preExt == nullptr) { // we are the first recording this transaction
        preExt = new tlm::scc::scv::tlm_recording_extension(h, this);
        if(trans.has_mm())
            trans.set_auto_extension(preExt);
        else
            trans.set_extension(preExt);
    } else {
        h.add_relation(tlm::scc::scv::rel_str(tlm::scc::scv::PREDECESSOR_SUCCESSOR), preExt->txHandle);
    }
    SCVNS scv_tr_handle preTx(preExt->txHandle);
    preExt->txHandle = h;
    get_fw_if()->b_transport(trans, delay);
    trans.get_extension(preExt);
    if(preExt->get_creator() == this) {
        // clean-up the extension if this is the original creator
        if(trans.has_mm())
            trans.set_auto_extension(static_cast<tlm::scc::scv::tlm_recording_extension*>(nullptr));
        else
            delete trans.set_extension(static_cast<tlm::scc::scv::tlm_recording_extension*>(nullptr));
    } else {
        preExt->txHandle = preTx;
    }

    trans.set_address(addr);
    tlm::scc::scv::record(h, trans);
    for(auto& ext : tlm::scc::scv::tlm_extension_recording_registry<TYPES>::inst().get())
        if(ext)
            ext->recordEndTx(h, trans);
    // End the transaction
    b_trHandle[trans.get_command()]->end_transaction(h, delay.value(), sc_core::sc_time_stamp());
    // and now the stuff for the timed tx
    if(b_streamHandleTimed) {
        b_timed_peq.notify(*req, tlm::END_RESP, delay);
    }
}

template <typename TYPES> void tlc_recorder<TYPES>::b_snoop(typename TYPES::tlm_payload_type& trans, sc_core::sc_time& delay) {
    tlm_recording_payload* req{nullptr};
    if(!b_streamHandleTimed) {
        get_fw_if()->b_transport(trans, delay);
        return;
    }
    // Get a handle for the new transaction
    SCVNS scv_tr_handle h = b_trHandle[trans.get_command()]->begin_transaction(delay.value(), sc_core::sc_time_stamp());
    /*************************************************************************
     * do the timed notification
     *************************************************************************/
    if(b_streamHandleTimed) {
        req = mm::get().allocate();
        req->acquire();
        (*req) = trans;
        req->parent = h;
        req->id = h.get_id();
        b_timed_peq.notify(*req, tlm::BEGIN_REQ, delay);
    }

    for(auto& ext : tlm::scc::scv::tlm_extension_recording_registry<TYPES>::inst().get())
        if(ext)
            ext->recordBeginTx(h, trans);
    tlm::scc::scv::tlm_recording_extension* preExt = NULL;

    trans.get_extension(preExt);
    if(preExt == NULL) { // we are the first recording this transaction
        preExt = new tlm::scc::scv::tlm_recording_extension(h, this);
        if(trans.has_mm())
            trans.set_auto_extension(preExt);
        else
            trans.set_extension(preExt);
    } else {
        h.add_relation(tlm::scc::scv::rel_str(tlm::scc::scv::PREDECESSOR_SUCCESSOR), preExt->txHandle);
    }
    SCVNS scv_tr_handle preTx(preExt->txHandle);
    preExt->txHandle = h;
    get_bw_if()->b_snoop(trans, delay);
    trans.get_extension(preExt);
    if(preExt->get_creator() == this) {
        // clean-up the extension if this is the original creator
        if(trans.has_mm())
            trans.set_auto_extension(static_cast<tlm::scc::scv::tlm_recording_extension*>(nullptr));
        else
            delete trans.set_extension(static_cast<tlm::scc::scv::tlm_recording_extension*>(nullptr));
    } else {
        preExt->txHandle = preTx;
    }

    tlm::scc::scv::record(h, trans);
    for(auto& ext : tlm::scc::scv::tlm_extension_recording_registry<TYPES>::inst().get())
        if(ext)
            ext->recordEndTx(h, trans);
    // End the transaction
    b_trHandle[trans.get_command()]->end_transaction(h, delay.value(), sc_core::sc_time_stamp());
    // and now the stuff for the timed tx
    if(b_streamHandleTimed) {
        b_timed_peq.notify(*req, tlm::END_RESP, delay);
    }
}

template <typename TYPES> void tlc_recorder<TYPES>::btx_cb(tlm_recording_payload& rec_parts, const typename TYPES::tlm_phase_type& phase) {
    SCVNS scv_tr_handle h;
    // Now process outstanding recordings
    switch(phase) {
    case tlm::BEGIN_REQ: {
        h = b_trTimedHandle[rec_parts.get_command()]->begin_transaction();
        h.add_relation(tlm::scc::scv::rel_str(tlm::scc::scv::PARENT_CHILD), rec_parts.parent);
        btx_handle_map[rec_parts.id] = h;
    } break;
    case tlm::END_RESP: {
        auto it = btx_handle_map.find(rec_parts.id);
        sc_assert(it != btx_handle_map.end());
        h = it->second;
        btx_handle_map.erase(it);
        tlm::scc::scv::record(h, rec_parts);
        h.end_transaction();
        rec_parts.release();
    } break;
    default:
        sc_assert(!"phase not supported!");
    }
    return;
}

template <typename TYPES>
tlm::tlm_sync_enum tlc_recorder<TYPES>::nb_transport_fw(typename TYPES::tlm_payload_type& trans, typename TYPES::tlm_phase_type& phase,
                                                        sc_core::sc_time& delay) {
    if(!isRecordingNonBlockingTxEnabled()) {
        return get_fw_if()->nb_transport_fw(trans, phase, delay);
    }
    /*************************************************************************
     * prepare recording
     *************************************************************************/
    // Get a handle for the new transaction
    SCVNS scv_tr_handle h = nb_trHandle[FW]->begin_transaction(phase.get_name());
    tlm::scc::scv::tlm_recording_extension* preExt = nullptr;
    trans.get_extension(preExt);
    if(phase == tlm::BEGIN_REQ && preExt == nullptr) { // we are the first recording this transaction
        preExt = new tlm::scc::scv::tlm_recording_extension(h, this);
        if(trans.has_mm())
            trans.set_auto_extension(preExt);
        else
            trans.set_extension(preExt);
    } else if(preExt != nullptr) {
        // link handle if we have a predecessor
        h.add_relation(tlm::scc::scv::rel_str(tlm::scc::scv::PREDECESSOR_SUCCESSOR), preExt->txHandle);
    } else {
        sc_assert(preExt != nullptr && "ERROR on forward path in phase other than tlm::BEGIN_REQ");
    }
    // update the extension
    preExt->txHandle = h;
    h.record_attribute("delay", delay.to_string());
    for(auto& ext : tlm::scc::scv::tlm_extension_recording_registry<TYPES>::inst().get())
        if(ext)
            ext->recordBeginTx(h, trans);
    /*************************************************************************
     * do the timed notification
     *************************************************************************/
    if(nb_streamHandleTimed) {
        tlm_recording_payload* req = mm::get().allocate();
        req->acquire();
        (*req) = trans;
        req->parent = h;
        req->is_snoop = phase == tlm::BEGIN_RESP;
        nb_timed_peq.notify(*req, phase, delay);
    }
    /*************************************************************************
     * do the access
     *************************************************************************/
    tlm::tlm_sync_enum status = get_fw_if()->nb_transport_fw(trans, phase, delay);
    /*************************************************************************
     * handle recording
     *************************************************************************/
    tlm::scc::scv::record(h, status);
    h.record_attribute("delay[return_path]", delay.to_string());
    tlm::scc::scv::record(h, trans);
    for(auto& ext : tlm::scc::scv::tlm_extension_recording_registry<TYPES>::inst().get())
        if(ext)
            ext->recordEndTx(h, trans);
    // get the extension and free the memory if it was mine
    if(status == tlm::TLM_COMPLETED || (phase == tilelink::ACK)) {
        // the transaction is finished
        trans.get_extension(preExt);
        if(preExt && preExt->get_creator() == this) {
            // clean-up the extension if this is the original creator
            if(trans.has_mm())
                trans.set_auto_extension(static_cast<tlm::scc::scv::tlm_recording_extension*>(nullptr));
            else
                delete trans.set_extension(static_cast<tlm::scc::scv::tlm_recording_extension*>(nullptr));
        }
        /*************************************************************************
         * do the timed notification if req. finished here
         *************************************************************************/
        if(nb_streamHandleTimed) {
            tlm_recording_payload* req = mm::get().allocate();
            req->acquire();
            (*req) = trans;
            req->parent = h;
            nb_timed_peq.notify(*req, (status == tlm::TLM_COMPLETED && phase == tlm::BEGIN_REQ) ? tlm::END_RESP : phase, delay);
        }
    } else if(nb_streamHandleTimed && status == tlm::TLM_UPDATED) {
        tlm_recording_payload* req = mm::get().allocate();
        req->acquire();
        (*req) = trans;
        req->parent = h;
        nb_timed_peq.notify(*req, phase, delay);
    }
    // End the transaction
    nb_trHandle[FW]->end_transaction(h, phase.get_name());
    return status;
}

template <typename TYPES>
tlm::tlm_sync_enum tlc_recorder<TYPES>::nb_transport_bw(typename TYPES::tlm_payload_type& trans, typename TYPES::tlm_phase_type& phase,
                                                        sc_core::sc_time& delay) {
    if(!isRecordingNonBlockingTxEnabled()) {
        return get_bw_if()->nb_transport_bw(trans, phase, delay);
    }
    /*************************************************************************
     * prepare recording
     *************************************************************************/
    // Get a handle for the new transaction
    SCVNS scv_tr_handle h = nb_trHandle[BW]->begin_transaction(phase.get_name());
    tlm::scc::scv::tlm_recording_extension* preExt = nullptr;
    trans.get_extension(preExt);
    if(phase == tlm::BEGIN_REQ && preExt == nullptr) { // we are the first recording this transaction
        preExt = new tlm::scc::scv::tlm_recording_extension(h, this);
        if(trans.has_mm())
            trans.set_auto_extension(preExt);
        else
            trans.set_extension(preExt);
    } else if(preExt != nullptr) {
        // link handle if we have a predecessor
        h.add_relation(tlm::scc::scv::rel_str(tlm::scc::scv::PREDECESSOR_SUCCESSOR), preExt->txHandle);
    } else {
        sc_assert(preExt != nullptr && "ERROR on backward path in phase other than tlm::BEGIN_REQ");
    }
    // and set the extension handle to this transaction
    preExt->txHandle = h;
    h.record_attribute("delay", delay.to_string());
    for(auto& ext : tlm::scc::scv::tlm_extension_recording_registry<TYPES>::inst().get())
        if(ext)
            ext->recordBeginTx(h, trans);
    /*************************************************************************
     * do the timed notification
     *************************************************************************/
    if(nb_streamHandleTimed) {
        tlm_recording_payload* req = mm::get().allocate();
        req->acquire();
        (*req) = trans;
        req->parent = h;
        req->is_snoop = phase == tlm::BEGIN_REQ;
        nb_timed_peq.notify(*req, phase, delay);
    }
    /*************************************************************************
     * do the access
     *************************************************************************/
    tlm::tlm_sync_enum status = get_bw_if()->nb_transport_bw(trans, phase, delay);
    /*************************************************************************
     * handle recording
     *************************************************************************/
    tlm::scc::scv::record(h, status);
    h.record_attribute("delay[return_path]", delay.to_string());
    tlm::scc::scv::record(h, trans);
    for(auto& ext : tlm::scc::scv::tlm_extension_recording_registry<TYPES>::inst().get())
        if(ext)
            ext->recordEndTx(h, trans);
    // get the extension and free the memory if it was mine
    if(status == tlm::TLM_COMPLETED || (status == tlm::TLM_UPDATED && phase == tilelink::ACK)) {
        // the transaction is finished
        trans.get_extension(preExt);
        if(preExt->get_creator() == this) {
            // clean-up the extension if this is the original creator
            if(trans.has_mm())
                trans.set_auto_extension(static_cast<tlm::scc::scv::tlm_recording_extension*>(nullptr));
            else
                delete trans.set_extension(static_cast<tlm::scc::scv::tlm_recording_extension*>(nullptr));
        }
        /*************************************************************************
         * do the timed notification if req. finished here
         *************************************************************************/
        if(nb_streamHandleTimed) {
            tlm_recording_payload* req = mm::get().allocate();
            req->acquire();
            (*req) = trans;
            req->parent = h;
            nb_timed_peq.notify(*req, (status == tlm::TLM_COMPLETED && phase == tlm::BEGIN_REQ) ? tlm::END_RESP : phase, delay);
        }
    } else if(nb_streamHandleTimed && status == tlm::TLM_UPDATED) {
        tlm_recording_payload* req = mm::get().allocate();
        req->acquire();
        (*req) = trans;
        req->parent = h;
        nb_timed_peq.notify(*req, phase, delay);
    }
    // End the transaction
    nb_trHandle[BW]->end_transaction(h, phase.get_name());
    return status;
}

template <typename TYPES> void tlc_recorder<TYPES>::nbtx_cb(tlm_recording_payload& rec_parts, const typename TYPES::tlm_phase_type& phase) {
    SCVNS scv_tr_handle h;
    // Now process outstanding recordings
    if(phase == tlm::BEGIN_REQ) {
        h = nb_trTimedHandle[REQ]->begin_transaction();
        tlm::scc::scv::record(h, rec_parts);
        h.add_relation(tlm::scc::scv::rel_str(tlm::scc::scv::PARENT_CHILD), rec_parts.parent);
        nbtx_req_handle_map[rec_parts.id] = h;
    } else if(phase == tlm::END_REQ) {
        auto it = nbtx_req_handle_map.find(rec_parts.id);
        sc_assert(it != nbtx_req_handle_map.end());
        h = it->second;
        nbtx_req_handle_map.erase(it);
        h.end_transaction();
        nbtx_last_req_handle_map[rec_parts.id] = h;
    } else if(phase == tlm::BEGIN_RESP) {
        auto it = nbtx_req_handle_map.find(rec_parts.id);
        if(it != nbtx_req_handle_map.end()) {
            h = it->second;
            nbtx_req_handle_map.erase(it);
            h.end_transaction();
            nbtx_last_req_handle_map[rec_parts.id] = h;
        }
        h = nb_trTimedHandle[RESP]->begin_transaction();
        tlm::scc::scv::record(h, rec_parts);
        h.add_relation(tlm::scc::scv::rel_str(tlm::scc::scv::PARENT_CHILD), rec_parts.parent);
        nbtx_resp_handle_map[rec_parts.id] = h;
        it = nbtx_last_req_handle_map.find(rec_parts.id);
        if(it != nbtx_last_req_handle_map.end()) {
            SCVNS scv_tr_handle& pred = it->second;
            h.add_relation(tlm::scc::scv::rel_str(tlm::scc::scv::PREDECESSOR_SUCCESSOR), pred);
            nbtx_last_req_handle_map.erase(it);
        } else {
            it = nbtx_last_resp_handle_map.find(rec_parts.id);
            if(it != nbtx_last_resp_handle_map.end()) {
                SCVNS scv_tr_handle& pred = it->second;
                h.add_relation(tlm::scc::scv::rel_str(tlm::scc::scv::PREDECESSOR_SUCCESSOR), pred);
                nbtx_last_resp_handle_map.erase(it);
            }
        }
    } else if(phase == tlm::END_RESP) {
        auto it = nbtx_resp_handle_map.find(rec_parts.id);
        if(it != nbtx_resp_handle_map.end()) {
            h = it->second;
            nbtx_resp_handle_map.erase(it);
            h.end_transaction();
        }
    } else if(phase == tilelink::ACK) {
        h = nb_trTimedHandle[ACK]->begin_transaction();
        tlm::scc::scv::record(h, rec_parts);
        h.add_relation(tlm::scc::scv::rel_str(tlm::scc::scv::PARENT_CHILD), rec_parts.parent);
        h.end_transaction();
    } else
        sc_assert(!"phase not supported!");
    rec_parts.release();
    return;
}

template <typename TYPES> bool tlc_recorder<TYPES>::get_direct_mem_ptr(typename TYPES::tlm_payload_type& trans, tlm::tlm_dmi& dmi_data) {
    if(!(m_db && enableDmiTracing.value))
        return get_fw_if()->get_direct_mem_ptr(trans, dmi_data);
    else if(!dmi_streamHandle)
        initialize_streams();
    SCVNS scv_tr_handle h = dmi_trGetHandle->begin_transaction();
    bool status = get_fw_if()->get_direct_mem_ptr(trans, dmi_data);
    tlm::scc::scv::record(h, trans);
    tlm::scc::scv::record(h, dmi_data);
    h.end_transaction();
    return status;
}
/*! \brief The direct memory interface backward function
 *
 * This type of transaction is just forwarded and not recorded.
 * \param start_addr is the start address of the memory area being invalid
 * \param end_addr is the end address of the memory area being invalid
 */
template <typename TYPES> void tlc_recorder<TYPES>::invalidate_direct_mem_ptr(sc_dt::uint64 start_addr, sc_dt::uint64 end_addr) {
    if(!(m_db && enableDmiTracing.value)) {
        get_bw_if()->invalidate_direct_mem_ptr(start_addr, end_addr);
        return;
    } else if(!dmi_streamHandle)
        initialize_streams();
    SCVNS scv_tr_handle h = dmi_trInvalidateHandle->begin_transaction(start_addr);
    get_bw_if()->invalidate_direct_mem_ptr(start_addr, end_addr);
    dmi_trInvalidateHandle->end_transaction(h, end_addr);
    return;
}
/*! \brief The debug transportfunction
 *
 * This type of transaction is just forwarded and not recorded.
 * \param trans is the generic payload of the transaction
 * \return the sync state of the transaction
 */
template <typename TYPES> unsigned int tlc_recorder<TYPES>::transport_dbg(typename TYPES::tlm_payload_type& trans) {
    unsigned int count = get_fw_if()->transport_dbg(trans);
    return count;
}
} // namespace scv
} // namespace tilelink
