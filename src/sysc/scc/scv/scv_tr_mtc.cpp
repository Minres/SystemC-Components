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
// ----------------------------------------------------------------------------
using namespace std;

// ----------------------------------------------------------------------------
enum EventType { BEGIN, RECORD, END };
using data_type = scv_extensions_if::data_type;
// ----------------------------------------------------------------------------
namespace {
class Base {
protected:
    boost::filesystem::path dir;
    std::ofstream out;
    bool const is_open;
    std::array<char, 1024> buffer;
    Base(const std::string& name)
    : dir(name.c_str())
    , out(name)
    , is_open(out.is_open()) {
        //        if(boost::filesystem::exists(dir))
        //            boost::filesystem::remove_all(dir);
        //        boost::filesystem::create_directory(dir);
    }
    ~Base() {
        if(is_open)
            out.close();
    }
};

struct Database : Base {
    uint64_t idx{0};
    Database(const std::string& name)
    : Base(name) {}

    inline uint64_t getIdOf(const std::string& str) { return idx; }

    inline void writeStream(uint64_t id, std::string const& name, std::string const& kind) {
        if(is_open) {
            auto len = sprintf(buffer.data(), "scv_tr_stream (ID %lu, name \"%s\", kind \"%s\")\n", id, name.c_str(),
                               kind.c_str());
            out.write(buffer.data(), len);
        }
    }

    inline void writeGenerator(uint64_t id, std::string const& name, uint64_t stream) {
        if(is_open) {
            auto len = sprintf(buffer.data(), "scv_tr_generator (ID %lu, name \"%s\", scv_tr_stream %lu,\n", id,
                               name.c_str(), stream);
            out.write(buffer.data(), len);
        }
    }

    inline uint64_t writeTransaction(uint64_t id, uint64_t generator) {
        return ++idx; // d.writeTx(id, generator, concurrencyLevel);
    }

    inline void writeTxTimepoint(uint64_t id, int type, uint64_t time, uint64_t file_offset) {
        //        t.append(type, time, file_offset);
    }

    inline void writeAttribute(uint64_t id, EventType event, const string& name, data_type type, const string& value) {
        //        d.writeAttribute(id, event, c.getIdOf(name), type, c.getIdOf(value));
    }

    inline void writeAttribute(uint64_t id, EventType event, const string& name, data_type type, uint64_t value) {
        //        d.writeAttribute(id, event, c.getIdOf(name), type, value);
    }

    inline void writeAttribute(uint64_t id, EventType event, const string& name, data_type type, double value) {
        //        int exponent;
        //        const double mantissa = frexp(value, &exponent);
    }

