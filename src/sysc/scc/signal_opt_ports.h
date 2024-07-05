/*******************************************************************************
 * Copyright 2001 - 2023 Accellera Systems Initiative Inc. (Accellera)
 * Copyright 2023 MINRES Technologies GmbH
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
 *  Created on: Dec 17, 2023
 *      Author: eyck
 */

#ifndef _SYSC_SCC_OPT_SIGNAL_PORTS_H_
#define _SYSC_SCC_OPT_SIGNAL_PORTS_H_

#include <systemc>

#if !defined(SC_DISABLE_VIRTUAL_BIND)
#define SCC_VIRT virtual
#else
#define SCC_VIRT /* non-virtual */
#endif

namespace scc {

template <class T> class sc_in_opt : public sc_core::sc_port<sc_core::sc_signal_in_if<T>, 1, sc_core::SC_ZERO_OR_MORE_BOUND> {
public:
    using data_type = T;
    using if_type = sc_core::sc_signal_in_if<data_type> ;
    using base_type = sc_core::sc_port<if_type, 1, sc_core::SC_ZERO_OR_MORE_BOUND> ;
    using this_type = sc_in_opt<data_type> ;
    using base_port_type = typename base_type::port_type ;

    using in_if_type = if_type ;
    using in_port_type = base_type;
    using inout_if_type = sc_core::sc_signal_inout_if<data_type>;
    using inout_port_type = sc_core::sc_port<inout_if_type, 1, sc_core::SC_ZERO_OR_MORE_BOUND>;

public:
    sc_in_opt()
    : base_type() {}

    explicit sc_in_opt(const char* name_)
    : base_type(name_) {}

    explicit sc_in_opt(const in_if_type& interface_)
    : base_type(const_cast<in_if_type&>(interface_)) {}

    sc_in_opt(const char* name_, const in_if_type& interface_)
    : base_type(name_, const_cast<in_if_type&>(interface_)) {}

    explicit sc_in_opt(in_port_type& parent_)
    : base_type(parent_) {}

    sc_in_opt(const char* name_, in_port_type& parent_)
    : base_type(name_, parent_) {}

    explicit sc_in_opt(inout_port_type& parent_)
    : base_type() {
        sc_core::sc_port_base::bind(parent_);
    }

    sc_in_opt(const char* name_, inout_port_type& parent_)
    : base_type(name_) {
        sc_core::sc_port_base::bind(parent_);
    }

    sc_in_opt(this_type& parent_)
    : base_type(parent_) {}

    sc_in_opt(const char* name_, this_type& parent_)
    : base_type(name_, parent_) {}

    sc_in_opt(const this_type&) = delete;

    virtual ~sc_in_opt() {}

    this_type& operator=(const this_type&) = delete;

    SCC_VIRT void bind(const in_if_type& interface_) { sc_core::sc_port_base::bind(const_cast<in_if_type&>(interface_)); }

    SCC_VIRT void bind(in_if_type& interface_) override { this->bind(const_cast<const in_if_type&>(interface_)); }

    void operator()(const in_if_type& interface_) { this->bind(interface_); }

    SCC_VIRT void bind(in_port_type& parent_) { sc_core::sc_port_base::bind(parent_); }

    void operator()(in_port_type& parent_) { this->bind(parent_); }

    SCC_VIRT void bind(sc_core::sc_port<if_type, 1, sc_core::SC_ONE_OR_MORE_BOUND>& parent_){ sc_core::sc_port_base::bind(parent_); }

    void operator()(sc_core::sc_port<if_type, 1, sc_core::SC_ONE_OR_MORE_BOUND>& parent_) { this->bind(parent_); }

    SCC_VIRT void bind(inout_port_type& parent_) { sc_core::sc_port_base::bind(parent_); }

    void operator()(inout_port_type& parent_) { this->bind(parent_); }

    SCC_VIRT void bind(sc_core::sc_port<inout_if_type, 1, sc_core::SC_ONE_OR_MORE_BOUND>& parent_) { sc_core::sc_port_base::bind(parent_); }

    void operator()(sc_core::sc_port<inout_if_type, 1, sc_core::SC_ONE_OR_MORE_BOUND>& parent_) { this->bind(parent_); }

    const sc_core::sc_event& default_event() const { return (*this)->default_event(); }

    const sc_core::sc_event& value_changed_event() const { return (*this)->value_changed_event(); }

    const data_type& read() const { return (*this)->read(); }

    operator const data_type&() const { return (*this)->read(); }

    bool event() const { return (*this)->event(); }

    virtual const char* kind() const override { return "sc_in"; }
};

template <typename T>::std::ostream& operator<<(::std::ostream& os, const sc_in_opt<T>& a) { return os << a->read(); }
} // namespace scc

