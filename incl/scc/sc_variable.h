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
#include <sstream>
#include <sysc/kernel/sc_simcontext.h>
#include <sysc/tracing/sc_trace.h>

#ifndef SC_API
#define SC_API
#endif

namespace scc {
/**
 * @struct sc_variable
 * @brief SystemC variable
 *
 * This class makes plain and composite C++ datatype variables visible to the SystemC kernel by registering
 * them in the SystemC object hierarchy.
 *
 */
struct sc_variable : sc_core::sc_object {
    /**
         * @fn  sc_variable(const char*)
     * @brief named contructor
     *
     * @param name the name
     */
    sc_variable(const char* name)
    : sc_core::sc_object(name) {}
/**
 * @fn const char kind*()const
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
};
/**
 * @struct sc_variable_t
 * @brief the sc_variable for a particular plain data type
 *
 * @tparam T the data type of the wrapped value
 */
template <typename T> struct sc_variable_t : public sc_variable {
    //! the wrapped value
    const T& value;
    /**
         * @fn const T operator *&()
     * @brief get a reference to the wrapped value
     *
     * @return
     */
    const T& operator*() { return value; }
/**
 * @fn  sc_variable_t(const std::string&, const T&)
 * @brief constructor taking a name and a reference of the variable to be wrapped
 *
 * @param name the name
 * @param value the variable reference to be wrapped
 */
    sc_variable_t(const std::string& name, const T& value)
    : sc_variable(name.c_str())
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
    void trace(sc_core::sc_trace_file* tf) const override { sc_trace(tf, value, name()); }
};
/**
 * @struct sc_variable_t
 * @brief specialization of  \ref template <typename T> struct sc_variable_t for \ref sc_core::sc_event
 *
 */
template <> struct sc_variable_t<sc_core::sc_event> : public sc_variable {
    const sc_core::sc_event& value;

    const sc_core::sc_event& operator*() { return value; }

    sc_variable_t(const std::string& name, const sc_core::sc_event& value)
    : sc_variable(name.c_str())
    , value(value) {}

    std::string to_string() const override { return ""; }
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
    void trace(sc_core::sc_trace_file* tf) const override {
#pragma GCC diagnostic pop
    }
};
/**
 * @struct sc_variable_t
 * @brief specialization of  \ref template <typename T> struct sc_variable_t for \ref std::vector<T>
 *
 */
template <typename T> struct sc_variable_t<std::vector<T>> : public sc_variable {
    const std::vector<T>& value;

    const std::vector<T>& operator*() { return value; }

    sc_variable_t(std::string const& name, std::vector<T> const& value)
    : sc_variable(name.c_str())
    , value(value) {}

    std::string to_string() const override {
        std::stringstream ss;
        for(const T& e : value)
            ss << e << ",";
        return ss.str();
    }

    void trace(sc_core::sc_trace_file* tf) const override {
        auto i = 0U;
        for(T const& e : value) {
            std::stringstream ss;
            ss << name() << "(" << i++ << ")";
            sc_trace(tf, e, ss.str());
        }
    }
};
/**
 * @struct sc_variable_t
 * @brief specialization of  \ref template <typename T> struct sc_variable_t for \ref std::array<T, S>
 *
 */
template <typename T, size_t S> struct sc_variable_t<std::array<T, S>> : public sc_variable {
    const std::array<T, S>& value;

    const std::array<T, S>& operator*() { return value; }

    sc_variable_t(std::string const& name, std::array<T, S> const& value)
    : sc_variable(name.c_str())
    , value(value) {}

    std::string to_string() const override {
        std::stringstream ss;
        for(const T& e : value)
            ss << e << ",";
        return ss.str();
    }

    void trace(sc_core::sc_trace_file* tf) const override {
        auto i = 0U;
        for(T const& e : value) {
            std::stringstream ss;
            ss << name() << "(" << i++ << ")";
            sc_trace(tf, e, ss.str());
        }
    }
};
/**
 * @struct sc_variable_t
 * @brief the sc_variable for a particular plain data type with limited bit width
 *
 * @tparam T the data type of the wrapped value
 */
template <typename T> struct sc_variable_masked_t : public sc_variable {
    const T& value;

    const T mask;

    sc_variable_masked_t(const std::string& name, const T& value, int width)
    : sc_variable(name.c_str())
    , value(value)
    , mask((1 << width) - 1) {}

    std::string to_string() const override{
        std::stringstream ss;
        ss << (value & mask);
        return ss.str();
    }

    void trace(sc_core::sc_trace_file* tf) const override { sc_trace(tf, value, name()); }
};

} // namespace scc

#endif /* _SCC_SC_VARIABLE_H_ */
