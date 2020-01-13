/*******************************************************************************
 * Copyright 2020 MINRES Technologies GmbH
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

#ifndef _SCC_SC_SIGNAL_GP_H_
#define _SCC_SC_SIGNAL_GP_H_

#include <sysc/communication/sc_signal.h>
#include <tlm_core/tlm_2/tlm_generic_payload/tlm_gp.h>
namespace scc {

template< sc_core::sc_writer_policy POL = sc_core::SC_ONE_WRITER>
class sc_signal_gp : public sc_core::sc_signal<tlm::tlm_generic_payload*,POL>
{
protected:
    using policy_type = sc_writer_policy_check<POL> ;
    using super = sc_signal<tlm::tlm_generic_payload*,POL>;
    using type = tlm::tlm_generic_payload*;
public: // constructors and destructor:

    sc_signal_gp()
    : sc_signal<tlm::tlm_generic_payload*,POL>( sc_gen_unique_name( "signal" ) )
    {}

    explicit sc_signal_gp( const char* name_ )
    : sc_signal<tlm::tlm_generic_payload*,POL>( name_ )
    {}

    sc_signal_gp( const char* name_, tlm::tlm_generic_payload* initial_value_ )
      : sc_signal<tlm::tlm_generic_payload*,POL>( name_ )
    {}

    virtual ~sc_signal_gp(){}

    // write the new value
    void write( const type& value_) override {
        bool value_changed = !( super::m_cur_val == value_ );
        if ( !policy_type::check_write(this, value_changed) ) return;
        // release a potentially previosu written value
        if(super::m_new_val && super::m_new_val != super::m_cur_val && super::m_new_val->has_mm()) m_new_val->release();
        super::m_new_val = value_;
        // acquire the scheduled value
        if(super::m_new_val && super::m_new_val->has_mm()) m_new_val->acquire();
        if( value_changed ) request_update();
    }

    using super::operator =;
protected:

    void update() override {
        if( !( super::m_new_val == super::m_cur_val ) ) {
          if(super::m_cur_val && super::m_cur_val->has_mm()) super::m_cur_val->release();
          super::update();
        }
    }

};

}
#endif /* _SCC_SC_SIGNAL_GP_H_ */
