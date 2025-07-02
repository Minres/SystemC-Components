/*****************************************************************************

  Licensed to Accellera Systems Initiative Inc. (Accellera) under one or
  more contributor license agreements.  See the NOTICE file distributed
  with this work for additional information regarding copyright ownership.
  Accellera licenses this file to you under the Apache License, Version 2.0
  (the "License"); you may not use this file except in compliance with the
  License.  You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
  implied.  See the License for the specific language governing
  permissions and limitations under the License.

 ****************************************************************************/

#ifndef CCI_CFG_CCI_PARAM_TYPED_H_INCLUDED_
#define CCI_CFG_CCI_PARAM_TYPED_H_INCLUDED_

#include "cci/cfg/cci_param_untyped.h"
#include "cci/cfg/cci_param_untyped_handle.h"
#include "cci/cfg/cci_report_handler.h"
#include "cci/cfg/cci_broker_manager.h"

#include <sstream> //std::stringstream

/**
 * @author Enrico Galli, Intel
 * @author Guillaume Delbergue, GreenSocs / Ericsson
 */

#if defined(__clang__) || \
   (defined(__GNUC__) && ((__GNUC__ * 1000 + __GNUC_MINOR__) >= 4006))
// ignore warning about hidden "register_post_read_callback()" overloads
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverloaded-virtual"
#endif

CCI_OPEN_NAMESPACE_

// Forward declaration
template<class T>
class cci_param_typed_handle;

///@cond CCI_HIDDEN_FROM_DOXYGEN
namespace cci_impl {
/// implementation defined helper to set/reset a boolean flag
struct scoped_true {
    explicit scoped_true(bool& ref) : ref_(ref) { ref_ = true; }
    ~scoped_true() { ref_ = false; }
private:
    bool& ref_;
}; // class scoped_true
}  // namespace cci_impl
///@endcond

/// Parameter class, internally forwarding calls to the implementation
/**
 * The implementation is in file cci_param_typed.hpp, which has to be included
 * in the headers as well because the class is a template class.
 *
 * Documentation for the undocumented functions can be found in
 * cci_param_if
 */
template<typename T, cci_param_mutable_type TM = CCI_MUTABLE_PARAM>
class cci_param_typed : public cci_param_untyped
{
public:
    ///@name Typed parameter value access
    //@{

    /// The parameter's value type.
    typedef T value_type;

    ///Assigns parameter a new value from another parameter
    /**
     * @param rhs New value to assign
     * @return reference to this object
     */
    cci_param_typed& operator= (const cci_param_typed & rhs);

    ///Assigns parameter a new value
    /**
     * @param rhs New value to assign
     * @return reference to this object
     */
    cci_param_typed& operator= (const value_type & rhs);

    ///Conversion operator to be able use cci_param_typed as a regular object
    operator const value_type& () const;

    ///Sets the stored value to a new value 
    /**
     * @param value new value to assign
     */
    void set_value(const value_type& value);
    ///Sets the stored value to a new value 
    /**
     * @param value new value to assign
     * @param pwd Password needed to unlock the param, ideally any pointer
     * address known only by the locking entity, default = NULL
     */
    void set_value(const value_type & value, const void * pwd);

    ///Get the value passed in via constructor
    const value_type & get_default_value() const;

    /// Get the current value
    const value_type& get_value() const;

    /// Get the current value (for an explicit originator)
    const value_type& get_value(const cci_originator& originator) const;

    //@}

    ///@name Type-punned value operations
    ///@{

    /// Returns a basic type this parameter can be converted to or from
    /// (which is not necessarily the actual parameter type).
    /**
     * @return Type
     */
    cci_param_data_category get_data_category() const;

    /// Returns the type information of the parameter
    /**
     * @return Type information
     */
    const std::type_info& get_type_info() const;

    /// Get the parameter's mutable type
    /**
     * @return Parameter mutable type
     */
    cci_param_mutable_type get_mutable_type() const;

    ///@}

    ///@name (Untyped) parameter value access
    ///@{

    using cci_param_if::set_cci_value;
    /// @copydoc cci_param_if::set_cci_value(const cci_value&, const void*, const cci_originator&)
    void set_cci_value(const cci_value& val, const void *pwd, const cci_originator& originator);

