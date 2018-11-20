/*******************************************************************************
 * Copyright (C) 2018, MINRES Technologies GmbH
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 * Contributors:
 *       eyck@minres.com - initial API and implementation
 ******************************************************************************/

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
    void descend(const sc_core::sc_object *) override;
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
