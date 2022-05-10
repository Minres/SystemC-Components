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
#include <fmt/format.h>
#ifdef WITH_LZ4
#include <util/lz4_streambuf.h>
#endif
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
struct AttrDesc {
    EventType const evt;
    scv_extensions_if::data_type const type;
    std::string const name;
    AttrDesc(EventType evt, scv_extensions_if::data_type type, std::string const& name)
    : evt(evt), type(type), name(name){}
};
using data_type = scv_extensions_if::data_type;
// ----------------------------------------------------------------------------
namespace {
const std::array<char const*, scv_extensions_if::STRING+1> data_type_str={{
        "BOOLEAN",                      // bool
        "ENUMERATION",                  // enum
        "INTEGER",                      // char, short, int, long, long long, sc_int, sc_bigint
        "UNSIGNED",                     // unsigned { char, short, int, long, long long }, sc_uint, sc_biguint
        "FLOATING_POINT_NUMBER",        // float, double
        "BIT_VECTOR",                   // sc_bit, sc_bv
        "LOGIC_VECTOR",                 // sc_logic, sc_lv
        "FIXED_POINT_INTEGER",          // sc_fixed
        "UNSIGNED_FIXED_POINT_INTEGER", // sc_ufixed
        "RECORD",                       // struct/class
        "POINTER",                      // T*
        "ARRAY",                        // T[N]
        "STRING"                        // string, std::string

}};
class FBase {
protected:
    std::ofstream out;
    std::array<char, 1024> buffer;
    FBase(const std::string& name) : out(name) {}
    FBase() = default;
    virtual ~FBase() {
        if(out.is_open()) out.close();
    }
public:
    bool open(const std::string& name) {
        out.open(name, std::ios::binary|std::ios::trunc);
        return out.is_open();
    }
    virtual void close(){
        out.close();
    }
    bool is_open(){return out.is_open();}
};
#ifdef WITH_LZ4
class LZ4Base {
    std::ofstream ofs;
    std::unique_ptr<util::lz4c_steambuf> strbuf;
protected:
    std::ostream out;
    LZ4Base(const std::string& name)
    : ofs(name)
    , strbuf(new util::lz4c_steambuf(ofs, 8192))
    , out(strbuf.get())
    {}
    LZ4Base()
    : ofs()
    , strbuf(new util::lz4c_steambuf(ofs, 8192))
    , out(strbuf.get())
    {}
    virtual ~LZ4Base() {
        if(ofs.is_open()) ofs.close();
    }
public:
    bool open(const std::string& name) {
        ofs.open(name, std::ios::binary|std::ios::trunc);
        return ofs.is_open();
    }
    virtual void close(){
        ofs.close();
    }
    bool is_open(){return ofs.is_open();}
};
#endif

template<typename Base>
struct Database : Base {
    Database(const std::string& name)
    : Base(name) {}

    Database()
    : Base() {}

    inline void writeStream(uint64_t id, std::string const& name, std::string const& kind) {
        if(Base::is_open()) {
            auto buf = fmt::format("scv_tr_stream (ID {}, name \"{}\", kind \"{}\")\n", id, name.c_str(), kind.c_str());
            this->out.write(buf.c_str(), buf.size());
        }
    }

    inline void writeGenerator(uint64_t id, std::string const& name, uint64_t stream, std::vector<AttrDesc> const& attributes) {
        if(this->is_open()) {
            auto buf = fmt::format("scv_tr_generator (ID {}, name \"{}\", scv_tr_stream {},\n", id,
                               name.c_str(), stream);
            this->out.write(buf.c_str(), buf.size());
            auto idx=0U;
            for(auto attr: attributes){
                if(attr.evt==BEGIN){
                    auto buf = fmt::format("begin_attribute (ID {}, name \"{}\", type \"{}\")\n",
                            idx, attr.name, data_type_str[attr.type]);
                    this->out.write(buf.c_str(), buf.size());
               } else if(attr.evt==END){
                   auto buf = fmt::format("end_attribute (ID {}, name \"{}\", type \"{}\")\n",
                           idx, attr.name, data_type_str[attr.type]);
                   this->out.write(buf.c_str(), buf.size());
                }
                ++idx;
            }
            this->out.write(")\n", 2);
        }
    }

    inline void writeTransaction(uint64_t id, uint64_t generator, EventType type, uint64_t time) {
        auto buf = type==BEGIN?
                fmt::format("tx_begin {} {} {} ps\n", id, generator, time):
                fmt::format("tx_end {} {} {} ps\n", id, generator, time);
        this->out.write(buf.c_str(), buf.size());
    }

