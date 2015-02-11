/*******************************************************************************
 * Copyright (c) 2014, 2015 MINRES Technologies GmbH and others.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors:
 *     MINRES Technologies GmbH - initial API and implementation
 *******************************************************************************/
#include <cstdio>
#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include "scv/scv_util.h"
#include "scv/scv_introspection.h"
#include "scv/scv_tr.h"
#include "sqlite3.h"
// ----------------------------------------------------------------------------
#define SQLITEWRAPPER_ERROR 1000
// ----------------------------------------------------------------------------
using namespace std;

struct SQLiteDB {
    struct SQLiteException : public runtime_error{
        SQLiteException(const int nErrCode, const char* msg, bool doFree=true)
        : runtime_error(msg), mnErrCode(0){
            if (doFree && msg) sqlite3_free(const_cast<char*>(msg));
        }
        const int errorCode() { return mnErrCode; }
        const char* errorMessage() { return what(); }
    private:
        int mnErrCode;
    };

    SQLiteDB():busyTimeoutMs(60000), db(NULL){}

    void open(const string szFile){
        int nRet = sqlite3_open(szFile.c_str(), &db);
        if (nRet != SQLITE_OK) throw SQLiteException(nRet, sqlite3_errmsg(db), false);
        sqlite3_busy_timeout(db, busyTimeoutMs);
    }

    inline
    bool isOpen(){return db!=NULL;}

    void close(){
        if (db){
            if (sqlite3_close(db) == SQLITE_OK)
                db = 0;
            else
                throw SQLiteException(SQLITEWRAPPER_ERROR, "Unable to close database", false);
        }
    }

    inline
    int exec(const string szSQL){return exec(szSQL.c_str());};

    int exec(const char* szSQL){
        checkDB();
        char* szError=0;
        int nRet = sqlite3_exec(db, szSQL, 0, 0, &szError);
        if (nRet == SQLITE_OK)
            return sqlite3_changes(db);
        else
            throw SQLiteException(nRet, szError);
    }
protected:
    inline void checkDB(){
        if (!db) throw SQLiteException(SQLITEWRAPPER_ERROR, "Database not open", false);
    }

private:
    int busyTimeoutMs;
    sqlite3* db;
};
// ----------------------------------------------------------------------------
static SQLiteDB db;
static ostringstream stringBuilder, queryBuilder;
static vector<vector<uint64_t>*> concurrencyLevel;
// ----------------------------------------------------------------------------
enum EventType {BEGIN, RECORD, END};
typedef scv_extensions_if::data_type data_type;
// ----------------------------------------------------------------------------
#define SIM_PROPS "ScvSimProps"
#define STREAM_TABLE "ScvStream"
#define GENERATOR_TABLE "ScvGenerator"
#define TX_TABLE "ScvTx"
#define TX_EVENT_TABLE "ScvTxEvent"
#define TX_ATTRIBUTE_TABLE "ScvTxAttribute"
#define TX_RELATION_TABLE "ScvTxRelation"

