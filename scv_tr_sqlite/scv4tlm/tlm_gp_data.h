/*
 * tlm_gp_data.h
 *
 *  Created on: 07.11.2015
 *      Author: eyck
 */

#ifndef TLM_GP_DATA_H_
#define TLM_GP_DATA_H_

#include <tlm>
#include <assert.h>

namespace scv4tlm {

struct tlm_gp_data {
	//---------------
	// Constructors
	//---------------

	// Default constructor
	tlm_gp_data()
	: address(0)
	, command(tlm::TLM_IGNORE_COMMAND)
	, data(0)
	, data_length(0)
	, response_status(tlm::TLM_INCOMPLETE_RESPONSE)
	, dmi(false)
	, byte_enable(0)
	, byte_enable_length(0)
	, streaming_width(0)
	, gp_option(tlm::TLM_MIN_PAYLOAD)
	, m_extensions(tlm::max_num_extensions())
	, m_ref_count(0)
	{
	}

	explicit tlm_gp_data(tlm::tlm_mm_interface* mm)
	: address(0)
	, command(tlm::TLM_IGNORE_COMMAND)
	, data(0)
	, data_length(0)
	, response_status(tlm::TLM_INCOMPLETE_RESPONSE)
	, dmi(false)
	, byte_enable(0)
	, byte_enable_length(0)
	, streaming_width(0)
	, gp_option(tlm::TLM_MIN_PAYLOAD)
	, m_extensions(tlm::max_num_extensions())
	, m_ref_count(0) {
	}

	int get_ref_count() const {
		return m_ref_count;
	}

	void reset() {
		//should the other members be reset too?
		gp_option = tlm::TLM_MIN_PAYLOAD;
		m_extensions.free_entire_cache();
	}
	;

	tlm_gp_data(const tlm::tlm_generic_payload& x)
	: address(x.get_address())
	, command(x.get_command())
	, data(x.get_data_ptr())
	, data_length(x.get_data_length())
	, response_status(x.get_response_status())
	, dmi(x.is_dmi_allowed())
	, byte_enable(x.get_byte_enable_ptr())
	, byte_enable_length(x.get_byte_enable_length())
	, streaming_width(x.get_streaming_width())
	, gp_option(x.get_gp_option())
	, m_extensions(tlm::max_num_extensions())
	, m_ref_count(0)
	{
	}
private:
	//disabled copy ctor and assignment operator.
	// Copy constructor
	tlm_gp_data(const tlm_gp_data& x)
	: address(x.get_address())
	, command(x.get_command())
	, data(x.get_data_ptr())
	, data_length(x.get_data_length())
	, response_status(x.get_response_status())
	, dmi(x.is_dmi_allowed())
	, byte_enable(x.get_byte_enable_ptr())
	, byte_enable_length(x.get_byte_enable_length())
	, streaming_width(x.get_streaming_width())
	, gp_option(x.gp_option)
	, m_extensions(tlm::max_num_extensions())
	, m_ref_count(0)
	{
	}
public:
	// Assignment operator needed for SCV introspection
	tlm_gp_data& operator=(const tlm_gp_data& x) {
		command = x.get_command();
		address = x.get_address();
		data = x.get_data_ptr();
		data_length = x.get_data_length();
		response_status = x.get_response_status();
		byte_enable = x.get_byte_enable_ptr();
		byte_enable_length = x.get_byte_enable_length();
		streaming_width = x.get_streaming_width();
		gp_option = x.get_gp_option();
		dmi = x.is_dmi_allowed();

		// extension copy: all extension arrays must be of equal size by
		// construction (i.e. it must either be constructed after C++
		// static construction time, or the resize_extensions() method must
		// have been called prior to using the object)
		for (unsigned int i = 0; i < m_extensions.size(); i++) {
			m_extensions[i] = x.get_extension(i);
		}
		return (*this);
	}