    inline void writeAttribute(uint64_t id, EventType event, const string& name, data_type type, const string& value) {
        //data_type::BOOLEAN, data_type::ENUMERATION, data_type::BIT_VECTOR, data_type::LOGIC_VECTOR, data_type::STRING
        auto buf = event == EventType::RECORD?
                fmt::format("tx_record_attribute {} \"{}\" {} = \"{}\"\n", id, name, data_type_str[type], value):
                fmt::format("a \"{}\"\n", value);
        this->out.write(buf.c_str(), buf.size());
    }

    inline void writeAttribute(uint64_t id, EventType event, const string& name, data_type type, int64_t value) {
        // data_type::INTEGER, data_type::UNSIGNED, data_type::FIXED_POINT_INTEGER, data_type::UNSIGNED_FIXED_POINT_INTEGER
        auto buf = event == EventType::RECORD?
                fmt::format("tx_record_attribute {} \"{}\" {} = {}\n", id, name, data_type_str[type], value):
                fmt::format("a {}\n", value);
        this->out.write(buf.c_str(), buf.size());
    }

    inline void writeAttribute(uint64_t id, EventType event, const string& name, data_type type, uint64_t value) {
        // data_type::INTEGER, data_type::UNSIGNED, data_type::FIXED_POINT_INTEGER, data_type::UNSIGNED_FIXED_POINT_INTEGER
        auto buf = event == EventType::RECORD?
                fmt::format("tx_record_attribute {} \"{}\" {} = {}\n", id, name, data_type_str[type], value):
                fmt::format("a {}\n", value);
        this->out.write(buf.c_str(), buf.size());
    }

    inline void writeAttribute(uint64_t id, EventType event, const string& name, data_type type, bool value) {
        // data_type::INTEGER, data_type::UNSIGNED, data_type::FIXED_POINT_INTEGER, data_type::UNSIGNED_FIXED_POINT_INTEGER
        auto buf = event == EventType::RECORD?
                fmt::format("tx_record_attribute {} \"{}\" {} = {}\n", id, name, data_type_str[type], value?"true":"false"):
                fmt::format("a {}\n", value?"true":"false");
        this->out.write(buf.c_str(), buf.size());
    }

    inline void writeAttribute(uint64_t id, EventType event, const string& name, data_type type, double value) {
        // data_type::FLOATING_POINT_NUMBER
        auto buf = event == EventType::RECORD?
                fmt::format("tx_record_attribute {} \"{}\" {} = {}\n", id, name, data_type_str[type], value):
                fmt::format("a {}\n", value);
        this->out.write(buf.c_str(), buf.size());
    }

