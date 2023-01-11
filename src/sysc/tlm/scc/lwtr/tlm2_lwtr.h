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

#pragma once

#include "lwtr4tlm2.h"
#include <cci_configuration>
#include <array>
#include <regex>
#include <sstream>
#include <string>
#include <tlm/scc/tlm_mm.h>
#include <tlm/scc/tlm_gp_shared.h>
#include <sysc/kernel/sc_dynamic_processes.h>
#include <scc/peq.h>
#include <tlm/scc/lwtr/lwtr4tlm2_extension_registry.h>
#include <unordered_map>

//! @brief SystemC Components Library
namespace tlm {
namespace scc {
namespace lwtr {

extern bool registered;
enum tx_rel {
	PARENT_CHILD = 0,     /*!< indicates parent child relationship */
	PREDECESSOR_SUCCESSOR /*!< indicates predecessor successor relationship */
};

/*! \brief The TLM2 transaction recorder
 *
 * This module records all TLM transaction to a SCV transaction stream for
 * further viewing and analysis.
 * The handle of the created transaction is storee in an tlm_extension so that
 * another instance of the scv_tlm_recorder
 * e.g. further down the opath can link to it.
 */
template <typename TYPES = tlm::tlm_base_protocol_types>
class tlm_recorder : public sc_core::sc_object, public virtual tlm::tlm_fw_transport_if<TYPES>, public virtual tlm::tlm_bw_transport_if<TYPES> {
	using tx_db = ::lwtr::tx_db;
	using tx_fiber = ::lwtr::tx_fiber;
	template <typename BEGIN = ::lwtr::no_data, typename END = ::lwtr::no_data>
	using tx_generator = ::lwtr::tx_generator<BEGIN, END>;
	using tx_handle = ::lwtr::tx_handle;
    using mm = tlm::scc::tlm_mm<>;

	std::string get_parent(char const* hier_name) {
		std::string ret(hier_name);
		auto pos = ret.rfind('.');
		return pos == std::string::npos ? ret : ret.substr(0, pos);
	}
	struct link_pred_ext : public tlm::tlm_extension<link_pred_ext> {
		tlm_extension_base* clone() const override {
			link_pred_ext* t = new link_pred_ext(this->txHandle, this->creator);
			return t;
		}
		void copy_from(tlm_extension_base const& from) override {
			txHandle = static_cast<link_pred_ext const&>(from).txHandle;
			creator = static_cast<link_pred_ext const&>(from).creator;
		}
		link_pred_ext(tx_handle handle, void const* creator_) : txHandle(handle), creator(creator_) {}
		tx_handle txHandle;
		void const* creator;
	};

	struct nb_rec_entry {
		tlm::scc::tlm_gp_shared_ptr tr;
		tlm::tlm_phase ph;
		uintptr_t id;
		tx_handle parent;
	};
	public:
	//! \brief the attribute to selectively enable/disable recording of blocking protocol tx
	cci::cci_param<bool> enableBlTracing;

	//! \brief the attribute to selectively enable/disable recording of non-blocking protocol tx
	cci::cci_param<bool> enableNbTracing;

	//! \brief the attribute to selectively enable/disable timed recording
	cci::cci_param<bool> enableTimedTracing{"enableTimedTracing", true};

	//! \brief the attribute to selectively enable/disable DMI recording
	cci::cci_param<bool> enableDmiTracing{"enableDmiTracing", false};

	//! \brief the port where fw accesses are forwarded to
	sc_core::sc_port<tlm::tlm_fw_transport_if<TYPES>> fw_port{"fw_port"};

	//! \brief the export where bw accesses are forwarded to
	sc_core::sc_export<tlm::tlm_bw_transport_if<TYPES>> bw_export{"bw_export"};

	//! \brief the port where bw accesses are forwarded to
	sc_core::sc_port<tlm::tlm_bw_transport_if<TYPES>> bw_port{"bw_port"};

	//! \brief the port where fw accesses are forwarded to
	sc_core::sc_export<tlm::tlm_fw_transport_if<TYPES>> fw_export{"fw_export"};

