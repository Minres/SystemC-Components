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
#include "leveldb/db.h"
#include <json/json.h>

#include <array>
#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
// clang-format off
#include "scv/scv_util.h"
#include "scv/scv_introspection.h"
#include "scv/scv_tr.h"
// clang-format on
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
using namespace std;
using namespace leveldb;
using namespace Json;

#ifdef _MSC_VER
#define scv_tr_TEXT_LLU "%I64u"
#define scv_tr_TEXT_LLX "%I64x"
#define scv_tr_TEXT_16LLX "%016I64x"
#else
#define scv_tr_TEXT_LLU "%llu"
#define scv_tr_TEXT_LLX "%llx"
#define scv_tr_TEXT_16LLX "%016lx"
#endif

// ----------------------------------------------------------------------------
enum EventType { BEGIN, RECORD, END };
const char* EventTypeStr[] = {"BEGIN", "RECORD", "END"};
using data_type = scv_extensions_if::data_type;
// ----------------------------------------------------------------------------
namespace {

struct Database {

    Database(const string& name)
    : key_len(1024) {
        wbuilder.settings_["indentation"] = "";
        CharReaderBuilder::strictMode(&rbuilder.settings_);
        Options options;
        options.create_if_missing = true;
        options.compression = kSnappyCompression;
        DestroyDB(name, options);
        if(!DB::Open(options, name, &db).ok())
            throw runtime_error("Could not create database");
        key_buf = new char[key_len];
    }

    ~Database() {
        delete db;
        delete key_buf;
    }
    /**
     *
     * @param key   the database key
     * @param val   the JSON Value to write
     */
    inline bool writeEntry(string& key, Value& val) {
        return db->Put(write_options, Slice(key.c_str(), key.size()), writeString(wbuilder, val)).ok();
    }
    /**
     *
     * @param key   the database key
     * @param val   the JSON Value to write
     */
    inline bool writeEntry(string&& key, Value& val) {
        return db->Put(write_options, Slice(key.c_str(), key.size()), writeString(wbuilder, val)).ok();
    }
    /**
     *
     * @param id    stream id
     * @param name  stream name
     * @param kind  stream kind
     */
    inline void writeStream(uint64_t id, string name, string kind) {
        auto len = sprintf(key_buf, "s~" scv_tr_TEXT_16LLX, id);
        Value node{objectValue};
        node["id"] = id;
        node["name"] = name;
        node["kind"] = kind;
        db->Put(write_options, Slice(key_buf, len), writeString(wbuilder, node));
    }
    /**
     *
     * @param id        generator id
     * @param name
     * @param stream
     */
    inline void writeGenerator(uint64_t id, string name, uint64_t stream) {
        auto len = sprintf(key_buf, "sg~" scv_tr_TEXT_16LLX "~" scv_tr_TEXT_16LLX, stream, id);
        Value node{objectValue};
        node["id"] = id;
        node["name"] = name;
        node["stream"] = stream;
        db->Put(write_options, Slice(key_buf, len), writeString(wbuilder, node));
    }
    /**
     *
     * @param id        transaction id
     * @param generator
     * @param concurrencyLevel
     */
    inline void writeTransaction(uint64_t id, uint64_t stream_id, uint64_t generator_id, uint64_t concurrencyLevel) {
        Value val{objectValue};
        val["id"] = id;
        val["s"] = stream_id;
        val["g"] = generator_id;
        val["conc"] = concurrencyLevel;
        db->Put(write_options,
                Slice(key_buf, sprintf(key_buf, "sgx~" scv_tr_TEXT_16LLX "~" scv_tr_TEXT_16LLX "~" scv_tr_TEXT_16LLX,
                                       stream_id, generator_id, id)),
                "");
        tx_lut[id] = val;
        // db->Put(write_options, Slice(key, sprintf(key, "x~" scv_tr_TEXT_16LLX, id)), writeString(wbuilder, val));
    }
    /**
     *
     * @param id        transaction id
     * @param streamid  stream transaction id
     * @param type
     * @param time
     */
    inline void writeTxTimepoint(uint64_t id, uint64_t streamid, EventType type, uint64_t time) {
        string value;
        Value node{arrayValue};
        auto len =
            sprintf(key_buf, "st~" scv_tr_TEXT_16LLX "~" scv_tr_TEXT_16LLX "~%s", streamid, time, EventTypeStr[type]);
        if(db->Get(read_options, key_buf, &value).ok() &&
           rbuilder.newCharReader()->parse(value.data(), value.data() + value.size(), &node, nullptr)) {
            node[node.size()] = Value(id);
        } else {
            node[0u] = Value(id);
        }
        db->Put(write_options, Slice(key_buf, len), writeString(wbuilder, node));
        updateTx(id, type, time);
    }

