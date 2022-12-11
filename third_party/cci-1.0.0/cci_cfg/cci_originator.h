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

#ifndef CCI_CFG_CCI_ORIGINATOR_H_INCLUDED_
#define CCI_CFG_CCI_ORIGINATOR_H_INCLUDED_

#include "cci_core/cci_cmnhdr.h"

CCI_OPEN_NAMESPACE_

class cci_broker_if;
class cci_param_if;

/// Originator class which is used to track owners, handles and value providers
/// of parameters.
/**
 * Static setter function is used by the parameter (proxy) to identify
 * originator changing parameter's value.
 *
 * Static getter function is used by the parameter implementation to record
 * originator's identity.
 */
class cci_originator
{
    friend class cci_broker_if;
    friend class cci_param_if;

    struct unknown_tag {};

    /// Internal constructor to create an unknown/invalid originator
    explicit cci_originator(unknown_tag)
      : m_originator_obj(), m_originator_str() {}

public:
    /// Default Constructor assumes current module is the originator
    inline cci_originator()
            : m_originator_obj(current_originator_object()),
              m_originator_str(NULL) {
        check_is_valid();
    }

    /// Constructor with an originator name
    /**
     * Constructor to associate explicitly a string name to an originator.
     * The provided name will be used as the default name in case the
     * originator is not build in the SystemC hierarchy.
     *
     * @param originator_name string name of the originator
     */
    cci_originator(const std::string& originator_name);

    /// Constructor with an originator (char *) name
    /**
     * This form (in addition to @see cci_originator(const std::string&))
     * is necessary to avoid ambiguity between the sc_object,
     * sc_module and std::string overloads for literal string constant
     * arguments.
     *
     * @param originator_name string name of the originator
     */
    explicit cci_originator(const char *originator_name);

    /// Copy constructor
    cci_originator(const cci_originator& originator);

#ifdef CCI_HAS_CXX_RVALUE_REFS
    /// Move constructor
    cci_originator(cci_originator&& originator);
#endif // CCI_HAS_CXX_RVALUE_REFS

    /// Destructor
    ~cci_originator();

    /// Returns a pointer to the current originator
    /**
     * Might return NULL if there is no current originator or the current
     * originator is only given by name (use get_originator_str() instead).
     *
     * @return Originator object pointer or NULL
     */
    const sc_core::sc_object* get_object() const;

    /// Returns the name of the current originator
    /**
     * Automatically uses the originator object name if the originator is
     * given by object pointer.
     *
     * @return Originator name or NULL
     */
    const char* name() const;

    /// Assignment operator overload
    cci_originator& operator=( const cci_originator& originator );

#ifdef CCI_HAS_CXX_RVALUE_REFS
    /// Assignment operator overload
    cci_originator& operator=( cci_originator&& originator );
#endif // CCI_HAS_CXX_RVALUE_REFS

    /// Compare operator overload
    bool operator==( const cci_originator& originator ) const;

    /// Less operator overload
    bool operator<(const cci_originator& originator) const;

    /// Swap originator object and string name with the provided originator.
    /**
     * Assists the assignment operator avoiding code duplication and creating
     * an exception safe implementation.
     *
     * @param that Originator to swap
     */
    void swap(cci_originator& that);

    /// Returns the validity of the current originator
    /**
     * @return true if the originator is unknown, otherwise false.
     */
    bool is_unknown() const;

protected:
    /// Return the current originator object pointer
    sc_core::sc_object* current_originator_object();

    /// Check originator is valid (sc_object or not empty string name)
    void check_is_valid() const;

    /// Returns the string name of the current originator
    const char* string_name() const;

private:
    /// Pointer to the current originator object (priority compared to
    /// name m_originator_str)
    const sc_core::sc_object* m_originator_obj;

    /// Name of the current originator
    const std::string* m_originator_str;
};

CCI_CLOSE_NAMESPACE_

#endif // CCI_CFG_CCI_ORIGINATOR_H_INCLUDED_

