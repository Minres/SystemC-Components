/*******************************************************************************
 * Copyright 2019 MINRES Technologies GmbH
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

#ifndef _SCC_SC_VARIABLE_H_
#define _SCC_SC_VARIABLE_H_

#include <array>
#include <functional>
#include "observer.h"
#include "report.h"
#include <sstream>
#include <sysc/kernel/sc_event.h>
#include <sysc/kernel/sc_simcontext.h>
#include <sysc/tracing/sc_trace.h>

#ifndef SC_API
#define SC_API
#endif

/** \ingroup scc-sysc
 *  @{
 */
/**@{*/
//! @brief SCC SystemC utilities
namespace scc {
class trace_handle;
/**
 * @struct sc_variable
 * @brief SystemC variable
 *
 * This class makes plain and composite C++ datatype variables visible to the SystemC kernel by registering
 * them in the SystemC object hierarchy.
 *
 */
struct sc_variable_b : sc_core::sc_object {
    /**
     * @fn  sc_variable(const char*)
     * @brief named contructor
     *
     * @param name the name
     */
    sc_variable_b(const char* name)
    : sc_core::sc_object(name) {}
    /**
     * @fn const char* kind()const
     * @brief get the kind of this sc_object
     *
     * @return the kind string
     */
    const char* kind() const { return "sc_variable"; }
    /**
     * @fn std::string to_string()const
     * @brief retrieve the textual representation of the value
     *
     * @return
     */
    virtual std::string to_string() const { return ""; };

