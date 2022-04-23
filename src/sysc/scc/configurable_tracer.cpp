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

#include "configurable_tracer.h"
#include "traceable.h"

using namespace sc_core;
using namespace scc;

#define EN_TRACING_STR "enableTracing"

configurable_tracer::configurable_tracer(std::string const&& name, file_type type, bool enable_vcd, bool default_enable, sc_core::sc_object* top)
: tracer(std::move(name), type, enable_vcd, top)
, cci_originator(this->name())
, cci_broker(cci::cci_get_global_broker(cci_originator))
{
    default_trace_enable=default_enable;
}

configurable_tracer::configurable_tracer(std::string const&& name, file_type type, sc_core::sc_trace_file* tf,
                                         bool default_enable, sc_core::sc_object* top)
: tracer(std::move(name), type, tf, top)
, cci_originator(this->name())
, cci_broker(cci::cci_get_global_broker(cci_originator))
{
    default_trace_enable=default_enable;
}

scc::configurable_tracer::~configurable_tracer() {
    for(auto ptr : params)
        delete ptr;
}

void configurable_tracer::descend(const sc_core::sc_object* obj, bool trace) {
    if(obj == this)
        return;
    const char* kind = obj->kind();
    if((types_to_trace & trace_types::SIGNALS) == trace_types::SIGNALS && strcmp(kind, "tlm_signal") == 0){
        if(trace)
            obj->trace(trf);
        return;
    } else if(strcmp(kind, "sc_vector") == 0) {
        if(trace)
            for(auto o : obj->get_child_objects())
                descend(o, trace);
        return;
    } else if(strcmp(kind, "sc_module") == 0) {
        auto trace_enable = get_trace_enabled(obj, default_trace_enable);
        if(trace_enable)
            obj->trace(trf);
        for(auto o : obj->get_child_objects())
            descend(o, trace_enable);
    } else if(strcmp(kind, "sc_variable") == 0) {
        if(trace && (types_to_trace & trace_types::VARIABLES) == trace_types::VARIABLES)
            obj->trace(trf);
    } else if(const auto* tr = dynamic_cast<const scc::traceable*>(obj)) {
        if(tr->is_trace_enabled())
            obj->trace(trf);
        for(auto o : obj->get_child_objects())
            descend(o, tr->is_trace_enabled());
    } else if((types_to_trace & trace_types::PORTS) == trace_types::PORTS && strcmp(kind, "sc_port") == 0) {
        if(trace) try_trace(trf, obj, types_to_trace);
    } else if((types_to_trace & trace_types::SIGNALS) == trace_types::SIGNALS && strcmp(kind, "sc_signal") == 0) {
        if(trace) try_trace(trf, obj, types_to_trace);
    } else if(trace)
        try_trace(trf, obj, types_to_trace);
}

auto scc::configurable_tracer::get_trace_enabled(const sc_core::sc_object* obj, bool fall_back) -> bool {
    auto* attr = obj->get_attribute(EN_TRACING_STR);
    if(attr != nullptr && dynamic_cast<const sc_core::sc_attribute<bool>*>(attr) != nullptr) {
        const auto* a = dynamic_cast<const sc_core::sc_attribute<bool>*>(attr);
        return a->value;
    } else {
        std::string hier_name{obj->name()};
        auto h = cci_broker.get_param_handle(hier_name.append("." EN_TRACING_STR));
        if(h.is_valid())
            return h.get_cci_value().get_bool();
    }
    return fall_back;
}

void configurable_tracer::augment_object_hierarchical(const sc_core::sc_object* obj) {
    if(dynamic_cast<const sc_core::sc_module*>(obj) != nullptr || dynamic_cast<const scc::traceable*>(obj) != nullptr) {
        auto* attr = obj->get_attribute(EN_TRACING_STR);
        if(attr == nullptr ||
           dynamic_cast<const sc_core::sc_attribute<bool>*>(attr) == nullptr) { // check if we have no sc_attribute
            std::string hier_name{obj->name()};
            hier_name += "." EN_TRACING_STR;
            auto h = cci_broker.get_param_handle(hier_name);
            if(!h.is_valid()) // we have no cci_param so create one
                params.push_back(new cci::cci_param<bool>(hier_name, default_trace_enable, "", cci::CCI_ABSOLUTE_NAME,
                                                          cci_originator));
        }
        for(auto* o : obj->get_child_objects())
            augment_object_hierarchical(o);
    }
}

void configurable_tracer::end_of_elaboration() {
    add_control();
    tracer::end_of_elaboration();
}