static void dbCb(const scv_tr_db& _scv_tr_db, scv_tr_db::callback_reason reason, void* data) {
    // This is called from the scv_tr_db ctor.
    static string fName("DEFAULT_scv_tr_sqlite");
    switch (reason) {
    case scv_tr_db::CREATE:
        if ((_scv_tr_db.get_name() != NULL) && (strlen(_scv_tr_db.get_name()) != 0))
            fName = _scv_tr_db.get_name();
        try {
            if(fName.size()<5 || fName.find(".txdb", fName.size() - 5) == string::npos)
                fName+=".txdb";
            remove(fName.c_str());
            db.open(fName.c_str());
            // performance related according to http://blog.quibb.org/2010/08/fast-bulk-inserts-into-sqlite/
            db.exec("PRAGMA synchronous=OFF");
            db.exec("PRAGMA count_changes=OFF");
            db.exec("PRAGMA journal_mode=MEMORY");
            db.exec("PRAGMA temp_store=MEMORY");
            // scv_out << "TB Transaction Recording has started, file = " << my_sqlite_file_name << endl;
            db.exec("CREATE TABLE  IF NOT EXISTS " STREAM_TABLE "(id INTEGER  NOT NULL PRIMARY KEY, name TEXT, kind TEXT);");
            db.exec("CREATE TABLE  IF NOT EXISTS " GENERATOR_TABLE "(id INTEGER  NOT NULL PRIMARY KEY, stream INTEGER REFERENCES " STREAM_TABLE "(id), name TEXT, begin_attr INTEGER, end_attr INTEGER);");
            db.exec("CREATE TABLE  IF NOT EXISTS " TX_TABLE "(id INTEGER  NOT NULL PRIMARY KEY, generator INTEGER REFERENCES " GENERATOR_TABLE "(id), stream INTEGER REFERENCES " STREAM_TABLE "(id), concurrencyLevel INTEGER);");
            db.exec("CREATE TABLE  IF NOT EXISTS " TX_EVENT_TABLE "(tx INTEGER REFERENCES " TX_TABLE "(id), type INTEGER, time INTEGER);");
            db.exec("CREATE TABLE  IF NOT EXISTS " TX_ATTRIBUTE_TABLE "(tx INTEGER REFERENCES " TX_TABLE "(id), type INTEGER, name TEXT, data_type INTEGER, data_value TEXT);");
            db.exec("CREATE TABLE  IF NOT EXISTS " TX_RELATION_TABLE "(name TEXT, src INTEGER REFERENCES " TX_TABLE "(id), sink INTEGER REFERENCES " TX_TABLE "(id));");
            db.exec("CREATE TABLE  IF NOT EXISTS " SIM_PROPS "(time_resolution INTEGER);");
            db.exec("BEGIN TRANSACTION");
            queryBuilder.str("");
            queryBuilder << "INSERT INTO " SIM_PROPS " (time_resolution) values ("
                    << (long)(sc_get_time_resolution().to_seconds()*1e15) << ");";
            db.exec(queryBuilder.str().c_str());
        } catch (SQLiteDB::SQLiteException& e) {
            _scv_message::message(_scv_message::TRANSACTION_RECORDING_INTERNAL, "Can't open recording file");
        }
        break;
    case scv_tr_db::DELETE:
        try {
            // scv_out << "Transaction Recording is closing file: " << my_sqlite_file_name << endl;
            db.exec("COMMIT TRANSACTION");
            db.close();
        } catch (SQLiteDB::SQLiteException& e) {
            _scv_message::message(_scv_message::TRANSACTION_RECORDING_INTERNAL, "Can't close recording file");
        }
        break;
    default:
        _scv_message::message(_scv_message::TRANSACTION_RECORDING_INTERNAL, "Unknown reason in scv_tr_db callback");
    }
}
// ----------------------------------------------------------------------------
static void streamCb(const scv_tr_stream& s, scv_tr_stream::callback_reason reason, void* data) {
    if (reason == scv_tr_stream::CREATE && db.isOpen()) {
        try {
            queryBuilder.str("");
            queryBuilder << "INSERT INTO " STREAM_TABLE " (id, name, kind) values (" << s.get_id() << ",'" << s.get_name() << "','"
                    << (s.get_stream_kind() ? s.get_stream_kind() : "<unnamed>") << "');";
            db.exec(queryBuilder.str().c_str());
            if(concurrencyLevel.size()<=s.get_id())concurrencyLevel.resize(s.get_id()+1);
            concurrencyLevel[s.get_id()]=new vector<uint64_t>();
        } catch (SQLiteDB::SQLiteException& e) {
            _scv_message::message(_scv_message::TRANSACTION_RECORDING_INTERNAL, "Can't create stream");
        }
    }
}
// ----------------------------------------------------------------------------
void recordAttribute(uint64_t id, EventType event, const string& name, data_type type, const string& value) {
    try {
        queryBuilder.str("");
        queryBuilder << "INSERT INTO " TX_ATTRIBUTE_TABLE " (tx,type,name,data_type,data_value)" << " values ("
                << id << "," << event << ",'"<< name << "'," << type << ",'" << value << "');";
        db.exec(queryBuilder.str().c_str());
    } catch (SQLiteDB::SQLiteException& e) {
        _scv_message::message(_scv_message::TRANSACTION_RECORDING_INTERNAL, "Can't create attribute entry");
    }
}
// ----------------------------------------------------------------------------
inline
void recordAttribute(uint64_t id, EventType event, const string& name, data_type type, long long value) {
    stringBuilder.str("");
    stringBuilder <<value;
    recordAttribute(id,event,name,type,stringBuilder.str());
}
// ----------------------------------------------------------------------------
inline
void recordAttribute(uint64_t id, EventType event, const string& name, data_type type, double value) {
    stringBuilder.str("");
    stringBuilder <<value;
    recordAttribute(id,event,name,type,stringBuilder.str());
}
// ----------------------------------------------------------------------------
static void recordAttributes(uint64_t id, EventType eventType, string& prefix, const scv_extensions_if* my_exts_p){
    if (my_exts_p == 0) return;
    string name;
    if (prefix == "") {
        name = my_exts_p->get_name();
    } else {
        if ((my_exts_p->get_name() == 0) || (strlen(my_exts_p->get_name()) == 0)) {
            name = prefix;
        } else {
            name = prefix + "." + my_exts_p->get_name();
        }
    }
    if (name == "") name = "<unnamed>";
    switch (my_exts_p->get_type()) {
    case scv_extensions_if::RECORD:
        {
            int num_fields = my_exts_p->get_num_fields();
            if (num_fields > 0) {
                for (int field_counter = 0; field_counter < num_fields; field_counter++) {
                    const scv_extensions_if* field_data_p = my_exts_p->get_field(field_counter);
                    recordAttributes(id, eventType, prefix, field_data_p);
                }
            }
        }
        break;
    case scv_extensions_if::ENUMERATION:
        recordAttribute(id, eventType, name, scv_extensions_if::ENUMERATION, my_exts_p->get_enum_string((int) (my_exts_p->get_integer())));
        break;
    case scv_extensions_if::BOOLEAN:
        recordAttribute(id, eventType, name, scv_extensions_if::BOOLEAN, my_exts_p->get_bool()?"TRUE":"FALSE");
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
    case scv_extensions_if::BIT_VECTOR:
        {
            sc_bv_base tmp_bv(my_exts_p->get_bitwidth());
            my_exts_p->get_value(tmp_bv);
            recordAttribute(id, eventType, name, scv_extensions_if::BIT_VECTOR, tmp_bv.to_string());
        }
        break;
    case scv_extensions_if::LOGIC_VECTOR:
        {
            sc_lv_base tmp_lv(my_exts_p->get_bitwidth());
            my_exts_p->get_value(tmp_lv);
            recordAttribute(id, eventType, name, scv_extensions_if::LOGIC_VECTOR, tmp_lv.to_string());
        }
        break;
    case scv_extensions_if::ARRAY:
        for (int array_elt_index = 0; array_elt_index < my_exts_p->get_array_size(); array_elt_index++) {
            const scv_extensions_if* field_data_p = my_exts_p->get_array_elt(array_elt_index);
            recordAttributes(id, eventType, prefix, field_data_p);
        }
        break;
    default: {
        char tmpString[100];
        sprintf(tmpString, "Unsupported attribute type = %d", my_exts_p->get_type());
        _scv_message::message(_scv_message::TRANSACTION_RECORDING_INTERNAL, tmpString);
    }
    }
}
// ----------------------------------------------------------------------------
static void generatorCb(const scv_tr_generator_base& g, scv_tr_generator_base::callback_reason reason, void* data) {
    if (reason == scv_tr_generator_base::CREATE && db.isOpen()) {
        try {
            queryBuilder.str("");
            queryBuilder << "INSERT INTO " GENERATOR_TABLE " (id,stream, name)"
                    <<" values (" << g.get_id() << ","<<g.get_scv_tr_stream().get_id()<<",'" << g.get_name() << "');";
            db.exec(queryBuilder.str().c_str());
        } catch (SQLiteDB::SQLiteException& e) {
            _scv_message::message(_scv_message::TRANSACTION_RECORDING_INTERNAL, "Can't create generator entry");
        }
    }
}
// ----------------------------------------------------------------------------
static void transactionCb(const scv_tr_handle& t, scv_tr_handle::callback_reason reason, void* data) {
    if (!db.isOpen())  return;
    if (t.get_scv_tr_stream().get_scv_tr_db() == NULL) return;
    if (t.get_scv_tr_stream().get_scv_tr_db()->get_recording() == false) return;

    uint64_t id = t.get_id();
    uint64_t streamId = t.get_scv_tr_stream().get_id();
    vector<uint64_t>::size_type concurrencyIdx;
    const scv_extensions_if* my_exts_p;
    switch (reason) {
    case scv_tr_handle::BEGIN:{
        try {
            if(concurrencyLevel.size()<=streamId)
                concurrencyLevel.resize(streamId+1);
            vector<uint64_t>* levels=concurrencyLevel[streamId];
            if(levels==NULL){
                levels=new vector<uint64_t>();
                concurrencyLevel[id]=levels;
            }
            for(concurrencyIdx=0; concurrencyIdx<levels->size(); ++concurrencyIdx)
                if((*levels)[concurrencyIdx]==0) break;
            if(concurrencyIdx==levels->size())
                levels->push_back(id);
            else
                (*levels)[concurrencyIdx]=id;
            queryBuilder.str("");
            queryBuilder << "INSERT INTO " TX_TABLE " (id,generator,stream, concurrencyLevel)" << " values (" << id << "," << t.get_scv_tr_generator_base().get_id()
                         <<","<<t.get_scv_tr_stream().get_id()<< ","<< concurrencyIdx <<");";
            db.exec(queryBuilder.str().c_str());

            queryBuilder.str("");
            queryBuilder << "INSERT INTO " TX_EVENT_TABLE " (tx,type,time)" << " values (" << id << "," << BEGIN << ","
                    << t.get_begin_sc_time().value() << ");";
            db.exec(queryBuilder.str().c_str());

        } catch (SQLiteDB::SQLiteException& e) {
            _scv_message::message(_scv_message::TRANSACTION_RECORDING_INTERNAL, e.errorMessage());
        }
        my_exts_p = t.get_begin_exts_p();
        if (my_exts_p == NULL) {
            my_exts_p = t.get_scv_tr_generator_base().get_begin_exts_p();
        }
        string tmp_str =
                t.get_scv_tr_generator_base().get_begin_attribute_name() ? t.get_scv_tr_generator_base().get_begin_attribute_name() : "";
        recordAttributes(id, BEGIN, tmp_str, my_exts_p);
    } break;
    case scv_tr_handle::END:{
        try {
            vector<uint64_t>* levels=concurrencyLevel[streamId];
            for(concurrencyIdx=0; concurrencyIdx<levels->size(); ++concurrencyIdx)
                if((*levels)[concurrencyIdx]==id) break;
            if(concurrencyIdx==levels->size())
                levels->push_back(id);
            else
                levels->at(concurrencyIdx)=id;
            queryBuilder.str("");
            queryBuilder << "INSERT INTO " TX_EVENT_TABLE " (tx,type,time)" << " values (" << t.get_id() << "," << END << ","
                    << t.get_end_sc_time().value() << ");";
            db.exec(queryBuilder.str().c_str());

        } catch (SQLiteDB::SQLiteException& e) {
            _scv_message::message(_scv_message::TRANSACTION_RECORDING_INTERNAL, "Can't create transaction end");
        }
        my_exts_p = t.get_end_exts_p();
        if (my_exts_p == NULL) {
            my_exts_p = t.get_scv_tr_generator_base().get_end_exts_p();
        }
        string tmp_str =
                t.get_scv_tr_generator_base().get_end_attribute_name() ? t.get_scv_tr_generator_base().get_end_attribute_name() : "";
        recordAttributes(t.get_id(), END, tmp_str, my_exts_p);
    } break;
    default:
        ;
    }
}
// ----------------------------------------------------------------------------
static void attributeCb(const scv_tr_handle& t, const char* name, const scv_extensions_if* ext, void* data) {
    if (!db.isOpen()) return;
    if (t.get_scv_tr_stream().get_scv_tr_db() == NULL) return;
    if (t.get_scv_tr_stream().get_scv_tr_db()->get_recording() == false) return;
    string tmp_str(name == 0?"":name);
    recordAttributes(t.get_id(), RECORD, tmp_str, ext);
}
// ----------------------------------------------------------------------------
static void relationCb(const scv_tr_handle& tr_1, const scv_tr_handle& tr_2, void* data,
        scv_tr_relation_handle_t relation_handle) {
    if (!db.isOpen()) return;
    if (tr_1.get_scv_tr_stream().get_scv_tr_db() == NULL) return;
    if (tr_1.get_scv_tr_stream().get_scv_tr_db()->get_recording() == false) return;
    try {
        queryBuilder.str("");
        queryBuilder << "INSERT INTO " TX_RELATION_TABLE " (name,sink,src)"
                << "values ('" << tr_1.get_scv_tr_stream().get_scv_tr_db()->get_relation_name(relation_handle) << "',"
                << tr_1.get_id() << ","<< tr_2.get_id() << ");";
        db.exec(queryBuilder.str().c_str());

    } catch (SQLiteDB::SQLiteException& e) {
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
