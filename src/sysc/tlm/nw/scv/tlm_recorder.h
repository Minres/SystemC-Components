/*******************************************************************************
 * Copyright 2016-2022 MINRES Technologies GmbH
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

#ifndef TLM_NW_RECORDER_H_
#define TLM_NW_RECORDER_H_

#include "tlm/scc/scv/tlm_recorder.h"
#include <cci/cfg/cci_param_typed.h>
#include <scc/report.h>
#include <sstream>
#include <string>
#include <sysc/kernel/sc_dynamic_processes.h>
#include <tlm/nw/tlm_network_sockets.h>
#include <tlm/scc/scv/tlm_extension_recording_registry.h>
#include <tlm/scc/scv/tlm_recording_extension.h>
#include <tlm/scc/tlm_mm.h>
#include <tlm_utils/peq_with_cb_and_phase.h>
#ifdef HAS_SCV
#include <scv.h>
#else
#include <scv-tr.h>
#ifndef SCVNS
#define SCVNS ::scv_tr::
#endif
#endif

//! @brief SystemC TLM
namespace tlm {
//! @brief SCC TLM utilities
namespace nw {
//! @brief SCC SCV4TLM classes and functions
namespace scv {

template <typename T> void record(SCVNS scv_tr_handle&, T const&) {}
inline void record(SCVNS scv_tr_handle& h, tlm::tlm_phase const& e) { ::tlm::scc::scv::record(h, e); }
inline void record(SCVNS scv_tr_handle& h, tlm::tlm_sync_enum e) { ::tlm::scc::scv::record(h, e); }

namespace impl {
template <typename TYPES> class tlm_recording_payload : public TYPES::tlm_payload_type {
public:
    using super = typename TYPES::tlm_payload_type;
    SCVNS scv_tr_handle parent;
    uint64_t id;
    tlm_recording_payload& operator=(const super& x) {
        super::operator=(x);
        id = reinterpret_cast<uintptr_t>(&x);
        return (*this);
    }
    explicit tlm_recording_payload(tlm::nw::tlm_base_mm_interface* mm)
    : TYPES::tlm_payload_type(mm)
    , parent()
    , id(0) {}
};
template <typename TYPES = tlm::tlm_base_protocol_types> struct tlm_recording_types {
    using tlm_payload_type = tlm_recording_payload<TYPES>;
    using tlm_phase_type = typename TYPES::tlm_phase_type;
};
} // namespace impl
template <typename TYPES> void record(SCVNS scv_tr_handle& h, impl::tlm_recording_payload<TYPES> const& e) {
    record(h, static_cast<typename TYPES::tlm_payload_type const&>(e));
}
} // namespace scv
} // namespace nw
namespace scc {
template <typename TYPES> struct tlm_mm_traits<tlm::nw::scv::impl::tlm_recording_types<TYPES>> {
    using mm_if_type = tlm::nw::tlm_base_mm_interface;
    using payload_base = tlm::nw::tlm_network_payload_base;
};
} // namespace scc
namespace nw {
namespace scv {

/*! \brief The TLM2 transaction recorder
 *
 * This module records all TLM transaction to a SCV transaction stream for
 * further viewing and analysis.
 * The handle of the created transaction is storee in an tlm_extension so that
 * another instance of the scv_tlm_recorder
 * e.g. further down the opath can link to it.
 */
