/*******************************************************************************
 * Copyright 2022 MINRES Technologies GmbH
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

#ifndef _SCC_SC_ATTRIBUTE_RANDOMIZED_H_
#define _SCC_SC_ATTRIBUTE_RANDOMIZED_H_

#include "mt19937_rng.h"
#include <sysc/kernel/sc_attribute.h>
#include <type_traits>

/** \ingroup scc-sysc
 *  @{
 */
/**@{*/
//! @brief SCC SystemC utilities
namespace scc {
template <typename T>
class sc_attribute_randomized: public sc_core::sc_attribute<T> {
public:
    sc_attribute_randomized( const std::string& name_ )
        : sc_core::sc_attribute<T>( name_ )
        {}

    sc_attribute_randomized( const std::string& name_, const T& value_ )
        : sc_core::sc_attribute<T>( name_ )
        {}

    sc_attribute_randomized( const sc_core::sc_attribute<T>& a )
        : sc_core::sc_attribute<T>( a.name() )
        {}


    // destructor (does nothing)

    virtual ~sc_attribute_randomized() {}

    template< bool cond, typename U >
    using resolvedType  = typename std::enable_if< cond, U >::type;

    template< typename U = T >
    resolvedType< std::is_signed<T>::value, U >
    get_value(){
        if(this->value < 0)
            return MT19937::uniform(0, -this->value);
        else
            return this->value;
    }

    template< typename U = T >
    resolvedType< std::is_unsigned<T>::value, U >
    get_value(){
            return this->value;
    }
};
}
/** @} */ // end of scc-sysc
#endif /* _SCC_SC_ATTRIBUTE_RANDOMIZED_H_ */
