/*******************************************************************************
 * Copyright 2016, 2020 MINRES Technologies GmbH
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

#ifndef TLM_RECORDER_REGISTRY_H_
#define TLM_RECORDER_REGISTRY_H_

#include <memory>
#include <tlm>
#include <unordered_set>
#include <util/delegate.h>
#ifdef HAS_SCV
#include <scv.h>
#ifndef SCVNS
#define SCVNS
#endif
#else
#include <scv-tr.h>
#ifndef SCVNS
#define SCVNS ::scv_tr::
#endif
#endif
#include <tlm>

//! @brief SystemC TLM
namespace tlm {
//! @brief SCC TLM utilities
namespace scc {
//! @brief SCC SCV4TLM classes and functions
namespace scv {
struct tlm_extension_record_if {
    /*! \brief recording attributes in extensions at the beginning, it is intended
     * to be overload as it does nothing
     *
     */
    util::delegate<void(SCVNS scv_tr_handle&, tlm::tlm_extension_base*, std::string const&)> recordBegin;
    /*! \brief recording attributes in extensions at the end, it is intended to be
     * overload as it does nothing
     *
     */
    util::delegate<void(SCVNS scv_tr_handle&, tlm::tlm_extension_base*, std::string const&)> recordEnd;

    virtual ~tlm_extension_record_if() = default;
};
struct tlm_extension_record_registry {
    using recording_vector = std::vector<std::unique_ptr<tlm_extension_record_if>>;

    static tlm_extension_record_registry& get() {
        static tlm_extension_record_registry reg;
        return reg;
    }

    void register_ext_rec(size_t id, std::unique_ptr<tlm_extension_record_if> ext) {
        if(id == 0)
            return;
        if(id >= ext_rec.size())
            ext_rec.resize(id + 1);
        ext_rec.at(id) = std::move(ext);
    }

    bool is_ext_registered(size_t id) { return id == 0 || (id < ext_rec.size() && ext_rec[id]); }

    size_t size() const { return ext_rec.size(); }

    typename recording_vector::iterator begin() { return ext_rec.begin(); }

    typename recording_vector::iterator end() { return ext_rec.end(); }

    tlm_extension_record_if* operator[](size_t n) {
#ifdef NDEBUG
        return ext_rec[n].get();
#else
        return ext_rec.at(n).get();
#endif
    }
    /*! \brief recording attributes in extensions at the beginning
     *
     */
    inline void recordBeginTx(size_t id, SCVNS scv_tr_handle& handle, tlm::tlm_extension_base* ext, std::string const& prefix = "") {
        assert(ext_rec.size() > id);
        if(ext && ext_rec[id] && ext_rec[id]->recordBegin)
            ext_rec[id]->recordBegin(handle, ext, prefix);
    }
    /*! \brief recording attributes in extensions at the end
     *
     */
    inline void recordEndTx(size_t id, SCVNS scv_tr_handle& handle, tlm::tlm_extension_base* ext, std::string const& prefix = "") {
        assert(ext_rec.size() > id);
        if(ext && ext_rec[id] && ext_rec[id]->recordEnd)
            ext_rec[id]->recordEnd(handle, ext, prefix);
    }

private:
    tlm_extension_record_registry() = default;
    ~tlm_extension_record_registry() = default;

    recording_vector ext_rec{};
};

/*! \brief The TLM transaction extensions recorder interface
 *
 * This interface is used by the TLM transaction recorder. It can be used to
 * register custom recorder functionality
 * to also record the payload extensions
 */
template <typename TYPES = tlm::tlm_base_protocol_types> struct tlm_extensions_recording_if {
public:
    using record_tx_fct = util::delegate<void(SCVNS scv_tr_handle&, typename TYPES::tlm_payload_type&)>;
    /*! \brief recording attributes in extensions at the beginning, it is intended
     * to be overload as it does nothing
     *
     */
    void recordBeginTx(SCVNS scv_tr_handle& h, typename TYPES::tlm_payload_type& trans) {
        if(recordBegin)
            recordBegin(h, trans);
    }
    /*! \brief recording attributes in extensions at the end, it is intended to be
     * overload as it does nothing
     *
     */
    void recordEndTx(SCVNS scv_tr_handle& h, typename TYPES::tlm_payload_type& trans) {
        if(recordEnd)
            recordEnd(h, trans);
    }

protected:
    record_tx_fct recordBegin;
    record_tx_fct recordEnd;
};

/*! \brief The TLM transaction extensions recorder registry
 *
 * This registry is used by the TLM transaction recorder. It can be used to
 * register custom recorder functionality
 * to also record the payload extensions
 */
template <typename TYPES = tlm::tlm_base_protocol_types> class tlm_extension_recording_registry {
public:
    using recording_vector = std::vector<std::unique_ptr<tlm_extensions_recording_if<TYPES>>>;
    static tlm_extension_recording_registry& get() {
        static tlm_extension_recording_registry reg;
        return reg;
    }
    void register_ext_rec(size_t id, std::unique_ptr<tlm_extensions_recording_if<TYPES>> ext) {
        ext_rec.push_back(std::move(ext));
        registered_ids.insert(id);
    }

    bool is_ext_registered(size_t id) { return registered_ids.count(id); }

    typename recording_vector::iterator begin() { return ext_rec.begin(); }

    typename recording_vector::iterator end() { return ext_rec.end(); }

private:
    tlm_extension_recording_registry() = default;
    recording_vector ext_rec{};
    std::unordered_set<size_t> registered_ids;
};

} // namespace scv
} // namespace scc
} // namespace tlm
#endif /* TLM_RECORDER_REGISTRY_H_ */