	/**
	 * @fn  tlm_recorder(sc_core::sc_port_b<tlm::tlm_fw_transport_if<TYPES>>&,
	 * sc_core::sc_port_b<tlm::tlm_bw_transport_if<TYPES>>&, bool=true, scv_tr_db*=scv_tr_db::get_default_db())
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
	tlm_recorder(bool recording_enabled = true, tx_db* tr_db = tx_db::get_default_db())
	: tlm_recorder(sc_core::sc_gen_unique_name("tlm_recorder"), recording_enabled, tr_db) {}
	/**
	 * @fn  tlm_recorder(const char*, sc_core::sc_port_b<tlm::tlm_fw_transport_if<TYPES>>&,
	 * sc_core::sc_port_b<tlm::tlm_bw_transport_if<TYPES>>&, bool=true, scv_tr_db*=scv_tr_db::get_default_db())
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
	tlm_recorder(const char* name, bool recording_enabled = true, tx_db* tr_db = tx_db::get_default_db())
	: sc_core::sc_object(name)
	, enableBlTracing("enableBlTracing", recording_enabled)
	, enableNbTracing("enableNbTracing", recording_enabled)
    , nb_timed_peq()
	, m_db(tr_db)
	{
		fw_export(*this);
		bw_export(*this);
		sc_core::sc_spawn_options opts;
		opts.set_sensitivity(&nb_timed_peq.event());
		sc_core::sc_spawn([this](){nbtx_cb();}, nullptr, &opts);
	}

	virtual ~tlm_recorder() override {
		btx_handle_map.clear();
		nbtx_req_handle_map.clear();
		nbtx_last_req_handle_map.clear();
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
	inline bool isRecordingBlockingTxEnabled() const { return m_db && enableBlTracing.get_value(); }
	/*! \brief get the current state of transaction recording
	 *
	 * \return if true transaction recording is enabled otherwise transaction
	 * recording is bypassed
	 */
	inline bool isRecordingNonBlockingTxEnabled() const { return m_db && enableNbTracing.get_value(); }

	template <unsigned int BUSWIDTH = 32,
			typename FW_IF = tlm::tlm_fw_transport_if<>,
			typename BW_IF = tlm::tlm_bw_transport_if<>,
			int N = 1,
			sc_core::sc_port_policy POL = sc_core::SC_ONE_OR_MORE_BOUND>
	void bind(tlm::tlm_base_initiator_socket<BUSWIDTH, FW_IF, BW_IF, N, POL>& sck){
		sck.get_base_port().bind(fw_export);
		bw_port.bind(sck.get_base_export());
	}
	template <unsigned int BUSWIDTH = 32,
			typename FW_IF = tlm::tlm_fw_transport_if<>,
			typename BW_IF = tlm::tlm_bw_transport_if<>,
			int N = 1,
			sc_core::sc_port_policy POL = sc_core::SC_ONE_OR_MORE_BOUND>
	void bind(tlm::tlm_base_target_socket<BUSWIDTH, FW_IF, BW_IF, N, POL>& sck){
		sck.get_base_port().bind(bw_export);
		fw_port.bind(sck.get_base_export());
	}
	private:
    //! event queue to hold time points of non-blocking transactions
    ::scc::peq<nb_rec_entry> nb_timed_peq;
    /*! \brief The thread processing the non-blocking requests with their
     * annotated times
     * to generate the timed view of non-blocking tx
     */
    void nbtx_cb();
	//! transaction recording database
	tx_db* m_db{nullptr};
	//! the relationship name handles
	::lwtr::tx_relation_handle pred_succ_hndl{0}, par_chld_hndl{0};
	//! blocking transaction recording stream handle
	tx_fiber* b_streamHandle{nullptr};
	//! transaction generator handle for blocking transactions
	std::array<tx_generator<sc_dt::uint64, sc_dt::uint64>*, 3> b_trHandle{{nullptr, nullptr, nullptr}};
	//! timed blocking transaction recording stream handle
	tx_fiber* b_streamHandleTimed{nullptr};
	//! transaction generator handle for blocking transactions with annotated
	//! delays
	std::array<tx_generator<>*, 3> b_trTimedHandle{{nullptr, nullptr, nullptr}};
	std::unordered_map<uint64_t, tx_handle> btx_handle_map;

	enum DIR { FW, BW, REQ = FW, RESP = BW };
	//! non-blocking transaction recording stream handle
	tx_fiber* nb_streamHandle{nullptr};
	//! non-blocking transaction recording stream handle with timing
	tx_fiber* nb_streamHandleTimed{nullptr};
	//! transaction generator handle for non-blocking transactions
	std::array<tx_generator<std::string, std::string>*, 2> nb_trHandle{{nullptr, nullptr}};
	//! transaction generator handle for non-blocking transactions with annotated delays
	std::array<tx_generator<>*, 2> nb_trTimedHandle{{nullptr, nullptr}};
	std::unordered_map<uint64_t, tx_handle> nbtx_req_handle_map;
	std::unordered_map<uint64_t, tx_handle> nbtx_last_req_handle_map;

