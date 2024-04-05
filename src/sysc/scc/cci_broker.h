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

#include <cci_utils/broker.h>
#include <regex>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace scc {
class cci_broker : public cci_utils::consuming_broker {
    std::unordered_set<std::string> expose;

    bool has_parent;
    cci::cci_broker_if& m_parent;

    cci::cci_broker_if& get_parent_broker() {
        if(sc_core::sc_get_current_object()) {
            has_parent = true;
            return unwrap_broker(cci::cci_get_broker());
        } else {
            // We ARE the global broker
            has_parent = false;
            return *this;
        }
    }

    bool sendToParent(const std::string& parname) const;

    void insert_matching_preset_value(const std::string& parname);

    struct wildcard_entry {
        std::regex rr;
        cci::cci_value value;
        cci::cci_originator originator;
    };
    std::unordered_map<std::string, wildcard_entry> wildcard_presets;
    std::unordered_map<std::string, std::regex> wildcard_locks;

public:
    cci::cci_originator get_value_origin(const std::string& parname) const override;

    cci_broker(const std::string& name);

    ~cci_broker();

    bool has_preset_value(const std::string& parname) const override;

    cci::cci_originator get_preset_value_origin(const std::string& parname) const override;

    cci::cci_value get_preset_cci_value(const std::string& parname) const override;
    /**
     * @brief Set a parameter's preset value.
     *
     * Set a parameter's preset value using globbing expression or regular expression.
     *
     * The globbing supports ?,*,**, and character classes ([a-z] as well as [!a-z]). '.' acts as
     * hierarchy delimiter and is only matched with **
     * Regular expression must start with a carret ('^') so that it can be identified as regex.
     *
     * The preset value has priority to the default value being set by the owner!
     *
     * @exception        cci::cci_report::set_param_failed Setting parameter
     *                   object failed
     * @param parname    Full hierarchical parameter name.
     * @param cci_value  cci::cci_value representation of the preset value the
     *                   parameter has to be set to.
     * @param originator originator reference to the originator
     *                   (not applicable in case of broker handle)
     */
    void set_preset_cci_value(const std::string& parname, const cci::cci_value& cci_value, const cci::cci_originator& originator) override;

    /**
     * @brief Lock a parameter's preset value.
     *
     * Lock so that this parameter's preset value cannot be overwritten by
     * any subsequent set_preset_value call. This allows to emulate a hierarchical
     * precendence since a top-level module can prevent the childs from setting
     * preset values by locking the preset value before creating the subsystem.
     *
     * The globbing supports ?,*,**, and character classes ([a-z] as well as [!a-z]). '.' acts as
     * hierarchy delimiter and is only matched with **
     * Regular expression must start with a carret ('^') so that it can be identified as regex.
     *
     *
     * Throws (and does not lock) if no preset value exists
     * that can be locked or if a preset value is already locked or if the
     * parameter is already existing as object.
     *
     * @exception     cci::cci_report::set_param_failed Locking parameter object failed
     * @param parname Hierarchical parameter name.
     */
    void lock_preset_value(const std::string& parname) override;

    cci::cci_value get_cci_value(const std::string& parname, const cci::cci_originator& originator = cci::cci_originator()) const override;

    cci::cci_param_untyped_handle get_param_handle(const std::string& parname, const cci::cci_originator& originator) const override;

    std::vector<cci::cci_param_untyped_handle> get_param_handles(const cci::cci_originator& originator) const override;

    void add_param(cci::cci_param_if* par) override;

    void remove_param(cci::cci_param_if* par) override;

    bool is_global_broker() const override;
};

} // namespace scc
#endif // _SYSC_SCC_CCI_BROKER_H_