template <typename TYPES>
class tlm_recorder : public virtual tlm::nw::tlm_network_fw_transport_if<TYPES>,
                     public virtual tlm::nw::tlm_network_bw_transport_if<TYPES> {
    std::string get_parent(char const* hier_name) {
        std::string ret(hier_name);
        auto pos = ret.rfind('.');
        return pos == std::string::npos ? ret : ret.substr(0, pos);
    }

public:
    using recording_types = impl::tlm_recording_types<TYPES>;
    using mm = tlm::scc::tlm_mm<impl::tlm_recording_types<TYPES>>;
    using payload_type = typename TYPES::tlm_payload_type;
    using tlm_recording_payload = impl::tlm_recording_payload<TYPES>;

    //! \brief the attribute to selectively enable/disable recording of blocking protocol tx
    cci::cci_param<bool> enableBlTracing;

    //! \brief the attribute to selectively enable/disable recording of non-blocking protocol tx
    cci::cci_param<bool> enableNbTracing;

    //! \brief the attribute to selectively enable/disable timed recording
    cci::cci_param<bool> enableTimedTracing{"enableTimedTracing", true};

    //! \brief the attribute to selectively enable/disable DMI recording
    cci::cci_param<bool> enableDmiTracing{"enableDmiTracing", false};

    //! \brief the port where fw accesses are forwarded to
    sc_core::sc_port_b<tlm::nw::tlm_network_fw_transport_if<TYPES>>& fw_port;

    //! \brief the port where bw accesses are forwarded to
    sc_core::sc_port_b<tlm::nw::tlm_network_bw_transport_if<TYPES>>& bw_port;

    /**
     * @fn  tlm_recorder(sc_core::sc_port_b<tlm::nw::tlm_network_fw_transport_if<TYPES>>&,
     * sc_core::sc_port_b<tlm::nw::tlm_network_bw_transport_if<TYPES>>&, bool=true, scv_tr_db*=scv_tr_db::get_default_db())
     * @brief The constructor of the component
     *
     * @param fw_port the forward port to use in the forward path
     * @param bw_port the backward port to use in the backward path
     * @param recording_enabled if true recording is enabled by default
     * @param tr_db is a pointer to a transaction recording database. If none is
     * provided the default one is retrieved.
     *        If this database is not initialized (e.g. by not calling
     * scv_tr_db::set_default_db() ) recording is disabled.
     */
    tlm_recorder(sc_core::sc_port_b<tlm::nw::tlm_network_fw_transport_if<TYPES>>& fw_port,
                 sc_core::sc_port_b<tlm::nw::tlm_network_bw_transport_if<TYPES>>& bw_port, bool recording_enabled = true,
                 SCVNS scv_tr_db* tr_db = SCVNS scv_tr_db::get_default_db())
    : tlm_recorder(sc_core::sc_gen_unique_name("tlm_recorder"), fw_port, bw_port, recording_enabled, tr_db) {}
    /**
     * @fn  tlm_recorder(const char*, sc_core::sc_port_b<tlm::nw::tlm_network_fw_transport_if<TYPES>>&,
     * sc_core::sc_port_b<tlm::nw::tlm_network_bw_transport_if<TYPES>>&, bool=true, scv_tr_db*=scv_tr_db::get_default_db())
     * @brief
     *
     * @param name is the SystemC module name of the recorder
     * @param fw_port the forward port to use in the forward path
     * @param bw_port the backward port to use in the backward path
     * @param recording_enabled if true recording is enabled by default
     * @param tr_db is a pointer to a transaction recording database. If none is
     * provided the default one is retrieved.
     *        If this database is not initialized (e.g. by not calling
     * scv_tr_db::set_default_db() ) recording is disabled.
     */
    tlm_recorder(const char* name, sc_core::sc_port_b<tlm::nw::tlm_network_fw_transport_if<TYPES>>& fw_port,
                 sc_core::sc_port_b<tlm::nw::tlm_network_bw_transport_if<TYPES>>& bw_port, bool recording_enabled = true,
                 SCVNS scv_tr_db* tr_db = SCVNS scv_tr_db::get_default_db())
    : enableBlTracing("enableBlTracing", recording_enabled)
    , enableNbTracing("enableNbTracing", recording_enabled)
    , fw_port(fw_port)
    , bw_port(bw_port)
    , m_db(tr_db)
    , fixed_basename(name) {}

    virtual ~tlm_recorder() override {
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
    tlm::tlm_sync_enum nb_transport_fw(payload_type& trans, typename TYPES::tlm_phase_type& phase, sc_core::sc_time& delay) override;

    /*! \brief The non-blocking backward transport function
     *
     * This type of transaction is forwarded and recorded to a transaction stream
     * named "nb_bw" with current timestamps.
     * \param trans is the generic payload of the transaction
     * \param phase is the current phase of the transaction
     * \param delay is the annotated delay
     * \return the sync state of the transaction
     */
    tlm::tlm_sync_enum nb_transport_bw(payload_type& trans, typename TYPES::tlm_phase_type& phase, sc_core::sc_time& delay) override;

    /*! \brief The blocking transport function
     *
     * This type of transaction is forwarded and recorded to a transaction stream
     * named "b_tx" with current timestamps. Additionally a "b_tx_timed"
     * is been created recording the transactions at their annotated delay
     * \param trans is the generic payload of the transaction
     * \param delay is the annotated delay
     */
    void b_transport(payload_type& trans, sc_core::sc_time& delay) override;
    /*! \brief The debug transportfunction
     *
     * This type of transaction is just forwarded and not recorded.
     * \param trans is the generic payload of the transaction
     * \return the sync state of the transaction
     */
    unsigned int transport_dbg(payload_type& trans) override;
    /*! \brief get the current state of transaction recording
     *
     * \return if true transaction recording is enabled otherwise transaction
     * recording is bypassed
     */
    inline bool isRecordingBlockingTxEnabled() const { return m_db && enableBlTracing.get_value(); }
    /*! \brief get the current state of transaction recording
     *
     * \return if true transaction recording is enabled otherwise transaction
     * recording is bypassed
     */
    inline bool isRecordingNonBlockingTxEnabled() const { return m_db && enableNbTracing.get_value(); }

private:
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

    enum DIR { FW, BW, REQ = FW, RESP = BW };
    //! non-blocking transaction recording stream handle
    SCVNS scv_tr_stream* nb_streamHandle{nullptr};
    //! non-blocking transaction recording stream handle with timing
    SCVNS scv_tr_stream* nb_streamHandleTimed{nullptr};
    //! transaction generator handle for non-blocking transactions
    std::array<SCVNS scv_tr_generator<std::string, std::string>*, 2> nb_trHandle{{nullptr, nullptr}};
    //! transaction generator handle for non-blocking transactions with annotated delays
    std::array<SCVNS scv_tr_generator<>*, 2> nb_trTimedHandle{{nullptr, nullptr}};
    std::unordered_map<payload_type*, SCVNS scv_tr_handle> pending_fw_resp;
    std::unordered_map<payload_type*, SCVNS scv_tr_handle> pending_bw_resp;

public:
    void initialize_streams() {
        if(isRecordingBlockingTxEnabled() && !b_streamHandle) {
            b_streamHandle = new SCVNS scv_tr_stream((fixed_basename + "_bl").c_str(), "[TLM][base-protocol][b]", m_db);
            b_trHandle[tlm::TLM_READ_COMMAND] =
                new SCVNS scv_tr_generator<sc_dt::uint64, sc_dt::uint64>("read", *b_streamHandle, "start_delay", "end_delay");
            b_trHandle[tlm::TLM_WRITE_COMMAND] =
                new SCVNS scv_tr_generator<sc_dt::uint64, sc_dt::uint64>("write", *b_streamHandle, "start_delay", "end_delay");
            b_trHandle[tlm::TLM_IGNORE_COMMAND] =
                new SCVNS scv_tr_generator<sc_dt::uint64, sc_dt::uint64>("ignore", *b_streamHandle, "start_delay", "end_delay");
            if(enableTimedTracing.get_value()) {
                b_streamHandleTimed =
                    new SCVNS scv_tr_stream((fixed_basename + "_bl_timed").c_str(), "[TLM][base-protocol][b][timed]", m_db);
                b_trTimedHandle[tlm::TLM_READ_COMMAND] = new SCVNS scv_tr_generator<>("read", *b_streamHandleTimed);
                b_trTimedHandle[tlm::TLM_WRITE_COMMAND] = new SCVNS scv_tr_generator<>("write", *b_streamHandleTimed);
                b_trTimedHandle[tlm::TLM_IGNORE_COMMAND] = new SCVNS scv_tr_generator<>("ignore", *b_streamHandleTimed);
            }
        }
        if(isRecordingNonBlockingTxEnabled() && !nb_streamHandle) {
            nb_streamHandle = new SCVNS scv_tr_stream((fixed_basename + "_nb").c_str(), "[TLM][base-protocol][nb]", m_db);
            nb_trHandle[FW] =
                new SCVNS scv_tr_generator<std::string, std::string>("fw", *nb_streamHandle, "tlm_phase", "tlm_phase[return_path]");
            nb_trHandle[BW] =
                new SCVNS scv_tr_generator<std::string, std::string>("bw", *nb_streamHandle, "tlm_phase", "tlm_phase[return_path]");
            if(enableTimedTracing.get_value()) {
                nb_streamHandleTimed =
                    new SCVNS scv_tr_stream((fixed_basename + "_nb_timed").c_str(), "[TLM][base-protocol][nb][timed]", m_db);
                nb_trTimedHandle[FW] = new SCVNS scv_tr_generator<>("request", *nb_streamHandleTimed);
                nb_trTimedHandle[BW] = new SCVNS scv_tr_generator<>("response", *nb_streamHandleTimed);
            }
        }
    }

private:
    const std::string fixed_basename;
    inline std::string phase2string(const tlm::tlm_phase& p) {
        std::stringstream ss;
        ss << p;
        return ss.str();
    }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// implementations of functions
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template <typename TYPES> void tlm_recorder<TYPES>::b_transport(payload_type& trans, sc_core::sc_time& delay) {
    tlm_recording_payload* req{nullptr};
    if(!isRecordingBlockingTxEnabled()) {
        fw_port->b_transport(trans, delay);
        return;
    } else if(!b_streamHandle)
        initialize_streams();
    // Get a handle for the new transaction
    SCVNS scv_tr_handle h =
        b_trHandle[static_cast<unsigned>(trans.get_command())]->begin_transaction(delay.value(), sc_core::sc_time_stamp());
    /*************************************************************************
     * do the timed notification
     *************************************************************************/
    SCVNS scv_tr_handle bh;
    if(b_streamHandleTimed) {
        bh = b_trTimedHandle[static_cast<unsigned>(trans.get_command())]->begin_transaction(sc_core::sc_time_stamp() + delay);
        bh.add_relation(rel_str(tlm::scc::scv::PARENT_CHILD), h);
    }

    if(trans.get_extension_count())
        for(auto& extensionRecording : tlm::scc::scv::tlm_extension_recording_registry<TYPES>::inst().get())
            if(extensionRecording)
                extensionRecording->recordBeginTx(h, trans);
    tlm::scc::scv::tlm_recording_extension* preExt = nullptr;

    trans.get_extension(preExt);
    if(preExt == nullptr) { // we are the first recording this transaction
        preExt = new tlm::scc::scv::tlm_recording_extension(h, this);
        if(trans.has_mm())
            trans.set_auto_extension(preExt);
        else
            trans.set_extension(preExt);
    } else {
        h.add_relation(rel_str(tlm::scc::scv::PREDECESSOR_SUCCESSOR), preExt->txHandle);
    }
    SCVNS scv_tr_handle preTx{preExt->txHandle};
    preExt->txHandle = h;
    fw_port->b_transport(trans, delay);
    if(preExt && preExt->get_creator() == this) {
        // clean-up the extension if this is the original creator
        trans.set_extension(static_cast<tlm::scc::scv::tlm_recording_extension*>(nullptr));
        if(!trans.has_mm()) {
            delete preExt;
        }
    } else {
        preExt->txHandle = preTx;
    }
    record(h, trans);
    if(trans.get_extension_count())
        for(auto& extensionRecording : tlm::scc::scv::tlm_extension_recording_registry<TYPES>::inst().get())
            if(extensionRecording)
                extensionRecording->recordEndTx(h, trans);
    // End the transaction
    b_trHandle[static_cast<unsigned>(trans.get_command())]->end_transaction(h, delay.value(), sc_core::sc_time_stamp());
    // and now the stuff for the timed tx
    if(b_streamHandleTimed) {
        record(bh, trans);
        b_trTimedHandle[static_cast<unsigned>(trans.get_command())]->end_transaction(bh, sc_core::sc_time_stamp()+ delay);
    }
}

template <typename TYPES>
tlm::tlm_sync_enum tlm_recorder<TYPES>::nb_transport_fw(payload_type& trans, typename TYPES::tlm_phase_type& phase,
                                                        sc_core::sc_time& delay) {
    if(!isRecordingNonBlockingTxEnabled())
        return fw_port->nb_transport_fw(trans, phase, delay);
    else if(!nb_streamHandle)
        initialize_streams();
    /*************************************************************************
     * prepare recording
     *************************************************************************/
    // Get a handle for the new transaction
    SCVNS scv_tr_handle h = nb_trHandle[FW]->begin_transaction(phase2string(phase));
    tlm::scc::scv::tlm_recording_extension* preExt = nullptr;
    trans.get_extension(preExt);
    if(preExt == nullptr) { // we are the first recording this transaction
        preExt = new tlm::scc::scv::tlm_recording_extension(h, this);
        if(trans.has_mm())
            trans.set_auto_extension(preExt);
        else
            trans.set_extension(preExt);
    } else {
        // link handle if we have a predecessor
        h.add_relation(rel_str(tlm::scc::scv::PREDECESSOR_SUCCESSOR), preExt->txHandle);
    }
    // update the extension
    if(preExt)
        preExt->txHandle = h;
    h.record_attribute("delay", delay.to_string());
    if(trans.get_extension_count())
        for(auto& extensionRecording : tlm::scc::scv::tlm_extension_recording_registry<TYPES>::inst().get())
            if(extensionRecording)
                extensionRecording->recordBeginTx(h, trans);
    /*************************************************************************
     * do the timed notification
     *************************************************************************/
    SCVNS scv_tr_handle bh;
    if(nb_streamHandleTimed) {
        if(phase == tlm::nw::REQUEST || phase == tlm::nw::INDICATION) {
            bh = nb_trTimedHandle[static_cast<unsigned>(trans.get_command())]->begin_transaction(sc_core::sc_time_stamp() + delay);
            bh.add_relation(rel_str(tlm::scc::scv::PARENT_CHILD), h);
        }
    }
    /*************************************************************************
     * do the access
     *************************************************************************/
    tlm::tlm_sync_enum status = fw_port->nb_transport_fw(trans, phase, delay);
    /*************************************************************************
     * handle recording
     *************************************************************************/
    record(h, status);
    h.record_attribute("delay[return_path]", delay.to_string());
    record(h, trans);
    if(trans.get_extension_count())
        for(auto& extensionRecording : tlm::scc::scv::tlm_extension_recording_registry<TYPES>::inst().get())
            if(extensionRecording)
                extensionRecording->recordEndTx(h, trans);
    // get the extension and free the memory if it was mine
    if(status == tlm::TLM_COMPLETED || (status == tlm::TLM_UPDATED && (phase == tlm::nw::CONFIRM || phase == tlm::nw::RESPONSE))) {
        trans.get_extension(preExt);
        if(preExt && preExt->get_creator() == this) {
            trans.set_extension(static_cast<tlm::scc::scv::tlm_recording_extension*>(nullptr));
            if(!trans.has_mm()) {
                delete preExt;
            }
        }
        /*************************************************************************
         * do the timed notification if req. finished here
         *************************************************************************/
        if(nb_streamHandleTimed) {
            if(trans.get_command() == cxs::CXS_CMD::FLIT) {
                record(bh, trans);
                nb_trTimedHandle[static_cast<unsigned>(trans.get_command())]->end_transaction(bh, sc_core::sc_time_stamp()+ delay);
            } else {
                auto it = pending_fw_resp.find(&trans);
                if(it!=pending_fw_resp.end()) {
                    nb_trTimedHandle[static_cast<unsigned>(trans.get_command())]->end_transaction(it->second, sc_core::sc_time_stamp()+ delay);
                    pending_fw_resp.erase(it);
                } else {
                    SCCWARN(fixed_basename.c_str())<<"Could not find entry in pending_bw_resp";
                }
            }
        }
    } else if(nb_streamHandleTimed) {
        if(status == tlm::TLM_ACCEPTED && (phase == tlm::nw::REQUEST || phase == tlm::nw::INDICATION))
            pending_bw_resp.insert({&trans, bh});
    }
    // End the transaction
    nb_trHandle[FW]->end_transaction(h, phase2string(phase));
    return status;
}

template <typename TYPES>
tlm::tlm_sync_enum tlm_recorder<TYPES>::nb_transport_bw(payload_type& trans, typename TYPES::tlm_phase_type& phase,
                                                        sc_core::sc_time& delay) {
    if(!isRecordingNonBlockingTxEnabled())
        return bw_port->nb_transport_bw(trans, phase, delay);
    else if(!nb_streamHandle)
        initialize_streams();
    /*************************************************************************
     * prepare recording
     *************************************************************************/
    tlm::scc::scv::tlm_recording_extension* preExt = nullptr;
    trans.get_extension(preExt);
    // sc_assert(preExt != nullptr && "ERROR on backward path");
    // Get a handle for the new transaction
    SCVNS scv_tr_handle h = nb_trHandle[BW]->begin_transaction(phase2string(phase));
    // link handle if we have a predecessor and that's not ourself
    if(preExt) {
        h.add_relation(rel_str(tlm::scc::scv::PREDECESSOR_SUCCESSOR), preExt->txHandle);
        // and set the extension handle to this transaction
        preExt->txHandle = h;
    }
    h.record_attribute("delay", delay.to_string());
    if(trans.get_extension_count())
        for(auto& extensionRecording : tlm::scc::scv::tlm_extension_recording_registry<TYPES>::inst().get())
            if(extensionRecording)
                extensionRecording->recordBeginTx(h, trans);
    /*************************************************************************
     * do the timed notification (nothing to do)
     *************************************************************************/
    SCVNS scv_tr_handle bh;
    if(nb_streamHandleTimed) {
        if(trans.get_command() != cxs::CXS_CMD::FLIT) {
            bh = nb_trTimedHandle[static_cast<unsigned>(trans.get_command())]->begin_transaction(sc_core::sc_time_stamp() + delay);
            bh.add_relation(rel_str(tlm::scc::scv::PARENT_CHILD), h);
        }
    }
    /*************************************************************************
     * do the access
     *************************************************************************/
    tlm::tlm_sync_enum status = bw_port->nb_transport_bw(trans, phase, delay);
    /*************************************************************************
     * handle recording
     *************************************************************************/
    record(h, status);
    h.record_attribute("delay[return_path]", delay.to_string());
    record(h, trans);
    if(trans.get_extension_count())
        for(auto& extensionRecording : tlm::scc::scv::tlm_extension_recording_registry<TYPES>::inst().get())
            if(extensionRecording)
                extensionRecording->recordEndTx(h, trans);
    // End the transaction
    nb_trHandle[BW]->end_transaction(h, phase2string(phase));
    if(status == tlm::TLM_COMPLETED || (status == tlm::TLM_UPDATED && (phase == tlm::nw::CONFIRM || tlm::nw::RESPONSE))) {
        // the transaction is finished
        if(preExt && preExt->get_creator() == this) {
            // clean-up the extension if this is the original creator
            trans.set_extension(static_cast<tlm::scc::scv::tlm_recording_extension*>(nullptr));
            if(!trans.has_mm()) {
                delete preExt;
            }
        }
        /*************************************************************************
         * do the timed notification if req. finished here
         *************************************************************************/
        if(nb_streamHandleTimed) {
            if(trans.get_command() != cxs::CXS_CMD::FLIT) {
                record(bh, trans);
                nb_trTimedHandle[static_cast<unsigned>(trans.get_command())]->end_transaction(bh, sc_core::sc_time_stamp()+ delay);
            } else {
                auto it = pending_bw_resp.find(&trans);
                if(it!=pending_bw_resp.end()) {
                    nb_trTimedHandle[static_cast<unsigned>(trans.get_command())]->end_transaction(it->second, sc_core::sc_time_stamp()+ delay);
                    pending_bw_resp.erase(it);
                } else {
                    SCCWARN(fixed_basename.c_str())<<"Could not find entry in pending_bw_resp";
                }
            }
        }
    } else if(nb_streamHandleTimed) {
        if(status == tlm::TLM_ACCEPTED && (phase == tlm::nw::REQUEST || phase == tlm::nw::INDICATION))
            pending_fw_resp.insert({&trans, bh});
    }
    return status;
}

/*! \brief The debug transportfunction
 *
 * This type of transaction is just forwarded and not recorded.
 * \param trans is the generic payload of the transaction
 * \return the sync state of the transaction
 */
template <typename TYPES> unsigned int tlm_recorder<TYPES>::transport_dbg(payload_type& trans) {
    unsigned int count = fw_port->transport_dbg(trans);
    return count;
}

} // namespace scv
} // namespace nw
} // namespace tlm

#endif /* TLM2_RECORDER_H_ */