	//! dmi transaction recording stream handle
	tx_fiber* dmi_streamHandle{nullptr};
	//! transaction generator handle for DMI transactions
	tx_generator<>* dmi_trGetHandle{nullptr};
	tx_generator<sc_dt::uint64, sc_dt::uint64>* dmi_trInvalidateHandle{nullptr};

	public:
	void initialize_streams() {
		if(m_db) {
			pred_succ_hndl = m_db->create_relation("PREDECESSOR_SUCCESSOR");
			par_chld_hndl = m_db->create_relation("PARENT_CHILD");
		}
		if(isRecordingBlockingTxEnabled() && !b_streamHandle) {
			b_streamHandle = new tx_fiber((std::string(name()) + "_bl").c_str(), "[TLM][base-protocol][b]", m_db);
			b_trHandle[tlm::TLM_READ_COMMAND] = new tx_generator<sc_dt::uint64, sc_dt::uint64>(
					"read", *b_streamHandle, "start_delay", "end_delay");
			b_trHandle[tlm::TLM_WRITE_COMMAND] = new tx_generator<sc_dt::uint64, sc_dt::uint64>(
					"write", *b_streamHandle, "start_delay", "end_delay");
			b_trHandle[tlm::TLM_IGNORE_COMMAND] = new tx_generator<sc_dt::uint64, sc_dt::uint64>(
					"ignore", *b_streamHandle, "start_delay", "end_delay");
			if(enableTimedTracing.get_value()) {
				b_streamHandleTimed = new tx_fiber((std::string(name()) + "_bl_timed").c_str(),
						"[TLM][base-protocol][b][timed]", m_db);
				b_trTimedHandle[tlm::TLM_READ_COMMAND] = new tx_generator<>("read", *b_streamHandleTimed);
				b_trTimedHandle[tlm::TLM_WRITE_COMMAND] = new tx_generator<>("write", *b_streamHandleTimed);
				b_trTimedHandle[tlm::TLM_IGNORE_COMMAND] = new tx_generator<>("ignore", *b_streamHandleTimed);
			}
		}
		if(isRecordingNonBlockingTxEnabled() && !nb_streamHandle) {
			nb_streamHandle =
					new tx_fiber((std::string(name()) + "_nb").c_str(), "[TLM][base-protocol][nb]", m_db);
			nb_trHandle[FW] = new tx_generator<std::string, std::string>("fw", *nb_streamHandle, "tlm_phase",
					"tlm_phase[return_path]");
			nb_trHandle[BW] = new tx_generator<std::string, std::string>("bw", *nb_streamHandle, "tlm_phase",
					"tlm_phase[return_path]");
			if(enableTimedTracing.get_value()) {
				nb_streamHandleTimed = new tx_fiber((std::string(name()) + "_nb_timed").c_str(),
						"[TLM][base-protocol][nb][timed]", m_db);
				nb_trTimedHandle[FW] = new tx_generator<>("request", *nb_streamHandleTimed);
				nb_trTimedHandle[BW] = new tx_generator<>("response", *nb_streamHandleTimed);
			}
		}
		if(m_db && enableDmiTracing.get_value() && !dmi_streamHandle) {
			dmi_streamHandle =
					new tx_fiber((std::string(name()) + "_dmi").c_str(), "[TLM][base-protocol][dmi]", m_db);
			dmi_trGetHandle = new tx_generator<>("get", *dmi_streamHandle);
			dmi_trInvalidateHandle = new tx_generator<sc_dt::uint64, sc_dt::uint64>(
					"invalidate", *dmi_streamHandle, "start_addr", "end_addr");
		}
	}

