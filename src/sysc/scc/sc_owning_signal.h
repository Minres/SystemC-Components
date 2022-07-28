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
#include <tlm>
/** \ingroup scc-sysc
 *  @{
 */
/**@{*/
//! @brief SCC SystemC utilities
namespace scc {
/**
 * @class sc_owning_signal
 * @brief sc_signal which takes ownership of the data (acquire()/release())
 *
 * @tparam T
 * @tparam POL
 */
template <class T, sc_core::sc_writer_policy POL = sc_core::SC_ONE_WRITER>
class sc_owning_signal : public sc_core::sc_signal<T*, POL> {
protected:
    using policy_type = sc_core::sc_writer_policy_check<POL>;
    using super = sc_core::sc_signal<T*, POL>;
    using type = T*;

public: // constructors and destructor:
    sc_owning_signal()
    : sc_core::sc_signal<T*, POL>(sc_core::sc_gen_unique_name("signal")) {}

    explicit sc_owning_signal(const char* name_)
    : sc_core::sc_signal<T*, POL>(name_, nullptr) {}

    sc_owning_signal(const char* name_, T* initial_value_)
    : sc_core::sc_signal<T*, POL>(name_, initial_value_) {}

    virtual ~sc_owning_signal() {}

    // write the new value
    void write(const type& value_) override {
        bool value_changed = !(super::m_cur_val == value_);
        if(!policy_type::check_write(this, value_changed))
            return;
        // release a potentially previosu written value
        if(super::m_new_val && super::m_new_val != super::m_cur_val && super::m_new_val->has_mm())
            super::m_new_val->release();
        super::m_new_val = value_;
        // acquire the scheduled value
        if(super::m_new_val && super::m_new_val->has_mm())
            super::m_new_val->acquire();
        if(value_changed)
            super::request_update();
    }

    void clear() {
        super::m_new_val = nullptr;
        if(super::m_new_val != super::m_cur_val)
            super::request_update();
    }

    using super::operator=;

protected:
    void update() override {
        if(!(super::m_new_val == super::m_cur_val)) {
            if(super::m_cur_val && super::m_cur_val->has_mm())
                super::m_cur_val->release();
            super::update();
        }
    }
};

} // namespace scc
/** @} */ // end of scc-sysc
#endif /* _SCC_SC_SIGNAL_GP_H_ */