    using cci_param_if::get_cci_value;
    /// @copydoc cci_param_if::get_cci_value(const cci_originator&) const
    cci_value get_cci_value(const cci_originator& originator) const;

    /// @copydoc cci_param_if::get_default_cci_value() const
    cci_value get_default_cci_value() const;

    ///@}

    ///@name Parameter Value Status
    //@{

    /// @copydoc cci_param_untyped::is_default_value()
    bool is_default_value() const;

    /// @copydoc cci_param_untyped::is_preset_value()
    bool is_preset_value() const;

    ///@}

    /// @copydoc cci_param_untyped::lock
    bool lock(const void* pwd = NULL);

    /// @name Post write callback handling
    /// @{

    // Untyped callbacks

    /// @copydoc cci_param_untyped::register_post_write_callback(const cci_param_post_write_callback_untyped, cci_untyped_tag)
    cci_callback_untyped_handle register_post_write_callback(
            const cci_param_post_write_callback_untyped &cb,
            cci_untyped_tag);

    /// @copydoc cci_param_untyped::register_post_write_callback(cci_param_post_write_callback_untyped::signature, C*, cci_untyped_tag)
    template<typename C>
    cci_callback_untyped_handle register_post_write_callback(
            cci_param_post_write_callback_untyped::signature (C::*cb), C* obj,
            cci_untyped_tag);

    // Typed callbacks

    /// Typed write callback type
    typedef typename cci_param_post_write_callback<value_type>::type
            cci_param_post_write_callback_typed;

    /// Register a typed write callback.
    /**
     * @param cb Typed write callback
     * @param cci_typed_tag Typed tag to avoid compiler ambiguity
     *
     * @return Untyped callback handle
     */
    cci_callback_untyped_handle register_post_write_callback(
            const cci_param_post_write_callback_typed &cb,
            cci_typed_tag<value_type> = cci_typed_tag<value_type>());

    /// Register a typed write callback with a method as callback
    /**
     * @param cb Typed write callback method
     * @param obj Associated object instance pointer
     * @param cci_typed_tag Typed tag to avoid compiler ambiguity
     *
     * @return Untyped callback handle
     */
    template<typename C>
    cci_callback_untyped_handle register_post_write_callback(
            typename cci_param_post_write_callback_typed::signature (C::*cb),
            C* obj, cci_typed_tag<value_type> = cci_typed_tag<value_type>());

    /// @}

    /// @name Pre write callback handling
    /// @{

    // Untyped callbacks

    /// @copydoc cci_param_untyped::register_pre_write_callback(const cci_param_pre_write_callback_untyped, cci_untyped_tag)
    cci_callback_untyped_handle register_pre_write_callback(
            const cci_param_pre_write_callback_untyped &cb,
            cci_untyped_tag);

    /// @copydoc cci_param_untyped::register_pre_write_callback(cci_param_pre_write_callback_untyped::signature, C*, cci_untyped_tag)
    template<typename C>
    cci_callback_untyped_handle register_pre_write_callback(
            cci_param_pre_write_callback_untyped::signature (C::*cb),
            C* obj, cci_untyped_tag);

    // Typed callbacks

    /// Typed pre write callback type
    typedef typename cci_param_pre_write_callback<value_type>::type
            cci_param_pre_write_callback_typed;

    /// Register a typed pre write callback.
    /**
     * @param cb Typed pre write callback
     * @param cci_typed_tag Typed tag to avoid compiler ambiguity
     *
     * @return Untyped callback handle
     */
    cci_callback_untyped_handle register_pre_write_callback(
            const cci_param_pre_write_callback_typed &cb,
            cci_typed_tag<value_type> = cci_typed_tag<value_type>());

    /// Register a typed pre write callback with a method as callback
    /**
     * @param cb Typed pre write callback method
     * @param obj Associated object instance pointer
     * @param cci_typed_tag Typed tag to avoid compiler ambiguity
     *
     * @return Untyped callback handle
     */
    template<typename C>
    cci_callback_untyped_handle register_pre_write_callback(
            typename cci_param_pre_write_callback_typed::signature
            (C::*cb), C* obj,
            cci_typed_tag<value_type> = cci_typed_tag<value_type>());

