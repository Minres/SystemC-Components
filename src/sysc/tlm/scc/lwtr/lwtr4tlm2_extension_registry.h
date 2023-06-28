/*******************************************************************************
 * Copyright 2022 MINRES Technologies GmbH
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

#ifndef _TLM_SCC_LWTR_LWTR4TLM2_EXTENSION__REGISTRY_H
#define _TLM_SCC_LWTR_LWTR4TLM2_EXTENSION__REGISTRY_H

#include "lwtr4tlm2.h"

//! @brief SystemC TLM
namespace tlm {
//! @brief SCC TLM utilities
namespace scc {
//! @brief SCC SCV4TLM classes and functions
namespace lwtr {
/*! \brief The TLM transaction extensions recorder interface
 *
 * This interface is used by the TLM transaction recorder. It can be used to
 * register custom recorder functionality
 * to also record the payload extensions
 */
template <typename TYPES = tlm::tlm_base_protocol_types> class lwtr4tlm2_extension_registry_if {
public:
    /*! \brief recording attributes in extensions at the beginning, it is intended
     * to be overload as it does nothing
     *
     */
    virtual void recordBeginTx(::lwtr::tx_handle& handle, typename TYPES::tlm_payload_type& trans) = 0;

    /*! \brief recording attributes in extensions at the end, it is intended to be
     * overload as it does nothing
     *
     */
    virtual void recordEndTx(::lwtr::tx_handle& handle, typename TYPES::tlm_payload_type& trans) = 0;

    virtual ~lwtr4tlm2_extension_registry_if() = default;
};

/*! \brief The TLM transaction extensions recorder registry
 *
 * This registry is used by the TLM transaction recorder. It can be used to
 * register custom recorder functionality
 * to also record the payload extensions
 */
template <typename TYPES = tlm::tlm_base_protocol_types> class lwtr4tlm2_extension_registry {
public:
    static lwtr4tlm2_extension_registry& inst() {
        static lwtr4tlm2_extension_registry reg;
        return reg;
    }

    void register_ext_rec(size_t id, lwtr4tlm2_extension_registry_if<TYPES>* ext) {
        if(id == 0)
            return;
        if(id >= ext_rec.size())
            ext_rec.resize(id + 1);
        if(ext_rec[id])
            delete ext_rec[id];
        ext_rec[id] = ext;
    }

    const std::vector<lwtr4tlm2_extension_registry_if<TYPES>*>& get() { return ext_rec; }

    inline void recordBeginTx(size_t id, ::lwtr::tx_handle& handle, typename TYPES::tlm_payload_type& trans) {
        if(ext_rec.size() > id && ext_rec[id])
            ext_rec[id]->recordBeginTx(handle, trans);
    }

    /*! \brief recording attributes in extensions at the end, it is intended to be
     * overload as it does nothing
     *
     */
    inline void recordEndTx(size_t id, ::lwtr::tx_handle& handle, typename TYPES::tlm_payload_type& trans) {
        if(ext_rec.size() > id && ext_rec[id])
            ext_rec[id]->recordEndTx(handle, trans);
    }

private:
    lwtr4tlm2_extension_registry() = default;
    ~lwtr4tlm2_extension_registry() {
        for(auto& ext : ext_rec)
            delete(ext);
    }
    std::vector<lwtr4tlm2_extension_registry_if<TYPES>*> ext_rec{};
};

} // namespace lwtr
} // namespace scc
} // namespace tlm
#endif /* _TLM_SCC_LWTR_LWTR4TLM2_EXTENSION__REGISTRY_H */