    inline void writeRelation(const std::string& name, uint64_t sink_id, uint64_t src_id) {
        //        d.writeRelation(c.getIdOf(name), src_id, sink_id);
    }
};

Database* db;
std::unordered_map<uint64_t, uint64_t> id2offset;

void dbCb(const scv_tr_db& _scv_tr_db, scv_tr_db::callback_reason reason, void* data) {
    // This is called from the scv_tr_db ctor.
    static string fName("DEFAULT_scv_tr_sqlite");
    switch(reason) {
    case scv_tr_db::CREATE:
        if((_scv_tr_db.get_name() != nullptr) && (strlen(_scv_tr_db.get_name()) != 0))
            fName = _scv_tr_db.get_name();
        try {
            db = new Database(fName);
        } catch(...) {
            _scv_message::message(_scv_message::TRANSACTION_RECORDING_INTERNAL, "Can't open recording file");
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
void streamCb(const scv_tr_stream& s, scv_tr_stream::callback_reason reason, void* data) {
    if(reason == scv_tr_stream::CREATE) {
        try {
            db->writeStream(s.get_id(), s.get_name(), s.get_stream_kind());
        } catch(std::runtime_error& e) {
            _scv_message::message(_scv_message::TRANSACTION_RECORDING_INTERNAL, "Can't create stream");
        }
    }
}
// ----------------------------------------------------------------------------
inline void recordAttribute(uint64_t id, EventType event, const string& name, data_type type, const string& value) {
    try {
        db->writeAttribute(id, event, name, type, value);
    } catch(std::runtime_error& e) {
        _scv_message::message(_scv_message::TRANSACTION_RECORDING_INTERNAL, "Can't create attribute entry");
    }
}
// ----------------------------------------------------------------------------
inline void recordAttribute(uint64_t id, EventType event, const string& name, data_type type, long long value) {
    try {
        db->writeAttribute(id, event, name, type, static_cast<uint64_t>(value));
    } catch(std::runtime_error& e) {
        _scv_message::message(_scv_message::TRANSACTION_RECORDING_INTERNAL, "Can't create attribute entry");
    }
}
// ----------------------------------------------------------------------------
inline void recordAttribute(uint64_t id, EventType event, const string& name, data_type type, double value) {
    try {
        db->writeAttribute(id, event, name, type, value);
    } catch(std::runtime_error& e) {
        _scv_message::message(_scv_message::TRANSACTION_RECORDING_INTERNAL, "Can't create attribute entry");
    }
}

inline std::string get_name(const char* prefix, const scv_extensions_if* my_exts_p) {
    static thread_local std::unordered_map<const scv_extensions_if*, std::string> name_lut;
    auto it = name_lut.find(my_exts_p);
    if(it != name_lut.end())
        return it->second;
    string name;
    if(!prefix || strlen(prefix) == 0) {
        name = my_exts_p->get_name();
    } else {
        if((my_exts_p->get_name() == nullptr) || (strlen(my_exts_p->get_name()) == 0)) {
            name = prefix;
        } else {
            name = std::string(prefix) + "." + my_exts_p->get_name();
        }
    }
    name_lut[my_exts_p] = (name == "") ? "<unnamed>" : name;
    return (name == "") ? "<unnamed>" : name;
}

// ----------------------------------------------------------------------------
void recordAttributes(uint64_t id, EventType eventType, char const* prefix, const scv_extensions_if* my_exts_p) {
    if(my_exts_p == nullptr)
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
        recordAttribute(id, eventType, name, scv_extensions_if::ENUMERATION,
                        my_exts_p->get_enum_string((int)(my_exts_p->get_integer())));
        break;
    case scv_extensions_if::BOOLEAN:
        recordAttribute(id, eventType, name, scv_extensions_if::BOOLEAN, my_exts_p->get_bool() ? "TRUE" : "FALSE");
        break;
    case scv_extensions_if::INTEGER:
    case scv_extensions_if::FIXED_POINT_INTEGER:
        recordAttribute(id, eventType, name, scv_extensions_if::INTEGER, my_exts_p->get_integer());
        break;
    case scv_extensions_if::UNSIGNED:
        recordAttribute(id, eventType, name, scv_extensions_if::UNSIGNED, my_exts_p->get_integer());
        break;
    case scv_extensions_if::POINTER:
        recordAttribute(id, eventType, name, scv_extensions_if::POINTER, (long long)my_exts_p->get_pointer());
        break;
    case scv_extensions_if::STRING:
        recordAttribute(id, eventType, name, scv_extensions_if::STRING, my_exts_p->get_string());
        break;
    case scv_extensions_if::FLOATING_POINT_NUMBER:
        recordAttribute(id, eventType, name, scv_extensions_if::FLOATING_POINT_NUMBER, my_exts_p->get_double());
        break;
    case scv_extensions_if::BIT_VECTOR: {
        sc_bv_base tmp_bv(my_exts_p->get_bitwidth());
        my_exts_p->get_value(tmp_bv);
        recordAttribute(id, eventType, name, scv_extensions_if::BIT_VECTOR, tmp_bv.to_string());
    } break;
    case scv_extensions_if::LOGIC_VECTOR: {
        sc_lv_base tmp_lv(my_exts_p->get_bitwidth());
        my_exts_p->get_value(tmp_lv);
        recordAttribute(id, eventType, name, scv_extensions_if::LOGIC_VECTOR, tmp_lv.to_string());
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
void generatorCb(const scv_tr_generator_base& g, scv_tr_generator_base::callback_reason reason, void* data) {
    if(reason == scv_tr_generator_base::CREATE && db) {
        try {
            db->writeGenerator(g.get_id(), g.get_name(), g.get_scv_tr_stream().get_id());
        } catch(std::runtime_error& e) {
            _scv_message::message(_scv_message::TRANSACTION_RECORDING_INTERNAL, "Can't create generator entry");
        }
    }
}
// ----------------------------------------------------------------------------
void transactionCb(const scv_tr_handle& t, scv_tr_handle::callback_reason reason, void* data) {
    if(!db)
        return;
    if(t.get_scv_tr_stream().get_scv_tr_db() == nullptr)
        return;
    if(t.get_scv_tr_stream().get_scv_tr_db()->get_recording() == false)
        return;

    uint64_t id = t.get_id();
    vector<uint64_t>::size_type concurrencyIdx;
    const scv_extensions_if* my_exts_p;
    switch(reason) {
    case scv_tr_handle::BEGIN: {
        my_exts_p = t.get_begin_exts_p();
        if(my_exts_p == nullptr)
            my_exts_p = t.get_scv_tr_generator_base().get_begin_exts_p();
        if(my_exts_p) {
            auto tmp_str = t.get_scv_tr_generator_base().get_begin_attribute_name()
                               ? t.get_scv_tr_generator_base().get_begin_attribute_name()
                               : "";
            recordAttributes(id, BEGIN, tmp_str, my_exts_p);
        }
    } break;
    case scv_tr_handle::END: {
        my_exts_p = t.get_end_exts_p();
        if(my_exts_p == nullptr)
            my_exts_p = t.get_scv_tr_generator_base().get_end_exts_p();
        if(my_exts_p) {
            auto tmp_str = t.get_scv_tr_generator_base().get_end_attribute_name()
                               ? t.get_scv_tr_generator_base().get_end_attribute_name()
                               : "";
            recordAttributes(t.get_id(), END, tmp_str, my_exts_p);
        }
    } break;
    default:;
    }
}
// ----------------------------------------------------------------------------
void attributeCb(const scv_tr_handle& t, const char* name, const scv_extensions_if* ext, void* data) {
    if(!db)
        return;
    if(t.get_scv_tr_stream().get_scv_tr_db() == nullptr)
        return;
    if(t.get_scv_tr_stream().get_scv_tr_db()->get_recording() == false)
        return;
    recordAttributes(t.get_id(), RECORD, name == nullptr ? "" : name, ext);
}
// ----------------------------------------------------------------------------
void relationCb(const scv_tr_handle& tr_1, const scv_tr_handle& tr_2, void* data,
                scv_tr_relation_handle_t relation_handle) {
    if(!db)
        return;
    if(tr_1.get_scv_tr_stream().get_scv_tr_db() == nullptr)
        return;
    if(tr_1.get_scv_tr_stream().get_scv_tr_db()->get_recording() == false)
        return;
    try {
        db->writeRelation(tr_1.get_scv_tr_stream().get_scv_tr_db()->get_relation_name(relation_handle), tr_1.get_id(),
                          tr_2.get_id());
    } catch(std::runtime_error& e) {
        _scv_message::message(_scv_message::TRANSACTION_RECORDING_INTERNAL, "Can't create transaction relation");
    }
}
} // namespace
// ----------------------------------------------------------------------------
void scv_tr_mtc_init() {
    scv_tr_db::register_class_cb(dbCb);
    scv_tr_stream::register_class_cb(streamCb);
    scv_tr_generator_base::register_class_cb(generatorCb);
    scv_tr_handle::register_class_cb(transactionCb);
    scv_tr_handle::register_record_attribute_cb(attributeCb);
    scv_tr_handle::register_relation_cb(relationCb);
}
// ----------------------------------------------------------------------------
#ifndef HAS_SCV
}
#endif