    /// @}

    /// @name Pre read callback handling
    /// @{

    // Untyped callbacks

    /// @copydoc cci_param_untyped::register_pre_read_callback(const cci_param_pre_read_callback_untyped, cci_untyped_tag)
    cci_callback_untyped_handle register_pre_read_callback(
            const cci_param_pre_read_callback_untyped &cb,
            cci_untyped_tag);

    /// @copydoc cci_param_untyped::register_pre_read_callback(cci_param_pre_read_callback_untyped::signature, C*, cci_untyped_tag)
    template<typename C>
    cci_callback_untyped_handle register_pre_read_callback(
            cci_param_pre_read_callback_untyped::signature (C::*cb), C* obj,
            cci_untyped_tag);

    // Typed callbacks

    /// Typed read callback type
    typedef typename cci_param_pre_read_callback<value_type>::type
            cci_param_pre_read_callback_typed;

    /// Register a typed read callback.
    /**
     * @param cb Typed read callback
     * @param cci_typed_tag Typed tag to avoid compiler ambiguity
     *
     * @return Untyped callback handle
     */
    cci_callback_untyped_handle register_pre_read_callback(
            const cci_param_pre_read_callback_typed &cb,
            cci_typed_tag<value_type> = cci_typed_tag<value_type>());

    /// Register a typed read callback with a method as callback
    /**
     * @param cb Typed read callback method
     * @param obj Associated object instance pointer
     * @param cci_typed_tag Typed tag to avoid compiler ambiguity
     *
     * @return Untyped callback handle
     */
    template<typename C>
    cci_callback_untyped_handle register_pre_read_callback(
            typename cci_param_pre_read_callback_typed::signature (C::*cb),
            C* obj, cci_typed_tag<value_type> = cci_typed_tag<value_type>());

    /// @}

    /// @name Post read callback handling
    /// @{

    // Untyped callbacks

    /// @copydoc cci_param_untyped::register_post_read_callback(const cci_param_post_read_callback_untyped, cci_untyped_tag)
    cci_callback_untyped_handle register_post_read_callback(
            const cci_param_post_read_callback_untyped &cb,
            cci_untyped_tag);

    /// @copydoc cci_param_untyped::register_post_read_callback(cci_param_post_read_callback_untyped::signature, C*, cci_untyped_tag)
    template<typename C>
    cci_callback_untyped_handle register_post_read_callback(
            cci_param_post_read_callback_untyped::signature (C::*cb), C* obj,
            cci_untyped_tag);

    // Typed callbacks

    /// Typed read callback type
    typedef typename cci_param_post_read_callback<value_type>::type
            cci_param_post_read_callback_typed;

    /// Register a typed post read callback.
    /**
     * @param cb Typed post read callback
     * @param cci_typed_tag Typed tag to avoid compiler ambiguity
     *
     * @return Untyped callback handle
     */
    cci_callback_untyped_handle register_post_read_callback(
            const cci_param_post_read_callback_typed &cb,
            cci_typed_tag<value_type> = cci_typed_tag<value_type>());

    /// Register a typed post read callback with a method as callback
    /**
     * @param cb Typed post read callback method
     * @param obj Associated object instance pointer
     * @param cci_typed_tag Typed tag to avoid compiler ambiguity
     *
     * @return Untyped callback handle
     */
    template<typename C>
    cci_callback_untyped_handle register_post_read_callback(
            typename cci_param_post_read_callback_typed::signature (C::*cb),
            C* obj, cci_typed_tag<value_type> = cci_typed_tag<value_type>());

    /// @}

    /** @name Constructors */
    //@{
    /**
     * Constructor with (local/hierarchical) name, default value,
     * description and originator.
     *
     * @param name Name of the parameter
     * @param default_value Default value of the parameter (Typed value)
     * @param desc Description of the parameter
     * @param name_type Either the name should be absolute or relative
     * @param originator Originator of the parameter
     */
    cci_param_typed(const std::string& name, const value_type& default_value,
                    const std::string& desc = "",
                    cci_name_type name_type = CCI_RELATIVE_NAME,
                    const cci_originator& originator = cci_originator());

