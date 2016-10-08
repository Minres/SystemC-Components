/*
 * tlm2_tx_record_extension.h
 *
 *  Created on: 07.11.2015
 *      Author: eyck
 */

#ifndef TLM_RECORDING_EXTENSION_H_
#define TLM_RECORDING_EXTENSION_H_

#include <scv.h>

namespace scv4tlm {

//! transaction relationships
enum tx_rel {
	PARENT_CHILD = 0, /*!< indicates parent child relationship */
	PREDECESSOR_SUCCESSOR /*!< indicates predecessor successor relationship */
};
//! the string representation of the tx_rel
static char const* tx_rel_str[] = { "PARENT/CHILD", "PRED/SUCC" };
/*! \brief cast the tx_rel enum to a string
 *
 * \param tc_rel is the relationship enum
 */
inline const char* rel_str(tx_rel rel) {
	return (tx_rel_str[rel]);
}

/*! \brief generic payload extension class holding the handle of the last recorded SCV transaction
 *
 * This extension is been used in the \ref scv_tlm2_recorder. The recorder stores the handle to the generated SCV transaction and
 * forwrds it along with the generic payload. If the recorder finds an extension containing a valid handle it links the generated
 * SCV transdaction to the found one using the \ref PREDECESSOR releationship
 */
struct tlm_recording_extension: public tlm::tlm_extension<tlm_recording_extension> {
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
		txHandle = static_cast<tlm_recording_extension const &>(from).txHandle;
		creator = static_cast<tlm_recording_extension const &>(from).creator;
	}
	/*! \brief constructor storing the handle of the transaction and the owner of this extension
	 *
	 * \param handle is the handle of the created SCV transaction.
	 * \param creator_ is the pointer to the owner of this extension (usually an instance of scv_tlm2_recorder).
	 */
	tlm_recording_extension(scv_tr_handle handle, void* creator_) :
			txHandle(handle), creator(creator_) {
	}
	/*! \brief accessor to the owner, the property is read only.
	 *
	 */
	void* get_creator() {
		return creator;
	}
	/*! \brief accessor to the SCV transaction handle.
	 *
	 */
	scv_tr_handle txHandle;
private:
	//! the owner of this transaction
	void* creator;
};

}

#endif /* TLM_RECORDING_EXTENSION_H_ */
