/*******************************************************************************
 * Copyright 2014, 2018 MINRES Technologies GmbH
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
#include "sqlite3.h"
#include <array>
#include <cstdio>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
#ifdef HAS_SCV
#include <scv.h>
#else
#include <scv-tr.h>
namespace scv_tr {
#endif
// ----------------------------------------------------------------------------
constexpr auto SQLITEWRAPPER_ERROR = 1000;
// ----------------------------------------------------------------------------
using namespace std;

class SQLiteDB {
public:
    class SQLiteException : public runtime_error {
    public:
        SQLiteException(const int nErrCode, const char* msg, bool doFree = true)
        : runtime_error(msg)
        , mnErrCode(0) {
            if(doFree && msg)
                sqlite3_free(const_cast<char*>(msg));
        }
        const int errorCode() { return mnErrCode; }
        const char* errorMessage() { return what(); }

    private:
        int mnErrCode;
    };

    SQLiteDB() = default;

    void open(const string szFile) {
        int nRet = sqlite3_open(szFile.c_str(), &db);
        if(nRet != SQLITE_OK)
            throw SQLiteException(nRet, sqlite3_errmsg(db), false);
        sqlite3_busy_timeout(db, busyTimeoutMs);
    }

    inline bool isOpen() { return db != nullptr; }

    void close() {
        if(db) {
            int nRet = sqlite3_close(db);
            if(nRet == SQLITE_OK)
                db = nullptr;
            else if(nRet == SQLITE_BUSY) {
                while(nRet == SQLITE_BUSY) { // maybe include _LOCKED
                    sqlite3_stmt* stmt = sqlite3_next_stmt(db, nullptr);
                    if(stmt)
                        sqlite3_finalize(stmt); // don't trap, can't handle it anyway
                    nRet = sqlite3_close(db);
                }
                if(nRet != SQLITE_OK)
                    throw SQLiteException(SQLITEWRAPPER_ERROR, "Unable to close database", false);
                db = nullptr;
            } else
                throw SQLiteException(SQLITEWRAPPER_ERROR, "Unable to close database", false);
        }
    }

    inline int exec(const string szSQL) { return exec(szSQL.c_str()); }
    inline sqlite3_stmt* prepare(const string szSQL) {
        sqlite3_stmt* ret = nullptr;
        const char* tail;
        sqlite3_prepare_v2(db, szSQL.c_str(), szSQL.size(), &ret, &tail);
        return ret;
    }

    int exec(const char* szSQL) {
        checkDB();
        char* szError = nullptr;
        int nRet = sqlite3_exec(db, szSQL, nullptr, nullptr, &szError);
        if(nRet == SQLITE_OK)
            return sqlite3_changes(db);
        else
            throw SQLiteException(nRet, szError);
    }

    int exec(sqlite3_stmt* stmt) {
        checkDB();
        int nRet = sqlite3_step(stmt);
        if(nRet == SQLITE_OK || nRet == SQLITE_DONE) {
            sqlite3_reset(stmt);
            return sqlite3_changes(db);
        } else
            throw SQLiteException(nRet, sqlite3_errmsg(db));
    }

protected:
    inline void checkDB() {
        if(!db)
            throw SQLiteException(SQLITEWRAPPER_ERROR, "Database not open", false);
    }

private:
    int busyTimeoutMs{60000};
    sqlite3* db{nullptr};
};
// ----------------------------------------------------------------------------
static SQLiteDB db;
static vector<vector<uint64_t>*> concurrencyLevel;
static sqlite3_stmt *stream_stmt, *gen_stmt, *tx_stmt, *evt_stmt, *attr_stmt, *rel_stmt;

// ----------------------------------------------------------------------------
enum EventType { BEGIN, RECORD, END };
using data_type = scv_extensions_if::data_type;
// ----------------------------------------------------------------------------
#define SIM_PROPS "ScvSimProps"
#define STREAM_TABLE "ScvStream"
#define GENERATOR_TABLE "ScvGenerator"
#define TX_TABLE "ScvTx"
#define TX_EVENT_TABLE "ScvTxEvent"
#define TX_ATTRIBUTE_TABLE "ScvTxAttribute"
#define TX_RELATION_TABLE "ScvTxRelation"

static void dbCb(const scv_tr_db& _scv_tr_db, scv_tr_db::callback_reason reason, void* data) {
    char* tail = nullptr;
    // This is called from the scv_tr_db ctor.
    static string fName("DEFAULT_scv_tr_sqlite");
    switch(reason) {
    case scv_tr_db::CREATE:
        if((_scv_tr_db.get_name() != nullptr) && (strlen(_scv_tr_db.get_name()) != 0))
            fName = _scv_tr_db.get_name();
        try {
            remove(fName.c_str());
            db.open(fName);
            // performance related according to
            // http://blog.quibb.org/2010/08/fast-bulk-inserts-into-sqlite/
            db.exec("PRAGMA synchronous=OFF");
            db.exec("PRAGMA count_changes=OFF");
            db.exec("PRAGMA journal_mode=MEMORY");
            db.exec("PRAGMA temp_store=MEMORY");
            // scv_out << "TB Transaction Recording has started, file = " <<
            // my_sqlite_file_name << endl;
            db.exec("CREATE TABLE  IF NOT EXISTS " STREAM_TABLE
                    "(id INTEGER  NOT null PRIMARY KEY, name TEXT, kind TEXT);");
            db.exec("CREATE TABLE  IF NOT EXISTS " GENERATOR_TABLE "(id INTEGER  NOT null PRIMARY KEY, stream INTEGER "
                    "REFERENCES " STREAM_TABLE "(id), name TEXT, begin_attr INTEGER, end_attr INTEGER);");
            db.exec("CREATE TABLE  IF NOT EXISTS " TX_TABLE "(id INTEGER  NOT null PRIMARY KEY, generator INTEGER "
                    "REFERENCES " GENERATOR_TABLE "(id), stream INTEGER REFERENCES " STREAM_TABLE
                    "(id), concurrencyLevel INTEGER);");
            db.exec("CREATE TABLE  IF NOT EXISTS " TX_EVENT_TABLE "(tx INTEGER REFERENCES " TX_TABLE
                    "(id), type INTEGER, time INTEGER);");
            db.exec("CREATE TABLE  IF NOT EXISTS " TX_ATTRIBUTE_TABLE "(tx INTEGER REFERENCES " TX_TABLE
                    "(id), type INTEGER, name "
                    "TEXT, data_type INTEGER, "
                    "data_value TEXT);");
            db.exec("CREATE TABLE  IF NOT EXISTS " TX_RELATION_TABLE "(name TEXT, src INTEGER REFERENCES " TX_TABLE
                    "(id), sink INTEGER REFERENCES " TX_TABLE "(id));");
            db.exec("CREATE TABLE  IF NOT EXISTS " SIM_PROPS "(time_resolution INTEGER);");
            db.exec("BEGIN TRANSACTION");
            std::ostringstream ss;
            ss << "INSERT INTO " SIM_PROPS " (time_resolution) values ("
               << (long)(sc_core::sc_get_time_resolution().to_seconds() * 1e15) << ");";
            db.exec(ss.str().c_str());
            stream_stmt = db.prepare("INSERT INTO " STREAM_TABLE " (id, name, kind) values (@ID,@NAME,@KIND);");
            gen_stmt = db.prepare("INSERT INTO " GENERATOR_TABLE " (id,stream, name)"
                                  " values (@ID,@STRM_DI,@NAME);");
            tx_stmt = db.prepare("INSERT INTO " TX_TABLE " (id,generator,stream, concurrencyLevel)"
                                 " values (@ID,@GEN_ID,@STREAM_ID,@CONC_LEVEL);");
            evt_stmt = db.prepare("INSERT INTO " TX_EVENT_TABLE " (tx,type,time)"
                                  " values (@TX_ID,@TYPE,@TIMESTAMP);");
            attr_stmt = db.prepare("INSERT INTO " TX_ATTRIBUTE_TABLE " (tx,type,name,data_type,data_value) "
                                   "values (@ID,@EVENTID,@NAME,@TYPE,@VALUE);");
            rel_stmt = db.prepare("INSERT INTO " TX_RELATION_TABLE " (name,sink,src)"
                                  "values (@NAME,@ID1,@ID2);");

        } catch(SQLiteDB::SQLiteException& e) {
            _scv_message::message(_scv_message::TRANSACTION_RECORDING_INTERNAL, "Can't open recording file");
        }
        break;
    case scv_tr_db::DELETE:
        try {
            // scv_out << "Transaction Recording is closing file: " <<
            // my_sqlite_file_name << endl;
            db.exec("COMMIT TRANSACTION");
            db.close();
        } catch(SQLiteDB::SQLiteException& e) {
            _scv_message::message(_scv_message::TRANSACTION_RECORDING_INTERNAL, "Can't close recording file");
        }
        break;
    default:
        _scv_message::message(_scv_message::TRANSACTION_RECORDING_INTERNAL, "Unknown reason in scv_tr_db callback");
    }
}
// ----------------------------------------------------------------------------
static void streamCb(const scv_tr_stream& s, scv_tr_stream::callback_reason reason, void* data) {
    if(reason == scv_tr_stream::CREATE && db.isOpen()) {
        try {
            sqlite3_bind_int64(stream_stmt, 1, s.get_id());
            sqlite3_bind_text(stream_stmt, 2, s.get_name(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stream_stmt, 3, s.get_stream_kind() ? s.get_stream_kind() : "<unnamed>", -1,
                              SQLITE_TRANSIENT);
            db.exec(stream_stmt);
            if(concurrencyLevel.size() <= s.get_id())
                concurrencyLevel.resize(s.get_id() + 1);
            concurrencyLevel[s.get_id()] = new vector<uint64_t>();
        } catch(SQLiteDB::SQLiteException& e) {
            _scv_message::message(_scv_message::TRANSACTION_RECORDING_INTERNAL, "Can't create stream");
        }
    }
}
// ----------------------------------------------------------------------------
void recordAttribute(uint64_t id, EventType event, const string& name, data_type type, const string& value) {
    try {
        sqlite3_bind_int64(attr_stmt, 1, id);
        sqlite3_bind_int(attr_stmt, 2, event);
        sqlite3_bind_text(attr_stmt, 3, name.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(attr_stmt, 4, type);
        sqlite3_bind_text(attr_stmt, 5, value.c_str(), -1, SQLITE_TRANSIENT);
        db.exec(attr_stmt);
    } catch(SQLiteDB::SQLiteException& e) {
        _scv_message::message(_scv_message::TRANSACTION_RECORDING_INTERNAL, "Can't create attribute entry");
    }
}
// ----------------------------------------------------------------------------
inline void recordAttribute(uint64_t id, EventType event, const string& name, data_type type, long long value) {
    recordAttribute(id, event, name, type, std::to_string(value));
}
// ----------------------------------------------------------------------------
inline void recordAttribute(uint64_t id, EventType event, const string& name, data_type type, double value) {
    recordAttribute(id, event, name, type, std::to_string(value));
}
// ----------------------------------------------------------------------------
static void recordAttributes(uint64_t id, EventType eventType, string& prefix, const scv_extensions_if* my_exts_p) {
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
        std::array<char, 100> tmpString{};
        sprintf(tmpString.data(), "Unsupported attribute type = %d", my_exts_p->get_type());
        _scv_message::message(_scv_message::TRANSACTION_RECORDING_INTERNAL, tmpString.data());
    }
    }
}
// ----------------------------------------------------------------------------
static void generatorCb(const scv_tr_generator_base& g, scv_tr_generator_base::callback_reason reason, void* data) {
    if(reason == scv_tr_generator_base::CREATE && db.isOpen()) {
        try {
            sqlite3_bind_int64(gen_stmt, 1, g.get_id());
            sqlite3_bind_int64(gen_stmt, 2, g.get_scv_tr_stream().get_id());
            sqlite3_bind_text(gen_stmt, 3, g.get_name(), -1, SQLITE_TRANSIENT);
            db.exec(gen_stmt);
        } catch(SQLiteDB::SQLiteException& e) {
            _scv_message::message(_scv_message::TRANSACTION_RECORDING_INTERNAL, "Can't create generator entry");
        }
    }
}
// ----------------------------------------------------------------------------
static void transactionCb(const scv_tr_handle& t, scv_tr_handle::callback_reason reason, void* data) {
    if(!db.isOpen())
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
            vector<uint64_t>* levels = concurrencyLevel[streamId];
            if(levels == nullptr) {
                levels = new vector<uint64_t>();
                concurrencyLevel[id] = levels;
            }
            for(concurrencyIdx = 0; concurrencyIdx < levels->size(); ++concurrencyIdx)
                if((*levels)[concurrencyIdx] == 0)
                    break;
            if(concurrencyIdx == levels->size())
                levels->push_back(id);
            else
                (*levels)[concurrencyIdx] = id;

            sqlite3_bind_int64(tx_stmt, 1, id);
            sqlite3_bind_int64(tx_stmt, 2, t.get_scv_tr_generator_base().get_id());
            sqlite3_bind_int64(tx_stmt, 3, t.get_scv_tr_stream().get_id());
            sqlite3_bind_int64(tx_stmt, 3, concurrencyIdx);
            db.exec(tx_stmt);

            sqlite3_bind_int64(evt_stmt, 1, id);
            sqlite3_bind_int(evt_stmt, 2, BEGIN);
            sqlite3_bind_int64(evt_stmt, 3, t.get_begin_sc_time().value());
            db.exec(evt_stmt);

        } catch(SQLiteDB::SQLiteException& e) {
            _scv_message::message(_scv_message::TRANSACTION_RECORDING_INTERNAL, e.errorMessage());
        }
        my_exts_p = t.get_begin_exts_p();
        if(my_exts_p == nullptr) {
            my_exts_p = t.get_scv_tr_generator_base().get_begin_exts_p();
        }
        string tmp_str = t.get_scv_tr_generator_base().get_begin_attribute_name()
                             ? t.get_scv_tr_generator_base().get_begin_attribute_name()
                             : "";
        recordAttributes(id, BEGIN, tmp_str, my_exts_p);
    } break;
    case scv_tr_handle::END: {
        try {
            vector<uint64_t>* levels = concurrencyLevel[streamId];
            for(concurrencyIdx = 0; concurrencyIdx < levels->size(); ++concurrencyIdx)
                if((*levels)[concurrencyIdx] == id)
                    break;
            if(concurrencyIdx == levels->size())
                levels->push_back(id);
            else
                levels->at(concurrencyIdx) = id;

            sqlite3_bind_int64(evt_stmt, 1, t.get_id());
            sqlite3_bind_int(evt_stmt, 2, END);
            sqlite3_bind_int64(evt_stmt, 3, t.get_end_sc_time().value());
            db.exec(evt_stmt);

        } catch(SQLiteDB::SQLiteException& e) {
            _scv_message::message(_scv_message::TRANSACTION_RECORDING_INTERNAL, "Can't create transaction end");
        }
        my_exts_p = t.get_end_exts_p();
        if(my_exts_p == nullptr) {
            my_exts_p = t.get_scv_tr_generator_base().get_end_exts_p();
        }
        string tmp_str = t.get_scv_tr_generator_base().get_end_attribute_name()
                             ? t.get_scv_tr_generator_base().get_end_attribute_name()
                             : "";
        recordAttributes(t.get_id(), END, tmp_str, my_exts_p);
    } break;
    default:;
    }
}
// ----------------------------------------------------------------------------
static void attributeCb(const scv_tr_handle& t, const char* name, const scv_extensions_if* ext, void* data) {
    if(!db.isOpen())
        return;
    if(t.get_scv_tr_stream().get_scv_tr_db() == nullptr)
        return;
    if(t.get_scv_tr_stream().get_scv_tr_db()->get_recording() == false)
        return;
    string tmp_str(name == nullptr ? "" : name);
    recordAttributes(t.get_id(), RECORD, tmp_str, ext);
}
// ----------------------------------------------------------------------------
static void relationCb(const scv_tr_handle& tr_1, const scv_tr_handle& tr_2, void* data,
                       scv_tr_relation_handle_t relation_handle) {
    if(!db.isOpen())
        return;
    if(tr_1.get_scv_tr_stream().get_scv_tr_db() == nullptr)
        return;
    if(tr_1.get_scv_tr_stream().get_scv_tr_db()->get_recording() == false)
        return;
    try {
        sqlite3_bind_text(rel_stmt, 1, tr_1.get_scv_tr_stream().get_scv_tr_db()->get_relation_name(relation_handle), -1,
                          SQLITE_TRANSIENT);
        sqlite3_bind_int64(rel_stmt, 2, tr_1.get_id());
        sqlite3_bind_int64(rel_stmt, 3, tr_2.get_id());
        db.exec(rel_stmt);
    } catch(SQLiteDB::SQLiteException& e) {
        _scv_message::message(_scv_message::TRANSACTION_RECORDING_INTERNAL, "Can't create transaction relation");
    }
}
// ----------------------------------------------------------------------------
void scv_tr_sqlite_init() {
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
