/*******************************************************************************
 * Copyright 2016, 2018 MINRES Technologies GmbH
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

#ifndef _SCC_TRACABLE_H_
#define _SCC_TRACABLE_H_

/** \ingroup scc-sysc
 *  @{
 */
/**@{*/
//! @brief SCC SystemC utilities
namespace scc {
/**
 * @class traceable
 * @brief interface defining a traceable component
 *
 * This overlaps with the trace function of sc_core::sc_object. In fact it serves as a signaling interface
 */
class traceable {
public:
    traceable() = default;

    virtual ~traceable() = default;
    /**
     * @fn bool is_trace_enabled()const
     * @brief returns of this component shall be traced
     *
     * @return true if this component shall be traced
     */
    virtual bool is_trace_enabled() const { return true; }
};

} /* namespace scc */
/** @} */ // end of scc-sysc
#endif /* _SCC_TRACABLE_H_ */
