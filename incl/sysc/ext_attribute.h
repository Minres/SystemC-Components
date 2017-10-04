/*
 * atrribute_mixin.h
 *
 *  Created on: 27.09.2017
 *      Author: eyck
 */

#ifndef _SYSC_EXT_ATTRIBUTE_H_
#define _SYSC_EXT_ATTRIBUTE_H_

#include "configurer.h"
#include "utilities.h"

namespace sysc {
template <typename T>
class ext_attribute: public sc_core::sc_attribute<T> {
public:
    using base_type = sc_core::sc_attribute<T>;

    ext_attribute( const std::string& name_, sc_core::sc_module* owner)
    : base_type(name_), owner(owner)
    {
        owner->add_attribute(*this);
        configurer::instance().set_configuration_value(this, owner);
    }

    ext_attribute( const std::string& name_, const T& value_, sc_core::sc_module* owner)
    : base_type(name_, value_), owner(owner)
    {
        owner->add_attribute(*this);
        configurer::instance().set_configuration_value(this, owner);
    }

    ext_attribute( const ext_attribute<T>& a ) = delete;

    ~ext_attribute() = default;

    const sc_core::sc_module* owner;
};
};

#endif /* _SYSC_EXT_ATTRIBUTE_H_ */
