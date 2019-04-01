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

#ifndef TLM2_RECORDER_H_
#define TLM2_RECORDER_H_

#include "tlm_gp_data_ext.h"
#include "tlm_recording_extension.h"
#include <array>
#include <map>
#include <regex>
#include <scv.h>
#include <string>
#include <tlm>
#include <tlm_utils/peq_with_cb_and_phase.h>
#include <vector>

namespace scv4tlm {
/*! \brief The TLM2 transaction extensions recorder interface
 *
 * This interface is used by the TLM2 transaction recorder. It can be used to
 * register custom recorder functionality
 * to also record the payload extensions
 */
template <typename TYPES = tlm::tlm_base_protocol_types>
class tlm2_extensions_recording_if {
public:
    /*! \brief recording attributes in extensions at the beginning, it is intended
     * to be overload as it does nothing
     *
     */
    virtual void recordBeginTx(scv_tr_handle &handle, typename TYPES::tlm_payload_type &trans) = 0;

    /*! \brief recording attributes in extensions at the end, it is intended to be
     * overload as it does nothing
     *
     */
    virtual void recordEndTx(scv_tr_handle &handle, typename TYPES::tlm_payload_type &trans) = 0;

    virtual ~tlm2_extensions_recording_if() = default;
};
/*! \brief The TLM2 transaction extensions recorder registry
 *
 * This registry is used by the TLM2 transaction recorder. It can be used to
 * register custom recorder functionality
 * to also record the payload extensions
 */
template <typename TYPES = tlm::tlm_base_protocol_types>
class tlm2_extension_recording_registry {
public:
    static tlm2_extension_recording_registry& inst(){
        static tlm2_extension_recording_registry reg;
        return reg;
    }

    void register_ext_rec(size_t id, tlm2_extensions_recording_if<TYPES>* ext){
        if(id>=ext_rec.size()) ext_rec.resize(id+1);
        if(ext_rec[id]) delete ext_rec[id];
        ext_rec[id]=ext;
    }

    const std::vector<tlm2_extensions_recording_if<TYPES>*>& get(){
        return ext_rec;
    }

    inline void recordBeginTx(size_t id, scv_tr_handle &handle, typename TYPES::tlm_payload_type &trans){
       if(ext_rec.size()>id && ext_rec[id])
           ext_rec[id]->recordBeginTx(handle, trans);
    }