SC_API_TEMPLATE_DECL_ sc_core::sc_port<sc_core::sc_signal_in_if<bool>, 1, sc_core::SC_ZERO_OR_MORE_BOUND>;

namespace scc {
template <> class SC_API sc_in_opt<bool> : public sc_core::sc_port<sc_core::sc_signal_in_if<bool>, 1, sc_core::SC_ZERO_OR_MORE_BOUND> {
public:
    typedef bool data_type;

    typedef sc_core::sc_signal_in_if<data_type> if_type;
    typedef sc_core::sc_port<if_type, 1, sc_core::SC_ZERO_OR_MORE_BOUND> base_type;
    typedef sc_in_opt<data_type> this_type;
    typedef /* typename */ base_type::port_type base_port_type;

    typedef if_type in_if_type;
    typedef base_type in_port_type;
    typedef sc_core::sc_signal_inout_if<data_type> inout_if_type;
    typedef sc_core::sc_port<inout_if_type, 1, sc_core::SC_ZERO_OR_MORE_BOUND> inout_port_type;

public:
    sc_in_opt()
    : base_type() {}

    explicit sc_in_opt(const char* name_)
    : base_type(name_) {}

    explicit sc_in_opt(const in_if_type& interface_)
    : base_type(const_cast<in_if_type&>(interface_)) {}

    sc_in_opt(const char* name_, const in_if_type& interface_)
    : base_type(name_, const_cast<in_if_type&>(interface_)) {}

    explicit sc_in_opt(in_port_type& parent_)
    : base_type(parent_) {}

    sc_in_opt(const char* name_, in_port_type& parent_)
    : base_type(name_, parent_) {}

    explicit sc_in_opt(inout_port_type& parent_)
    : base_type() {
        sc_port_base::bind(parent_);
    }

    sc_in_opt(const char* name_, inout_port_type& parent_)
    : base_type(name_) {
        sc_port_base::bind(parent_);
    }

    sc_in_opt(this_type& parent_)
    : base_type(parent_) {}

    sc_in_opt(const char* name_, this_type& parent_)
    : base_type(name_, parent_) {}

    sc_in_opt(const this_type&) = delete;

    virtual ~sc_in_opt() = default;

    this_type& operator=(const this_type&) = delete;

    SCC_VIRT void bind(const in_if_type& interface_) { sc_port_base::bind(const_cast<in_if_type&>(interface_)); }

    SCC_VIRT void bind(in_if_type& interface_) override { this->bind(const_cast<const in_if_type&>(interface_)); }

    void operator()(const in_if_type& interface_) { this->bind(interface_); }

    SCC_VIRT void bind(in_port_type& parent_) { sc_port_base::bind(parent_); }

    void operator()(in_port_type& parent_) { this->bind(parent_); }

    SCC_VIRT void bind(inout_port_type& parent_) { sc_port_base::bind(parent_); }

    void operator()(inout_port_type& parent_) { this->bind(parent_); }

    const sc_core::sc_event& default_event() const { return (*this)->default_event(); }

    const sc_core::sc_event& value_changed_event() const { return (*this)->value_changed_event(); }

    const sc_core::sc_event& posedge_event() const { return (*this)->posedge_event(); }

    const sc_core::sc_event& negedge_event() const { return (*this)->negedge_event(); }

    const data_type& read() const { return (*this)->read(); }

    operator const data_type&() const { return (*this)->read(); }

    bool event() const { return (*this)->event(); }

    bool posedge() const { return (*this)->posedge(); }

    bool negedge() const { return (*this)->negedge(); }

    virtual const char* kind() const override { return "sc_in"; }
};

} // namespace scc

SC_API_TEMPLATE_DECL_ sc_core::sc_port<sc_core::sc_signal_in_if<sc_dt::sc_logic>, 1, sc_core::SC_ZERO_OR_MORE_BOUND>;

