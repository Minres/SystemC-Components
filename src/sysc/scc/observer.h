/*******************************************************************************
 * Copyright 2018-2021 MINRES Technologies GmbH
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

#ifndef _SCC_OBSERVER_H_
#define _SCC_OBSERVER_H_

#include <string>
//needed for typedefs like sc_dt::int64
#include "sysc/datatypes/int/sc_nbdefs.h"
// Some forward declarations
namespace sc_dt {
    class sc_bit;
    class sc_logic;
    class sc_bv_base;
    class sc_lv_base;
    class sc_signed;
    class sc_unsigned;
    class sc_int_base;
    class sc_uint_base;
    class sc_fxval;
    class sc_fxval_fast;
    class sc_fxnum;
    class sc_fxnum_fast;
}

namespace scc {
/**
 * @brief The interface defining an observer.
 */
struct observer {
    /**
     * @brief A handle to be used be the observed object to notify the observer about a change
     */
    struct notification_handle{
        virtual void notify() = 0;
        virtual ~notification_handle(){}
    };
#define DECL_REGISTER_METHOD_A(tp) virtual notification_handle* observe(tp const& o, std::string const& nm) = 0;
#define DECL_REGISTER_METHOD_B(tp) virtual notification_handle* observe(tp const& o, std::string const& nm, int width) = 0;
    DECL_REGISTER_METHOD_A( bool )
    DECL_REGISTER_METHOD_A( sc_dt::sc_bit )
    DECL_REGISTER_METHOD_A( sc_dt::sc_logic )

    DECL_REGISTER_METHOD_B( unsigned char )
    DECL_REGISTER_METHOD_B( unsigned short )
    DECL_REGISTER_METHOD_B( unsigned int )
    DECL_REGISTER_METHOD_B( unsigned long )
    DECL_REGISTER_METHOD_B( char )
    DECL_REGISTER_METHOD_B( short )
    DECL_REGISTER_METHOD_B( int )
    DECL_REGISTER_METHOD_B( long )
    DECL_REGISTER_METHOD_B( sc_dt::int64 )
    DECL_REGISTER_METHOD_B( sc_dt::uint64 )

    DECL_REGISTER_METHOD_A( float )
    DECL_REGISTER_METHOD_A( double )
    DECL_REGISTER_METHOD_A( sc_dt::sc_int_base )
    DECL_REGISTER_METHOD_A( sc_dt::sc_uint_base )
    DECL_REGISTER_METHOD_A( sc_dt::sc_signed )
    DECL_REGISTER_METHOD_A( sc_dt::sc_unsigned )

    DECL_REGISTER_METHOD_A( sc_dt::sc_fxval )
    DECL_REGISTER_METHOD_A( sc_dt::sc_fxval_fast )
    DECL_REGISTER_METHOD_A( sc_dt::sc_fxnum )
    DECL_REGISTER_METHOD_A( sc_dt::sc_fxnum_fast )

    DECL_REGISTER_METHOD_A( sc_dt::sc_bv_base )
    DECL_REGISTER_METHOD_A( sc_dt::sc_lv_base )


#undef DECL_REGISTER_METHOD_A
#undef DECL_REGISTER_METHOD_B
    virtual ~observer(){}

};

#define DECL_REGISTER_METHOD_A(tp) inline observer::notification_handle* observe(observer* obs, tp const& o, std::string const& nm){\
    return obs->observe(o, nm); }
#define DECL_REGISTER_METHOD_B(tp) inline observer::notification_handle* observe(observer* obs, tp const& o, std::string const& nm, int width = 8 * sizeof( tp )){\
    return obs->observe(o, nm, width); }

    DECL_REGISTER_METHOD_A( bool )
    DECL_REGISTER_METHOD_A( sc_dt::sc_bit )
    DECL_REGISTER_METHOD_A( sc_dt::sc_logic )

    DECL_REGISTER_METHOD_B( unsigned char )
    DECL_REGISTER_METHOD_B( unsigned short )
    DECL_REGISTER_METHOD_B( unsigned int )
    DECL_REGISTER_METHOD_B( unsigned long )
    DECL_REGISTER_METHOD_B( char )
    DECL_REGISTER_METHOD_B( short )
    DECL_REGISTER_METHOD_B( int )
    DECL_REGISTER_METHOD_B( long )
    DECL_REGISTER_METHOD_B( sc_dt::int64 )
    DECL_REGISTER_METHOD_B( sc_dt::uint64 )

    DECL_REGISTER_METHOD_A( float )
    DECL_REGISTER_METHOD_A( double )
    DECL_REGISTER_METHOD_A( sc_dt::sc_int_base )
    DECL_REGISTER_METHOD_A( sc_dt::sc_uint_base )
    DECL_REGISTER_METHOD_A( sc_dt::sc_signed )
    DECL_REGISTER_METHOD_A( sc_dt::sc_unsigned )

    DECL_REGISTER_METHOD_A( sc_dt::sc_fxval )
    DECL_REGISTER_METHOD_A( sc_dt::sc_fxval_fast )
    DECL_REGISTER_METHOD_A( sc_dt::sc_fxnum )
    DECL_REGISTER_METHOD_A( sc_dt::sc_fxnum_fast )

    DECL_REGISTER_METHOD_A( sc_dt::sc_bv_base )
    DECL_REGISTER_METHOD_A( sc_dt::sc_lv_base )


#undef DECL_REGISTER_METHOD_A
#undef DECL_REGISTER_METHOD_B

} /* namespace scc */

#endif /* _SCC_OBSERVER_H_ */
