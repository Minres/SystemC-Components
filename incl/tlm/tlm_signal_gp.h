/*
 * tlm_signal.h
 *
 *  Created on: 03.07.2018
 *      Author: eyck
 */

#ifndef _TLM_TLM_SIGNAL_GP_H_
#define _TLM_TLM_SIGNAL_GP_H_

#include <tlm_core/tlm_2/tlm_generic_payload/tlm_gp.h>

namespace tlm {

template<typename SIG=bool>
class tlm_signal_gp {

    tlm_signal_gp();

    explicit tlm_signal_gp(tlm_mm_interface* mm);

    void acquire() { sc_assert(m_mm != 0); m_ref_count++; }

    void release() {
        sc_assert(m_mm != 0 && m_ref_count > 0);
        if (--m_ref_count==0)
            m_mm->free(this);
    }

    int get_ref_count() const { return m_ref_count; }

    void set_mm(tlm_mm_interface* mm) { m_mm = mm; }
    bool has_mm() const { return m_mm != 0; }

    void reset();

    tlm_signal_gp(const tlm_signal_gp<SIG>& x) = delete ;
    tlm_signal_gp<SIG>& operator= (const tlm_signal_gp<SIG>& x) = delete;

    void update_extensions_from(const tlm_signal_gp<SIG> & other);

    // Free all extensions. Useful when reusing a cloned transaction that doesn't have memory manager.
    // normal and sticky extensions are freed and extension array cleared.
    void free_all_extensions();

    //--------------
    // Destructor
    //--------------
    virtual ~tlm_signal_gp();

    tlm_command          get_command() const {return m_command;}
    void                 set_command(const tlm_command command) {m_command = command;}

    SIG                  get_value() const {return m_value;}
    void                 set_command(const SIG value) {m_value = value;}

    // Response status related methods
    bool                 is_response_ok() const {return (m_response_status > 0);}
    bool                 is_response_error() const {return (m_response_status <= 0);}
    tlm_response_status  get_response_status() const {return m_response_status;}
    void                 set_response_status(const tlm_response_status response_status){m_response_status = response_status;}
    std::string          get_response_string() const;


protected:
    tlm_command m_command;
    SIG m_value;
    tlm_response_status m_response_status;

public:
    // Stick the pointer to an extension into the vector, return the
    // previous value:
    template <typename T> T* set_extension(T* ext)
    {
        return static_cast<T*>(set_extension(T::ID, ext));
    }

    // non-templatized version with manual index:
    tlm_extension_base* set_extension(unsigned int index,
                                      tlm_extension_base* ext);

    // Stick the pointer to an extension into the vector, return the
    // previous value and schedule its release
    template <typename T> T* set_auto_extension(T* ext)
    {
        return static_cast<T*>(set_auto_extension(T::ID, ext));
    }

    // non-templatized version with manual index:
    tlm_extension_base* set_auto_extension(unsigned int index,
                                           tlm_extension_base* ext);

    // Check for an extension, ext will point to 0 if not present
    template <typename T> void get_extension(T*& ext) const
    {
        ext = get_extension<T>();
    }
    template <typename T> T* get_extension() const
    {
        return static_cast<T*>(get_extension(T::ID));
    }
    // Non-templatized version with manual index:
    tlm_extension_base* get_extension(unsigned int index) const;

    //this call just removes the extension from the txn but does not
    // call free() or tells the MM to do so
    // it return false if there was active MM so you are now in an unsafe situation
    // recommended use: when 100% sure there is no MM
    template <typename T> void clear_extension(const T* ext)
    {
        clear_extension<T>();
    }

    //this call just removes the extension from the txn but does not
    // call free() or tells the MM to do so
    // it return false if there was active MM so you are now in an unsafe situation
    // recommended use: when 100% sure there is no MM
    template <typename T> void clear_extension()
    {
        clear_extension(T::ID);
    }

    //this call removes the extension from the txn and does
    // call free() or tells the MM to do so when the txn is finally done
    // recommended use: when not sure there is no MM
    template <typename T> void release_extension(T* ext)
    {
        release_extension<T>();
    }