namespace scc {
template <>
class SC_API sc_in_opt<sc_dt::sc_logic>
: public sc_core::sc_port<sc_core::sc_signal_in_if<sc_dt::sc_logic>, 1, sc_core::SC_ZERO_OR_MORE_BOUND> {
public:
    typedef sc_dt::sc_logic data_type;

    typedef sc_core::sc_signal_in_if<data_type> if_type;
    typedef sc_core::sc_port<if_type, 1, sc_core::SC_ZERO_OR_MORE_BOUND> base_type;
    typedef sc_in_opt<data_type> this_type;
    typedef /* typename */ base_type::port_type base_port_type;

    typedef if_type in_if_type;
    typedef base_type in_port_type;
    typedef sc_core::sc_signal_inout_if<data_type> inout_if_type;
    typedef sc_core::sc_port<inout_if_type, 1, sc_core::SC_ZERO_OR_MORE_BOUND> inout_port_type;

public:
    sc_in_opt()
    : base_type() {}

    explicit sc_in_opt(const char* name_)
    : base_type(name_) {}

    explicit sc_in_opt(const in_if_type& interface_)
    : base_type(const_cast<in_if_type&>(interface_)) {}

    sc_in_opt(const char* name_, const in_if_type& interface_)
    : base_type(name_, const_cast<in_if_type&>(interface_)) {}

    explicit sc_in_opt(in_port_type& parent_)
    : base_type(parent_) {}

    sc_in_opt(const char* name_, in_port_type& parent_)
    : base_type(name_, parent_) {}

    explicit sc_in_opt(inout_port_type& parent_)
    : base_type() {
        sc_port_base::bind(parent_);
    }

    sc_in_opt(const char* name_, inout_port_type& parent_)
    : base_type(name_) {
        sc_port_base::bind(parent_);
    }

    sc_in_opt(this_type& parent_)
    : base_type(parent_) {}

    sc_in_opt(const char* name_, this_type& parent_)
    : base_type(name_, parent_) {}

    sc_in_opt(const this_type&) = delete;

    virtual ~sc_in_opt() = default;

    this_type& operator=(const this_type&) = delete;

    SCC_VIRT void bind(const in_if_type& interface_) { sc_port_base::bind(const_cast<in_if_type&>(interface_)); }

    SCC_VIRT void bind(in_if_type& interface_) override { this->bind(const_cast<const in_if_type&>(interface_)); }

    void operator()(const in_if_type& interface_) { this->bind(interface_); }

    SCC_VIRT void bind(in_port_type& parent_) { sc_port_base::bind(parent_); }

    void operator()(in_port_type& parent_) { this->bind(parent_); }

    SCC_VIRT void bind(inout_port_type& parent_) { sc_port_base::bind(parent_); }

    void operator()(inout_port_type& parent_) { this->bind(parent_); }

    const sc_core::sc_event& default_event() const { return (*this)->default_event(); }

    const sc_core::sc_event& value_changed_event() const { return (*this)->value_changed_event(); }

    const sc_core::sc_event& posedge_event() const { return (*this)->posedge_event(); }

    const sc_core::sc_event& negedge_event() const { return (*this)->negedge_event(); }

    const data_type& read() const { return (*this)->read(); }

    operator const data_type&() const { return (*this)->read(); }

    bool event() const { return (*this)->event(); }

    bool posedge() const { return (*this)->posedge(); }

    bool negedge() const { return (*this)->negedge(); }

    virtual const char* kind() const override { return "sc_in"; }
};

template <class T> class sc_inout_opt : public sc_core::sc_port<sc_core::sc_signal_inout_if<T>, 1, sc_core::SC_ZERO_OR_MORE_BOUND> {
public:
    typedef T data_type;

    typedef sc_core::sc_signal_inout_if<data_type> if_type;
    typedef sc_core::sc_port<if_type, 1, sc_core::SC_ZERO_OR_MORE_BOUND> base_type;
    typedef sc_inout_opt<data_type> this_type;

    typedef sc_core::sc_signal_in_if<data_type> in_if_type;
    typedef sc_core::sc_port<in_if_type, 1, sc_core::SC_ZERO_OR_MORE_BOUND> in_port_type;
    typedef if_type inout_if_type;
    typedef base_type inout_port_type;

public:
    sc_inout_opt()
    : base_type()
    , m_init_val(0) {}

    explicit sc_inout_opt(const char* name_)
    : base_type(name_)
    , m_init_val(0) {}

    explicit sc_inout_opt(inout_if_type& interface_)
    : base_type(interface_)
    , m_init_val(0) {}

    sc_inout_opt(const char* name_, inout_if_type& interface_)
    : base_type(name_, interface_)
    , m_init_val(0) {}

    explicit sc_inout_opt(inout_port_type& parent_)
    : base_type(parent_)
    , m_init_val(0) {}

    sc_inout_opt(const char* name_, inout_port_type& parent_)
    : base_type(name_, parent_)
    , m_init_val(0) {}

    sc_inout_opt(this_type& parent_)
    : base_type(parent_)
    , m_init_val(0) {}

    sc_inout_opt(const char* name_, this_type& parent_)
    : base_type(name_, parent_)
    , m_init_val(0) {}

    sc_inout_opt(const this_type&) = delete;

    virtual ~sc_inout_opt() = default;

    const sc_core::sc_event& default_event() const { return (*this)->default_event(); }

    const sc_core::sc_event& value_changed_event() const { return (*this)->value_changed_event(); }

    const data_type& read() const { return (*this)->read(); }

    operator const data_type&() const { return (*this)->read(); }

    bool event() const { return (*this)->event(); }

    void write(const data_type& value_) { (*this)->write(value_); }

    this_type& operator=(const data_type& value_) {
        (*this)->write(value_);
        return *this;
    }

    this_type& operator=(const in_if_type& interface_) {
        (*this)->write(interface_.read());
        return *this;
    }

    this_type& operator=(const in_port_type& port_) {
        (*this)->write(port_->read());
        return *this;
    }

    this_type& operator=(const inout_port_type& port_) {
        (*this)->write(port_->read());
        return *this;
    }

    this_type& operator=(const this_type& port_) {
        (*this)->write(port_->read());
        return *this;
    }

    void initialize(const data_type& value_);

    void initialize(const in_if_type& interface_) { initialize(interface_.read()); }

    void end_of_elaboration() override;

    virtual const char* kind() const override { return "sc_inout"; }

protected:
    data_type* m_init_val;
};

template <typename T>::std::ostream& operator<<(::std::ostream& os, const sc_inout_opt<T>& a) { return os << a->read(); }

template <class T> inline void sc_inout_opt<T>::initialize(const data_type& value_) {
    inout_if_type* iface = dynamic_cast<inout_if_type*>(this->get_interface());
    if(iface != 0) {
        iface->write(value_);
    } else {
        if(m_init_val == 0) {
            m_init_val = new data_type;
        }
        *m_init_val = value_;
    }
}

template <class T> inline void sc_inout_opt<T>::end_of_elaboration() {
    if(m_init_val != 0) {
        write(*m_init_val);
        delete m_init_val;
        m_init_val = 0;
    }
}
} // namespace scc

