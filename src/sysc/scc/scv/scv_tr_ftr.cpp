/*******************************************************************************
 * Copyright 2018 MINRES Technologies GmbH
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
#include <array>
#include <boost/filesystem.hpp>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <fstream>
#include <ftr/ftr_writer.h>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
// clang-format off
#ifdef HAS_SCV
#include <scv.h>
#else
#include <scv-tr.h>
namespace scv_tr {
#endif
// clang-format on
// ----------------------------------------------------------------------------
using namespace std;
using namespace ftr;
// ----------------------------------------------------------------------------
namespace {
template <bool COMPRESSED> struct tx_db {
    static ftr_writer<COMPRESSED>* db;
    static void dbCb(const scv_tr_db& _scv_tr_db, scv_tr_db::callback_reason reason, void* data) {
        // This is called from the scv_tr_db ctor.
        static string fName("DEFAULT_scv_tr_cbor");
        switch(reason) {
        case scv_tr_db::CREATE:
            if((_scv_tr_db.get_name() != nullptr) && (strlen(_scv_tr_db.get_name()) != 0))
                fName = _scv_tr_db.get_name();
            try {
                db = new ftr_writer<COMPRESSED>(fName + ".ftr");
            } catch(...) {
                _scv_message::message(_scv_message::TRANSACTION_RECORDING_INTERNAL, "Can't open recording file");
            }
            if(!db->cw.enc.ofs.is_open()) {
                delete db;
                db = nullptr;
            } else {
                double secs = sc_core::sc_time::from_value(1ULL).to_seconds();
                auto exp = rint(log(secs) / log(10.0));
                db->writeInfo(static_cast<int8_t>(exp));
            }
            break;
        case scv_tr_db::DELETE:
            try {
                delete db;
            } catch(...) {
                _scv_message::message(_scv_message::TRANSACTION_RECORDING_INTERNAL, "Can't close recording file");
            }
            break;
        default:
            _scv_message::message(_scv_message::TRANSACTION_RECORDING_INTERNAL, "Unknown reason in scv_tr_db callback");
        }
    }
    // ----------------------------------------------------------------------------
    static void streamCb(const scv_tr_stream& s, scv_tr_stream::callback_reason reason, void* data) {
        if(db && reason == scv_tr_stream::CREATE) {
            try {
                db->writeStream(s.get_id(), s.get_name(), s.get_stream_kind());
            } catch(std::runtime_error& e) {
                _scv_message::message(_scv_message::TRANSACTION_RECORDING_INTERNAL, "Can't create stream");
            }
        }
    }
    // ----------------------------------------------------------------------------
    static inline void recordAttribute(uint64_t id, event_type event, const string& name, ftr::data_type type, const string& value) {
        if(db)
            try {
                db->writeAttribute(id, event, name, type, value);
            } catch(std::runtime_error& e) {
                _scv_message::message(_scv_message::TRANSACTION_RECORDING_INTERNAL, "Can't create attribute entry");
            }
    }
    // ----------------------------------------------------------------------------
    static inline void recordAttribute(uint64_t id, event_type event, const string& name, ftr::data_type type, char const* value) {
        if(db)
            try {
                db->writeAttribute(id, event, name, type, value);
            } catch(std::runtime_error& e) {
                _scv_message::message(_scv_message::TRANSACTION_RECORDING_INTERNAL, "Can't create attribute entry");
            }
    }
    // ----------------------------------------------------------------------------
    template <typename T>
    static inline void recordAttribute(uint64_t id, event_type event, const string& name, ftr::data_type type, T value) {
        if(db)
            try {
                db->writeAttribute(id, event, name, type, value);
            } catch(std::runtime_error& e) {
                _scv_message::message(_scv_message::TRANSACTION_RECORDING_INTERNAL, "Can't create attribute entry");
            }
    }
    // ----------------------------------------------------------------------------
    static inline std::string get_name(const char* prefix, const scv_extensions_if* my_exts_p) {
        string name{prefix};
        if(!prefix || strlen(prefix) == 0) {
            name = my_exts_p->get_name();
        } else {
            if((my_exts_p->get_name() != nullptr) && (strlen(my_exts_p->get_name()) > 0)) {
                name += ".";
                name += my_exts_p->get_name();
            }
        }
        return (name == "") ? "<unnamed>" : name;
    }
    // ----------------------------------------------------------------------------
    static void recordAttributes(uint64_t id, event_type eventType, char const* prefix, const scv_extensions_if* my_exts_p) {
        if(!db || my_exts_p == nullptr)
            return;
        auto name = get_name(prefix, my_exts_p);
        switch(my_exts_p->get_type()) {
        case scv_extensions_if::RECORD: {
            int num_fields = my_exts_p->get_num_fields();
            if(num_fields > 0) {
                for(int field_counter = 0; field_counter < num_fields; field_counter++) {
                    const scv_extensions_if* field_data_p = my_exts_p->get_field(field_counter);
                    recordAttributes(id, eventType, prefix, field_data_p);
                }
            }
        } break;
        case scv_extensions_if::ENUMERATION:
            recordAttribute(id, eventType, name, ftr::data_type::ENUMERATION, my_exts_p->get_enum_string((int)(my_exts_p->get_integer())));
            break;
        case scv_extensions_if::BOOLEAN:
            recordAttribute(id, eventType, name, ftr::data_type::BOOLEAN, my_exts_p->get_bool());
            break;
        case scv_extensions_if::INTEGER:
        case scv_extensions_if::FIXED_POINT_INTEGER:
            recordAttribute(id, eventType, name, ftr::data_type::INTEGER, my_exts_p->get_integer());
            break;
        case scv_extensions_if::UNSIGNED:
            recordAttribute(id, eventType, name, ftr::data_type::UNSIGNED, my_exts_p->get_integer());
            break;
        case scv_extensions_if::POINTER:
            recordAttribute(id, eventType, name, ftr::data_type::POINTER, (long long)my_exts_p->get_pointer());
            break;
        case scv_extensions_if::STRING:
            recordAttribute(id, eventType, name, ftr::data_type::STRING, my_exts_p->get_string());
            break;
        case scv_extensions_if::FLOATING_POINT_NUMBER:
            recordAttribute(id, eventType, name, ftr::data_type::FLOATING_POINT_NUMBER, my_exts_p->get_double());
            break;
        case scv_extensions_if::BIT_VECTOR: {
            sc_bv_base tmp_bv(my_exts_p->get_bitwidth());
            my_exts_p->get_value(tmp_bv);
            recordAttribute(id, eventType, name, ftr::data_type::BIT_VECTOR, tmp_bv.to_string());
        } break;
        case scv_extensions_if::LOGIC_VECTOR: {
            sc_lv_base tmp_lv(my_exts_p->get_bitwidth());
            my_exts_p->get_value(tmp_lv);
            recordAttribute(id, eventType, name, ftr::data_type::LOGIC_VECTOR, tmp_lv.to_string());
        } break;
        case scv_extensions_if::ARRAY:
            for(int array_elt_index = 0; array_elt_index < my_exts_p->get_array_size(); array_elt_index++) {
                const scv_extensions_if* field_data_p = my_exts_p->get_array_elt(array_elt_index);
                recordAttributes(id, eventType, prefix, field_data_p);
            }
            break;
        default: {
            std::array<char, 100> tmpString;
            sprintf(tmpString.data(), "Unsupported attribute type = %d", my_exts_p->get_type());
            _scv_message::message(_scv_message::TRANSACTION_RECORDING_INTERNAL, tmpString.data());
        }
        }
    }
    // ----------------------------------------------------------------------------
    static void generatorCb(const scv_tr_generator_base& g, scv_tr_generator_base::callback_reason reason, void* data) {
        if(db && reason == scv_tr_generator_base::CREATE) {
            try {
                db->writeGenerator(g.get_id(), g.get_name(), g.get_scv_tr_stream().get_id());
            } catch(std::runtime_error& e) {
                _scv_message::message(_scv_message::TRANSACTION_RECORDING_INTERNAL, "Can't create generator entry");
            }
        }
    }
    // ----------------------------------------------------------------------------
    static void transactionCb(const scv_tr_handle& t, scv_tr_handle::callback_reason reason, void* data) {
        if(!db || !t.get_scv_tr_stream().get_scv_tr_db() || !t.get_scv_tr_stream().get_scv_tr_db()->get_recording())
            return;
        uint64_t id = t.get_id();
        switch(reason) {
        case scv_tr_handle::BEGIN: {
            db->startTransaction(id, t.get_scv_tr_generator_base().get_id(), t.get_scv_tr_generator_base().get_scv_tr_stream().get_id(),
                                 t.get_begin_sc_time() / sc_core::sc_time(1, sc_core::SC_PS));

            auto my_exts_p = t.get_begin_exts_p();
            if(my_exts_p == nullptr)
                my_exts_p = t.get_scv_tr_generator_base().get_begin_exts_p();
            if(my_exts_p) {
                auto tmp_str = t.get_scv_tr_generator_base().get_begin_attribute_name()
                                   ? t.get_scv_tr_generator_base().get_begin_attribute_name()
                                   : "";
                recordAttributes(id, event_type::BEGIN, tmp_str, my_exts_p);
            }
        } break;
        case scv_tr_handle::END: {
            auto my_exts_p = t.get_end_exts_p();
            if(my_exts_p == nullptr)
                my_exts_p = t.get_scv_tr_generator_base().get_end_exts_p();
            if(my_exts_p) {
                auto tmp_str =
                    t.get_scv_tr_generator_base().get_end_attribute_name() ? t.get_scv_tr_generator_base().get_end_attribute_name() : "";
                recordAttributes(id, event_type::END, tmp_str, my_exts_p);
            }
            db->endTransaction(id, t.get_end_sc_time() / sc_core::sc_time(1, sc_core::SC_PS));
        } break;
        default:;
        }
    }
    // ----------------------------------------------------------------------------
    static void attributeCb(const scv_tr_handle& t, const char* name, const scv_extensions_if* ext, void* data) {
        if(!db || !t.get_scv_tr_stream().get_scv_tr_db() || !t.get_scv_tr_stream().get_scv_tr_db()->get_recording())
            return;
        recordAttributes(t.get_id(), event_type::RECORD, name == nullptr ? "" : name, ext);
    }
    // ----------------------------------------------------------------------------
    static void relationCb(const scv_tr_handle& tr_1, const scv_tr_handle& tr_2, void* data, scv_tr_relation_handle_t relation_handle) {
        auto& stream1 = tr_1.get_scv_tr_stream();
        auto txdb = stream1.get_scv_tr_db();
        if(!db || !txdb || !txdb->get_recording())
            return;
        try {
            auto& stream2 = tr_2.get_scv_tr_stream();
            db->writeRelation(txdb->get_relation_name(relation_handle), stream1.get_id(), tr_1.get_id(), stream2.get_id(), tr_2.get_id());
        } catch(std::runtime_error& e) {
            _scv_message::message(_scv_message::TRANSACTION_RECORDING_INTERNAL, "Can't create transaction relation");
        }
    }
};
template <bool COMPRESSED> ftr_writer<COMPRESSED>* tx_db<COMPRESSED>::db{nullptr};
} // namespace
// ----------------------------------------------------------------------------
void scv_tr_cbor_init(bool compressed) {
    if(compressed) {
        scv_tr_db::register_class_cb(tx_db<true>::dbCb);
        scv_tr_stream::register_class_cb(tx_db<true>::streamCb);
        scv_tr_generator_base::register_class_cb(tx_db<true>::generatorCb);
        scv_tr_handle::register_class_cb(tx_db<true>::transactionCb);
        scv_tr_handle::register_record_attribute_cb(tx_db<true>::attributeCb);
        scv_tr_handle::register_relation_cb(tx_db<true>::relationCb);

    } else {
        scv_tr_db::register_class_cb(tx_db<false>::dbCb);
        scv_tr_stream::register_class_cb(tx_db<false>::streamCb);
        scv_tr_generator_base::register_class_cb(tx_db<false>::generatorCb);
        scv_tr_handle::register_class_cb(tx_db<false>::transactionCb);
        scv_tr_handle::register_record_attribute_cb(tx_db<false>::attributeCb);
        scv_tr_handle::register_relation_cb(tx_db<false>::relationCb);
    }
}
// ----------------------------------------------------------------------------
#ifndef HAS_SCV
}
#endif