    /**
     * Constructor with (local/hierarchical) name, default value,
     * description and originator.
     *
     * @param name Name of the parameter
     * @param default_value Default value of the parameter (CCI value)
     * @param desc Description of the parameter
     * @param name_type Either the name should be absolute or relative
     * @param originator Originator of the parameter
     */
    cci_param_typed(const std::string& name, const cci_value& default_value,
                    const std::string& desc = "",
                    cci_name_type name_type = CCI_RELATIVE_NAME,
                    const cci_originator& originator = cci_originator());

    /**
     * Constructor with (local/hierarchical) name, default value,
     * private broker, description, name type and originator.
     *
     * @param name Name of the parameter
     * @param default_value Default value of the parameter (Typed value)
     * @param private_broker Associated private broker
     * @param desc Description of the parameter
     * @param name_type Either the name should be absolute or relative
     * @param originator Originator of the parameter
     */
    cci_param_typed(const std::string& name, const value_type& default_value,
                    cci_broker_handle private_broker,
                    const std::string& desc = "",
                    cci_name_type name_type = CCI_RELATIVE_NAME,
                    const cci_originator& originator = cci_originator());

    /**
     * Constructor with (local/hierarchical) name, default value,
     * private broker, description, name type and originator.
     *
     * @param name Name of the parameter
     * @param default_value Default value of the parameter (CCI value)
     * @param private_broker Associated private broker
     * @param desc Description of the parameter
     * @param name_type Either the name should be absolute or relative
     * @param originator Originator of the parameter
     */
    cci_param_typed(const std::string& name, const cci_value& default_value,
                    cci_broker_handle private_broker,
                    const std::string& desc = "",
                    cci_name_type name_type = CCI_RELATIVE_NAME,
                    const cci_originator& originator = cci_originator());
    //@}

    ///@copydoc cci_param_if::reset
    virtual bool reset();

    ~cci_param_typed()
      { destroy(m_broker_handle); }

protected:
    /// Value
    value_type m_value;

    /// Default value
    value_type m_default_value;

private:
    ///@copydoc cci_param_if::preset_cci_value
    virtual void preset_cci_value(const cci_value&, const cci_originator&);

    /// @copydoc cci_param_if::set_raw_value
    virtual void set_raw_value(const void *vp, const void *pwd,
                               const cci_originator &originator);

    /// @copydoc cci_param_if::get_raw_value
    virtual const void *get_raw_value(const cci_originator &originator) const;

    /// @copydoc cci_param_if::get_raw_default_value
    virtual const void *get_raw_default_value() const;

    /// Pre write callback
    bool
    pre_write_callback(value_type value,
                       const cci_originator &originator) const
    {
        // Already locked, skip nested invocation
        if (m_pre_write_callbacks.oncall)
            return false;

        // Lock the tag to prevent nested callback
        cci_impl::scoped_true oncall( m_pre_write_callbacks.oncall );

        bool result = true;
        // Validate write callbacks
        for (unsigned i = 0; i < m_pre_write_callbacks.vec.size(); ++i) {
            typename cci_param_pre_write_callback_handle<value_type>::type
                    typed_pre_write_cb(
                    m_pre_write_callbacks.vec[i].callback);

            // Prepare parameter handle for callback event
            cci_param_untyped_handle param_handle =
                    create_param_handle(m_pre_write_callbacks.vec[i].originator);

            // Write callback payload
            const cci_param_write_event<value_type>
              ev(m_value, value, originator, param_handle);

            if (!typed_pre_write_cb.invoke(ev)) {
                // Write denied
                cci_report_handler::set_param_failed(
                        "Value rejected by callback.", __FILE__, __LINE__);
                result = false;
            }
        }

        return result;
    }

