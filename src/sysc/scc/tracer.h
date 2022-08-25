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

#ifndef _SCC_TRACER_H_
#define _SCC_TRACER_H_

#include "tracer_base.h"
#include <string>
#include <vector>

#ifdef HAS_SCV
class scv_tr_db;
#else
namespace scv_tr {
class scv_tr_db;
}
#endif

namespace sc_core {
class sc_object;
class sc_trace_file;
} // namespace sc_core

/** \ingroup scc-sysc
 *  @{
 */
/**@{*/
//! @brief SCC SystemC utilities
namespace scc {
/**
 * @class tracer
 * @brief a component traversing the SystemC object hierarchy and tracing the objects
 *
 */
class tracer : public tracer_base {
public:
    /**
     * @enum file_type
     * @brief defines the transaction trace output type
     *
     * CUSTOM means the caller needs to initialize the database driver (scv_tr_text_init() or alike)
     */
    enum file_type { NONE, TEXT, COMPRESSED, SQLITE, CUSTOM };
    /**
     * @fn  tracer(const std::string&&, file_type, bool=true)
     * @brief the constructor
     *
     * @param name base name of the trace file(s)
     * @param type type of trace file for transactions
     * @param enable enable VCD (signal and POD) tracing
     */
    tracer(std::string const&& name, file_type type, bool enable = true, sc_core::sc_object* top = nullptr);
    /**
     * @fn  tracer(const std::string&, file_type, bool=true)
     * @brief the constructor
     *
     * @param name base name of the trace file(s)
     * @param type type of trace file for transactions
     * @param enable enable VCD (signal and POD) tracing
     */
    tracer(std::string const& name, file_type type, bool enable = true, sc_core::sc_object* top = nullptr)
    : tracer(std::string(name), type, enable, top) {}
    /**
     * @fn  tracer(const std::string&&, file_type, bool=true)
     * @brief the constructor
     *
     * @param name base name of the trace file(s)
     * @param type type of trace file for transactions
     * @param enable enable VCD (signal and POD) tracing
     */
    tracer(std::string const&& name, file_type type, sc_core::sc_trace_file* tf, sc_core::sc_object* top = nullptr);
    /**
     * @fn  tracer(const std::string&, file_type, bool=true)
     * @brief the constructor
     *
     * @param name base name of the trace file(s)
     * @param type type of trace file for transactions
     * @param enable enable VCD (signal and POD) tracing
     */
    tracer(std::string const& name, file_type type, sc_core::sc_trace_file* tf, sc_core::sc_object* top = nullptr)
    : tracer(std::string(name), type, tf, top) {}
    /**
     * @fn  ~tracer()
     * @brief the destructor
     */
    virtual ~tracer() override;

protected:
    void end_of_elaboration() override;
#ifdef HAS_SCV
    scv_tr_db* txdb;
#else
    scv_tr::scv_tr_db* txdb;
#endif

private:
    void init_scv_db(file_type type, std::string const&& name);
    bool owned{false};
    sc_core::sc_object* top{nullptr};
};

} /* namespace scc */
/** @} */ // end of scc-sysc
#endif /* _SCC_TRACER_H_ */