	// non-virtual deep-copying of the object
	void deep_copy_from(const scv4tlm::tlm_gp_data & other) {
		command = other.get_command();
		address = other.get_address();
		data_length = other.get_data_length();
		response_status = other.get_response_status();
		byte_enable_length = other.get_byte_enable_length();
		streaming_width = other.get_streaming_width();
		gp_option = other.get_gp_option();
		dmi = other.is_dmi_allowed();

		// deep copy data
		// there must be enough space in the target transaction!
		if (data && other.get_data_ptr()) {
			memcpy(data, other.get_data_ptr(), data_length);
		}
		// deep copy byte enables
		// there must be enough space in the target transaction!
		if (byte_enable && other.get_byte_enable_ptr()) {
			memcpy(byte_enable, other.get_byte_enable_ptr(), byte_enable_length);
		}
		// deep copy extensions (sticky and non-sticky)
		for (unsigned int i = 0; i < other.m_extensions.size(); i++) {
			if (other.get_extension(i)) {                       //original has extension i
				if (!m_extensions[i]) {                   //We don't: clone.
					tlm::tlm_extension_base *ext = other.get_extension(i)->clone();
					if (ext)     //extension may not be clonable.
					{
						set_extension(i, ext);
					}
				} else {                   //We already have such extension. Copy original over it.
					m_extensions[i]->copy_from(*other.get_extension(i));
				}
			}
		}
	}

	// To update the state of the original generic payload from a deep copy
	// Assumes that "other" was created from the original by calling deep_copy_from
	// Argument use_byte_enable_on_read determines whether to use or ignores byte enables
	// when copying back the data array on a read command

	void update_original_from(const tlm::tlm_generic_payload & other, bool use_byte_enable_on_read = true) {
		// Copy back extensions that are present on the original
		// update_extensions_from(other);

		// Copy back the response status and DMI hint attributes
		response_status = other.get_response_status();
		dmi = other.is_dmi_allowed();

		// Copy back the data array for a read command only
		// deep_copy_from allowed null pointers, and so will we
		// We assume the arrays are the same size
		// We test for equal pointers in case the original and the copy share the same array

		if (is_read() && data && other.get_data_ptr() && data != other.get_data_ptr()) {
			if (byte_enable && use_byte_enable_on_read) {
				if (byte_enable_length == 8 && data_length % 8 == 0) {
					// Optimized implementation copies 64-bit words by masking
					for (unsigned int i = 0; i < data_length; i += 8) {
						typedef sc_dt::uint64* u;
						*reinterpret_cast<u>(&data[i]) &= ~*reinterpret_cast<u>(byte_enable);
						*reinterpret_cast<u>(&data[i]) |= *reinterpret_cast<u>(&other.get_data_ptr()[i])
										& *reinterpret_cast<u>(byte_enable);
					}
				} else if (byte_enable_length == 4 && data_length % 4 == 0) {
					// Optimized implementation copies 32-bit words by masking
					for (unsigned int i = 0; i < data_length; i += 4) {
						typedef unsigned int* u;
						*reinterpret_cast<u>(&data[i]) &= ~*reinterpret_cast<u>(byte_enable);
						*reinterpret_cast<u>(&data[i]) |= *reinterpret_cast<u>(&other.get_data_ptr()[i])
										& *reinterpret_cast<u>(byte_enable);
					}
				} else
					// Unoptimized implementation
					for (unsigned int i = 0; i < data_length; i++)
						if (byte_enable[i % byte_enable_length])
							data[i] = other.get_data_ptr()[i];
			} else
				memcpy(data, other.get_data_ptr(), data_length);
		}
	}

	/*
	void update_extensions_from(const tlm::tlm_generic_payload & other) {
		// deep copy extensions that are already present
		for (unsigned int i = 0; i < tlm::max_num_extensions(); i++) {
			if (other.get_extension(i)) {                       //original has extension i
				if (m_extensions[i]) {                   //We have it too. copy.
					m_extensions[i]->copy_from(*other.get_extension(i));
				}
			}
		}
	}
	 */

