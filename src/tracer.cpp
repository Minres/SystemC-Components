/*******************************************************************************
 * Copyright 2017 MINRES Technologies GmbH
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

#include "scc/tracer.h"

#include <cstring>
#include <iostream>
#include <sstream>
#include "scc/scv_tr_db.h"
#include "scc/utilities.h"

using namespace sc_core;

namespace scc {

tracer::tracer(std::string &&name, file_type type, bool enable)
: sc_core::sc_module(sc_core::sc_module_name(sc_core::sc_gen_unique_name("tracer")))
, enabled(enable)
, trf(nullptr)
#ifdef WITH_SCV
, txdb(nullptr)
#endif
{
    if (enabled) {
        trf = sc_create_vcd_trace_file(name.c_str());
        trf->set_time_unit(1, SC_PS);
    }
#ifdef WITH_SCV
    if (type != NONE) {
        std::stringstream ss;
        ss << name;
        switch (type) {
        case TEXT:
            scv_tr_text_init();
            ss << ".txlog";
            break;
        case COMPRESSED:
            scv_tr_compressed_init();
            ss << ".txlog";
            break;
        case SQLITE:
            scv_tr_sqlite_init();
            break;
        }
        txdb = new scv_tr_db(ss.str().c_str());
        scv_tr_db::set_default_db(txdb);
    }
#endif
}

void tracer::end_of_elaboration() {
    if (enabled) descend(sc_get_top_level_objects(sc_curr_simcontext));
}

tracer::~tracer() {
    if (trf) sc_close_vcd_trace_file(trf);
#ifdef WITH_SCV
    delete txdb;
#endif
}

void tracer::descend(const std::vector<sc_object *> &objects) {
    for (auto obj : objects) {
        const char *name = obj->name();
        traceable *t = dynamic_cast<traceable *>(obj);
        if (t) t->trace(trf);
        const char *kind = obj->kind();
        if (strcmp(kind, "sc_signal") == 0) {
            try_trace_signal(obj);
        } else if (strcmp(kind, "sc_inout") == 0 || strcmp(kind, "sc_in") == 0 || strcmp(kind, "sc_port") == 0) {
            try_trace_port(obj);
        } else if (strcmp(kind, "tlm_signal") == 0) {
            obj->trace(trf);
        }
        descend(obj->get_child_objects());
    }
}

#ifndef GEN_TRACE
#define GEN_TRACE
#endif

#define GEN_TRACE_STD                                                                                                  \
    GEN_TRACE(bool);                                                                                                   \
    GEN_TRACE(char);                                                                                                   \
    GEN_TRACE(unsigned char);                                                                                          \
    GEN_TRACE(short);                                                                                                  \
    GEN_TRACE(unsigned short);                                                                                         \
    GEN_TRACE(int);                                                                                                    \
    GEN_TRACE(unsigned int);                                                                                           \
    GEN_TRACE(long);                                                                                                   \
    GEN_TRACE(unsigned long);                                                                                          \
    GEN_TRACE(long long);                                                                                              \
    GEN_TRACE(unsigned long long);                                                                                     \
    GEN_TRACE(sc_dt::int64);                                                                                           \
    GEN_TRACE(sc_dt::uint64);                                                                                          \
    GEN_TRACE(sc_core::sc_time);                                                                                       \
    GEN_TRACE(sc_dt::sc_bit);                                                                                          \
    GEN_TRACE(sc_dt::sc_logic);                                                                                        \
    GEN_TRACE(sc_dt::sc_int_base);                                                                                     \
    GEN_TRACE(sc_dt::sc_uint_base);                                                                                    \
    GEN_TRACE(sc_dt::sc_signed);                                                                                       \
    GEN_TRACE(sc_dt::sc_unsigned);                                                                                     \
    GEN_TRACE(sc_dt::sc_bv_base);                                                                                      \
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

void tracer::try_trace_signal(sc_core::sc_object *obj) {
#undef GEN_TRACE
#define GEN_TRACE(X)                                                                                                   \
    {                                                                                                                  \
        auto *sig = dynamic_cast<sc_core::sc_signal<X> *>(obj);                                                        \
        if (sig) sc_core::sc_trace(trf, sig->read(), std::string(sig->name()));                                        \
    }
    GEN_TRACE_STD;
    GEN_TRACE_FX
}

void tracer::try_trace_port(sc_core::sc_object *obj) {
#undef GEN_TRACE
#define GEN_TRACE(X)                                                                                                   \
    {                                                                                                                  \
        auto *in_port = dynamic_cast<sc_core::sc_in<X> *>(obj);                                                        \
        if (in_port) {                                                                                                 \
            sc_core::sc_trace(trf, *in_port, std::string(in_port->name()));                                            \
            return;                                                                                                    \
        }                                                                                                              \
    }
    GEN_TRACE_STD;
    GEN_TRACE_FX
#undef GEN_TRACE
#define GEN_TRACE(X)                                                                                                   \
    {                                                                                                                  \
        auto *io_port = dynamic_cast<sc_core::sc_inout<X> *>(obj);                                                     \
        if (io_port) {                                                                                                 \
            sc_core::sc_trace(trf, *io_port, std::string(io_port->name()));                                            \
            return;                                                                                                    \
        }                                                                                                              \
    }
    GEN_TRACE_STD;
    GEN_TRACE_FX
}

} /* namespace scc */