    inline void writeRelation(const std::string& name, uint64_t sink_id, uint64_t src_id) {
        auto buf = fmt::format("tx_relation \"{}\" {} {}\n", name, sink_id, src_id);
        this->out.write(buf.c_str(), buf.size());
    }
    static Database &get() {
      static Database db;
      return db;
    }
};
#ifdef WITH_LZ4
using DB=Database<LZ4Base>;
#else
using DB=Database<FBase>;
#endif
void dbCb(const scv_tr_db& _scv_tr_db, scv_tr_db::callback_reason reason, void* data) {
    // This is called from the scv_tr_db ctor.
    static string fName("DEFAULT_scv_tr_sqlite");
    switch(reason) {
    case scv_tr_db::CREATE:
        if((_scv_tr_db.get_name() != nullptr) && (strlen(_scv_tr_db.get_name()) != 0))
            fName = _scv_tr_db.get_name();
        try {
            DB::get().open(fName);
        } catch(...) {
            _scv_message::message(_scv_message::TRANSACTION_RECORDING_INTERNAL, "Can't open recording file");
        }
        break;
    case scv_tr_db::DELETE:
        try {
            DB::get().close();
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
            DB::get().writeStream(s.get_id(), s.get_name(), s.get_stream_kind());
        } catch(std::runtime_error& e) {
            _scv_message::message(_scv_message::TRANSACTION_RECORDING_INTERNAL, "Can't create stream");
        }
    }
}
// ----------------------------------------------------------------------------
inline void recordAttribute(uint64_t id, EventType event, const string& name, data_type type, const string& value) {
    try {
        DB::get().writeAttribute(id, event, name, type, value);
    } catch(std::runtime_error& e) {
        _scv_message::message(_scv_message::TRANSACTION_RECORDING_INTERNAL, "Can't create attribute entry");
    }
}
// ----------------------------------------------------------------------------
inline std::string get_name(const char* prefix, const scv_extensions_if* my_exts_p) {
    string name;
    if(!prefix || strlen(prefix) == 0) {
        name = my_exts_p->get_name();
    } else {
        if((my_exts_p->get_name() == nullptr) || (strlen(my_exts_p->get_name()) == 0)) {
            name = prefix;
        } else {
            name = fmt::format("{}.{}", prefix,  my_exts_p->get_name());
        }
    }
    return (name == "") ? "<unnamed>" : name;
}

// ----------------------------------------------------------------------------
inline void recordAttributes(uint64_t id, EventType eventType, char const* prefix, const scv_extensions_if* my_exts_p) {
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
    }
    break;
    case scv_extensions_if::POINTER:
        if(auto ptr = my_exts_p->get_pointer()){
            std::stringstream ss; ss<<prefix<<"*";
            recordAttributes(id, eventType, ss.str().c_str(), ptr);
        }
        break;
    case scv_extensions_if::ENUMERATION:
        DB::get().writeAttribute(id, eventType, name, scv_extensions_if::ENUMERATION,
                        my_exts_p->get_enum_string((int)(my_exts_p->get_integer())));
        break;
    case scv_extensions_if::BOOLEAN:
        DB::get().writeAttribute(id, eventType, name, scv_extensions_if::BOOLEAN, my_exts_p->get_bool());
        break;
    case scv_extensions_if::INTEGER:
    case scv_extensions_if::FIXED_POINT_INTEGER:
        DB::get().writeAttribute(id, eventType, name, scv_extensions_if::INTEGER, (int64_t)my_exts_p->get_integer());
        break;
    case scv_extensions_if::UNSIGNED:
        DB::get().writeAttribute(id, eventType, name, scv_extensions_if::UNSIGNED, (uint64_t)my_exts_p->get_unsigned());
        break;
    case scv_extensions_if::STRING:
        DB::get().writeAttribute(id, eventType, name, scv_extensions_if::STRING, my_exts_p->get_string());
        break;
    case scv_extensions_if::FLOATING_POINT_NUMBER:
        DB::get().writeAttribute(id, eventType, name, scv_extensions_if::FLOATING_POINT_NUMBER, my_exts_p->get_double());
        break;
    case scv_extensions_if::BIT_VECTOR: {
        sc_bv_base tmp_bv(my_exts_p->get_bitwidth());
        my_exts_p->get_value(tmp_bv);
        DB::get().writeAttribute(id, eventType, name, scv_extensions_if::BIT_VECTOR, tmp_bv.to_string());
    } break;
    case scv_extensions_if::LOGIC_VECTOR: {
        sc_lv_base tmp_lv(my_exts_p->get_bitwidth());
        my_exts_p->get_value(tmp_lv);
        DB::get().writeAttribute(id, eventType, name, scv_extensions_if::LOGIC_VECTOR, tmp_lv.to_string());
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
    if(reason == scv_tr_generator_base::CREATE && DB::get().is_open()) {
        try {
            std::vector<AttrDesc> attrs;
            const scv_extensions_if* my_begin_exts_p = g.get_begin_exts_p();
            if(my_begin_exts_p != nullptr) {
                attrs.emplace_back(BEGIN, my_begin_exts_p->get_type(), g.get_begin_attribute_name() ? g.get_begin_attribute_name() : "");
            }
            const scv_extensions_if* my_end_exts_p = g.get_end_exts_p();
            if(my_end_exts_p != nullptr) {
                attrs.emplace_back(END, my_end_exts_p->get_type(), g.get_end_attribute_name() ? g.get_end_attribute_name() : "");
            }
            DB::get().writeGenerator(g.get_id(), g.get_name(), g.get_scv_tr_stream().get_id(), attrs);
        } catch(std::runtime_error& e) {
            _scv_message::message(_scv_message::TRANSACTION_RECORDING_INTERNAL, "Can't create generator entry");
        }
    }
}
// ----------------------------------------------------------------------------
void transactionCb(const scv_tr_handle& t, scv_tr_handle::callback_reason reason, void* data) {
    if(!DB::get().is_open())
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
        DB::get().writeTransaction(t.get_id(), t.get_scv_tr_generator_base().get_id(), BEGIN, t.get_begin_sc_time().value());
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
        DB::get().writeTransaction(t.get_id(), t.get_scv_tr_generator_base().get_id(), END, t.get_begin_sc_time().value());
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
    if(!DB::get().is_open())
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
    if(!DB::get().is_open())
        return;
    if(tr_1.get_scv_tr_stream().get_scv_tr_db() == nullptr)
        return;
    if(tr_1.get_scv_tr_stream().get_scv_tr_db()->get_recording() == false)
        return;
    try {
        DB::get().writeRelation(tr_1.get_scv_tr_stream().get_scv_tr_db()->get_relation_name(relation_handle), tr_1.get_id(),
                          tr_2.get_id());
    } catch(std::runtime_error& e) {
        _scv_message::message(_scv_message::TRANSACTION_RECORDING_INTERNAL, "Can't create transaction relation");
    }
}
} // namespace
// ----------------------------------------------------------------------------
void scv_tr_lz4_init() {
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