    /*! \brief recording attributes in extensions at the end, it is intended to be
     * overload as it does nothing
     *
     */
    inline void recordEndTx(size_t id, scv_tr_handle &handle, typename TYPES::tlm_payload_type &trans){
        if(ext_rec.size()>id && ext_rec[id])
            ext_rec[id]->recordEndTx(handle, trans);
    }

private:
    tlm2_extension_recording_registry()=default;
    ~tlm2_extension_recording_registry(){
        for(auto& ext: ext_rec) delete(ext);
    }
    std::vector<tlm2_extensions_recording_if<TYPES>*>ext_rec;
};
/*! \brief The TLM2 transaction recorder
 *
 * This module records all TLM transaction to a SCV transaction stream for
 * further viewing and analysis.
 * The handle of the created transaction is storee in an tlm_extension so that
 * another instance of the scv_tlm2_recorder
 * e.g. further down the opath can link to it.
 */
template <typename TYPES = tlm::tlm_base_protocol_types>
class tlm2_recorder : public virtual tlm::tlm_fw_transport_if<TYPES>,
                      public virtual tlm::tlm_bw_transport_if<TYPES>,
                      public sc_core::sc_object {
    const std::regex regex_pat{"_rec$"};

public:
    SC_HAS_PROCESS(tlm2_recorder<TYPES>);// NOLINT

    //! \brief the attribute to selectively enable/disable recording
    sc_core::sc_attribute<bool> enableTracing;

    //! \brief the attribute to selectively enable/disable timed recording
    sc_core::sc_attribute<bool> enableTimed;

    //! \brief the port where fw accesses are forwarded to
    sc_core::sc_port<tlm::tlm_fw_transport_if<TYPES>> fw_port;

    //! \brief the port where bw accesses are forwarded to
    sc_core::sc_port<tlm::tlm_bw_transport_if<TYPES>> bw_port;

    /*! \brief The constructor of the component
     *
     * \param name is the SystemC module name of the recorder
     * \param tr_db is a pointer to a transaction recording database. If none is
     * provided the default one is retrieved.
     *        If this database is not initialized (e.g. by not calling
     * scv_tr_db::set_default_db() ) recording is disabled.
     */
    tlm2_recorder(bool recording_enabled = true, scv_tr_db *tr_db = scv_tr_db::get_default_db())
    : tlm2_recorder(sc_core::sc_gen_unique_name("tlm2_recorder"), recording_enabled, tr_db) {}
    /*! \brief The constructor of the component
     *
     * \param name is the SystemC module name of the recorder
     * \param tr_db is a pointer to a transaction recording database. If none is
     * provided the default one is retrieved.
     *        If this database is not initialized (e.g. by not calling
     * scv_tr_db::set_default_db() ) recording is disabled.
     */
    tlm2_recorder(const char *name, bool recording_enabled = true, scv_tr_db *tr_db = scv_tr_db::get_default_db())
    : sc_core::sc_object(name)
    , enableTracing("enableTracing", recording_enabled)
    , enableTimed("enableTimed", recording_enabled)
    , fw_port(sc_core::sc_gen_unique_name("fw"))
    , bw_port(sc_core::sc_gen_unique_name("bw"))
    , mm(new RecodingMemoryManager())
    , b_timed_peq(this, &tlm2_recorder::btx_cb)
    , nb_timed_peq(this, &tlm2_recorder::nbtx_cb)
    , m_db(tr_db)
    , b_streamHandle(NULL)
    , b_streamHandleTimed(NULL)
    , b_trTimedHandle(3)
    , nb_streamHandle(2)
    , nb_streamHandleTimed(2)
    , nb_fw_trHandle(3)
    , nb_txReqHandle(3)
    , nb_bw_trHandle(3)
    , nb_txRespHandle(3)
    , dmi_streamHandle(NULL)
    , dmi_trGetHandle(NULL)
    , dmi_trInvalidateHandle(NULL)
//    , fixed_basename(std::regex_replace(std::string(sc_core::sc_object::name()), regex_pat, std::string(""))) {
    , fixed_basename(sc_core::sc_object::name()) {
        this->add_attribute(enableTracing);
        this->add_attribute(enableTimed);
    }

    virtual ~tlm2_recorder() override {
        delete b_streamHandle;
        delete b_streamHandleTimed;
        for (size_t i = 0; i < b_trTimedHandle.size(); ++i) delete b_trTimedHandle[i];
        for (size_t i = 0; i < nb_streamHandle.size(); ++i) delete nb_streamHandle[i];
        for (size_t i = 0; i < nb_streamHandleTimed.size(); ++i) delete nb_streamHandleTimed[i];
        for (size_t i = 0; i < nb_fw_trHandle.size(); ++i) delete nb_fw_trHandle[i];
        for (size_t i = 0; i < nb_txReqHandle.size(); ++i) delete nb_txReqHandle[i];
        for (size_t i = 0; i < nb_bw_trHandle.size(); ++i) delete nb_bw_trHandle[i];
        for (size_t i = 0; i < nb_txRespHandle.size(); ++i) delete nb_txRespHandle[i];
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
    virtual tlm::tlm_sync_enum nb_transport_fw(typename TYPES::tlm_payload_type &trans,
                                               typename TYPES::tlm_phase_type &phase, sc_core::sc_time &delay);

    /*! \brief The non-blocking backward transport function
     *
     * This type of transaction is forwarded and recorded to a transaction stream
     * named "nb_bw" with current timestamps.
     * \param trans is the generic payload of the transaction
     * \param phase is the current phase of the transaction
     * \param delay is the annotated delay
     * \return the sync state of the transaction
     */
    virtual tlm::tlm_sync_enum nb_transport_bw(typename TYPES::tlm_payload_type &trans,
                                               typename TYPES::tlm_phase_type &phase, sc_core::sc_time &delay);

    /*! \brief The blocking transport function
     *
     * This type of transaction is forwarded and recorded to a transaction stream
     * named "b_tx" with current timestamps. Additionally a "b_tx_timed"
     * is been created recording the transactions at their annotated delay
     * \param trans is the generic payload of the transaction
     * \param delay is the annotated delay
     */
    virtual void b_transport(typename TYPES::tlm_payload_type &trans, sc_core::sc_time &delay);

    /*! \brief The direct memory interface forward function
     *
     * This type of transaction is just forwarded and not recorded.
     * \param trans is the generic payload of the transaction
     * \param dmi_data is the structure holding the dmi information
     * \return if the dmi structure is valid
     */
    virtual bool get_direct_mem_ptr(typename TYPES::tlm_payload_type &trans, tlm::tlm_dmi &dmi_data);
    /*! \brief The direct memory interface backward function
     *
     * This type of transaction is just forwarded and not recorded.
     * \param start_addr is the start address of the memory area being invalid
     * \param end_addr is the end address of the memory area being invalid
     */
    virtual void invalidate_direct_mem_ptr(sc_dt::uint64 start_addr, sc_dt::uint64 end_addr);
    /*! \brief The debug transportfunction
     *
     * This type of transaction is just forwarded and not recorded.
     * \param trans is the generic payload of the transaction
     * \return the sync state of the transaction
     */
    virtual unsigned int transport_dbg(typename TYPES::tlm_payload_type &trans);
    /*! \brief get the current state of transaction recording
     *
     * \return if true transaction recording is enabled otherwise transaction
     * recording is bypassed
     */
    const bool isRecordingEnabled() const { return m_db != NULL && enableTracing.value; }

private:
    //! \brief the class to hold the information to be recorded on the timed
    //! streams
    class tlm_recording_payload : public TYPES::tlm_payload_type {
    public:
        scv_tr_handle parent;
        uint64 id;
        tlm_recording_payload &operator=(const typename TYPES::tlm_payload_type &x) {
            id = (uint64)&x;
            this->set_command(x.get_command());
            this->set_address(x.get_address());
            this->set_data_ptr(x.get_data_ptr());
            this->set_data_length(x.get_data_length());
            this->set_response_status(x.get_response_status());
            this->set_byte_enable_ptr(x.get_byte_enable_ptr());
            this->set_byte_enable_length(x.get_byte_enable_length());
            this->set_streaming_width(x.get_streaming_width());
            return (*this);
        }
        explicit tlm_recording_payload(tlm::tlm_mm_interface *mm)
        : TYPES::tlm_payload_type(mm)
        , parent()
        , id(0) {}
    };
    //! \brief Memory manager for the tlm_recording_payload
    class RecodingMemoryManager : public tlm::tlm_mm_interface {
    public:
        RecodingMemoryManager()
        : free_list(0)
        , empties(0) {}
        tlm_recording_payload *allocate() {
            typename TYPES::tlm_payload_type *ptr;
            if (free_list) {
                ptr = free_list->trans;
                empties = free_list;
                free_list = free_list->next;
            } else {
                ptr = new tlm_recording_payload(this);
            }
            return (tlm_recording_payload *)ptr;
        }
        void free(tlm::tlm_generic_payload *trans) override {
            trans->reset();
            if (!empties) {
                empties = new access;
                empties->next = free_list;
                empties->prev = 0;
                if (free_list) free_list->prev = empties;
            }
            free_list = empties;
            free_list->trans = trans;
            empties = free_list->prev;
        }

    private:
        struct access {
            typename TYPES::tlm_payload_type *trans;
            access *next;
            access *prev;
        };
        access *free_list, *empties;
    };
    RecodingMemoryManager *mm;
    //! peq type definition
    struct recording_types {
        using tlm_payload_type = tlm_recording_payload;
        using tlm_phase_type = typename TYPES::tlm_phase_type;
    };
    //! event queue to hold time points of blocking transactions
    tlm_utils::peq_with_cb_and_phase<tlm2_recorder, recording_types> b_timed_peq;
    //! event queue to hold time points of non-blocking transactions
    tlm_utils::peq_with_cb_and_phase<tlm2_recorder, recording_types> nb_timed_peq;
    /*! \brief The thread processing the blocking accesses with their annotated
     * times
     *  to generate the timed view of blocking tx
     */
    void btx_cb(tlm_recording_payload &rec_parts, const typename TYPES::tlm_phase_type &phase);
    /*! \brief The thread processing the non-blocking requests with their
     * annotated times
     * to generate the timed view of non-blocking tx
     */
    void nbtx_cb(tlm_recording_payload &rec_parts, const typename TYPES::tlm_phase_type &phase);
    //! transaction recording database
    scv_tr_db *m_db;
    //! blocking transaction recording stream handle
    scv_tr_stream *b_streamHandle;
    //! transaction generator handle for blocking transactions
    std::array<scv_tr_generator<sc_dt::uint64, sc_dt::uint64> *, 3> b_trHandle;
    //! timed blocking transaction recording stream handle
    scv_tr_stream *b_streamHandleTimed;
    //! transaction generator handle for blocking transactions with annotated
    //! delays
    std::vector<scv_tr_generator<tlm::tlm_command, tlm::tlm_response_status> *> b_trTimedHandle;
    std::map<uint64, scv_tr_handle> btx_handle_map;

    enum DIR { FW, BW };
    //! non-blocking transaction recording stream handle
    std::vector<scv_tr_stream *> nb_streamHandle;
    //! non-blocking transaction recording stream handle
    std::vector<scv_tr_stream *> nb_streamHandleTimed;
    //! transaction generator handle for forward non-blocking transactions
    std::vector<scv_tr_generator<scv4tlm::tlm_phase_enum, tlm::tlm_sync_enum> *> nb_fw_trHandle;
    //! transaction generator handle for forward non-blocking transactions with
    //! annotated delays
    std::vector<scv_tr_generator<> *> nb_txReqHandle;
    map<uint64, scv_tr_handle> nbtx_req_handle_map;
    //! transaction generator handle for backward non-blocking transactions
    std::vector<scv_tr_generator<scv4tlm::tlm_phase_enum, tlm::tlm_sync_enum> *> nb_bw_trHandle;
    //! transaction generator handle for backward non-blocking transactions with
    //! annotated delays
    std::vector<scv_tr_generator<> *> nb_txRespHandle;
    map<uint64, scv_tr_handle> nbtx_last_req_handle_map;

    //! dmi transaction recording stream handle
    scv_tr_stream *dmi_streamHandle;
    //! transaction generator handle for DMI transactions
    scv_tr_generator<tlm_gp_data, tlm_dmi_data> *dmi_trGetHandle;
    scv_tr_generator<sc_dt::uint64, sc_dt::uint64> *dmi_trInvalidateHandle;

    const std::string fixed_basename;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// implementations of functions
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template <typename TYPES>
void tlm2_recorder<TYPES>::b_transport(typename TYPES::tlm_payload_type &trans, sc_core::sc_time &delay) {
    tlm_recording_payload *req;
    if (!isRecordingEnabled()) {
        fw_port->b_transport(trans, delay);
        return;
    }
    if (b_streamHandle == NULL) {
        b_streamHandle = new scv_tr_stream((fixed_basename + "_bl").c_str(), "TRANSACTOR", m_db);
        b_trHandle[tlm::TLM_READ_COMMAND] =
            new scv_tr_generator<sc_dt::uint64, sc_dt::uint64>("read", *b_streamHandle, "start_delay", "end_delay");
        b_trHandle[tlm::TLM_WRITE_COMMAND] =
            new scv_tr_generator<sc_dt::uint64, sc_dt::uint64>("write", *b_streamHandle, "start_delay", "end_delay");
        b_trHandle[tlm::TLM_IGNORE_COMMAND] =
            new scv_tr_generator<sc_dt::uint64, sc_dt::uint64>("ignore", *b_streamHandle, "start_delay", "end_delay");
    }
    // Get a handle for the new transaction
    scv_tr_handle h = b_trHandle[trans.get_command()]->begin_transaction(delay.value(), sc_time_stamp());
    tlm_gp_data tgd(trans);

    /*************************************************************************
     * do the timed notification
     *************************************************************************/
    if (enableTimed.value) {
        req = mm->allocate();
        req->acquire();
        (*req) = trans;
        req->parent = h;
        req->id = h.get_id();
#ifdef DEBUG
        cout << "notify addition of parent with id " << req->id << " to btx_handle_map with delay " << delay
             << std::endl;
#endif
        b_timed_peq.notify(*req, tlm::BEGIN_REQ, delay);
    }

    for(auto& extensionRecording: scv4tlm::tlm2_extension_recording_registry<TYPES>::inst().get())
        if (extensionRecording) extensionRecording->recordBeginTx(h, trans);
    tlm_recording_extension *preExt = NULL;

    trans.get_extension(preExt);
    if (preExt == NULL) { // we are the first recording this transaction
        preExt = new tlm_recording_extension(h, this);
        if(trans.has_mm())
            trans.set_auto_extension(preExt);
        else
            trans.set_extension(preExt);
    } else {
        h.add_relation(rel_str(PREDECESSOR_SUCCESSOR), preExt->txHandle);
    }
    scv_tr_handle preTx{preExt->txHandle};
    preExt->txHandle = h;
    if (trans.get_command() == tlm::TLM_WRITE_COMMAND && tgd.data_length < 8)
        h.record_attribute("trans.data_value", tgd.get_data_value());
    fw_port->b_transport(trans, delay);
    if (preExt && preExt->get_creator() == this) {
        // clean-up the extension if this is the original creator
        if(!trans.has_mm()){
            delete preExt;
            trans.set_extension((tlm_recording_extension *)NULL);
        }
    } else {
        preExt->txHandle = preTx;
    }

    tgd.response_status = trans.get_response_status();
    h.record_attribute("trans", tgd);
    if (trans.get_command() == tlm::TLM_READ_COMMAND && tgd.data_length < 8)
        h.record_attribute("trans.data_value", tgd.get_data_value());
    for(auto& extensionRecording: scv4tlm::tlm2_extension_recording_registry<TYPES>::inst().get())
        if (extensionRecording) extensionRecording->recordEndTx(h, trans);
    // End the transaction
    b_trHandle[trans.get_command()]->end_transaction(h, delay.value(), sc_time_stamp());
    // and now the stuff for the timed tx
    if (enableTimed.value) {
#ifdef DEBUG
        cout << "notify removal of parent with id " << req->id << " to btx_handle_map with delay " << delay
             << std::endl;
#endif
        b_timed_peq.notify(*req, tlm::END_RESP, delay);
    }
}

template <typename TYPES>
void tlm2_recorder<TYPES>::btx_cb(tlm_recording_payload &rec_parts, const typename TYPES::tlm_phase_type &phase) {
    scv_tr_handle h;
    if (b_trTimedHandle[0] == NULL) {
        b_streamHandleTimed = new scv_tr_stream((fixed_basename + "_bl_timed").c_str(), "TRANSACTOR", m_db);
        b_trTimedHandle[0] =
            new scv_tr_generator<tlm::tlm_command, tlm::tlm_response_status>("read", *b_streamHandleTimed);
        b_trTimedHandle[1] =
            new scv_tr_generator<tlm::tlm_command, tlm::tlm_response_status>("write", *b_streamHandleTimed);
        b_trTimedHandle[2] =
            new scv_tr_generator<tlm::tlm_command, tlm::tlm_response_status>("ignore", *b_streamHandleTimed);
    }
    // Now process outstanding recordings
    switch (phase) {
    case tlm::BEGIN_REQ: {
        tlm_gp_data tgd(rec_parts);
        h = b_trTimedHandle[rec_parts.get_command()]->begin_transaction(rec_parts.get_command());
        h.record_attribute("trans", tgd);
        h.add_relation(rel_str(PARENT_CHILD), rec_parts.parent);
#ifdef DEBUG
        cout << "adding parent with id " << rec_parts.id << " to btx_handle_map" << std::endl;
#endif
        btx_handle_map[rec_parts.id] = h;
    } break;
    case tlm::END_RESP: {
#ifdef DEBUG
        cout << "retrieving parent with id " << rec_parts.id << " from btx_handle_map" << std::endl;
#endif
        std::map<uint64, scv_tr_handle>::iterator it = btx_handle_map.find(rec_parts.id);
        sc_assert(it != btx_handle_map.end());
        h = it->second;
        btx_handle_map.erase(it);
        h.end_transaction(h, rec_parts.get_response_status());
        rec_parts.release();
    } break;
    default:
        sc_assert(!"phase not supported!");
    }
    return;
}

template <typename TYPES>
tlm::tlm_sync_enum tlm2_recorder<TYPES>::nb_transport_fw(typename TYPES::tlm_payload_type &trans,
                                                         typename TYPES::tlm_phase_type &phase,
                                                         sc_core::sc_time &delay) {
    if (!isRecordingEnabled()) return fw_port->nb_transport_fw(trans, phase, delay);
    // initialize stream and generator if not yet done
    if (nb_streamHandle[FW] == NULL) {
        nb_streamHandle[FW] = new scv_tr_stream((fixed_basename + "_nb_fw").c_str(), "TRANSACTOR", m_db);
        nb_fw_trHandle[tlm::TLM_READ_COMMAND] = new scv_tr_generator<scv4tlm::tlm_phase_enum, tlm::tlm_sync_enum>(
            "read", *nb_streamHandle[FW], "tlm_phase", "tlm_sync");
        nb_fw_trHandle[tlm::TLM_WRITE_COMMAND] = new scv_tr_generator<scv4tlm::tlm_phase_enum, tlm::tlm_sync_enum>(
            "write", *nb_streamHandle[FW], "tlm_phase", "tlm_sync");
        nb_fw_trHandle[tlm::TLM_IGNORE_COMMAND] = new scv_tr_generator<scv4tlm::tlm_phase_enum, tlm::tlm_sync_enum>(
            "ignore", *nb_streamHandle[FW], "tlm_phase", "tlm_sync");
    }
    /*************************************************************************
     * prepare recording
     *************************************************************************/
    // Get a handle for the new transaction
    scv_tr_handle h = nb_fw_trHandle[trans.get_command()]->begin_transaction((scv4tlm::tlm_phase_enum)(unsigned)phase);
    tlm_recording_extension *preExt = NULL;
    trans.get_extension(preExt);
    if (preExt == NULL) { // we are the first recording this transaction
        preExt = new tlm_recording_extension(h, this);
        if(trans.has_mm())
            trans.set_auto_extension(preExt);
        else
            trans.set_extension(preExt);
    } else {
        // link handle if we have a predecessor
        h.add_relation(rel_str(PREDECESSOR_SUCCESSOR), preExt->txHandle);
    }
    // update the extension
    if(preExt) preExt->txHandle = h;
    h.record_attribute("delay", delay.to_string());
    for(auto& extensionRecording: scv4tlm::tlm2_extension_recording_registry<TYPES>::inst().get())
        if (extensionRecording) extensionRecording->recordBeginTx(h, trans);
    tlm_gp_data tgd(trans);
    /*************************************************************************
     * do the timed notification
     *************************************************************************/
    if (enableTimed.value) {
        tlm_recording_payload *req = mm->allocate();
        req->acquire();
        (*req) = trans;
        req->parent = h;
        nb_timed_peq.notify(*req, phase, delay);
    }
    /*************************************************************************
     * do the access
     *************************************************************************/
    tlm::tlm_sync_enum status = fw_port->nb_transport_fw(trans, phase, delay);
    /*************************************************************************
     * handle recording
     *************************************************************************/
    tgd.response_status = trans.get_response_status();
    h.record_attribute("trans", tgd);
    if (tgd.data_length < 8) {
        uint64_t buf = 0;
        // FIXME: this is endianess dependent
        for (size_t i = 0; i < tgd.data_length; i++) buf += (*tgd.data) << i * 8;
        h.record_attribute("trans.data_value", buf);
    }
    for(auto& extensionRecording: scv4tlm::tlm2_extension_recording_registry<TYPES>::inst().get())
        if (extensionRecording) extensionRecording->recordEndTx(h, trans);
    h.record_attribute("tlm_phase[return_path]", (scv4tlm::tlm_phase_enum)(unsigned)phase);
    h.record_attribute("delay[return_path]", delay.to_string());
    // get the extension and free the memory if it was mine
    if (status == tlm::TLM_COMPLETED || (status == tlm::TLM_ACCEPTED && phase == tlm::END_RESP)) {
        trans.get_extension(preExt);
        if (preExt && preExt->get_creator() == this) {
            if(!trans.has_mm()){
                delete preExt;
                trans.set_extension((tlm_recording_extension *)NULL);
            }
        }
        /*************************************************************************
         * do the timed notification if req. finished here
         *************************************************************************/
        tlm_recording_payload *req = mm->allocate();
        req->acquire();
        (*req) = trans;
        req->parent = h;
        nb_timed_peq.notify(*req, (status == tlm::TLM_COMPLETED && phase == tlm::BEGIN_REQ) ? tlm::END_RESP : phase,
                            delay);
    } else if (status == tlm::TLM_UPDATED) {
        tlm_recording_payload *req = mm->allocate();
        req->acquire();
        (*req) = trans;
        req->parent = h;
        nb_timed_peq.notify(*req, phase, delay);
    }
    // End the transaction
    nb_fw_trHandle[trans.get_command()]->end_transaction(h, status);
    return status;
}

template <typename TYPES>
tlm::tlm_sync_enum tlm2_recorder<TYPES>::nb_transport_bw(typename TYPES::tlm_payload_type &trans,
                                                         typename TYPES::tlm_phase_type &phase,
                                                         sc_core::sc_time &delay) {
    if (!isRecordingEnabled()) return bw_port->nb_transport_bw(trans, phase, delay);
    if (nb_streamHandle[BW] == NULL) {
        nb_streamHandle[BW] = new scv_tr_stream((fixed_basename + "_nb_bw").c_str(), "TRANSACTOR", m_db);
        nb_bw_trHandle[tlm::TLM_READ_COMMAND] = new scv_tr_generator<scv4tlm::tlm_phase_enum, tlm::tlm_sync_enum>(
                "read", *nb_streamHandle[BW], "tlm_phase", "tlm_sync");
        nb_bw_trHandle[tlm::TLM_WRITE_COMMAND] = new scv_tr_generator<scv4tlm::tlm_phase_enum, tlm::tlm_sync_enum>(
                "write", *nb_streamHandle[BW], "tlm_phase", "tlm_sync");
        nb_bw_trHandle[tlm::TLM_IGNORE_COMMAND] = new scv_tr_generator<scv4tlm::tlm_phase_enum, tlm::tlm_sync_enum>(
                "ignore", *nb_streamHandle[BW], "tlm_phase", "tlm_sync");
    }
    /*************************************************************************
     * prepare recording
     *************************************************************************/
    tlm_recording_extension *preExt = NULL;
    trans.get_extension(preExt);
    //sc_assert(preExt != NULL && "ERROR on backward path");
    // Get a handle for the new transaction
    scv_tr_handle h = nb_bw_trHandle[trans.get_command()]->begin_transaction((scv4tlm::tlm_phase_enum)(unsigned)phase);
    // link handle if we have a predecessor and that's not ourself
    if(preExt) {
        h.add_relation(rel_str(PREDECESSOR_SUCCESSOR), preExt->txHandle);
        // and set the extension handle to this transaction
        preExt->txHandle = h;
    }
    h.record_attribute("delay", delay.to_string());
    for(auto& extensionRecording: scv4tlm::tlm2_extension_recording_registry<TYPES>::inst().get())
        if (extensionRecording) extensionRecording->recordBeginTx(h, trans);
    tlm_gp_data tgd(trans);
    /*************************************************************************
     * do the timed notification
     *************************************************************************/
    if (enableTimed.value) {
        tlm_recording_payload *req = mm->allocate();
        req->acquire();
        (*req) = trans;
        req->parent = h;
        nb_timed_peq.notify(*req, phase, delay);
    }
    /*************************************************************************
     * do the access
     *************************************************************************/
    tlm::tlm_sync_enum status = bw_port->nb_transport_bw(trans, phase, delay);
    /*************************************************************************
     * handle recording
     *************************************************************************/
    tgd.response_status = trans.get_response_status();
    h.record_attribute("trans", tgd);
    if (tgd.data_length < 8) {
        uint64_t buf = 0;
        // FIXME: this is endianess dependent
        for (size_t i = 0; i < tgd.data_length; i++) buf += (*tgd.data) << i * 8;
        h.record_attribute("trans.data_value", buf);
    }
    for(auto& extensionRecording: scv4tlm::tlm2_extension_recording_registry<TYPES>::inst().get())
        if (extensionRecording) extensionRecording->recordEndTx(h, trans);
    // phase and delay are already recorded
    h.record_attribute("tlm_phase[return_path]", (scv4tlm::tlm_phase_enum)(unsigned)phase);
    h.record_attribute("delay[return_path]", delay.to_string());
    // End the transaction
    nb_bw_trHandle[trans.get_command()]->end_transaction(h, status);
    if (status == tlm::TLM_COMPLETED || (status == tlm::TLM_UPDATED && phase == tlm::END_RESP)) {
        // the transaction is finished
        if (preExt && preExt->get_creator() == this) {
            if(!trans.has_mm()){
            // clean-up the extension if this is the original creator
            delete preExt;
            trans.set_extension((tlm_recording_extension *)NULL);
            }
        }
        /*************************************************************************
         * do the timed notification if req. finished here
         *************************************************************************/
        tlm_recording_payload *req = mm->allocate();
        req->acquire();
        (*req) = trans;
        req->parent = h;
        nb_timed_peq.notify(*req, phase, delay);
    }
    return status;
}

template <typename TYPES>
void tlm2_recorder<TYPES>::nbtx_cb(tlm_recording_payload &rec_parts, const typename TYPES::tlm_phase_type &phase) {
    scv_tr_handle h;
    std::map<uint64, scv_tr_handle>::iterator it;
    if (nb_streamHandleTimed[FW] == NULL) {
        nb_streamHandleTimed[FW] = new scv_tr_stream((fixed_basename + "_nb_req_timed").c_str(), "TRANSACTOR", m_db);
        nb_txReqHandle[0] = new scv_tr_generator<>("read", *nb_streamHandleTimed[FW]);
        nb_txReqHandle[1] = new scv_tr_generator<>("write", *nb_streamHandleTimed[FW]);
        nb_txReqHandle[2] = new scv_tr_generator<>("ignore", *nb_streamHandleTimed[FW]);
    }
    if (nb_streamHandleTimed[BW] == NULL) {
        nb_streamHandleTimed[BW] = new scv_tr_stream((fixed_basename + "_nb_resp_timed").c_str(), "TRANSACTOR", m_db);
        nb_txRespHandle[0] = new scv_tr_generator<>("read", *nb_streamHandleTimed[BW]);
        nb_txRespHandle[1] = new scv_tr_generator<>("write", *nb_streamHandleTimed[BW]);
        nb_txRespHandle[2] = new scv_tr_generator<>("ignore", *nb_streamHandleTimed[BW]);
    }
    tlm_gp_data tgd(rec_parts);
    switch (phase) { // Now process outstanding recordings
    case tlm::BEGIN_REQ:
        h = nb_txReqHandle[rec_parts.get_command()]->begin_transaction();
        h.record_attribute("trans", tgd);
        h.add_relation(rel_str(PARENT_CHILD), rec_parts.parent);
#ifdef NBDEBUG
        cout << "adding parent with id " << rec_parts.id << " to nbtx_req_handle_map" << std::endl;
#endif
        nbtx_req_handle_map[rec_parts.id] = h;
        break;
    case tlm::END_REQ:
#ifdef NBDEBUG
        cout << "retrieving parent with id " << rec_parts.id << " from nbtx_req_handle_map" << std::endl;
#endif
        it = nbtx_req_handle_map.find(rec_parts.id);
        sc_assert(it != nbtx_req_handle_map.end());
        h = it->second;
        nbtx_req_handle_map.erase(it);
        h.end_transaction();
        nbtx_last_req_handle_map[rec_parts.id] = h;
        break;
    case tlm::BEGIN_RESP:
#ifdef NBDEBUG
        cout << "retrieving parent with id " << rec_parts.id << " from nbtx_req_handle_map" << std::endl;
#endif
        it = nbtx_req_handle_map.find(rec_parts.id);
        if (it != nbtx_req_handle_map.end()) {
            h = it->second;
            nbtx_req_handle_map.erase(it);
            h.end_transaction();
            nbtx_last_req_handle_map[rec_parts.id] = h;
        }
        h = nb_txRespHandle[rec_parts.get_command()]->begin_transaction();
        h.record_attribute("trans", tgd);
        h.add_relation(rel_str(PARENT_CHILD), rec_parts.parent);
        nbtx_req_handle_map[rec_parts.id] = h;
        it = nbtx_last_req_handle_map.find(rec_parts.id);
        if (it != nbtx_last_req_handle_map.end()) {
            scv_tr_handle pred = it->second;
            nbtx_last_req_handle_map.erase(it);
            h.add_relation(rel_str(PREDECESSOR_SUCCESSOR), pred);
        }
        break;
    case tlm::END_RESP:
#ifdef NBDEBUG
        cout << "retrieving parent with id " << rec_parts.id << " from nbtx_req_handle_map" << std::endl;
#endif
        it = nbtx_req_handle_map.find(rec_parts.id);
        if (it != nbtx_req_handle_map.end()) {
            h = it->second;
            nbtx_req_handle_map.erase(it);
            h.end_transaction();
        }
        break;
    default:
        //sc_assert(!"phase not supported!");
        break;
    }
    rec_parts.release();
    return;
}

template <typename TYPES>
bool tlm2_recorder<TYPES>::get_direct_mem_ptr(typename TYPES::tlm_payload_type &trans, tlm::tlm_dmi &dmi_data) {
    if (!isRecordingEnabled()) {
        return fw_port->get_direct_mem_ptr(trans, dmi_data);
    }
    if (!dmi_streamHandle) dmi_streamHandle = new scv_tr_stream((fixed_basename + "_dmi").c_str(), "TRANSACTOR", m_db);
    if (!dmi_trGetHandle)
        dmi_trGetHandle =
            new scv_tr_generator<tlm_gp_data, tlm_dmi_data>("get_dmi_ptr", *dmi_streamHandle, "trans", "dmi_data");
    scv_tr_handle h = dmi_trGetHandle->begin_transaction(tlm_gp_data(trans));
    bool status = fw_port->get_direct_mem_ptr(trans, dmi_data);
    dmi_trGetHandle->end_transaction(h, tlm_dmi_data(dmi_data));
    return status;
}
/*! \brief The direct memory interface backward function
 *
 * This type of transaction is just forwarded and not recorded.
 * \param start_addr is the start address of the memory area being invalid
 * \param end_addr is the end address of the memory area being invalid
 */
template <typename TYPES>
void tlm2_recorder<TYPES>::invalidate_direct_mem_ptr(sc_dt::uint64 start_addr, sc_dt::uint64 end_addr) {
    if (!isRecordingEnabled()) {
        bw_port->invalidate_direct_mem_ptr(start_addr, end_addr);
        return;
    }
    if (!dmi_streamHandle) dmi_streamHandle = new scv_tr_stream((fixed_basename + "_dmi").c_str(), "TRANSACTOR", m_db);
    if (!dmi_trInvalidateHandle)
        dmi_trInvalidateHandle = new scv_tr_generator<sc_dt::uint64, sc_dt::uint64>(
            "invalidate_dmi_ptr", *dmi_streamHandle, "start_delay", "end_delay");

    scv_tr_handle h = dmi_trInvalidateHandle->begin_transaction(start_addr);
    bw_port->invalidate_direct_mem_ptr(start_addr, end_addr);
    dmi_trInvalidateHandle->end_transaction(h, end_addr);
    return;
}
/*! \brief The debug transportfunction
 *
 * This type of transaction is just forwarded and not recorded.
 * \param trans is the generic payload of the transaction
 * \return the sync state of the transaction
 */
template <typename TYPES> unsigned int tlm2_recorder<TYPES>::transport_dbg(typename TYPES::tlm_payload_type &trans) {
    unsigned int count = fw_port->transport_dbg(trans);
    return count;
}

} // namespace

#endif /* TLM2_RECORDER_H_ */