    inline void updateTx(uint64_t id, EventType type, uint64_t time) {
        static const char* typeStr[] = {"START_TIME", "", "END_TIME"};
        auto& node = tx_lut[id];
        node[typeStr[type]] = time;
        if(type == END) {
            db->Put(write_options, Slice(key_buf, sprintf(key_buf, "x~" scv_tr_TEXT_16LLX, id)),
                    writeString(wbuilder, node));
            tx_lut.erase(id);
        }
    }

    inline void updateTx(uint64_t id, Value&& val) {
        auto len = sprintf(key_buf, "x~" scv_tr_TEXT_16LLX, id);
        auto& node = tx_lut[id];
        auto& arrNode = node["attr"];
        if(arrNode.isNull()) {
            Value newNode{arrayValue};
            newNode.append(val);
            node["attr"] = newNode;
        } else {
            arrNode.append(val);
        }
    }

    /**
     *
     * @param id        transaction id
     * @param event
     * @param name
     * @param type
     * @param value
     */
    inline void writeAttribute(uint64_t id, EventType event, const string& name, data_type type, const string& value) {
        Value val;
        val["name"] = name;
        val["type"] = type;
        val["value"] = value;
        val["assoc"] = event;
        updateTx(id, move(val));
    }
    /**
     *
     * @param id        transaction id
     * @param event
     * @param name
     * @param type
     * @param value
     */
    inline void writeAttribute(uint64_t id, EventType event, const string& name, data_type type, uint64_t value) {
        Value val;
        val["name"] = name;
        val["type"] = type;
        val["value"] = value;
        val["assoc"] = event;
        updateTx(id, move(val));
    }
    /**
     *
     * @param id        transaction id
     * @param event
     * @param name
     * @param type
     * @param value
     */
    inline void writeAttribute(uint64_t id, EventType event, const string& name, data_type type, double value) {
        Value val;
        val["name"] = name;
        val["type"] = type;
        val["value"] = value;
        val["assoc"] = event;
        updateTx(id, move(val));
    }
    /**
     *
     * @param name
     * @param sink_id
     * @param src_id
     */
    inline void writeRelation(const string& name, uint64_t sink_id, uint64_t src_id) {
        if(key_len < (name.size() + 32 + 5)) { // reallocate buffer if needed, making sure no buffer overflow
            delete key_buf;
            key_len = name.size() + 32 + 5;
            key_buf = new char[key_len];
        }
        db->Put(write_options,
                Slice(key_buf, sprintf(key_buf, "ro~" scv_tr_TEXT_16LLX "~" scv_tr_TEXT_16LLX "~%s", src_id, sink_id,
                                       name.c_str())),
                "");
        db->Put(write_options,
                Slice(key_buf, sprintf(key_buf, "ri~" scv_tr_TEXT_16LLX "~" scv_tr_TEXT_16LLX "~%s", sink_id, src_id,
                                       name.c_str())),
                "");
    }

private:
    DB* db;
    ReadOptions read_options;
    WriteOptions write_options;
    char* key_buf;
    size_t key_len;
    StreamWriterBuilder wbuilder;
    CharReaderBuilder rbuilder;
    unordered_map<uint64_t, Value> tx_lut;
};

vector<vector<uint64_t>> concurrencyLevel;

Database* db;

void dbCb(const scv_tr_db& _scv_tr_db, scv_tr_db::callback_reason reason, void* data) {
    // This is called from the scv_tr_db ctor.
    static string fName("DEFAULT_scv_tr_sqlite");
    switch(reason) {
    case scv_tr_db::CREATE:
        if((_scv_tr_db.get_name() != nullptr) && (strlen(_scv_tr_db.get_name()) != 0))
            fName = _scv_tr_db.get_name();
        try {
            db = new Database(fName);
            Value val{objectValue};
            val["resolution"] = (long)(sc_get_time_resolution().to_seconds() * 1e15);
            db->writeEntry("__config", val);
        } catch(runtime_error& e) {
            _scv_message::message(_scv_message::TRANSACTION_RECORDING_INTERNAL, e.what());
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
        } catch(runtime_error& e) {
            _scv_message::message(_scv_message::TRANSACTION_RECORDING_INTERNAL, "Can't create stream");
        }
    }
}
// ----------------------------------------------------------------------------
void recordAttribute(uint64_t id, EventType event, const string& name, data_type type, const string& value) {
    try {
        db->writeAttribute(id, event, name, type, value);
    } catch(runtime_error& e) {
        _scv_message::message(_scv_message::TRANSACTION_RECORDING_INTERNAL, "Can't create attribute entry");
    }
}
// ----------------------------------------------------------------------------
void recordAttribute(uint64_t id, EventType event, const string& name, data_type type, long long value) {
    try {
        db->writeAttribute(id, event, name, type, static_cast<uint64_t>(value));
    } catch(runtime_error& e) {
        _scv_message::message(_scv_message::TRANSACTION_RECORDING_INTERNAL, "Can't create attribute entry");
    }
}
// ----------------------------------------------------------------------------
inline void recordAttribute(uint64_t id, EventType event, const string& name, data_type type, double value) {
    try {
        db->writeAttribute(id, event, name, type, value);
    } catch(runtime_error& e) {
        _scv_message::message(_scv_message::TRANSACTION_RECORDING_INTERNAL, "Can't create attribute entry");
    }
}
// ----------------------------------------------------------------------------
void recordAttributes(uint64_t id, EventType eventType, string& prefix, const scv_extensions_if* my_exts_p) {
    if(my_exts_p == nullptr)
        return;
    string name;
    if(prefix == "") {
        name = my_exts_p->get_name();
    } else {
        if((my_exts_p->get_name() == nullptr) || (strlen(my_exts_p->get_name()) == 0)) {
            name = prefix;
        } else {
            name = prefix + "." + my_exts_p->get_name();
        }
    }
    if(name == "")
        name = "<unnamed>";
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
        array<char, 100> tmpString;
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
        } catch(runtime_error& e) {
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
    uint64_t streamId = t.get_scv_tr_stream().get_id();
    vector<uint64_t>::size_type concurrencyIdx;
    const scv_extensions_if* my_exts_p;
    switch(reason) {
    case scv_tr_handle::BEGIN: {
        try {
            if(concurrencyLevel.size() <= streamId)
                concurrencyLevel.resize(streamId + 1);
            vector<uint64_t>& levels = concurrencyLevel.at(streamId);
            for(concurrencyIdx = 0; concurrencyIdx < levels.size(); ++concurrencyIdx) // find a free slot
                if(levels[concurrencyIdx] == 0)
                    break;
            if(concurrencyIdx == levels.size())
                levels.push_back(id);
            else
                levels[concurrencyIdx] = id;
            db->writeTransaction(id, t.get_scv_tr_stream().get_id(), t.get_scv_tr_generator_base().get_id(),
                                 concurrencyIdx);
            db->writeTxTimepoint(id, t.get_scv_tr_stream().get_id(), BEGIN, t.get_begin_sc_time().value());
        } catch(runtime_error& e) {
            _scv_message::message(_scv_message::TRANSACTION_RECORDING_INTERNAL, e.what());
        }
        my_exts_p = t.get_begin_exts_p();
        if(my_exts_p == nullptr)
            my_exts_p = t.get_scv_tr_generator_base().get_begin_exts_p();
        if(my_exts_p) {
            string tmp_str = t.get_scv_tr_generator_base().get_begin_attribute_name()
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
            string tmp_str = t.get_scv_tr_generator_base().get_end_attribute_name()
                                 ? t.get_scv_tr_generator_base().get_end_attribute_name()
                                 : "";
            recordAttributes(t.get_id(), END, tmp_str, my_exts_p);
        }
        try {
            db->writeTxTimepoint(id, t.get_scv_tr_stream().get_id(), END, t.get_end_sc_time().value());
            vector<uint64_t>& levels = concurrencyLevel[streamId];
            for(concurrencyIdx = 0; concurrencyIdx < levels.size(); ++concurrencyIdx)
                if(levels[concurrencyIdx] == id)
                    break;
            if(concurrencyIdx == levels.size())
                levels.push_back(0);
            else
                levels[concurrencyIdx] = 0;
        } catch(runtime_error& e) {
            _scv_message::message(_scv_message::TRANSACTION_RECORDING_INTERNAL, "Can't create transaction end");
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
    string tmp_str(name == nullptr ? "" : name);
    recordAttributes(t.get_id(), RECORD, tmp_str, ext);
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
    } catch(runtime_error& e) {
        _scv_message::message(_scv_message::TRANSACTION_RECORDING_INTERNAL, "Can't create transaction relation");
    }
}
} // namespace
// ----------------------------------------------------------------------------
void scv_tr_ldb_init() {
    scv_tr_db::register_class_cb(dbCb);
    scv_tr_stream::register_class_cb(streamCb);
    scv_tr_generator_base::register_class_cb(generatorCb);
    scv_tr_handle::register_class_cb(transactionCb);
    scv_tr_handle::register_record_attribute_cb(attributeCb);
    scv_tr_handle::register_relation_cb(relationCb);
}
// ----------------------------------------------------------------------------
