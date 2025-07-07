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
/**
 * @brief A SystemC module for dumping the hierarchy of objects in a specified format.
 *
 * The hierarchy_dumper class provides a method for dumping the hierarchy of SystemC objects to a file in various formats.
 * It can be used to visualize the structure of a SystemC design or to analyze the interactions between different components.
 *
 * @author Your Name
 * @date YYYY-MM-DD
 */
class hierarchy_dumper : public sc_core::sc_module {
public:
    /**
     * @brief The supported file formats for dumping the hierarchy.
     */
    enum file_type { ELKT, JSON, D3JSON, DBGJSON };
    /**
     * @brief Constructs a hierarchy_dumper object with the specified file name and format.
     *
     * @param filename The name of the file where the hierarchy will be dumped.
     * @param format The format in which the hierarchy will be dumped.
     */
    hierarchy_dumper(const std::string& filename, file_type format);
    /**
     * @brief Destroys the hierarchy_dumper object.
     */
    virtual ~hierarchy_dumper();

private:
    std::string dump_hier_file_name{""};
    /**
     * @brief Callback function that is invoked at the start of the simulation.
     *
     * This function is responsible for dumping the hierarchy of SystemC objects to the specified file.
     */
    void start_of_simulation() override;
    file_type const dump_format;
};
} // namespace scc
/** @} */ // end of scc-sysc
#endif    /* _SYSC_SCC_HIERARCHY_DUMPER_H_ */
