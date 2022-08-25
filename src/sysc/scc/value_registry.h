/*******************************************************************************
 * Copyright 2020-2022 MINRES Technologies GmbH
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

#ifndef _SCC_VALUE_REGISTRY_H_
#define _SCC_VALUE_REGISTRY_H_

#include "sc_variable.h"
#include "tracer_base.h"
#include <sstream>
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
struct value_registry_if {
    struct value_holder { // @suppress("Class has a virtual method and non-virtual destructor")
        const std::string name;
        virtual std::string to_string() = 0;
        value_holder(const std::string& name)
        : name(name) {}
    };

    virtual std::vector<std::string> get_names() const = 0;

    virtual value_holder& get_value(std::string& name) const = 0;

    virtual ~value_registry_if() {}
};

class SC_API value_registry : protected tracer_base {
public:
    value_registry();

    ~value_registry();

    std::vector<std::string> get_names() const;

    const sc_variable_b* get_value(std::string name) const;

protected:
    void end_of_elaboration() override;
};

} // namespace scc
/** @} */ // end of scc-sysc
#endif /* _SCC_VALUE_REGISTRY_H_ */
