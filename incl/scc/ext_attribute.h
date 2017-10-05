/*******************************************************************************
 * Copyright 2017 MINRES Technologies GmbH
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
/*
 * atrribute_mixin.h
 *
 *  Created on: 27.09.2017
 *      Author: eyck
 */

#ifndef _SYSC_EXT_ATTRIBUTE_H_
#define _SYSC_EXT_ATTRIBUTE_H_

#include "scc/configurer.h"
#include "scc/utilities.h"

namespace scc {
template <typename T> class ext_attribute : public sc_core::sc_attribute<T> {
public:
    using base_type = sc_core::sc_attribute<T>;

    ext_attribute(const std::string &name_, sc_core::sc_module *owner)
    : base_type(name_)
    , owner(owner) {
        owner->add_attribute(*this);
        configurer::instance().set_configuration_value(this, owner);
    }

    ext_attribute(const std::string &name_, const T &value_, sc_core::sc_module *owner)
    : base_type(name_, value_)
    , owner(owner) {
        owner->add_attribute(*this);
        configurer::instance().set_configuration_value(this, owner);
    }

    ext_attribute(const ext_attribute<T> &a) = delete;

    ~ext_attribute() = default;

    const sc_core::sc_module *owner;
};
};

#endif /* _SYSC_EXT_ATTRIBUTE_H_ */
