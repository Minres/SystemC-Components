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
/*
 * tracer.cpp
 *
 *  Created on: Nov 9, 2016
 *      Author: developer
 */

#include "scc/tracer_base.h"
#include "scc/traceable.h"
#include <string.h>


using namespace sc_core;
using namespace scc;

void tracer_base::descend(const sc_object *obj, bool trace_all) {
    if(obj==(sc_core::sc_object*)this) return;
    const char *name = obj->name();
    const char *kind = obj->kind();
    if (strcmp(kind, "sc_signal") == 0) {
        try_trace_signal(obj);
        return;
    } else if (strcmp(kind, "sc_out") == 0 || strcmp(kind, "sc_inout") == 0 || strcmp(kind, "sc_in") == 0 || strcmp(kind, "sc_port") == 0) {
        try_trace_port(obj);
        return;
    } else if (strcmp(kind, "tlm_signal") == 0) {
        obj->trace(trf);
        return;
    } else  if ((strcmp(kind, "sc_module") == 0 && trace_all) || dynamic_cast<const traceable *>(obj))
        obj->trace(trf);
    for (auto o : obj->get_child_objects())
        descend(o, trace_all);
}

#ifndef GEN_TRACE
#define GEN_TRACE
#endif

#define GEN_TRACE_STD               \
    GEN_TRACE(bool);                \
    GEN_TRACE(char);                \
    GEN_TRACE(unsigned char);       \
    GEN_TRACE(short);               \
    GEN_TRACE(unsigned short);      \
    GEN_TRACE(int);                 \
    GEN_TRACE(unsigned int);        \
    GEN_TRACE(long);                \
    GEN_TRACE(unsigned long);       \
    GEN_TRACE(long long);           \
    GEN_TRACE(unsigned long long);  \
    GEN_TRACE(float);               \
    GEN_TRACE(double);              \
    GEN_TRACE(sc_dt::int64);        \
    GEN_TRACE(sc_dt::uint64);       \
    GEN_TRACE(sc_core::sc_time);    \
    GEN_TRACE(sc_dt::sc_bit);       \
    GEN_TRACE(sc_dt::sc_logic);     \
    GEN_TRACE(sc_dt::sc_int_base);  \
    GEN_TRACE(sc_dt::sc_uint_base); \
    GEN_TRACE(sc_dt::sc_signed);    \
    GEN_TRACE(sc_dt::sc_unsigned);  \
    GEN_TRACE(sc_dt::sc_bv_base);   \
    GEN_TRACE(sc_dt::sc_lv_base)

#ifdef SC_INCLUDE_FX
#define GEN_TRACE_FX                                                                                                   \
    GEN_TRACE(sc_dt::sc_fxval);                                                                                        \
    GEN_TRACE(sc_dt::sc_fxval_fast);                                                                                   \
    GEN_TRACE(sc_dt::sc_fxnum);                                                                                        \
    GEN_TRACE(sc_dt::sc_fxnum_fast)
#else
#define GEN_TRACE_FX
#endif

void tracer_base::try_trace_signal(const sc_core::sc_object *obj) {
#undef GEN_TRACE
#define GEN_TRACE(X)                                                                                      \
    {                                                                                                     \
        const auto *sig = dynamic_cast<const sc_core::sc_signal<X> *>(obj);                               \
        if (sig){ sc_core::sc_trace(trf, sig->read(), std::string(sig->name())); return; }                \
    }                                                                                                     \
    {                                                                                                     \
        const auto *sig = dynamic_cast<const sc_core::sc_signal<X,sc_core::SC_MANY_WRITERS> *>(obj);      \
        if (sig){ sc_core::sc_trace(trf, sig->read(), std::string(sig->name())); return; }                \
    }                                                                                                     \
    {                                                                                                     \
        const auto *sig = dynamic_cast<const sc_core::sc_signal<X,sc_core::SC_UNCHECKED_WRITERS> *>(obj); \
        if (sig){ sc_core::sc_trace(trf, sig->read(), std::string(sig->name())); return; }                \
    }
    GEN_TRACE_STD
    GEN_TRACE_FX
}

void tracer_base::try_trace_port(const sc_core::sc_object *obj) {
#undef GEN_TRACE
#define GEN_TRACE(X)                                                                             \
    {                                                                                            \
        const auto *in_port = dynamic_cast<const sc_core::sc_in<X> *>(obj);                      \
        if (in_port) { sc_core::sc_trace(trf, *in_port, std::string(in_port->name())); return; } \
    }                                                                                            \
    {                                                                                            \
        const auto *io_port = dynamic_cast<const sc_core::sc_inout<X> *>(obj);                   \
        if (io_port) { sc_core::sc_trace(trf, *io_port, std::string(io_port->name())); return; } \
    }
    GEN_TRACE_STD;
    GEN_TRACE_FX
#undef GEN_TRACE
}
