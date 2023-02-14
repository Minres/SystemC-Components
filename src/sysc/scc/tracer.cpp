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

#include "tracer.h"
#include "report.h"
#include "sc_vcd_trace.h"
#include "scv/scv_tr_db.h"
#include "utilities.h"
#ifdef HAS_SCV
#include <scv.h>
#ifndef SCVNS
#define SCVNS
#endif
#else
#include <scv-tr.h>
#ifndef SCVNS
#define SCVNS ::scv_tr::
#endif
#endif
#include <lwtr/lwtr.h>
#include <cstring>
#include <iostream>
#include <sstream>

using namespace sc_core;
using namespace scc;

tracer::tracer(std::string const&& name, file_type type, bool enable, sc_core::sc_object* top)
: tracer_base(sc_core::sc_module_name(sc_core::sc_gen_unique_name("$$$tracer$$$")))
, txdb(nullptr)
, owned{enable} {
    if(enable) {
        trf = sc_create_vcd_trace_file(name.c_str());
        trf->set_time_unit(1, SC_PS);
    }
    init_scv_db(type, std::move(name));
}

tracer::tracer(std::string const&& name, file_type type, sc_core::sc_trace_file* tf, sc_core::sc_object* top)
: tracer_base(sc_core::sc_module_name(sc_core::sc_gen_unique_name("$$$tracer$$$")))
, txdb(nullptr)
, owned{false} {
    trf = tf;
    init_scv_db(type, std::move(name));
}

tracer::~tracer() {
    delete txdb;
    if(trf && owned)
        scc_close_vcd_trace_file(trf);
}

void tracer::init_scv_db(file_type type, std::string const&& name) {
    if(type != NONE) {
        std::stringstream ss;
        ss << name;
        switch(type) {
        default:
            SCVNS scv_tr_text_init();
            ss << ".txlog";
            break;
        case COMPRESSED: {
            auto* val = getenv("SCC_SCV_TR_COMPRESSION_LEVEL");
            auto level = val? atoi(val):std::numeric_limits<unsigned>::max();
            switch(level){
            case 0:
                SCVNS scv_tr_plain_init();
                break;
            case 2:
                SCVNS scv_tr_compressed_init();
                break;
            default:
                SCVNS scv_tr_lz4_init();
                break;
            }
            ss << ".txlog";
        }
        break;
#ifdef WITH_SQLITE
        case SQLITE:
            SCVNS scv_tr_sqlite_init();
            ss << ".txdb";
            break;
#endif
        case FTR:
            SCVNS scv_tr_cbor_init();
            break;
        case CUSTOM:
            SCVNS scv_tr_mtc_init();
            ss << ".txlog";
            break;
        }
        txdb = new SCVNS scv_tr_db(ss.str().c_str());
        SCVNS scv_tr_db::set_default_db(txdb);
//        lwtr::tx_text_lz4_init();
//        lwtr_db = new lwtr::tx_db(name.c_str());
        if(trf) {
            trf->write_comment(std::string("TXREC: ") + ss.str());
        }
    }
}

void tracer::end_of_elaboration() {
    if(trf) {
        if(top) {
            descend(top, trf);
        } else {
            for(auto o : sc_get_top_level_objects())
                descend(o, default_trace_enable);
        }
    }
}