	private:
	inline std::string phase2string(const tlm::tlm_phase& p) {
		std::stringstream ss;
		ss << p;
		return ss.str();
	}
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// implementations of functions
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template <typename TYPES>
void tlm_recorder<TYPES>::b_transport(typename TYPES::tlm_payload_type& trans, sc_core::sc_time& delay) {
	typename TYPES::tlm_payload_type* req{nullptr};
	if(!isRecordingBlockingTxEnabled()) {
		fw_port->b_transport(trans, delay);
		return;
	} else if(!b_streamHandle)
		initialize_streams();
	// Get a handle for the new transaction
	tx_handle h = b_trHandle[trans.get_command()]->begin_tx(delay.value());
	tx_handle htim;
	/*************************************************************************
	 * do the timed notification
	 *************************************************************************/
	if(b_streamHandleTimed)
		htim = b_trTimedHandle[trans.get_command()]->begin_tx(sc_core::sc_time_stamp()+delay, par_chld_hndl, h);

	if(registered)
		for(auto& extensionRecording : lwtr4tlm2_extension_registry<TYPES>::inst().get())
			if(extensionRecording) {
				extensionRecording->recordBeginTx(h, trans);
				if(htim.is_valid())
					extensionRecording->recordBeginTx(htim, trans);
			}
	link_pred_ext* preExt = nullptr;

	trans.get_extension(preExt);
	if(preExt == nullptr) { // we are the first recording this transaction
		preExt = new link_pred_ext(h, this);
		if(trans.has_mm())
			trans.set_auto_extension(preExt);
		else
			trans.set_extension(preExt);
	} else {
		h.add_relation(pred_succ_hndl, preExt->txHandle);
	}
	tx_handle preTx{preExt->txHandle};
	preExt->txHandle = h;
	fw_port->b_transport(trans, delay);
	if(preExt && preExt->creator == this) {
		// clean-up the extension if this is the original creator
		trans.set_extension(static_cast<link_pred_ext*>(nullptr));
		if(!trans.has_mm()) {
			delete preExt;
		}
	} else {
		preExt->txHandle = preTx;
	}
	h.record_attribute("trans", trans);
	if(registered)
		for(auto& extensionRecording : lwtr4tlm2_extension_registry<TYPES>::inst().get())
			if(extensionRecording) {
				extensionRecording->recordEndTx(h, trans);
				if(htim.is_active())
					extensionRecording->recordEndTx(htim, trans);
			}
	// End the transaction
	h.end_tx(delay.value());
	// and now the stuff for the timed tx
	if(htim.is_valid()) {
		htim.record_attribute("trans", trans);
		htim.end_tx(::lwtr::no_data(), sc_core::sc_time_stamp()+delay);
	}
}

template <typename TYPES>
tlm::tlm_sync_enum tlm_recorder<TYPES>::nb_transport_fw(typename TYPES::tlm_payload_type& trans,
		typename TYPES::tlm_phase_type& phase,
		sc_core::sc_time& delay) {
	if(!isRecordingNonBlockingTxEnabled())
		return fw_port->nb_transport_fw(trans, phase, delay);
	else if(!nb_streamHandle)
		initialize_streams();
	/*************************************************************************
	 * prepare recording
	 *************************************************************************/
	// Get a handle for the new transaction
	tx_handle h = nb_trHandle[FW]->begin_tx(phase2string(phase));
	link_pred_ext* preExt = nullptr;
	trans.get_extension(preExt);
	if(preExt == nullptr) { // we are the first recording this transaction
		preExt = new link_pred_ext(h, this);
		if(trans.has_mm())
			trans.set_auto_extension(preExt);
		else
			trans.set_extension(preExt);
	} else {
		// link handle if we have a predecessor
		h.add_relation(pred_succ_hndl, preExt->txHandle);
	}
	// update the extension
	if(preExt)
		preExt->txHandle = h;
	h.record_attribute("delay", delay.to_string());
	if(registered)
		for(auto& extensionRecording : lwtr4tlm2_extension_registry<TYPES>::inst().get())
			if(extensionRecording)
				extensionRecording->recordBeginTx(h, trans);
	/*************************************************************************
	 * do the timed notification
	 *************************************************************************/
	if(nb_streamHandleTimed) {
		nb_rec_entry rec;
		rec.tr = mm::get().allocate();
		rec.ph = phase;
		rec.id = reinterpret_cast<uint64_t>(&trans);
		rec.parent = h;
        nb_timed_peq.notify(rec, delay);
	}
	/*************************************************************************
	 * do the access
	 *************************************************************************/
	tlm::tlm_sync_enum status = fw_port->nb_transport_fw(trans, phase, delay);
	/*************************************************************************
	 * handle recording
	 *************************************************************************/
	h.record_attribute("status", status);
	h.record_attribute("delay[return_path]", delay.to_string());
	h.record_attribute("trans", trans);
	if(registered)
		for(auto& extensionRecording : lwtr4tlm2_extension_registry<TYPES>::inst().get())
			if(extensionRecording)
				extensionRecording->recordEndTx(h, trans);
	// get the extension and free the memory if it was mine
	if(status == tlm::TLM_COMPLETED || (status == tlm::TLM_ACCEPTED && phase == tlm::END_RESP)) {
		trans.get_extension(preExt);
		if(preExt && preExt->creator == this) {
			trans.set_extension(static_cast<link_pred_ext*>(nullptr));
			if(!trans.has_mm()) {
				delete preExt;
			}
		}
		/*************************************************************************
		 * do the timed notification if req. finished here
		 *************************************************************************/
		if(nb_streamHandleTimed) {
			nb_rec_entry rec;
			rec.tr = mm::get().allocate();
			rec.ph = phase;
			rec.id = reinterpret_cast<uint64_t>(&trans);
			rec.parent = h;
	        nb_timed_peq.notify(rec, delay);
		}
	} else if(nb_streamHandleTimed && status == tlm::TLM_UPDATED) {
		nb_rec_entry rec;
		rec.tr = mm::get().allocate();
		rec.ph = phase;
		rec.id = reinterpret_cast<uint64_t>(&trans);
		rec.parent = h;
        nb_timed_peq.notify(rec, delay);
	}
	// End the transaction
	nb_trHandle[FW]->end_tx(h, phase2string(phase));
	return status;
}

template <typename TYPES>
tlm::tlm_sync_enum tlm_recorder<TYPES>::nb_transport_bw(typename TYPES::tlm_payload_type& trans,
		typename TYPES::tlm_phase_type& phase,
		sc_core::sc_time& delay) {
	if(!isRecordingNonBlockingTxEnabled())
		return bw_port->nb_transport_bw(trans, phase, delay);
	else if(!nb_streamHandle)
		initialize_streams();
	/*************************************************************************
	 * prepare recording
	 *************************************************************************/
	link_pred_ext* preExt = nullptr;
	trans.get_extension(preExt);
	// sc_assert(preExt != nullptr && "ERROR on backward path");
	// Get a handle for the new transaction
	tx_handle h = nb_trHandle[BW]->begin_tx(phase2string(phase));
	// link handle if we have a predecessor and that's not ourself
	if(preExt) {
		h.add_relation(pred_succ_hndl, preExt->txHandle);
		// and set the extension handle to this transaction
		preExt->txHandle = h;
	}
	h.record_attribute("delay", delay.to_string());
	for(auto& extensionRecording : lwtr4tlm2_extension_registry<TYPES>::inst().get())
		if(extensionRecording)
			extensionRecording->recordBeginTx(h, trans);
	/*************************************************************************
	 * do the timed notification
	 *************************************************************************/
	if(nb_streamHandleTimed) {
		nb_rec_entry rec;
		rec.tr = mm::get().allocate();
		rec.ph = phase;
		rec.id = reinterpret_cast<uint64_t>(&trans);
		rec.parent = h;
        nb_timed_peq.notify(rec, delay);
	}
	/*************************************************************************
	 * do the access
	 *************************************************************************/
	tlm::tlm_sync_enum status = bw_port->nb_transport_bw(trans, phase, delay);
	/*************************************************************************
	 * handle recording
	 *************************************************************************/
	h.record_attribute("status", status);
	h.record_attribute("delay[return_path]", delay.to_string());
	h.record_attribute("trans", trans);
	if(registered)
		for(auto& extensionRecording : lwtr4tlm2_extension_registry<TYPES>::inst().get())
			if(extensionRecording)
				extensionRecording->recordEndTx(h, trans);
	// End the transaction
	nb_trHandle[BW]->end_tx(h, phase2string(phase));
	if(status == tlm::TLM_COMPLETED || (status == tlm::TLM_UPDATED && phase == tlm::END_RESP)) {
		// the transaction is finished
		if(preExt && preExt->creator == this) {
			// clean-up the extension if this is the original creator
			trans.set_extension(static_cast<link_pred_ext*>(nullptr));
			if(!trans.has_mm()) {
				delete preExt;
			}
		}
		/*************************************************************************
		 * do the timed notification if req. finished here
		 *************************************************************************/
		if(nb_streamHandleTimed) {
			nb_rec_entry rec;
			rec.tr = mm::get().allocate();
			rec.ph = phase;
			rec.id = reinterpret_cast<uint64_t>(&trans);
			rec.parent = h;
	        nb_timed_peq.notify(rec, delay);
		}
	}
	return status;
}

template <typename TYPES>
void tlm_recorder<TYPES>::nbtx_cb() {
	auto opt = nb_timed_peq.get_next();
	if(opt) {
		auto& e = opt.get();
		tx_handle h;
		std::unordered_map<uint64_t, tx_handle>::iterator it;
		switch(e.ph) { // Now process outstanding recordings
		case tlm::BEGIN_REQ:
			h = nb_trTimedHandle[REQ]->begin_tx("PARENT_CHILD", e.parent);
			h.record_attribute(*e.tr);
			nbtx_req_handle_map[e.id] = h;
			break;
		case tlm::END_REQ:
			it = nbtx_req_handle_map.find(e.id);
			sc_assert(it != nbtx_req_handle_map.end());
			h = it->second;
			nbtx_req_handle_map.erase(it);
			h.end_tx();
			nbtx_last_req_handle_map[e.id] = h;
			break;
		case tlm::BEGIN_RESP:
			it = nbtx_req_handle_map.find(e.id);
			if(it != nbtx_req_handle_map.end()) {
				h = it->second;
				nbtx_req_handle_map.erase(it);
				h.end_tx();
				nbtx_last_req_handle_map[e.id] = h;
			}
			h = nb_trTimedHandle[RESP]->begin_tx("PARENT_CHILD", e.parent);
			h.record_attribute(*e.tr);
			nbtx_req_handle_map[e.id] = h;
			it = nbtx_last_req_handle_map.find(e.id);
			if(it != nbtx_last_req_handle_map.end()) {
				tx_handle pred = it->second;
				nbtx_last_req_handle_map.erase(it);
				h.add_relation("PREDECESSOR_SUCCESSOR", pred);
			}
			break;
		case tlm::END_RESP:
			it = nbtx_req_handle_map.find(e.id);
			if(it != nbtx_req_handle_map.end()) {
				h = it->second;
				nbtx_req_handle_map.erase(it);
				h.end_tx();
			}
			break;
		default:
			// sc_assert(!"phase not supported!");
			break;
		}
	}
    return;
}

template <typename TYPES>
bool tlm_recorder<TYPES>::get_direct_mem_ptr(typename TYPES::tlm_payload_type& trans, tlm::tlm_dmi& dmi_data) {
	if(!(m_db && enableDmiTracing.get_value()))
		return fw_port->get_direct_mem_ptr(trans, dmi_data);
	else if(!dmi_streamHandle)
		initialize_streams();
	tx_handle h = dmi_trGetHandle->begin_tx();
	bool status = fw_port->get_direct_mem_ptr(trans, dmi_data);
	h.record_attribute("trans", trans);
	h.record_attribute("dmi_data", dmi_data);
	h.end_tx();
	return status;
}
/*! \brief The direct memory interface backward function
 *
 * This type of transaction is just forwarded and not recorded.
 * \param start_addr is the start address of the memory area being invalid
 * \param end_addr is the end address of the memory area being invalid
 */
template <typename TYPES>
void tlm_recorder<TYPES>::invalidate_direct_mem_ptr(sc_dt::uint64 start_addr, sc_dt::uint64 end_addr) {
	if(!(m_db && enableDmiTracing.get_value())) {
		bw_port->invalidate_direct_mem_ptr(start_addr, end_addr);
		return;
	} else if(!dmi_streamHandle)
		initialize_streams();
	tx_handle h = dmi_trInvalidateHandle->begin_tx(start_addr);
	bw_port->invalidate_direct_mem_ptr(start_addr, end_addr);
	dmi_trInvalidateHandle->end_tx(h, end_addr);
	return;
}
/*! \brief The debug transportfunction
 *
 * This type of transaction is just forwarded and not recorded.
 * \param trans is the generic payload of the transaction
 * \return the sync state of the transaction
 */
template <typename TYPES> unsigned int tlm_recorder<TYPES>::transport_dbg(typename TYPES::tlm_payload_type& trans) {
	unsigned int count = fw_port->transport_dbg(trans);
	return count;
}
} // namespece lwtr
} // namespace scc
} // namespace tlm