    virtual void trace(observer* obs) const = 0;

};
/**
 * @struct sc_variable
 * @brief the sc_variable for a particular plain data type
 *
 * @tparam T the data type of the wrapped value
 */
template <typename T> struct sc_variable : public sc_variable_b {
    using this_type = sc_variable<T>;
    /**
     * @fn const T& operator *()
     * @brief get a reference to the wrapped value
     *
     * @return
     */
    const T& operator*() { return value; }
    /**
     * @fn  sc_variable(const std::string&, const T&)
     * @brief constructor taking a name and a reference of the variable to be wrapped
     *
     * @param name the name
     * @param value the variable reference to be wrapped
     */
    sc_variable(const std::string& name, const T& value)
    : sc_variable_b(name.c_str())
    , value(value)
    , hndl{} {}
    /**
     * @fn std::string to_string()const
     * @brief create a textual representation of the wrapped value
     *
     * @return the string representing the value
     */
    std::string to_string() const override {
        std::stringstream ss;
        ss << value;
        return ss.str();
    }
    /**
     * @brief value getter
     *
     */
    T get() const { return value; }
    /**
     * @brief bool conversion operator
     *
     */
    operator bool() const { return value; }

    /**
     * @brief cast operator
     *
     */
    operator T() const { return value; }
    // assignment operator overload
    sc_variable& operator=(const T other) {
        value = other;
        for(auto h : hndl)
            h->notify();
        return *this;
    }
    /**
     * equality comparison
     * @param other
     * @return
     */
    bool operator==(T other) const { return value == other; }
    /**
     * inequality comparison
     * @param other
     * @return
     */
    bool operator!=(T other) const { return value != other; }
    /**
     * greater than comparison
     * @param other
     * @return
     */
    bool operator>(T other) const { return value > other; }
    /**
     * less than comparison
     * @param other
     * @return
     */
    bool operator<(T other) const { return value < other; }
    /**
     * greater than or equal comparison
     * @param other
     * @return
     */
    bool operator>=(T other) const { return value >= other; }
    /**
     * less than or equal comparison
     * @param other
     * @return
     */
    bool operator<=(T other) const { return value <= other; }
    //! overloaded prefix ++ operator
    sc_variable& operator++() {
        ++value; // increment this object
        for(auto h : hndl)
            h->notify();
        return *this;
    }

    //! overloaded postfix ++ operator
    T operator++(int) {
        auto orig = value;
        ++value;
        for(auto h : hndl)
            h->notify();
        return orig;
    }
    //! overloaded prefix -- operator
    sc_variable& operator--() {
        --value; // increment this object
        for(auto h : hndl)
            h->notify();
        return *this;
    }

    //! overloaded postfix -- operator
    T operator--(int) {
        auto orig = value;
        --value;
        for(auto h : hndl)
            h->notify();
        return orig;
    }

    //" arithmetic operator overloads
    T operator+=(const T other) {
        value += other;
        for(auto h : hndl)
            h->notify();
        return value;
    }
    T operator-=(const T other) {
        value += other;
        for(auto h : hndl)
            h->notify();
        return value;
    }
    T operator*=(const T other) {
        value *= other;
        for(auto h : hndl)
            h->notify();
        return value;
    }
    T operator/=(const T other) {
        value /= other;
        for(auto h : hndl)
            h->notify();
        return value;
    }
    T operator+(const T other) const { return value - other; }
    T operator-(const T other) const { return value - other; }
    T operator*(const T other) const { return value * other; }
    T operator/(const T other) const { return value / other; }
    T operator+(const this_type& other) const { return value + other.value; }
    T operator-(const this_type& other) const { return value - other.value; }
    T operator*(const this_type& other) const { return value * other.value; }
    T operator/(const this_type& other) const { return value / other.value; }
    /**
     * @fn void trace(sc_core::sc_trace_file*)const
     * @brief register the value with the SystemC trace implementation
     *
     * @param tf
     */
    void trace(sc_core::sc_trace_file* tf) const override {
        if(auto* obs = dynamic_cast<observer*>(tf))
            hndl.push_back(observe(obs, value, name()));
        else
            sc_core::sc_trace(tf, value, name());
    }

    void trace(observer* obs) const override { hndl.push_back(observe(obs, value, name())); }

    static sc_variable<T> create(const char* n, size_t i, T default_val) {
        std::ostringstream os;
        os << n << "[" << i << "];";
        return scc::sc_variable<T>(os.str(), default_val);
    }

    struct creator {
        creator(T const& default_val)
        : default_val{default_val} {}
        sc_variable<T>* operator()(const char* n, size_t i) { return new sc_variable<T>(n, default_val); }

    private:
        T default_val{};
    };

private:
    //! the wrapped value
    T value;
    //! the observer handle
    mutable std::vector<observer::notification_handle*> hndl;
};

template <typename T> T operator+(sc_variable<T> const& a, sc_variable<T> const& b) { return a.get() + b.get(); }
template <typename T> T operator-(sc_variable<T> const& a, sc_variable<T> const& b) { return a.get() - b.get(); }
template <typename T> T operator*(sc_variable<T> const& a, sc_variable<T> const& b) { return a.get() * b.get(); }
template <typename T> T operator/(sc_variable<T> const& a, sc_variable<T> const& b) { return a.get() / b.get(); }
template <typename T> T operator+(T const& a, sc_variable<T> const& b) { return a + b.get(); }
template <typename T> T operator-(T const& a, sc_variable<T> const& b) { return a - b.get(); }
template <typename T> T operator*(T const& a, sc_variable<T> const& b) { return a * b.get(); }
template <typename T> T operator/(T const& a, sc_variable<T> const& b) { return a / b.get(); }

/**
 * specialization for bool as the cast operator for bool would be ambigous
 */
template <> struct sc_variable<bool> : public sc_variable_b {
    const bool& operator*() { return value; }
    sc_variable(const std::string& name, const bool& value)
    : sc_variable_b(name.c_str())
    , value(value)
    , hndl{} {}
    std::string to_string() const override {
        std::stringstream ss;
        ss << value;
        return ss.str();
    }
    bool get() const { return value; }
    operator bool() const { return value; }
    sc_variable& operator=(const bool other) {
        value = other;
        for(auto h : hndl)
            h->notify();
        return *this;
    }
    bool operator==(bool other) const { return value == other; }
    bool operator!=(bool other) const { return value != other; }
    void trace(sc_core::sc_trace_file* tf) const override {
        if(auto* obs = dynamic_cast<observer*>(tf))
            hndl.push_back(observe(obs, value, name()));
        else
            sc_core::sc_trace(tf, value, name());
    }
    void trace(observer* obs) const override { hndl.push_back(observe(obs, value, name())); }
    static scc::sc_variable<bool> create(const char* n, size_t i, bool default_val) {
        std::ostringstream os;
        os << n << "[" << i << "];";
        return sc_variable<bool>(os.str(), default_val);
    }

    struct creator {
        creator(bool const& default_val)
        : default_val{default_val} {}
        sc_variable<bool>* operator()(const char* n, size_t i) {
            return new sc_variable<bool>(n, default_val);
        }

    private:
        bool default_val{};
    };

private:
    bool value;
    mutable std::vector<observer::notification_handle*> hndl;
};
/**
 * a vector holding sc_variable. It can be used as a sparse array by providing a creator function or
 * as a normal vector when providing a default value upon creating or resizing
 *
 * @note after end of elaboration the size of the vector cannot be change. It is also not possible to
 * add elements e.g. when using as a sparse array.
 *
 * @tparam T
 */
template <typename T> struct sc_variable_vector {

    sc_variable_vector(std::string const& name, size_t size)
    : name(name)
    , values(size, nullptr)
    , creator{} {}

    sc_variable_vector(std::string const& name, size_t size, T const& def_val)
    : name(name)
    , values(size, nullptr) {
        resize(size, def_val);
    }

    sc_variable_vector(std::string const& name, size_t size,
                       std::function<sc_variable<T>*(char const*, size_t)> creator)
    : name(name)
    , values(size, nullptr)
    , creator(creator) {}

    size_t size() { return values.size(); }

    void resize(size_t sz) {
        assert(!sc_core::sc_get_curr_simcontext()->elaboration_done());
        values.resize(sz);
    }

    void resize(size_t sz, T def_val) {
        assert(!sc_core::sc_get_curr_simcontext()->elaboration_done());
        values.resize(sz);
        auto idx = 0U;
        for(auto& e : values) {
            std::stringstream ss;
            ss << name << "(" << idx++ << ")";
            e = new sc_variable<T>(ss.str().c_str(), def_val);
        }
    }

    bool is_valid(size_t idx) const { return values.size()> idx && values.at(idx) != nullptr; }

    sc_variable<T>& operator[](size_t idx) {
        auto ret = values.at(idx);
        if(!ret) {
            if(sc_core::sc_get_curr_simcontext()->elaboration_done())
                SCCFATAL(sc_core::sc_get_current_object()->name())<<"Trying to create a sc_variable vector entry in "<<name <<
                " with index "<<idx<<" while elaboration finished is not allowed";
            assert(!sc_core::sc_get_curr_simcontext()->elaboration_done());
            assert(creator);
            std::stringstream ss;
            ss << name << "(" << idx << ")";
            ret = values.at(idx) = creator(ss.str().c_str(), idx);
        }
        return *ret;
    }

    sc_variable<T> const& operator[](size_t idx) const {
        assert(values.at(idx) && "No initialized value in sc_variable_vector position");
        return *values.at(idx);
    }
    ~sc_variable_vector() {
        for(auto p : values)
            delete p;
    }

private:
    std::string name{};
    std::vector<sc_variable<T>*> values;
    std::function<sc_variable<T>*(char const*, size_t)> creator;
};
/**
 * @struct sc_ref_variable
 * @brief the sc_ref_variable for a particular plain data type. This marks an existing C++
 * variable as discoverable via the sc_object tree. Whenever possible sc_variable should be used
 * as this does not support value change callback.
 *
 * @tparam T the data type of the wrapped value
 */
template <typename T> struct sc_ref_variable : public sc_variable_b {
    //! the wrapped value
    const T& value;
    /**
     * @fn const T& operator *()
     * @brief get a reference to the wrapped value
     *
     * @return
     */
    const T& operator*() { return value; }
    /**
     * @fn  sc_variable(const std::string&, const T&)
     * @brief constructor taking a name and a reference of the variable to be wrapped
     *
     * @param name the name
     * @param value the variable reference to be wrapped
     */
    sc_ref_variable(const std::string& name, const T& value)
    : sc_variable_b(name.c_str())
    , value(value) {}
    /**
     * @fn std::string to_string()const
     * @brief create a textual representation of the wrapped value
     *
     * @return the string representing the value
     */
    std::string to_string() const override {
        std::stringstream ss;
        ss << value;
        return ss.str();
    }
    /**
     * @fn void trace(sc_core::sc_trace_file*)const
     * @brief register the value with the SystemC trace implementation
     *
     * @param tf
     */
    void trace(sc_core::sc_trace_file* tf) const override { sc_core::sc_trace(tf, value, name()); }

    void trace(observer* obs) const override {}
};
template <> struct sc_ref_variable<sc_core::sc_event> : public sc_variable_b {
    const sc_core::sc_event& value;
    const sc_core::sc_event& operator*() { return value; }
    sc_ref_variable(const std::string& name, const sc_core::sc_event& value)
    : sc_variable_b(name.c_str())
    , value(value) {}
    std::string to_string() const override {
        std::stringstream ss;
        ss << value.name();
        return ss.str();
    }

    void trace(observer* obs) const override { }

    void trace(sc_core::sc_trace_file* tf) const override { sc_core::sc_trace(tf, value, name()); }
};
/**
 * @struct sc_ref_variable_masked
 * @brief the sc_variable for a particular plain data type with limited bit width
 *
 * @tparam T the data type of the wrapped value
 */
template <typename T> struct sc_ref_variable_masked : public sc_variable_b {
    const T& value;

    const T mask;

    sc_ref_variable_masked(const std::string& name, const T& value, int width)
    : sc_variable_b(name.c_str())
    , value(value)
    , mask((1 << width) - 1) {}

    std::string to_string() const override {
        std::stringstream ss;
        ss << (value & mask);
        return ss.str();
    }

    void trace(observer* obs) const override { }

    void trace(sc_core::sc_trace_file* tf) const override { sc_core::sc_trace(tf, value, name()); }
};

} // namespace scc

namespace sc_core {
template <class T> inline void sc_trace(sc_trace_file* tf, const ::scc::sc_variable<T>& object, const char* name) {
    object.trace(tf);
}
template <class T> inline void sc_trace(sc_trace_file* tf, const ::scc::sc_variable<T>* object, const char* name) {
    object->trace(tf);
}

template <class T> inline void sc_trace(sc_trace_file* tf, const ::scc::sc_ref_variable<T>& object, const char* name) {
    object.trace(tf);
}
template <class T> inline void sc_trace(sc_trace_file* tf, const ::scc::sc_ref_variable<T>* object, const char* name) {
    object->trace(tf);
}

} // namespace sc_core
/** @} */ // end of scc-sysc
#endif /* _SCC_SC_VARIABLE_H_ */