    /// Post write callback
    void post_write_callback(const value_type& old_value,
                             const value_type& new_value,
                             const cci_originator &originator) const
    {
        // Already locked, skip nested invocation
        if (m_post_write_callbacks.oncall)
            return;

        // Lock the tag to prevent nested callback
        cci_impl::scoped_true oncall( m_post_write_callbacks.oncall );

        // Write callbacks
        for (unsigned i = 0; i < m_post_write_callbacks.vec.size(); ++i) {
            typename cci_param_post_write_callback_handle<value_type>::type
                    typed_post_write_cb(m_post_write_callbacks.vec[i].callback);
            if (typed_post_write_cb.valid()) {

                // Prepare parameter handle for callback event
                cci_param_untyped_handle param_handle = create_param_handle(
                        m_post_write_callbacks.vec[i].originator);

                // Write callback payload
                const cci_param_write_event<value_type>
                  ev(old_value, new_value, originator, param_handle);

                typed_post_write_cb.invoke(ev);
            }
        }
    }

    /// Pre read callback
    void pre_read_callback(const value_type& value,
                           const cci_originator &originator) const
    {
        // Already locked, skip nested invocation
        if(m_pre_read_callbacks.oncall)
          return;

        // Lock the tag to prevent nested callback
        cci_impl::scoped_true oncall( m_pre_read_callbacks.oncall );

        // Read callbacks
        for (unsigned i = 0; i < m_pre_read_callbacks.vec.size(); ++i) {
            typename cci_param_pre_read_callback_handle<value_type>::type
                    typed_pre_read_cb(m_pre_read_callbacks.vec[i].callback);
            if (typed_pre_read_cb.valid()) {

                // Prepare parameter handle for callback event
                cci_param_untyped_handle param_handle = create_param_handle(
                        m_pre_read_callbacks.vec[i].originator);

                // Read callback payload
                const cci_param_read_event<value_type>
                  ev(value, originator, param_handle);

                typed_pre_read_cb.invoke(ev);
            }
        }
    }

    /// Post read callback
    void post_read_callback(const value_type& value,
                            const cci_originator &originator) const
    {
        // Already locked, skip nested invocation
        if(m_post_read_callbacks.oncall)
            return;

        // Lock the tag to prevent nested callback
        cci_impl::scoped_true oncall( m_post_read_callbacks.oncall );

        // Read callbacks
        for (unsigned i = 0; i < m_post_read_callbacks.vec.size(); ++i) {
            typename cci_param_post_read_callback_handle<value_type>::type
                    typed_pre_read_cb(m_post_read_callbacks.vec[i].callback);
            if (typed_pre_read_cb.valid()) {

                // Prepare parameter handle for callback event
                cci_param_untyped_handle param_handle = create_param_handle(
                        m_post_read_callbacks.vec[i].originator);

                // Read callback payload
                const cci_param_read_event<value_type>
                  ev(value, originator, param_handle);

                typed_pre_read_cb.invoke(ev);
            }
        }
    }

cci_broker_handle find_broker_convenience(const cci_originator &originator)
    {
      if (!sc_core::sc_get_current_object()) {
        return cci_get_global_broker(originator);
      } else {
        return cci_get_broker();
      }
    }

};

template<typename T, cci_param_mutable_type TM>
cci_param_typed<T,TM>&
cci_param_typed<T,TM>::operator=(const cci_param_typed<T,TM>& rhs)
{
    set_value(rhs.get_value());
    return *this;
}

template <typename T, cci_param_mutable_type TM>
cci_param_typed<T, TM>&
cci_param_typed<T, TM>::operator=(const T& rhs)
{
    set_value(rhs);
    return *this;
}

template <typename T, cci_param_mutable_type TM>
cci_param_typed<T, TM>::operator const T&() const
{
    return get_value();
}

template <typename T, cci_param_mutable_type TM>
const T& cci_param_typed<T, TM>::get_value() const
{
  if (cci_param_untyped::fast_read) {
    // this is totally safe, there are no callbacks, and the originator is only
    // used by the callbacks
    return m_value;
  }
  return get_value(get_originator());
}

template <typename T, cci_param_mutable_type TM>
const T& cci_param_typed<T, TM>::get_value(const cci_originator& originator) const
{
  return *static_cast<const value_type *>(get_raw_value(originator));
}

