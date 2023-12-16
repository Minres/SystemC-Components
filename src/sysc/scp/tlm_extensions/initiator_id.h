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

#ifndef _SCP_INITIATOR_ID_EXTENSION_H
#define _SCP_INITIATOR_ID_EXTENSION_H

#include <systemc>
#include <tlm>

namespace scp {
namespace tlm_extensions {
/**
 * @class initiator ID recording TLM extension
 *
 * @brief initiator ID recording TLM extension
 *
 * @details Ignorable Extension type that can be used to add a initiator ID
 * (uint64_t). This is typically used for e.g. evaluating exclusive accesses.
 */

class initiator_id : public tlm::tlm_extension<initiator_id> {
    uint64_t m_id;

public:
    initiator_id(uint64_t id) { m_id = id; }
    initiator_id(const initiator_id&) = default;

    virtual tlm_extension_base* clone() const override { return new initiator_id(*this); }

    virtual void copy_from(const tlm_extension_base& ext) override {
        const initiator_id& other = static_cast<const initiator_id&>(ext);
        *this = other;
    }

    operator uint64_t() { return m_id; };

#define overload(_OP)                                                                                                                      \
    initiator_id& operator _OP(const uint64_t id) {                                                                                        \
        this->m_id _OP id;                                                                                                                 \
        return *this;                                                                                                                      \
    }
    overload(+=);
    overload(-=);
    overload(*=);
    overload(/=);
    overload(%=);
    overload(&=);
    overload(|=);
    overload(^=);
    overload(<<=);
    overload(>>=);

    initiator_id& operator=(const uint64_t id) {
        m_id = id;
        return *this;
    }
};
} // namespace tlm_extensions
} // namespace scp
#endif
