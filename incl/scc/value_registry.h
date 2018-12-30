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

namespace scc {
struct value_registry_if {
    struct value_holder {
        const std::string name;
        virtual std::string to_string() = 0;
        value_holder(const std::string& name):name(name){}
    };

    virtual std::vector<std::string> get_names() =0;

    virtual value_holder& get_value(std::string& name) = 0;

    virtual ~value_registry_if(){}
};

class value_registry : protected tracer_base, public value_registry_if {
public:
    value_registry();

    ~value_registry();

    std::vector<std::string> get_names() override;

    value_registry_if::value_holder& get_value(std::string& name) override;

protected:
    void end_of_elaboration() override;
};

}

#endif /* _SCC_VALUE_REGISTRY_H_ */
