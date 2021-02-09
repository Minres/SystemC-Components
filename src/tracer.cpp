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

#include "scc/tracer.h"

#include "scc/report.h"
#include "scc/scv_tr_db.h"
#include "scc/utilities.h"
#include <cstring>
#include <iostream>
#include <sstream>

using namespace sc_core;
using namespace scc;

tracer::tracer(const std::string&& name, file_type type, bool enable)
: tracer_base(sc_core::sc_module_name(sc_core::sc_gen_unique_name("tracer")))
#ifdef WITH_SCV
, txdb(nullptr)
#endif
{
    if(enable) {
        trf = sc_create_vcd_trace_file(name.c_str());
        trf->set_time_unit(1, SC_PS);
        owned = true;
    }
#ifdef WITH_SCV
    if(type != NONE) {
        std::stringstream ss;
        ss << name;
        switch(type) {
        case COMPRESSED:
#ifdef WITH_ZLIB
            scv_tr_compressed_init();
            ss << ".txlog";
            break;
#endif
        case TEXT:
            scv_tr_text_init();
            ss << ".txlog";
            break;
        case SQLITE:
            scv_tr_sqlite_init();
            ss << ".txdb";
            break;
        default:
            break;
        }
        txdb = new scv_tr_db(ss.str().c_str());
        scv_tr_db::set_default_db(txdb);
    }
#endif
}

void tracer::end_of_elaboration() {
    if(trf)
        for(auto o : sc_get_top_level_objects(sc_curr_simcontext))
            descend(o, true);
}

tracer::~tracer() {
#ifdef WITH_SCV
    delete txdb;
#endif
}
