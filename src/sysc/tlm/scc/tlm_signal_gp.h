/*******************************************************************************
 * Copyright 2018-2022 MINRES Technologies GmbH
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

#ifndef _TLM_TLM_SIGNAL_GP_H_
#define _TLM_TLM_SIGNAL_GP_H_

#include <deque>
#ifdef CWR_SYSTEMC
#include <tlm_h/tlm_generic_payload/tlm_gp.h>
#else
#include <tlm_core/tlm_2/tlm_generic_payload/tlm_gp.h>
#endif

//! @brief SystemC TLM
namespace tlm {
//! @brief SCC TLM utilities
namespace scc {

struct tlm_generic_payload_base;

class tlm_base_mm_interface {
public:
    virtual void free(tlm_generic_payload_base*) = 0;
    virtual ~tlm_base_mm_interface() {}
};

struct tlm_generic_payload_base {
    tlm_generic_payload_base()
    : tlm_generic_payload_base(nullptr) {}

    explicit tlm_generic_payload_base(tlm_base_mm_interface* mm)
    : m_extensions(max_num_extensions())
    , m_mm(mm)
    , m_ref_count(0){};

    virtual ~tlm_generic_payload_base() {
        for(unsigned int i = 0; i < m_extensions.size(); i++)
            if(m_extensions[i])
                m_extensions[i]->free();
    }

    void reset() { m_extensions.free_entire_cache(); }

    void acquire() {
        sc_assert(m_mm != 0);
        m_ref_count++;
    }

    void release() {
        sc_assert(m_mm != 0 && m_ref_count > 0);
        if(--m_ref_count == 0)
            m_mm->free(this);
    }

    int get_ref_count() const { return m_ref_count; }

    void set_mm(tlm_base_mm_interface* mm) { m_mm = mm; }
    bool has_mm() const { return m_mm != 0; }

    void copy_extensions_from(const tlm_generic_payload_base& other);

    void update_extensions_from(const tlm_generic_payload_base& other);
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
    tlm_array<tlm_extension_base*> m_extensions;
    tlm_base_mm_interface* m_mm;
    unsigned int m_ref_count;
};

template <typename SIG = bool> struct tlm_signal_gp : public tlm_generic_payload_base {

    tlm_signal_gp();

    explicit tlm_signal_gp(tlm_base_mm_interface* mm);

    tlm_signal_gp(const tlm_signal_gp<SIG>& x) = delete;
    tlm_signal_gp<SIG>& operator=(const tlm_signal_gp<SIG>& x) = delete;

    //--------------
    // Destructor
    //--------------
    virtual ~tlm_signal_gp(){};
    // non-virtual deep-copying of the object

    void deep_copy_from(const tlm_signal_gp& other);

    tlm_command get_command() const { return m_command; }
    void set_command(const tlm_command command) { m_command = command; }

    SIG get_value() const { return m_value; }
    void set_value(const SIG value) { m_value = value; }

    // Response status related methods
    bool is_response_ok() const { return (m_response_status > 0); }
    bool is_response_error() const { return (m_response_status <= 0); }
    tlm_response_status get_response_status() const { return m_response_status; }
    void set_response_status(const tlm_response_status response_status) { m_response_status = response_status; }
    std::string get_response_string() const;

    struct gp_mm : public tlm_base_mm_interface {
        tlm_signal_gp<SIG>* create() {
            if(pool.size()) {
                auto ret = pool.front();
                pool.pop_front();
                return ret;
            } else
                return new tlm_signal_gp<SIG>(this);
        }
        void free(tlm_generic_payload_base* gp) override {
            auto t = dynamic_cast<tlm_signal_gp<SIG>*>(gp);
            t->free_all_extensions();
            pool.push_back(t);
        }
        ~gp_mm() {
            for(auto n : pool)
                delete n;
        }

    private:
        std::deque<tlm_signal_gp<SIG>*> pool;
    };

    static tlm_signal_gp<SIG>* create() {
        static thread_local gp_mm mm;
        return mm.create();
    }

protected:
    tlm_command m_command;
    SIG m_value;
    tlm_response_status m_response_status;

public:
};

template <typename SIG>
inline tlm_signal_gp<SIG>::tlm_signal_gp()
: tlm_generic_payload_base()
, m_command(TLM_IGNORE_COMMAND)
, m_response_status(TLM_OK_RESPONSE) {}

template <typename SIG>
inline tlm_signal_gp<SIG>::tlm_signal_gp(tlm_base_mm_interface* mm)
: tlm_generic_payload_base(mm)
, m_command(TLM_IGNORE_COMMAND)
, m_response_status(TLM_OK_RESPONSE) {}

inline void tlm_generic_payload_base::free_all_extensions() {
    m_extensions.free_entire_cache();
    for(unsigned int i = 0; i < m_extensions.size(); i++) {
        if(m_extensions[i]) {
            m_extensions[i]->free();
            m_extensions[i] = 0;
        }
    }
}

template <typename SIG> void tlm_signal_gp<SIG>::deep_copy_from(const tlm_signal_gp& other) {
    m_command = other.get_command();
    m_response_status = other.get_response_status();
    m_value = other.get_value();
    copy_extensions_from(other);
}

template <typename SIG> std::string tlm_signal_gp<SIG>::get_response_string() const {
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

inline tlm_extension_base* tlm_generic_payload_base::set_extension(unsigned int index, tlm_extension_base* ext) {
    sc_assert(index < m_extensions.size());
    tlm_extension_base* tmp = m_extensions[index];
    m_extensions[index] = ext;
    return tmp;
}

inline tlm_extension_base* tlm_generic_payload_base::set_auto_extension(unsigned int index, tlm_extension_base* ext) {
    sc_assert(index < m_extensions.size());
    tlm_extension_base* tmp = m_extensions[index];
    m_extensions[index] = ext;
    if(!tmp)
        m_extensions.insert_in_cache(&m_extensions[index]);
    sc_assert(m_mm != 0);
    return tmp;
}

inline tlm_extension_base* tlm_generic_payload_base::get_extension(unsigned int index) const {
    sc_assert(index < m_extensions.size());
    return m_extensions[index];
}

inline void tlm_generic_payload_base::clear_extension(unsigned int index) {
    sc_assert(index < m_extensions.size());
    m_extensions[index] = static_cast<tlm_extension_base*>(0);
}

inline void tlm_generic_payload_base::release_extension(unsigned int index) {
    sc_assert(index < m_extensions.size());
    if(m_mm) {
        m_extensions.insert_in_cache(&m_extensions[index]);
    } else {
        m_extensions[index]->free();
        m_extensions[index] = static_cast<tlm_extension_base*>(0);
    }
}

inline void tlm_generic_payload_base::update_extensions_from(const tlm_generic_payload_base& other) {
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

inline void tlm_generic_payload_base::copy_extensions_from(const tlm_generic_payload_base& other) {
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

inline void tlm_generic_payload_base::resize_extensions() { m_extensions.expand(max_num_extensions()); }
} // namespace scc
} // namespace tlm
#endif /* _TLM_TLM_SIGNAL_GP_H_ */
