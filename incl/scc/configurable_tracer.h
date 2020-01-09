/*******************************************************************************
 * Copyright 2017, 2018 MINRES Technologies GmbH
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

#ifndef _SCC_CONFIGURABLE_TRACER_H_
#define _SCC_CONFIGURABLE_TRACER_H_

#include "tracer.h"
#include <cci_configuration>

namespace scc {
/**
 *
 */
class configurable_tracer : public scc::tracer {
public:
    /**
     * constructs a tracer object
     *
     * @param name basename of the trace file(s)
     * @param type type of trace file for transactions
     * @param enable enable VCD (signal based) tracing
     * @param default value of attribute enableTracing if not defined by module or CCIs
     */
    configurable_tracer(const std::string&&, file_type, bool = true, bool = false);
    /**
     * constructs a tracer object
     *
     * @param name basename of the trace file(s)
     * @param type type of trace file for transactions
     * @param enable enable VCD (signal based) tracing
     * @param default value of attribute enableTracing if not defined by module or CCIs
     */
    configurable_tracer(const std::string& name, file_type type, bool enable_vcd = true, bool default_enable = false)
    :configurable_tracer(std::string(name), type, enable_vcd, default_enable)
    {}
    /**
     * constructs a tracer object
     *
     * @param name basename of the trace file(s)
     * @param type type of trace file for transactions
     * @param the trace file to use for signal and POD tracing
     * @param default value of attribute enableTracing if not defined by module or CCIs
     */
    configurable_tracer(const std::string&&, file_type, sc_core::sc_trace_file* = nullptr, bool = false);
    /**
     * constructs a tracer object
     *
     * @param name basename of the trace file(s)
     * @param type type of trace file for transactions
     * @param enable enable VCD (signal based) tracing
     * @param default value of attribute enableTracing if not defined by module or CCIs
     */
    configurable_tracer(const std::string& name, file_type type, sc_core::sc_trace_file* tf = nullptr, bool default_enable = false)
    :configurable_tracer(std::string(name), type, tf, default_enable)
    {}
    /**
     * destructor
     */
    ~configurable_tracer();
    /**
     * adds default trace control attribute of name 'enableTracing' to each sc_module in a design hierarchy
     */
    void add_control() {
        for (auto *o : sc_core::sc_get_top_level_objects(sc_core::sc_curr_simcontext)) augment_object_hierarchical(o);
    }

protected:
    //! the default for tracing if no attribute is configured
    const bool default_trace_enable;
    //! depth-first walk thru the design hierarchy and trace signals resp. call trace() function
    void descend(const sc_core::sc_object *, bool trace_all = false) override;
    //! check for existence of 'enableTracing' attribute and return value of default otherwise
    bool get_trace_enabled(const sc_core::sc_object *, bool = false);
    //! add the 'enableTracing' attribute to sc_module
    void augment_object_hierarchical(const sc_core::sc_object *);
    //! the originator of cci values
    cci::cci_originator cci_originator;
    //! the cci broker
    cci::cci_broker_handle cci_broker;
    //! array of created cci parameter
    std::vector<cci::cci_param_untyped *> params;
};

} /* namespace scc */
#endif /* _SCC_CONFIGURABLE_TRACER_H_ */
