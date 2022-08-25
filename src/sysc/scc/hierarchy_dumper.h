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

#ifndef _SYSC_SCC_HIERARCHY_DUMPER_H_
#define _SYSC_SCC_HIERARCHY_DUMPER_H_

#include <systemc>

/** \ingroup scc-sysc
 *  @{
 */
/**@{*/
//! @brief SCC SystemC utilities
namespace scc {

class hierarchy_dumper: public sc_core::sc_module {
public:
    enum file_type { ELKT, JSON, D3JSON, DBGJSON};

    hierarchy_dumper(const std::string& filename, file_type format);

    virtual ~hierarchy_dumper();
private:
    std::string dump_hier_file_name{""};
    void start_of_simulation() override;
    file_type const dump_format;
};
}
/** @} */ // end of scc-sysc
#endif /* _SYSC_SCC_HIERARCHY_DUMPER_H_ */