template <typename T, cci_param_mutable_type TM>
void cci_param_typed<T, TM>::set_raw_value(const void* value,
                                           const void *pwd,
                                           const cci_originator& originator)
{
  const value_type& new_value = *static_cast<const value_type*>(value);

  if (!this->set_cci_value_allowed(TM))
    return;

  if(!pwd) {
    if (cci_param_untyped::is_locked()) {
      cci_report_handler::set_param_failed("Parameter locked.", __FILE__, __LINE__);
      return;
    }
  } else {
    if (pwd != m_lock_pwd) {
      cci_report_handler::set_param_failed("Wrong key.", __FILE__, __LINE__);
      return;
    }
  }

  if (!pre_write_callback(new_value, originator))
    return;

  // Actual write
  value_type old_value = m_value;
  m_value = new_value;

  // Update value's origin
  m_value_origin = originator;

  // Write callback(s)
  post_write_callback(old_value, new_value, originator);

  cci_param_untyped::fast_write =
    TM == CCI_MUTABLE_PARAM &&
    !cci_param_untyped::is_locked() &&
    m_pre_write_callbacks.vec.size()==0 &&
    m_post_write_callbacks.vec.size()==0 &&
    originator==m_originator;
}

template <typename T, cci_param_mutable_type TM>
void cci_param_typed<T, TM>::set_value(const T& value)
{
  // fast_write tracks whether, we have no callbacks, no lock, we're allowed to
  // do the write (it's not immutable) AND the originator (last time) was,
  // indeed, the original m_originator. The _only_ way of getting here is from
  // the owner of the param, hence the originator must be m_originator.
  if (cci_param_untyped::fast_write) {
    m_value = value;
  } else {
    set_raw_value(&value, NULL, get_originator());
  }
}

template <typename T, cci_param_mutable_type TM>
void cci_param_typed<T, TM>::set_value(const T& value, const void *pwd)
{
    set_raw_value(&value, pwd, get_originator());
}

template <typename T, cci_param_mutable_type TM>
const void* cci_param_typed<T, TM>::get_raw_value(
        const cci_originator &originator) const
{
    pre_read_callback(m_value, originator);
    const void *v = &m_value;
    post_read_callback(m_value, originator);

    const_cast<cci_param_typed<T,TM>* >(this)->cci_param_untyped::fast_read =
        m_pre_read_callbacks.vec.size()==0 &&
        m_post_read_callbacks.vec.size()==0;

    return v;
}

template <typename T, cci_param_mutable_type TM>
cci_param_data_category cci_param_typed<T, TM>::get_data_category() const
{
	switch (get_cci_value().category())
	{
    case CCI_BOOL_VALUE:
        return CCI_BOOL_PARAM;
	case CCI_INTEGRAL_VALUE:
		return CCI_INTEGRAL_PARAM;
	case CCI_REAL_VALUE:
		return CCI_REAL_PARAM;
	case CCI_STRING_VALUE:
		return CCI_STRING_PARAM;
	case CCI_LIST_VALUE:
		return CCI_LIST_PARAM;
	case CCI_NULL_VALUE:
	case CCI_OTHER_VALUE:
    default:
		return CCI_OTHER_PARAM;
	}
}

template <typename T, cci_param_mutable_type TM>
const std::type_info& cci_param_typed<T, TM>::get_type_info() const
{
    return typeid(value_type);
}

template <typename T, cci_param_mutable_type TM>
const void* cci_param_typed<T, TM>::get_raw_default_value() const {
    return &get_default_value();
}

template <typename T, cci_param_mutable_type TM>
const typename cci_param_typed<T, TM>::value_type&
cci_param_typed<T, TM>::get_default_value() const
{
    return this->m_default_value;
}

template <typename T, cci_param_mutable_type TM>
cci_param_mutable_type cci_param_typed<T, TM>::get_mutable_type() const
{
    return TM;
}

template <typename T, cci_param_mutable_type TM>
void cci_param_typed<T, TM>::set_cci_value(const cci_value& val,
                                           const void *pwd,
                                           const cci_originator& originator)
{
    value_type v = val.get<value_type>();
    set_raw_value(&v, pwd, originator);
}

template <typename T, cci_param_mutable_type TM>
void cci_param_typed<T, TM>::preset_cci_value(const cci_value& val,
                                              const cci_originator& originator)
{
    value_type old_value = m_value;
    value_type new_value = val.get<value_type>();

    if (!pre_write_callback(new_value, originator))
      return;

    // Actual write
    m_value = new_value;

    // Update value's origin
    m_value_origin = originator;

    // Write callback(s)
    post_write_callback(old_value, new_value, originator);
}