#ifndef CWR_SYSTEMC
SC_API_TEMPLATE_DECL_ sc_core::sc_port<sc_core::sc_signal_inout_if<bool>, 1, sc_core::SC_ZERO_OR_MORE_BOUND>;
#endif

namespace scc {
template <>
class SC_API sc_inout_opt<bool> : public sc_core::sc_port<sc_core::sc_signal_inout_if<bool>, 1, sc_core::SC_ZERO_OR_MORE_BOUND> {
public:
    typedef bool data_type;

    typedef sc_core::sc_signal_inout_if<data_type> if_type;
    typedef sc_core::sc_port<if_type, 1, sc_core::SC_ZERO_OR_MORE_BOUND> base_type;
    typedef sc_inout_opt<data_type> this_type;

    typedef sc_core::sc_signal_in_if<data_type> in_if_type;
    typedef sc_core::sc_port<in_if_type, 1, sc_core::SC_ZERO_OR_MORE_BOUND> in_port_type;
    typedef if_type inout_if_type;
    typedef base_type inout_port_type;

public:
    sc_inout_opt()
    : base_type()
    , m_init_val(0) {}

    explicit sc_inout_opt(const char* name_)
    : base_type(name_)
    , m_init_val(0) {}

    explicit sc_inout_opt(inout_if_type& interface_)
    : base_type(interface_)
    , m_init_val(0) {}

    sc_inout_opt(const char* name_, inout_if_type& interface_)
    : base_type(name_, interface_)
    , m_init_val(0) {}

    explicit sc_inout_opt(inout_port_type& parent_)
    : base_type(parent_)
    , m_init_val(0) {}

    sc_inout_opt(const char* name_, inout_port_type& parent_)
    : base_type(name_, parent_)
    , m_init_val(0) {}

    sc_inout_opt(this_type& parent_)
    : base_type(parent_)
    , m_init_val(0) {}

