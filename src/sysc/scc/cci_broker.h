/*******************************************************************************
 * Copyright 2022 MINRES Technologies GmbH
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

#ifndef _SYSC_SCC_CCI_BROKER_H_
#define _SYSC_SCC_CCI_BROKER_H_

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <regex>
#include <vector>
#include <cci_utils/broker.h>

namespace scc {
class cci_broker: public cci_utils::broker {
    using super = cci_utils::broker;
public:
    explicit cci_broker(const std::string& name): cci_utils::broker(name) { }

    ~cci_broker() = default;

    void set_preset_cci_value(
            const std::string &parname,
            const cci::cci_value & value,
            const cci::cci_originator& originator) override;

    cci::cci_originator get_preset_value_origin(const std::string &parname) const override;

    cci::cci_value get_preset_cci_value(const std::string &parname) const override ;

    void lock_preset_value(const std::string &parname) override;

    cci::cci_value get_cci_value(const std::string &parname, const cci::cci_originator &originator = cci::cci_originator()) const override;

    bool has_preset_value(const std::string &parname) const override;
protected:
    std::unordered_map<std::string, std::regex> wildcard_presets;
};
}
#endif // _SYSC_SCC_CCI_BROKER_H_