template <typename T, cci_param_mutable_type TM>
cci_value
cci_param_typed<T, TM>::get_cci_value(const cci_originator& originator) const
{
    return cci_value( get_value(originator) );
}

template <typename T, cci_param_mutable_type TM>
cci_value cci_param_typed<T, TM>::get_default_cci_value() const {
    return cci_value(m_default_value);
}

template <typename T, cci_param_mutable_type TM>
bool cci_param_typed<T, TM>::is_default_value() const
{
  return m_default_value == m_value;
}

template <typename T, cci_param_mutable_type TM>
bool cci_param_typed<T, TM>::is_preset_value() const
{
  if (m_broker_handle.has_preset_value(name()))
  {
    cci_value init_value = m_broker_handle.get_preset_cci_value(name());
    T i;
    if (init_value.try_get<T>(i)) {
      return i == m_value;
    }
  }
  return false;
}

template <typename T, cci_param_mutable_type TM>
bool cci_param_typed<T, TM>::lock(const void* pwd)
{
  cci_param_untyped::fast_write=false;
  return cci_param_untyped::lock(pwd);
}

// Callbacks

#define CCI_PARAM_TYPED_CALLBACK_IMPL_(name)                                   \
template <typename T, cci_param_mutable_type TM>                               \
cci_callback_untyped_handle                                                    \
cci_param_typed<T, TM>::register_##name##_callback(                            \
        const cci_param_##name##_callback_untyped &cb,                         \
        cci_untyped_tag)                                                       \
{                                                                              \
    cci_param_untyped::fast_read=false;                                        \
    cci_param_untyped::fast_write=false;                                       \
    return cci_param_untyped::register_##name##_callback(cb);                  \
}                                                                              \
                                                                               \
template <typename T, cci_param_mutable_type TM>                               \
template<typename C>                                                           \
cci_callback_untyped_handle                                                    \
cci_param_typed<T, TM>::register_##name##_callback(                            \
        cci_param_##name##_callback_untyped::signature (C::*cb), C* obj,       \
        cci_untyped_tag)                                                       \
{                                                                              \
    cci_param_untyped::fast_read=false;                                        \
    cci_param_untyped::fast_write=false;                                       \
    return cci_param_untyped::register_##name##_callback(cb, obj);             \
}                                                                              \
                                                                               \
template <typename T, cci_param_mutable_type TM>                               \
cci_callback_untyped_handle                                                    \
cci_param_typed<T, TM>::register_##name##_callback(                            \
        const cci_param_##name##_callback_typed &cb, cci_typed_tag<T>)         \
{                                                                              \
    cci_param_untyped::fast_read=false;                                        \
    cci_param_untyped::fast_write=false;                                       \
    return cci_param_untyped::register_##name##_callback(cb, get_originator());\
}                                                                              \
                                                                               \
template <typename T, cci_param_mutable_type TM>                               \
template <typename C>                                                          \
cci_callback_untyped_handle                                                    \
cci_param_typed<T, TM>::register_##name##_callback(                            \
        typename cci_param_##name##_callback_typed::signature (C::*cb),        \
        C* obj, cci_typed_tag<T>)                                              \
{                                                                              \
    cci_param_untyped::fast_read=false;                                        \
    cci_param_untyped::fast_write=false;                                       \
    return register_##name##_callback(sc_bind(cb, obj, sc_unnamed::_1));       \
}

// Pre write callback
CCI_PARAM_TYPED_CALLBACK_IMPL_(pre_write)

// Post write callback
CCI_PARAM_TYPED_CALLBACK_IMPL_(post_write)

// Pre read callback
CCI_PARAM_TYPED_CALLBACK_IMPL_(pre_read)

// Post read callback
CCI_PARAM_TYPED_CALLBACK_IMPL_(post_read)

/// Constructors

# define CCI_PARAM_CHECK_ANY_VALUE_FAILURE(rpt) ((void)0)