    sc_inout_opt(const char* name_, this_type& parent_)
    : base_type(name_, parent_)
    , m_init_val(0) {}

    sc_inout_opt(const this_type&) = delete;

    virtual ~sc_inout_opt() = default;

    const sc_core::sc_event& default_event() const { return (*this)->default_event(); }

    const sc_core::sc_event& value_changed_event() const { return (*this)->value_changed_event(); }

    const sc_core::sc_event& posedge_event() const { return (*this)->posedge_event(); }

    const sc_core::sc_event& negedge_event() const { return (*this)->negedge_event(); }

    const data_type& read() const { return (*this)->read(); }

    operator const data_type&() const { return (*this)->read(); }

    bool event() const { return (*this)->event(); }

    bool posedge() const { return (*this)->posedge(); }

    bool negedge() const { return (*this)->negedge(); }

    void write(const data_type& value_) { (*this)->write(value_); }

    this_type& operator=(const data_type& value_) {
        (*this)->write(value_);
        return *this;
    }

    this_type& operator=(const in_if_type& interface_) {
        (*this)->write(interface_.read());
        return *this;
    }

    this_type& operator=(const in_port_type& port_) {
        (*this)->write(port_->read());
        return *this;
    }

    this_type& operator=(const inout_port_type& port_) {
        (*this)->write(port_->read());
        return *this;
    }

    this_type& operator=(const this_type& port_) {
        (*this)->write(port_->read());
        return *this;
    }

    void initialize(const data_type& value_);

    void initialize(const in_if_type& interface_) { initialize(interface_.read()); }

    void end_of_elaboration() override {
        if(m_init_val != 0) {
            write(*m_init_val);
            delete m_init_val;
            m_init_val = 0;
        }
    }

    virtual const char* kind() const override { return "sc_inout"; }

protected:
    data_type* m_init_val;
};
} // namespace scc

#ifndef CWR_SYSTEMC
SC_API_TEMPLATE_DECL_ sc_core::sc_port<sc_core::sc_signal_inout_if<sc_dt::sc_logic>, 1, sc_core::SC_ZERO_OR_MORE_BOUND>;
#endif

