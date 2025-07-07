/*******************************************************************************
 * Copyright 2024 MINRES Technologies GmbH
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

#ifndef _TLM_NW_TLM_NETWROK_GP_H_
#define _TLM_NW_TLM_NETWROK_GP_H_

#include <deque>
#include <vector>
#ifdef CWR_SYSTEMC
#include <tlm_h/tlm_generic_payload/tlm_gp.h>
#else
#include <tlm_core/tlm_2/tlm_generic_payload/tlm_array.h>
#include <tlm_core/tlm_2/tlm_generic_payload/tlm_gp.h>
#endif
#include <cstdint>

/**
 * @brief The SystemC Transaction Level Model (TLM) Network TLM utilities.
 *
 * @ingroup tlm-nw
 */
/**@{*/ namespace tlm {
namespace nw {

struct tlm_network_payload_base;

class tlm_base_mm_interface {
public:
    virtual void free(tlm_network_payload_base*) = 0;
    virtual ~tlm_base_mm_interface() {}
};
/**
 * \brief A base class for TLM network payloads.
 *
 * The tlm_network_payload_base class provides a foundation for creating custom TLM network payloads.
 * It includes methods for managing extensions, memory management, and response status.
 *
 */
struct tlm_network_payload_base {
    /**
     * \brief Default constructor.
     *
     * Initializes the tlm_network_payload_base object with default values.
     */
    tlm_network_payload_base()
    : tlm_network_payload_base(nullptr) {}
    /**
     * \brief Constructor with memory management interface.
     *
     * Initializes the tlm_network_payload_base object with a specified memory management interface.
     *
     * @param mm The memory management interface.
     */
    explicit tlm_network_payload_base(tlm_base_mm_interface* mm)
    : m_extensions(max_num_extensions())
    , m_mm(mm)
    , m_ref_count(0) {};
    /**
     * virtual destructor.
     */
    virtual ~tlm_network_payload_base() {
        for(unsigned int i = 0; i < m_extensions.size(); i++)
            if(m_extensions[i])
                m_extensions[i]->free();
    }
    /**
     * \brief Constructor with memory management interface.
     *
     * Initializes the tlm_network_payload_base object with a specified memory management interface.
     *
     * @param mm The memory management interface.
     */
    void reset() { m_extensions.free_entire_cache(); }
    /**
     * \brief Acquires a reference to the payload.
     *
     * Increments the reference count of the payload, indicating that it is being used.
     */
    void acquire() {
        sc_assert(m_mm != 0);
        m_ref_count++;
    }
    /**
     * \brief Releases a reference to the payload.
     *
     * Decrements the reference count of the payload, indicating that it is no longer being used.
     * If the reference count reaches 0, the payload is freed using the memory management interface.
     */
    void release() {
        sc_assert(m_mm != 0 && m_ref_count > 0);
        if(--m_ref_count == 0)
            m_mm->free(this);
    }
    /**
     * \brief Gets the reference count of the payload.
     *
     * Returns the current reference count of the payload.
     *
     * @return The reference count.
     */
    int get_ref_count() const { return m_ref_count; }

    void set_mm(tlm_base_mm_interface* mm) { m_mm = mm; }
    bool has_mm() const { return m_mm != 0; }

    void copy_extensions_from(const tlm_network_payload_base& other);

    void update_extensions_from(const tlm_network_payload_base& other);
    // Free all extensions. Useful when reusing a cloned transaction that doesn't have memory manager.
    // normal and sticky extensions are freed and extension array cleared.
    void free_all_extensions();
    // Stick the pointer to an extension into the vector, return the
    // previous value:
    template <typename T> T* set_extension(T* ext) { return static_cast<T*>(set_extension(T::ID, ext)); }

    // non-templatized version with manual index:
    tlm_extension_base* set_extension(unsigned int index, tlm_extension_base* ext);

    // Stick the pointer to an extension into the vector, return the
    // previous value and schedule its release
    template <typename T> T* set_auto_extension(T* ext) { return static_cast<T*>(set_auto_extension(T::ID, ext)); }

    // non-templatized version with manual index:
    tlm_extension_base* set_auto_extension(unsigned int index, tlm_extension_base* ext);

    // Check for an extension, ext will point to 0 if not present
    template <typename T> void get_extension(T*& ext) const { ext = get_extension<T>(); }
    template <typename T> T* get_extension() const { return static_cast<T*>(get_extension(T::ID)); }
    // Non-templatized version with manual index:
    tlm_extension_base* get_extension(unsigned int index) const;

    // this call just removes the extension from the txn but does not
    // call free() or tells the MM to do so
    // it return false if there was active MM so you are now in an unsafe situation
    // recommended use: when 100% sure there is no MM
    template <typename T> void clear_extension(const T* ext) { clear_extension<T>(); }

    // this call just removes the extension from the txn but does not
    // call free() or tells the MM to do so
    // it return false if there was active MM so you are now in an unsafe situation
    // recommended use: when 100% sure there is no MM
    template <typename T> void clear_extension() { clear_extension(T::ID); }

    // this call removes the extension from the txn and does
    // call free() or tells the MM to do so when the txn is finally done
    // recommended use: when not sure there is no MM
    template <typename T> void release_extension(T* ext) { release_extension<T>(); }

    // this call removes the extension from the txn and does
    // call free() or tells the MM to do so when the txn is finally done
    // recommended use: when not sure there is no MM
    template <typename T> void release_extension() { release_extension(T::ID); }

private:
    // Non-templatized version with manual index
    void clear_extension(unsigned int index);
    // Non-templatized version with manual index
    void release_extension(unsigned int index);

public:
    // Make sure the extension array is large enough. Can be called once by
    // an initiator module (before issuing the first transaction) to make
    // sure that the extension array is of correct size. This is only needed
    // if the initiator cannot guarantee that the generic payload object is
    // allocated after C++ static construction time.
    void resize_extensions();

private:
    tlm::tlm_array<tlm_extension_base*> m_extensions;
    tlm_base_mm_interface* m_mm;
    unsigned int m_ref_count;
};
/**
 * \brief A class for TLM network payloads with support for extensions and memory management.
 *
 * The tlm_network_payload class provides a foundation for creating custom TLM network payloads.
 * It includes methods for managing extensions, memory management, and response status.
 *
 * @tparam CMDENUM The type of command enumeration used by the payload.
 *
 * @author Your Name
 * @date YYYY-MM-DD
 */
template <typename CMDENUM> struct tlm_network_payload : public tlm_network_payload_base {
    /**
     * \brief Default constructor.
     *
     * Initializes the tlm_network_payload object with default values.
     */
    tlm_network_payload();
    /**
     * \brief Constructor with memory management interface.
     *
     * Initializes the tlm_network_payload object with a specified memory management interface.
     *
     * @param mm The memory management interface.
     */
    explicit tlm_network_payload(tlm_base_mm_interface* mm);

    tlm_network_payload(const tlm_network_payload<CMDENUM>& x) = delete;

    tlm_network_payload<CMDENUM>& operator=(const tlm_network_payload<CMDENUM>& x) = delete;

    /**
     * virtual destructor.
     */
    virtual ~tlm_network_payload() = default;
    /**
     * \brief Non-virtual deep-copying of the object.
     *
     * Performs a deep copy of the object, including copying the command, response status, data, and extensions.
     *
     * @param other The object to copy from.
     */
    void deep_copy_from(const tlm_network_payload& other);
    /**
     * \brief Gets the command from the payload.
     *
     * Returns the command stored in the payload.
     *
     * @return The command.
     */
    CMDENUM get_command() const { return m_command; }
    /**
     * \brief Sets the command in the payload.
     *
     * Updates the command stored in the payload.
     *
     * @param command The new command.
     */
    void set_command(const CMDENUM command) { m_command = command; }
    /**
     * \brief Gets the data from the payload.
     *
     * Returns a reference to the data stored in the payload.
     *
     * @return A reference to the data.
     */
    std::vector<uint8_t> const& get_data() const { return m_data; }
    /**
     * \brief Gets the data from the payload.
     *
     * Returns a reference to the data stored in the payload.
     *
     * @return A reference to the data.
     */
    std::vector<uint8_t>& get_data() { return m_data; }
    /**
     * \brief Sets the data in the payload.
     *
     * Updates the data stored in the payload.
     *
     * @param value The new data.
     */
    void set_data(std::vector<uint8_t> const& value) { m_data = value; }
    /**
     * \brief Checks if the response status is OK.
     *
     * Returns true if the response status is OK, indicating a successful transaction.
     *
     * @return True if the response status is OK, false otherwise.
     */
    bool is_response_ok() const { return (m_response_status > 0); }
    /**
     * \brief Checks if the response status is an error.
     *
     * Returns true if the response status is an error, indicating an unsuccessful transaction.
     *
     * @return True if the response status is an error, false otherwise.
     */
    bool is_response_error() const { return (m_response_status <= 0); }
    /**
     * \brief Gets the response status from the payload.
     *
     * Returns the response status stored in the payload.
     *
     * @return The response status.
     */
    tlm_response_status get_response_status() const { return m_response_status; }
    /**
     * \brief Sets the response status in the payload.
     *
     * Updates the response status stored in the payload.
     *
     * @param response_status The new response status.
     */
    void set_response_status(const tlm_response_status response_status) { m_response_status = response_status; }
    /**
     * \brief Gets the response status as a string.
     *
     * Returns the response status stored in the payload as a string.
     *
     * @return The response status as a string.
     */
    std::string get_response_string() const;

    struct gp_mm : public tlm_base_mm_interface {
        tlm_network_payload<CMDENUM>* create() {
            if(pool.size()) {
                auto ret = pool.front();
                pool.pop_front();
                return ret;
            } else
                return new tlm_network_payload<CMDENUM>(this);
        }
        void free(tlm_network_payload_base* gp) override {
            auto t = dynamic_cast<tlm_network_payload<CMDENUM>*>(gp);
            t->free_all_extensions();
            pool.push_back(t);
        }
        ~gp_mm() {
            for(auto n : pool)
                delete n;
        }

    private:
        std::deque<tlm_network_payload<CMDENUM>*> pool;
    };
    /**
     * \brief Creates a new instance of the tlm_network_payload using a thread-local memory manager.
     *
     * Returns a new instance of the tlm_network_payload, using a thread-local memory manager to allocate the object.
     *
     * @return A new instance of the tlm_network_payload.
     */
    static tlm_network_payload<CMDENUM>* create() {
        static thread_local gp_mm mm;
        return mm.create();
    }

protected:
    CMDENUM m_command;
    std::vector<uint8_t> m_data;
    tlm_response_status m_response_status;

public:
};

template <typename CMDENUM>
inline tlm_network_payload<CMDENUM>::tlm_network_payload()
: tlm_network_payload_base()
, m_command(static_cast<CMDENUM>(0))
, m_response_status(TLM_OK_RESPONSE) {}

template <typename CMDENUM>
inline tlm_network_payload<CMDENUM>::tlm_network_payload(tlm_base_mm_interface* mm)
: tlm_network_payload_base(mm)
, m_command(static_cast<CMDENUM>(0))
, m_response_status(TLM_OK_RESPONSE) {}

inline void tlm_network_payload_base::free_all_extensions() {
    m_extensions.free_entire_cache();
    for(unsigned int i = 0; i < m_extensions.size(); i++) {
        if(m_extensions[i]) {
            m_extensions[i]->free();
            m_extensions[i] = 0;
        }
    }
}

template <typename CMDENUM> void tlm_network_payload<CMDENUM>::deep_copy_from(const tlm_network_payload& other) {
    m_command = other.get_command();
    m_response_status = other.get_response_status();
    m_data = other.get_data();
    copy_extensions_from(other);
}

template <typename CMDENUM> std::string tlm_network_payload<CMDENUM>::get_response_string() const {
    switch(m_response_status) {
    case TLM_OK_RESPONSE:
        return "TLM_OK_RESPONSE";
    case TLM_INCOMPLETE_RESPONSE:
        return "TLM_INCOMPLETE_RESPONSE";
    case TLM_GENERIC_ERROR_RESPONSE:
        return "TLM_GENERIC_ERROR_RESPONSE";
    case TLM_ADDRESS_ERROR_RESPONSE:
        return "TLM_ADDRESS_ERROR_RESPONSE";
    case TLM_COMMAND_ERROR_RESPONSE:
        return "TLM_COMMAND_ERROR_RESPONSE";
    case TLM_BURST_ERROR_RESPONSE:
        return "TLM_BURST_ERROR_RESPONSE";
    case TLM_BYTE_ENABLE_ERROR_RESPONSE:
        return "TLM_BYTE_ENABLE_ERROR_RESPONSE";
    }
    return "TLM_UNKNOWN_RESPONSE";
}

inline tlm_extension_base* tlm_network_payload_base::set_extension(unsigned int index, tlm_extension_base* ext) {
    sc_assert(index < m_extensions.size());
    tlm_extension_base* tmp = m_extensions[index];
    m_extensions[index] = ext;
    return tmp;
}

inline tlm_extension_base* tlm_network_payload_base::set_auto_extension(unsigned int index, tlm_extension_base* ext) {
    sc_assert(index < m_extensions.size());
    tlm_extension_base* tmp = m_extensions[index];
    m_extensions[index] = ext;
    if(!tmp)
        m_extensions.insert_in_cache(&m_extensions[index]);
    sc_assert(m_mm != 0);
    return tmp;
}

inline tlm_extension_base* tlm_network_payload_base::get_extension(unsigned int index) const {
    sc_assert(index < m_extensions.size());
    return m_extensions[index];
}

inline void tlm_network_payload_base::clear_extension(unsigned int index) {
    sc_assert(index < m_extensions.size());
    m_extensions[index] = static_cast<tlm_extension_base*>(0);
}

inline void tlm_network_payload_base::release_extension(unsigned int index) {
    sc_assert(index < m_extensions.size());
    if(m_mm) {
        m_extensions.insert_in_cache(&m_extensions[index]);
    } else {
        m_extensions[index]->free();
        m_extensions[index] = static_cast<tlm_extension_base*>(0);
    }
}

inline void tlm_network_payload_base::update_extensions_from(const tlm_network_payload_base& other) {
    // deep copy extensions that are already present
    sc_assert(m_extensions.size() <= other.m_extensions.size());
    for(unsigned int i = 0; i < m_extensions.size(); i++) {
        if(other.m_extensions[i]) { // original has extension i
            if(m_extensions[i]) {   // We have it too. copy.
                m_extensions[i]->copy_from(*other.m_extensions[i]);
            }
        }
    }
}

inline void tlm_network_payload_base::copy_extensions_from(const tlm_network_payload_base& other) {
    // deep copy extensions (sticky and non-sticky)
    if(m_extensions.size() < other.m_extensions.size())
        m_extensions.expand(other.m_extensions.size());
    for(unsigned int i = 0; i < other.m_extensions.size(); i++) {
        if(other.m_extensions[i]) { // original has extension i
            if(!m_extensions[i]) {  // We don't: clone.
                tlm_extension_base* ext = other.m_extensions[i]->clone();
                if(ext) {          // extension may not be clonable.
                    if(has_mm()) { // mm can take care of removing cloned extensions
                        set_auto_extension(i, ext);
                    } else { // no mm, user will call free_all_extensions().
                        set_extension(i, ext);
                    }
                }
            } else { // We already have such extension. Copy original over it.
                m_extensions[i]->copy_from(*other.m_extensions[i]);
            }
        }
    }
}

inline void tlm_network_payload_base::resize_extensions() { m_extensions.expand(max_num_extensions()); }
} // namespace nw
} // namespace tlm
#endif /* _TLM_NW_TLM_NETWROK_GP_H_ */