#define CCI_PARAM_CONSTRUCTOR_CCI_VALUE_IMPL(signature, broker)                \
  template <typename T, cci_param_mutable_type TM>                             \
  cci_param_typed<T, TM>::cci_param_typed signature                            \
  try                                                                          \
    : cci_param_untyped(name, name_type, broker, desc, originator)             \
    , m_value(default_value.get<T>())                                          \
    , m_default_value(m_value)                                                 \
  {                                                                            \
    this->init(m_broker_handle);                                               \
  }                                                                            \
  catch (const sc_core::sc_report& rpt)                                        \
  {                                                                            \
    CCI_PARAM_CHECK_ANY_VALUE_FAILURE(rpt);                                    \
    throw rpt;                                                                 \
  }

#define CCI_PARAM_CONSTRUCTOR_IMPL(signature, broker)                          \
template <typename T, cci_param_mutable_type TM>                               \
cci_param_typed<T, TM>::cci_param_typed signature                              \
: cci_param_untyped(name, name_type, broker, desc, originator),                \
  m_value(default_value),                                                      \
  m_default_value(default_value)                                               \
{                                                                              \
    this->init(m_broker_handle);                                               \
}


/// Constructor with (local/hierarchical) name, default value, description,
/// name type and originator.
CCI_PARAM_CONSTRUCTOR_IMPL((const std::string& name,
                            const value_type& default_value,
                            const std::string& desc,
                            cci_name_type name_type,
                            const cci_originator& originator),
                            find_broker_convenience(originator))

/// Constructor with (local/hierarchical) name, default value, description,
/// name type and originator.
CCI_PARAM_CONSTRUCTOR_CCI_VALUE_IMPL((const std::string& name,
                                      const cci_value& default_value,
                                      const std::string& desc,
                                      cci_name_type name_type,
                                      const cci_originator& originator),
                                      find_broker_convenience(originator))

/// Constructor with (local/hierarchical) name, default value, private broker,
/// description, name type and originator.
CCI_PARAM_CONSTRUCTOR_IMPL((const std::string& name,
                            const value_type& default_value,
                            cci_broker_handle private_broker,
                            const std::string& desc,
                            cci_name_type name_type,
                            const cci_originator& originator),
                            private_broker)

/// Constructor with (local/hierarchical) name, default value, private broker,
/// description, name type and originator.
CCI_PARAM_CONSTRUCTOR_CCI_VALUE_IMPL((const std::string& name,
                                      const cci_value& default_value,
                                      cci_broker_handle private_broker,
                                      const std::string& desc,
                                      cci_name_type name_type,
                                      const cci_originator& originator),
                                      private_broker)

#undef CCI_PARAM_CHECK_ANY_VALUE_FAILURE
#undef CCI_PARAM_CONSTRUCTOR_IMPL
#undef CCI_PARAM_CONSTRUCTOR_CCI_VALUE_IMPL
#undef CCI_PARAM_TYPED_CALLBACK_IMPL_

template <typename T, cci_param_mutable_type TM>
bool cci_param_typed<T, TM>::reset()
{
  if (is_locked())
    return false;
  const std::string& nm = name();
  if (m_broker_handle.has_preset_value(nm)) {
    // Apply preset value if it exists
    cci_value preset = m_broker_handle.get_preset_cci_value(nm);
    preset_cci_value(preset, m_broker_handle.get_preset_value_origin(nm));
  } else {
    // Otherwise apply the default value
    // Can't just call set_raw_value(); won't work for IMMUTABLE params.
    if (!pre_write_callback(get_default_value(), m_originator))
        return false;

    // Actual write
    value_type old_value = m_value;
    m_value = get_default_value();

    // Update value's origin
    m_value_origin = m_originator;

    // Write callback(s)
    post_write_callback(old_value, get_default_value(), m_originator);
  }
  return true;
}

/// Convenience shortcut for typed parameters
template <typename T, cci_param_mutable_type TM = CCI_MUTABLE_PARAM>
using cci_param = cci_param_typed<T,TM>;

CCI_CLOSE_NAMESPACE_

#if defined(__clang__) || \
   (defined(__GNUC__) && ((__GNUC__ * 1000 + __GNUC_MINOR__) >= 4006))
#pragma GCC diagnostic pop
#endif

#endif //CCI_CFG_CCI_PARAM_TYPED_H_INCLUDED_
