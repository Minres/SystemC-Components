/*****************************************************************************
  Licensed to Accellera Systems Initiative Inc. (Accellera) under one or
  more contributor license agreements.  See the NOTICE file distributed
  with this work for additional information regarding copyright ownership.
  Accellera licenses this file to you under the Apache License, Version 2.0
  (the "License"); you may not use this file except in compliance with the
  License.  You may obtain a copy of the License at
    http://www.apache.org/licenses/LICENSE-2.0
  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
  implied.  See the License for the specific language governing
  permissions and limitations under the License.
 ****************************************************************************/

#ifndef _SCP_PATHTRACE_EXTENSION_H
#define _SCP_PATHTRACE_EXTENSION_H

#include <sstream>
#include <systemc>
#include <tlm>

namespace scp {
namespace tlm_extensions {

/**
 * @class Path recording TLM extension
 *
 * @brief Path recording TLM extension
 *
 * @details Ignorable Extension type that can be used to trace
 *          transactions as they pass through a network.
 */

class path_trace : public tlm::tlm_extension<path_trace> {
    std::vector<sc_core::sc_object*> m_path;

public:
    path_trace() = default;
    path_trace(const path_trace&) = default;

    virtual tlm_extension_base* clone() const override { return new path_trace(*this); }

    virtual void copy_from(const tlm_extension_base& ext) override {
        const path_trace& other = static_cast<const path_trace&>(ext);
        *this = other;
    }

    /**
     * @brief Stamp object into the PathTrace
     * @param obj  Object to add to the PathTrace
     */
    void stamp(sc_core::sc_object* obj) { m_path.push_back(obj); }

    /**
     * @brief Convenience function to clear vector (eg. before returning to a
     * pool)
     */
    void reset() { m_path.clear(); }
    /**
     * @brief convert extension to a string
     * @param separator (default "->")
     * @return a string consisting of the names of each object stamped into the
     * path separated with the separator provided.
     */
    std::string to_string(std::string separator = "->") {
        std::stringstream info;
        std::string s;
        for(auto o : m_path) {
            info << s << o->name();
            s = separator;
        }
        return info.str();
    }
};
} // namespace tlm_extensions
} // namespace scp
#endif