	// Free all extensions. Useful when reusing a cloned transaction that doesn't have memory manager.
	// normal and sticky extensions are freed and extension array cleared.
	void free_all_extensions() {
		m_extensions.free_entire_cache();
		for (unsigned int i = 0; i < m_extensions.size(); i++) {
			if (m_extensions[i]) {
				m_extensions[i]->free();
				m_extensions[i] = 0;
			}
		}
	}
	//--------------
	// Destructor
	//--------------
	virtual ~tlm_gp_data() {
		for(unsigned int i=0; i<m_extensions.size(); i++)
			if(m_extensions[i]) m_extensions[i]->free();
	}

	//----------------
	// API (including setters & getters)
	//---------------

	// Command related method
	bool is_read() const {
		return (command == tlm::TLM_READ_COMMAND);
	}
	void set_read() {
		command = tlm::TLM_READ_COMMAND;
	}
	bool is_write() const {
		return (command == tlm::TLM_WRITE_COMMAND);
	}
	void set_write() {
		command = tlm::TLM_WRITE_COMMAND;
	}
	tlm::tlm_command get_command() const {
		return command;
	}
	void set_command(const tlm::tlm_command command) {
		this->command = command;
	}

	// Address related methods
	sc_dt::uint64 get_address() const {
		return address;
	}
	void set_address(const sc_dt::uint64 address) {
		this->address = address;
	}

	// Data related methods
	unsigned char* get_data_ptr() const {
		return data;
	}
	void set_data_ptr(unsigned char* data) {
		this->data = data;
	}

	// Transaction length (in bytes) related methods
	unsigned int get_data_length() const {
		return data_length;
	}
	void set_data_length(const unsigned int length) {
		data_length = length;
	}

	// Response status related methods
	bool is_response_ok() const {
		return (response_status > 0);
	}
	bool is_response_error() const {
		return (response_status <= 0);
	}
	tlm::tlm_response_status get_response_status() const {
		return response_status;
	}
	void set_response_status(const tlm::tlm_response_status response_status) {
		this->response_status = response_status;
	}
	std::string get_response_string() const {
		switch (response_status) {
		case tlm::TLM_OK_RESPONSE:
			return "TLM_OK_RESPONSE";
		case tlm::TLM_INCOMPLETE_RESPONSE:
			return "TLM_INCOMPLETE_RESPONSE";
		case tlm::TLM_GENERIC_ERROR_RESPONSE:
			return "TLM_GENERIC_ERROR_RESPONSE";
		case tlm::TLM_ADDRESS_ERROR_RESPONSE:
			return "TLM_ADDRESS_ERROR_RESPONSE";
		case tlm::TLM_COMMAND_ERROR_RESPONSE:
			return "TLM_COMMAND_ERROR_RESPONSE";
		case tlm::TLM_BURST_ERROR_RESPONSE:
			return "TLM_BURST_ERROR_RESPONSE";
		case tlm::TLM_BYTE_ENABLE_ERROR_RESPONSE:
			return "TLM_BYTE_ENABLE_ERROR_RESPONSE";
		}
		return "TLM_UNKNOWN_RESPONSE";
	}

	// Streaming related methods
	unsigned int get_streaming_width() const {
		return streaming_width;
	}
	void set_streaming_width(const unsigned int streaming_width) {
		this->streaming_width = streaming_width;
	}

	// Byte enable related methods
	unsigned char* get_byte_enable_ptr() const {
		return byte_enable;
	}
	void set_byte_enable_ptr(unsigned char* byte_enable) {
		this->byte_enable = byte_enable;
	}
	unsigned int get_byte_enable_length() const {
		return byte_enable_length;
	}
	void set_byte_enable_length(const unsigned int byte_enable_length) {
		this->byte_enable_length = byte_enable_length;
	}

	// This is the "DMI-hint" a slave can set this to true if it
	// wants to indicate that a DMI request would be supported:
	void set_dmi_allowed(bool dmi_allowed) {
		dmi = dmi_allowed;
	}
	bool is_dmi_allowed() const {
		return dmi;
	}

	// Use full set of attributes in DMI/debug?
	tlm::tlm_gp_option get_gp_option() const {
		return gp_option;
	}
	void set_gp_option(const tlm::tlm_gp_option gp_opt) {
		gp_option = gp_opt;
	}

public:

	/* --------------------------------------------------------------------- */
	/* Generic Payload attributes:                                           */
	/* --------------------------------------------------------------------- */
	/* - m_command         : Type of transaction. Three values supported:    */
	/*                       - TLM_WRITE_COMMAND                             */
	/*                       - TLM_READ_COMMAND                              */
	/*                       - TLM_IGNORE_COMMAND                            */
	/* - m_address         : Transaction base address (byte-addressing).     */
	/* - m_data            : When m_command = TLM_WRITE_COMMAND contains a   */
	/*                       pointer to the data to be written in the target.*/
	/*                       When m_command = TLM_READ_COMMAND contains a    */
	/*                       pointer where to copy the data read from the    */
	/*                       target.                                         */
	/* - m_length          : Total number of bytes of the transaction.       */
	/* - m_response_status : This attribute indicates whether an error has   */
	/*                       occurred during the transaction.                */
	/*                       Values supported are:                           */
	/*                       - TLM_OK_RESP                                   */
	/*                       - TLM_INCOMPLETE_RESP                           */
	/*                       - TLM_GENERIC_ERROR_RESP                        */
	/*                       - TLM_ADDRESS_ERROR_RESP                        */
	/*                       - TLM_COMMAND_ERROR_RESP                        */
	/*                       - TLM_BURST_ERROR_RESP                          */
	/*                       - TLM_BYTE_ENABLE_ERROR_RESP                    */
	/*                                                                       */
	/* - m_byte_enable     : It can be used to create burst transfers where  */
	/*                    the address increment between each beat is greater */
	/*                    than the word length of each beat, or to place     */
	/*                    words in selected byte lanes of a bus.             */
	/* - m_byte_enable_length : For a read or a write command, the target    */
	/*                    interpret the byte enable length attribute as the  */
	/*                    number of elements in the bytes enable array.      */
	/* - m_streaming_width  :                                                */
	/* --------------------------------------------------------------------- */

	sc_dt::uint64 address;
	tlm::tlm_command command;
	unsigned char* data;
	unsigned int data_length;
	tlm::tlm_response_status response_status;
	bool dmi;
	unsigned char* byte_enable;
	unsigned int byte_enable_length;
	unsigned int streaming_width;
	tlm::tlm_gp_option gp_option;

public:

	/* --------------------------------------------------------------------- */
	/* Dynamic extension mechanism:                                          */
	/* --------------------------------------------------------------------- */
	/* The extension mechanism is intended to enable initiator modules to    */
	/* optionally and transparently add data fields to the                   */
	/* tlm_generic_payload. Target modules are free to check for extensions  */
	/* and may or may not react to the data in the extension fields. The     */
	/* definition of the extensions' semantics is solely in the              */
	/* responsibility of the user.                                           */
	/*                                                                       */
	/* The following rules apply:                                            */
	/*                                                                       */
	/* - Every extension class must be derived from tlm_extension, e.g.:     */
	/*     class my_extension : public tlm_extension<my_extension> { ... }   */
	/*                                                                       */
	/* - A tlm_generic_payload object should be constructed after C++        */
	/*   static initialization time. This way it is guaranteed that the      */
	/*   extension array is of sufficient size to hold all possible          */
	/*   extensions. Alternatively, the initiator module can enforce a valid */
	/*   extension array size by calling the resize_extensions() method      */
	/*   once before the first transaction with the payload object is        */
	/*   initiated.                                                          */
	/*                                                                       */
	/* - Initiators should use the the set_extension(e) or clear_extension(e)*/
	/*   methods for manipulating the extension array. The type of the       */
	/*   argument must be a pointer to the specific registered extension     */
	/*   type (my_extension in the above example) and is used to             */
	/*   automatically locate the appropriate index in the array.            */
	/*                                                                       */
	/* - Targets can check for a specific extension by calling               */
	/*   get_extension(e). e will point to zero if the extension is not      */
	/*   present.                                                            */
	/*                                                                       */
	/* --------------------------------------------------------------------- */