namespace scc {
template <>
class SC_API sc_inout_opt<sc_dt::sc_logic>
: public sc_core::sc_port<sc_core::sc_signal_inout_if<sc_dt::sc_logic>, 1, sc_core::SC_ZERO_OR_MORE_BOUND> {
public:
    typedef sc_dt::sc_logic data_type;

    typedef sc_core::sc_signal_inout_if<data_type> if_type;
    typedef sc_core::sc_port<if_type, 1, sc_core::SC_ZERO_OR_MORE_BOUND> base_type;
    typedef sc_inout_opt<data_type> this_type;

    typedef sc_core::sc_signal_in_if<data_type> in_if_type;
    typedef sc_core::sc_port<in_if_type, 1, sc_core::SC_ZERO_OR_MORE_BOUND> in_port_type;
    typedef if_type inout_if_type;
    typedef base_type inout_port_type;

public:
    sc_inout_opt()
    : base_type()
    , m_init_val(0) {}

    explicit sc_inout_opt(const char* name_)
    : base_type(name_)
    , m_init_val(0) {}

    explicit sc_inout_opt(inout_if_type& interface_)
    : base_type(interface_)
    , m_init_val(0) {}

    sc_inout_opt(const char* name_, inout_if_type& interface_)
    : base_type(name_, interface_)
    , m_init_val(0) {}

    explicit sc_inout_opt(inout_port_type& parent_)
    : base_type(parent_)
    , m_init_val(0) {}

    sc_inout_opt(const char* name_, inout_port_type& parent_)
    : base_type(name_, parent_)
    , m_init_val(0) {}

    sc_inout_opt(this_type& parent_)
    : base_type(parent_)
    , m_init_val(0) {}

    sc_inout_opt(const char* name_, this_type& parent_)
    : base_type(name_, parent_)
    , m_init_val(0) {}

    sc_inout_opt(const this_type&) = delete;

    virtual ~sc_inout_opt();

    const sc_core::sc_event& default_event() const { return (*this)->default_event(); }

    const sc_core::sc_event& value_changed_event() const { return (*this)->value_changed_event(); }

    const sc_core::sc_event& posedge_event() const { return (*this)->posedge_event(); }

    const sc_core::sc_event& negedge_event() const { return (*this)->negedge_event(); }

    const data_type& read() const { return (*this)->read(); }

    operator const data_type&() const { return (*this)->read(); }

    bool event() const { return (*this)->event(); }

    bool posedge() const { return (*this)->posedge(); }

    bool negedge() const { return (*this)->negedge(); }

    void write(const data_type& value_) { (*this)->write(value_); }

    this_type& operator=(const data_type& value_) {
        (*this)->write(value_);
        return *this;
    }

    this_type& operator=(const in_if_type& interface_) {
        (*this)->write(interface_.read());
        return *this;
    }

    this_type& operator=(const in_port_type& port_) {
        (*this)->write(port_->read());
        return *this;
    }

    this_type& operator=(const inout_port_type& port_) {
        (*this)->write(port_->read());
        return *this;
    }

    this_type& operator=(const this_type& port_) {
        (*this)->write(port_->read());
        return *this;
    }

    void initialize(const data_type& value_);

    void initialize(const in_if_type& interface_) { initialize(interface_.read()); }

    // called when elaboration is done
    void end_of_elaboration() override {
        if(m_init_val != 0) {
            write(*m_init_val);
            delete m_init_val;
            m_init_val = 0;
        }
    }

    virtual const char* kind() const override { return "sc_inout"; }

protected:
    data_type* m_init_val;
};

template <class T> class sc_out_opt : public sc_inout_opt<T> {
public:
    typedef T data_type;

    typedef sc_out_opt<data_type> this_type;
    typedef sc_inout_opt<data_type> base_type;

    typedef typename base_type::in_if_type in_if_type;
    typedef typename base_type::in_port_type in_port_type;
    typedef typename base_type::inout_if_type inout_if_type;
    typedef typename base_type::inout_port_type inout_port_type;

public:
    sc_out_opt()
    : base_type() {}

    explicit sc_out_opt(const char* name_)
    : base_type(name_) {}

    explicit sc_out_opt(inout_if_type& interface_)
    : base_type(interface_) {}

    sc_out_opt(const char* name_, inout_if_type& interface_)
    : base_type(name_, interface_) {}

    explicit sc_out_opt(inout_port_type& parent_)
    : base_type(parent_) {}

    sc_out_opt(const char* name_, inout_port_type& parent_)
    : base_type(name_, parent_) {}

    sc_out_opt(this_type& parent_)
    : base_type(parent_) {}

    sc_out_opt(const char* name_, this_type& parent_)
    : base_type(name_, parent_) {}

    virtual ~sc_out_opt() {}

    this_type& operator=(const data_type& value_) {
        (*this)->write(value_);
        return *this;
    }

    this_type& operator=(const in_if_type& interface_) {
        (*this)->write(interface_.read());
        return *this;
    }

    this_type& operator=(const in_port_type& port_) {
        (*this)->write(port_->read());
        return *this;
    }

    this_type& operator=(const inout_port_type& port_) {
        (*this)->write(port_->read());
        return *this;
    }

    this_type& operator=(const this_type& port_) {
        (*this)->write(port_->read());
        return *this;
    }

    virtual const char* kind() const override{ return "sc_out"; }

private:
    sc_out_opt(const this_type&) = delete;
};

template <class T> inline void sc_trace(sc_core::sc_trace_file* tf, const sc_in_opt<T>& port, const std::string& name) {
    const sc_core::sc_signal_in_if<T>* iface = 0;
    if(sc_core::sc_get_curr_simcontext()->elaboration_done()) {
        iface = dynamic_cast<const sc_core::sc_signal_in_if<T>*>(port.get_interface());
    }
    if(iface)
        sc_trace(tf, iface->read(), name);
    else
        port.add_trace_internal(tf, name);
}

template <class T> inline void sc_trace(sc_core::sc_trace_file* tf, const sc_inout_opt<T>& port, const std::string& name) {
    const sc_core::sc_signal_in_if<T>* iface = 0;
    if(sc_core::sc_get_curr_simcontext()->elaboration_done()) {
        iface = dynamic_cast<const sc_core::sc_signal_in_if<T>*>(port.get_interface());
    }

    if(iface)
        sc_trace(tf, iface->read(), name);
    else
        port.add_trace_internal(tf, name);
}

} // namespace scc

#undef SCC_VIRT

#if defined(_MSC_VER) && !defined(SC_WIN_DLL_WARN)
#pragma warning(pop)
#endif
#endif /* SCC_SRC_SYSC_SCC_OPT_SIGNAL_PORTS_H_ */
