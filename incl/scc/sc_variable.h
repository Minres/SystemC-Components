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

#include <sysc/kernel/sc_simcontext.h>
#include <sysc/tracing/sc_trace.h>
#include <sstream>

#ifndef SC_API
#define SC_API
#endif

namespace scc {

struct sc_variable : sc_core::sc_object {

    sc_variable(const char* name)
    : sc_core::sc_object(name) {}

    const char* kind() const { return "sc_variable"; }

    virtual std::string to_string() const {return "";};
};


template <typename T>
struct sc_variable_t : public sc_variable {
    const T& value;

    const T& operator*(){return value;}

    sc_variable_t(const std::string& name, const T& value)
    : sc_variable(name.c_str())
    , value(value) {}

    std::string to_string() const override {
        std::stringstream ss;
        ss << value;
        return ss.str();
    }

    void trace( sc_core::sc_trace_file* tf ) const override {
        sc_trace(tf, value, name());
    }
};

template <typename T>
struct sc_variable_t<std::vector<T>> : public sc_variable {
    const std::vector<T>& value;

    const std::vector<T>& operator*(){return value;}

    sc_variable_t(std::string const& name, std::vector<T> const& value)
    : sc_variable(name.c_str())
    , value(value) {}

    std::string to_string() const override {
        std::stringstream ss;
        for(const T& e: value)
            ss<<e<<",";
        return ss.str();
    }

    void trace( sc_core::sc_trace_file* tf ) const override {
        auto i = 0U;
        for(T const& e :value){
            std::stringstream ss; ss<<name()<<"("<<i++<<")";
            sc_trace(tf, e, ss.str());
        }
    }
};

template <typename T, size_t S>
struct sc_variable_t<std::array<T, S>> : public sc_variable {
    const std::array<T, S>& value;

    const std::array<T, S>& operator*(){return value;}

    sc_variable_t(std::string const& name, std::array<T, S> const& value)
    : sc_variable(name.c_str())
    , value(value) {}

    std::string to_string() const override {
        std::stringstream ss;
        for(const T& e: value)
            ss<<e<<",";
        return ss.str();
    }

    void trace( sc_core::sc_trace_file* tf ) const override {
        auto i = 0U;
        for(T const& e :value){
            std::stringstream ss; ss<<name()<<"("<<i++<<")";
            sc_trace(tf, e, ss.str());
        }
    }
};

template <typename T> struct sc_variable_masked_t : public sc_variable {
    const T& value;

    const T mask;

    sc_variable_masked_t(const std::string& name, const T& value, int width)
    : sc_variable(name.c_str())
    , value(value)
    , mask((1 << width) - 1) {}

    std::string to_string() const {
        std::stringstream ss;
        ss << (value & mask);
        return ss.str();
    }

    void trace( sc_core::sc_trace_file* tf ) const override {
        sc_trace(tf, value, name());
    }
};

} // namespace scc

#endif /* _SCC_SC_VARIABLE_H_ */
