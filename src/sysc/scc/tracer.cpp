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
#include <scc/sc_vcd_trace.h>
#include <scc/trace.h>
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
#include <cstring>
#include <iostream>
#include <lwtr/lwtr.h>
#include <sstream>

using namespace sc_core;
using namespace scc;

tracer::tracer(std::string const&& name, file_type tx_type, file_type sig_type, sc_core::sc_object* top, sc_core::sc_module_name const& nm)
: tracer_base(nm)
, cci_broker(cci::cci_get_broker())
, txdb(nullptr)
, lwtr_db(nullptr)
, owned{sig_type != NONE} {
    if(sig_type == ENABLE)
        sig_type = static_cast<file_type>(sig_trace_type.get_value());
    if(sig_type != NONE) {
        switch(sig_type) {
        default:
            trf = sc_create_vcd_trace_file(name.c_str());
            break;
        case PULL_VCD:
            trf = scc::create_vcd_pull_trace_file(name.c_str());
            break;
        case PUSH_VCD:
            trf = scc::create_vcd_push_trace_file(name.c_str());
            break;
        case FST:
            trf = scc::create_fst_trace_file(name.c_str());
            break;
        }
    }
    if(trf)
        trf->set_time_unit(1, SC_PS);
    init_tx_db(tx_type == ENABLE ? static_cast<file_type>(tx_trace_type.get_value()) : tx_type, std::move(name));
}

tracer::tracer(std::string const&& name, file_type tx_type, sc_core::sc_trace_file* tf, sc_core::sc_object* top,
               sc_core::sc_module_name const& nm)
: tracer_base(nm)
, cci_broker(cci::cci_get_broker())
, txdb(nullptr)
, lwtr_db(nullptr)
, owned{false} {
    trf = tf;
    init_tx_db(tx_type == ENABLE ? static_cast<file_type>(tx_trace_type.get_value()) : tx_type, std::move(name));
}

tracer::~tracer() {
    delete txdb;
    delete lwtr_db;
    if(trf && owned)
        scc_close_vcd_trace_file(trf);
}

void tracer::init_tx_db(file_type type, std::string const&& name) {
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
            auto level = val ? atoi(val) : std::numeric_limits<unsigned>::max();
            switch(level) {
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
        } break;
#ifdef WITH_SQLITE
        case SQLITE:
            SCVNS scv_tr_sqlite_init();
            ss << ".txdb";
            break;
#endif
        case FTR:
            SCVNS scv_tr_ftr_init(false);
            break;
        case CFTR:
            SCVNS scv_tr_ftr_init(true);
            break;
        case LWFTR:
            lwtr::tx_ftr_init(false);
            break;
        case LWCFTR:
            lwtr::tx_ftr_init(true);
            break;
        case CUSTOM:
            SCVNS scv_tr_mtc_init();
            ss << ".txlog";
            break;
        }
        if(type == LWFTR || type == LWCFTR) {
            lwtr_db = new lwtr::tx_db(name.c_str());
        } else {
            txdb = new SCVNS scv_tr_db(ss.str().c_str());
        }
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

void tracer::end_of_simulation() {
    if(close_db_in_eos.get_value()) {
        delete txdb;
        txdb = nullptr;
        delete lwtr_db;
        lwtr_db = nullptr;
        if(trf && owned) {
            scc_close_vcd_trace_file(trf);
            trf = nullptr;
        }
    }
}