    //this call removes the extension from the txn and does
    // call free() or tells the MM to do so when the txn is finally done
    // recommended use: when not sure there is no MM
    template <typename T> void release_extension()
    {
        release_extension(T::ID);
    }

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
    tlm_mm_interface*              m_mm;
    unsigned int                   m_ref_count;
};

template<typename SIG>
inline tlm_signal_gp<SIG>::tlm_signal_gp()
:m_command(TLM_IGNORE_COMMAND)
, m_response_status(TLM_OK_RESPONSE)
, m_extensions(max_num_extensions())
, m_ref_count(0)
, m_mm(nullptr)
{
}

template<typename SIG>
inline tlm_signal_gp<SIG>::tlm_signal_gp(tlm_mm_interface* mm)
:m_command(TLM_IGNORE_COMMAND)
, m_response_status(TLM_OK_RESPONSE)
, m_extensions(max_num_extensions())
, m_ref_count(0)
, m_mm(mm)
{
}

template<typename SIG>
inline void tlm_signal_gp<SIG>::reset() {
    m_extensions.free_entire_cache();
}

template<typename SIG>
void tlm_signal_gp<SIG>::update_extensions_from(const tlm_signal_gp<SIG>& other) {
    // deep copy extensions that are already present
    sc_assert(m_extensions.size() <= other.m_extensions.size());
    for(unsigned int i=0; i<m_extensions.size(); i++){
        if(other.m_extensions[i]){ //original has extension i
            if(m_extensions[i]){                   //We have it too. copy.
                m_extensions[i]->copy_from(*other.m_extensions[i]);
            }
        }
    }
}

template<typename SIG >
void tlm_signal_gp<SIG>::free_all_extensions() {
    m_extensions.free_entire_cache();
    for(unsigned int i=0; i<m_extensions.size(); i++){
        if(m_extensions[i]){
            m_extensions[i]->free();
            m_extensions[i] = 0;
        }
    }
}

template<typename SIG >
inline tlm_signal_gp<SIG>::~tlm_signal_gp() {
    for(unsigned int i=0; i<m_extensions.size(); i++)
        if(m_extensions[i]) m_extensions[i]->free();
}

template<typename SIG >
std::string tlm_signal_gp<SIG>::get_response_string() const {
    switch(m_response_status){
    case TLM_OK_RESPONSE:            return "TLM_OK_RESPONSE";
    case TLM_INCOMPLETE_RESPONSE:    return "TLM_INCOMPLETE_RESPONSE";
    case TLM_GENERIC_ERROR_RESPONSE: return "TLM_GENERIC_ERROR_RESPONSE";
    case TLM_ADDRESS_ERROR_RESPONSE: return "TLM_ADDRESS_ERROR_RESPONSE";
    case TLM_COMMAND_ERROR_RESPONSE: return "TLM_COMMAND_ERROR_RESPONSE";
    case TLM_BURST_ERROR_RESPONSE:   return "TLM_BURST_ERROR_RESPONSE";
    case TLM_BYTE_ENABLE_ERROR_RESPONSE: return "TLM_BYTE_ENABLE_ERROR_RESPONSE";
    }
    return "TLM_UNKNOWN_RESPONSE";
}

template<typename SIG >
tlm_extension_base* tlm_signal_gp<SIG>::set_extension(unsigned int index, tlm_extension_base* ext) {
    sc_assert(index < m_extensions.size());
    tlm_extension_base* tmp = m_extensions[index];
    m_extensions[index] = ext;
    return tmp;
}

template<typename SIG >
tlm_extension_base* tlm_signal_gp<SIG>::set_auto_extension(unsigned int index, tlm_extension_base* ext) {
    sc_assert(index < m_extensions.size());
    tlm_extension_base* tmp = m_extensions[index];
    m_extensions[index] = ext;
    if (!tmp) m_extensions.insert_in_cache(&m_extensions[index]);
    sc_assert(m_mm != 0);
    return tmp;
}

template<typename SIG >
inline tlm_extension_base* tlm_signal_gp<SIG>::get_extension(unsigned int index) const {
    sc_assert(index < m_extensions.size());
    return m_extensions[index];
}

template<typename SIG >
void tlm_signal_gp<SIG>::clear_extension(unsigned int index) {
    sc_assert(index < m_extensions.size());
    m_extensions[index] = static_cast<tlm_extension_base*>(0);
}

template<typename SIG >
void tlm_signal_gp<SIG>::release_extension(unsigned int index) {
    sc_assert(index < m_extensions.size());
     if (m_mm) {
         m_extensions.insert_in_cache(&m_extensions[index]);
     } else {
         m_extensions[index]->free();
         m_extensions[index] = static_cast<tlm_extension_base*>(0);
     }
}

template<typename SIG >
inline void tlm_signal_gp<SIG>::resize_extensions() {
    m_extensions.expand(max_num_extensions());
}

}
#endif /* _TLM_TLM_SIGNAL_GP_H_ */
