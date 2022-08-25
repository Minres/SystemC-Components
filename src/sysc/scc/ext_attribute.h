/*******************************************************************************
 * Copyright 2017, 2018 MINRES Technologies GmbH
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

#ifndef _SCC_EXT_ATTRIBUTE_H_
#define _SCC_EXT_ATTRIBUTE_H_

#include "configurer.h"
#include "utilities.h"

/** \ingroup scc-sysc
 *  @{
 */
/**@{*/
//! @brief SCC SystemC utilities
namespace scc {
/**
 * @class ext_attribute
 * @brief extended sc_attribute
 *
 * extended attribute inheriting from sc_attribute. The attribute consults with the \ref scc::configurer to retrieve a
 default value
 * upon construction

 * @tparam T type name of value to hold in the attribute
 */
template <typename T> class ext_attribute : public sc_core::sc_attribute<T> {
public:
    using base_type = sc_core::sc_attribute<T>;
    /**
     * create an extended attribute based on name and owner with default value
     *
     * @param name_
     * @param owner
     */
    ext_attribute(const std::string& name_, sc_core::sc_module* owner)
    : base_type(name_)
    , owner(owner) {
        owner->add_attribute(*this);
        configurer::instance().set_configuration_value(this, owner);
    }
    /**
     * create an extended attribute based on name, value and owner
     *
     * @param name_
     * @param value_
     * @param owner
     */
    ext_attribute(const std::string& name_, const T& value_, sc_core::sc_module* owner)
    : base_type(name_, value_)
    , owner(owner) {
        owner->add_attribute(*this);
        configurer::instance().set_configuration_value(this, owner);
    }
    /**
     * no copy constructor
     *
     * @param a
     */
    ext_attribute(const ext_attribute<T>& a) = delete;
    /**
     * a default destructor
     */
    ~ext_attribute() = default;
    /**
     * the owner of this attribute (a backward reference)
     */
    const sc_core::sc_module* owner;
};
}; // namespace scc
/** @} */ // end of scc-sysc
#endif /* _SYSC_EXT_ATTRIBUTE_H_ */
