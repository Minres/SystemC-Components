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

#ifndef _SCV4TLM_TLM_RECORDING_EXTENSION_H_
#define _SCV4TLM_TLM_RECORDING_EXTENSION_H_

#include <array>
#ifdef HAS_SCV
#include <scv.h>
#else
#include <scv-tr.h>
#endif
#include <tlm>

//! @brief SystemC TLM
namespace tlm {
//! @brief SCC TLM utilities
namespace scc {
//! @brief SCC SCV4TLM classes and functions
namespace scv {
#ifndef HAS_SCV
#ifndef SCVNS
#define SCVNS ::scv_tr::
#endif
#endif

//! transaction relationships
enum tx_rel {
    PARENT_CHILD = 0,     /*!< indicates parent child relationship */
    PREDECESSOR_SUCCESSOR /*!< indicates predecessor successor relationship */
};
//! the string representation of the tx_rel
static std::array<const char*, 2> tx_rel_str = {{"PARENT/CHILD", "PRED/SUCC"}};
/*! \brief cast the tx_rel enum to a string
 *
 * \param rel is the relationship enum
 */
inline const char* rel_str(tx_rel rel) { return (tx_rel_str[rel]); }
/**
 * @class tlm_recording_extension
 * @brief generic payload extension class holding the handle of the last recorded SCV transaction
 *
 * This extension is been used in the \ref tlm2_recorder. The recorder
 * stores the handle to the generated SCV transaction and
 * forwards it along with the generic payload. If the recorder finds an extension
 * containing a valid handle it links the generated
 * SCV transaction to the found one using the \ref PREDECESSOR_SUCCESSOR relationship
 */
class tlm_recording_extension : public tlm::tlm_extension<tlm_recording_extension> {
public:
    /*! \brief clone the given extension and duplicate the SCV transaction handle.
     *
     */
    virtual tlm_extension_base* clone() const {
        tlm_recording_extension* t = new tlm_recording_extension(this->txHandle, this->creator);
        return t;
    }
    /*! \brief copy data between extensions.
     *
     * \param from is the source extension.
     */
    virtual void copy_from(tlm_extension_base const& from) {
        txHandle = static_cast<tlm_recording_extension const&>(from).txHandle;
        creator = static_cast<tlm_recording_extension const&>(from).creator;
    }
    /*! \brief constructor storing the handle of the transaction and the owner of
     * this extension
     *
     * \param handle is the handle of the created SCV transaction.
     * \param creator_ is the pointer to the owner of this extension (usually an
     * instance of scv_tlm2_recorder).
     */
    tlm_recording_extension(SCVNS scv_tr_handle handle, void* creator_)
    : txHandle(handle)
    , creator(creator_) {}
    /*! \brief accessor to the owner, the property is read only.
     *
     */
    void* get_creator() { return creator; }
    /*! \brief accessor to the SCV transaction handle.
     *
     */
    SCVNS scv_tr_handle txHandle;

private:
    //! the owner of this transaction
    void* creator;
};
} // namespace scv
} // namespace scc
} // namespace tlm

#endif /* TLM_RECORDING_EXTENSION_H_ */
