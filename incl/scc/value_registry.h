/*
 * value_registry.h
 *
 *  Created on: 30.12.2018
 *      Author: eyck
 */

#ifndef _SCC_VALUE_REGISTRY_H_
#define _SCC_VALUE_REGISTRY_H_

#include "tracer_base.h"
#include <sysc/tracing/sc_trace.h>
#include <sysc/kernel/sc_simcontext.h>
#include <sstream>
#ifndef SC_API
#define SC_API
#endif

namespace scc {
struct value_registry_if {
    struct value_holder {
        const std::string name;
        virtual std::string to_string() = 0;
        value_holder(const std::string& name):name(name){}
    };

    virtual std::vector<std::string> get_names() const = 0;

    virtual value_holder& get_value(std::string& name) const = 0;

    virtual ~value_registry_if(){}
};


struct sc_variable: sc_core::sc_object {

    sc_variable(const char* name):sc_core::sc_object(name) { }

    const char* kind() const { return "sc_variable"; }

    virtual std::string to_string() const = 0;
};

template<typename T>
struct sc_variable_t : public sc_variable {
    const T& value;

    sc_variable_t(const std::string& name, const T& value):sc_variable(name.c_str()), value(value){}

    std::string to_string() const {std::stringstream ss;ss<<value;return ss.str();}
};

template<typename T>
struct sc_variable_masked_t : public sc_variable {
    const T& value;

    const T& mask;

    sc_variable_masked_t(const std::string& name, const T& value, int width)
    : sc_variable(name.c_str()), value(value), mask((1<<width)-1){}

    std::string to_string() const {std::stringstream ss;ss<<(value&mask); return ss.str();}
};

class SC_API value_registry : protected tracer_base {
public:
    value_registry();

    ~value_registry();

    std::vector<std::string> get_names() const;

    const sc_variable* get_value(std::string name) const;

protected:
    void end_of_elaboration() override;
};

}

#endif /* _SCC_VALUE_REGISTRY_H_ */