	// Stick the pointer to an extension into the vector, return the
	// previous value:
	template<typename T> T* set_extension(T* ext) {
		return static_cast<T*>(set_extension(T::ID, ext));
	}

	// non-templatized version with manual index:
	tlm::tlm_extension_base* set_extension(unsigned int index, tlm::tlm_extension_base* ext) {
		tlm::tlm_extension_base* tmp = m_extensions[index];
		m_extensions[index] = ext;
		return tmp;
	}

	// Check for an extension, ext will point to 0 if not present
	template<typename T> void get_extension(T*& ext) const {
		ext = get_extension<T>();
	}
	template<typename T> T* get_extension() const {
		return static_cast<T*>(get_extension(T::ID));
	}
	// Non-templatized version with manual index:
	tlm::tlm_extension_base* get_extension(unsigned int index) const {
		return m_extensions[index];
	}

	//this call just removes the extension from the txn but does not
	// call free() or tells the MM to do so
	// it return false if there was active MM so you are now in an unsafe situation
	// recommended use: when 100% sure there is no MM
	template<typename T> void clear_extension(const T* ext) {
		clear_extension<T>();
	}

	//this call just removes the extension from the txn but does not
	// call free() or tells the MM to do so
	// it return false if there was active MM so you are now in an unsafe situation
	// recommended use: when 100% sure there is no MM
	template<typename T> void clear_extension() {
		clear_extension(T::ID);
	}

	//this call removes the extension from the txn and does
	// call free() or tells the MM to do so when the txn is finally done
	// recommended use: when not sure there is no MM
	template<typename T> void release_extension(T* ext) {
		release_extension<T>();
	}

	//this call removes the extension from the txn and does
	// call free() or tells the MM to do so when the txn is finally done
	// recommended use: when not sure there is no MM
	template<typename T> void release_extension() {
		release_extension(T::ID);
	}

private:
	// Non-templatized version with manual index
	void clear_extension(unsigned int index) {
		m_extensions[index] = static_cast<tlm::tlm_extension_base*>(0);
	}
	// Non-templatized version with manual index
	void release_extension(unsigned int index) {
		m_extensions[index]->free();
		m_extensions[index] = static_cast<tlm::tlm_extension_base*>(0);
	}

public:
	// Make sure the extension array is large enough. Can be called once by
	// an initiator module (before issuing the first transaction) to make
	// sure that the extension array is of correct size. This is only needed
	// if the initiator cannot guarantee that the generic payload object is
	// allocated after C++ static construction time.
	void resize_extensions() {
		m_extensions.expand(tlm::max_num_extensions());
	}

private:
	tlm::tlm_array<tlm::tlm_extension_base*> m_extensions;
	unsigned int m_ref_count;
};


struct tlm_dmi_data {
	tlm_dmi_data()
	:dmi_ptr(0)
	,dmi_start_address(0)
	,dmi_end_address(0)
	,dmi_access(tlm::tlm_dmi::DMI_ACCESS_NONE)
	,dmi_read_latency(0)
	,dmi_write_latency(0)
	{}

	tlm_dmi_data(tlm::tlm_dmi& dmi_data)
	:dmi_ptr(dmi_data.get_dmi_ptr())
	,dmi_start_address(dmi_data.get_start_address())
	,dmi_end_address(dmi_data.get_end_address())
	,dmi_access(dmi_data.get_granted_access())
	,dmi_read_latency(dmi_data.get_read_latency().value())
	,dmi_write_latency(dmi_data.get_write_latency().value())
	{}
	//--------------
	// Destructor
	//--------------
	virtual ~tlm_dmi_data() {}

	unsigned char*   dmi_ptr;
	sc_dt::uint64    dmi_start_address;
	sc_dt::uint64    dmi_end_address;
	tlm::tlm_dmi::dmi_access_e     dmi_access;
	sc_dt::uint64 dmi_read_latency;
	sc_dt::uint64 dmi_write_latency;

};


}
#endif /* TLM_GP_DATA_H_ */
