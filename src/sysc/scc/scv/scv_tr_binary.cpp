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
/**
 * Record structure:
 * timing file
 * ===========
 * 20kB chunks, containing records of fixed length 20bytes, little endian encoded:
 * 32bit type, 64bit time, 64bit file offset
 *     types: 00 - fill
 *            01 - signal change
 *            02 - transaction start
 *            03 - zero length transaction (start & end)
 *            04 - transaction end
 *
 * data file
 * ===========
 * 1MB chunks containing records of variable length, little endian encoded
 * record type(4)
 *     0 - fill:        0x0( until end of chunk).
 *     1 - transaction: id(8), generator(8),concurrencyLevel(4)
 *     2 - attribute:   tx_id(8),type(2),name(8),data_type(2),data_value(8)
 *     3 - ext. attr.:  tx_id(8),type(2),name(8),data_type(2),data_value(8),data_value(4)
 *     4 - relation:    name(8), src id(8), tgt id(8)
 *
 * control file
 * ==============
 * record type(4)
 *     0 - fill:        0x0(...).
 *     1 - string map:  id(8), length(4), value(...)
 *     2 - stream:      id(8), name(8), kind(8)
 *     3 - generator:   id(8), name(8), stream_id(8)
 *
 *
 */
// ----------------------------------------------------------------------------
#ifdef _MSC_VER
#define scv_tr_TEXT_LLU "%I64u"
#define scv_tr_TEXT_LLX "%I64x"
#else
#define scv_tr_TEXT_LLU "%llu"
#define scv_tr_TEXT_LLX "%llx"
#endif

// ----------------------------------------------------------------------------
enum EventType { BEGIN, RECORD, END };
using data_type = scv_extensions_if::data_type;
// ----------------------------------------------------------------------------
namespace {

struct ByteBufferWriter {
    ByteBufferWriter(size_t reserve = 32) { buf.reserve(reserve); }
    template <typename T> ByteBufferWriter& append(const T& v) {
        const auto* ptr = reinterpret_cast<const unsigned char*>(&v);
        for(size_t i = 0; i < sizeof(T); ++i, ++ptr)
            buf.push_back(*ptr);
        return *this;
    }

    size_t length() { return buf.size(); }

    unsigned char* operator()() { return buf.data(); }

    void write(int file_des) {
        ssize_t written = ::write(file_des, buf.data(), buf.size());
        if(written != buf.size())
            throw std::runtime_error("not written"); // TODO: implement error handling
    }

private:
    std::vector<unsigned char> buf;
};

template <> ByteBufferWriter& ByteBufferWriter::append<std::string>(const std::string& v) {
    auto it = buf.end();
    buf.resize(buf.size() + v.length());
    std::copy(v.begin(), v.end(), it);
    return *this;
}

const int open_flags{O_WRONLY | O_CREAT | O_TRUNC};
const auto open_mode{00644};

struct ControlBuffer {
    ControlBuffer(const boost::filesystem::path& name) { file_des = open(name.string().c_str(), open_flags, open_mode); }

    ~ControlBuffer() { close(file_des); }

    uint64_t getIdOf(const std::string& str) {
        auto strid = std::hash<std::string>{}(str);
        if(lookup.find(strid) == lookup.end()) {
            ByteBufferWriter bw(sizeof(uint32_t) + sizeof(strid) + sizeof(uint32_t) + str.length());
            bw.append<uint32_t>(1U).append(strid).append<uint32_t>(str.length()).append(str).write(file_des);
            lookup.insert(strid);
        }
        return strid;
    }

    void writeStream(uint64_t id, std::string& name, std::string& kind) {
        ByteBufferWriter bw;
        bw.append<uint32_t>(2U).append(id).append(getIdOf(name)).append(getIdOf(kind)).write(file_des);
    }

    void writeGenerator(uint64_t id, std::string& name, uint64_t stream) {
        ByteBufferWriter bw;
        bw.append<uint32_t>(3U).append(id).append(getIdOf(name)).append(stream).write(file_des);
    }

private:
    int file_des = 0;
    std::unordered_set<uint64_t> lookup;
};

class DataBuffer {
public:
    DataBuffer(const boost::filesystem::path& name) { file_des = open(name.string().c_str(), open_flags, open_mode); }

    ~DataBuffer() {
        if(bufTail > 0) {
            std::fill(buf.data() + bufTail, buf.data() + buf.size(), 0);
            write(file_des, buf.data(), buf.size());
        }
        close(file_des);
    }

    uint64_t writeTx(uint64_t id, uint64_t generator, uint64_t concurrencyLevel) {
        // type(4)=1, id(8), generator(8),concurrencyLevel(4)
        ByteBufferWriter bw;
        bw.append<uint32_t>(1U).append(id).append(generator).append(concurrencyLevel);
        return append(bw(), bw.length());
    }

    void writeAttribute(uint64_t id, EventType event, uint64_t name, data_type typ, uint64_t value) {
        // type(4)=2, tx_id(8),type(2),name(8),data_type(2),data_value(8)
        ByteBufferWriter bw;
        bw.append<uint32_t>(2U).append(id).append(event).append(name).append(static_cast<uint16_t>(typ)).append(value);
        append(bw(), bw.length());
    }

    void writeAttribute(uint64_t id, EventType event, uint64_t name, data_type typ, uint64_t value0, uint32_t value1) {
        // type(4)=3, tx_id(8),type(2),name(8),data_type(2),data_value(8)
        ByteBufferWriter bw;
        bw.append<uint32_t>(3U).append(id).append(event).append(name).append(static_cast<uint16_t>(typ)).append(value0).append(value1);
        append(bw(), bw.length());
    }

    void writeRelation(uint64_t name, uint64_t src, uint64_t sink) {
        // type(4)=4, id(8), src(8), tgt(8)
        ByteBufferWriter bw;
        bw.append<uint32_t>(4U).append(name).append(src).append(sink);
        append(bw(), bw.length());
    }
    /**
     * returns the offset of the record
     */
    template <typename T> uint64_t append(const T& val) { return append(reinterpret_cast<const unsigned char*>(&val), sizeof(T)); }

    uint64_t getActualFilePos() { return blockCount * buf.size() + bufTail; }

private:
    uint64_t append(const unsigned char* p, size_t len) {
        if((bufTail + len) > buf.size()) {
            std::fill(buf.data() + bufTail, buf.data() + buf.size(), 0);
            auto written = write(file_des, buf.data(), buf.size());
            if(written != buf.size())
                throw std::runtime_error("not written"); // TODO: implement error handling
            blockCount++;
            bufTail = 0;
        }
        uint64_t ret = blockCount * buf.size() + bufTail;
        std::copy(p, p + len, buf.data() + bufTail);
        bufTail += len;
        return ret;
    }

    int file_des = 0;
    size_t blockCount = 0;
    size_t bufTail = 0;
    std::array<unsigned char, 1024 * 1024> buf;
};

struct TimingBuffer {
    TimingBuffer(const boost::filesystem::path& name) { file_des = open(name.string().c_str(), open_flags, open_mode); }

    ~TimingBuffer() {
        if(bufTail > 0) {
            std::fill(buf.data() + bufTail, buf.data() + buf.size(), 0);
            write(file_des, buf.data(), buf.size());
        }
        close(file_des);
    }

    void append(uint32_t type, uint64_t time, uint64_t file_offset) {
        const size_t len = sizeof(type) + sizeof(time) + sizeof(file_offset);
        if((bufTail + len) > buf.size()) {
            std::fill(buf.data() + bufTail, buf.data() + buf.size(), 0);
            ssize_t written = write(file_des, buf.data(), buf.size());
            if(written != buf.size())
                throw std::runtime_error("not written"); // TODO: implement error handling
            bufTail = 0;
        }
        ByteBufferWriter bw(len);
        bw.append(type).append(time).append(file_offset);
        std::copy(bw(), bw() + bw.length(), buf.data() + bufTail);
        bufTail += len;
    }

private:
    int file_des = 0;
    size_t bufTail = 0;
    std::array<unsigned char, 20 * 1024> buf;
};

class Base {
protected:
    boost::filesystem::path dir;
    Base(const std::string& name)
    : dir(name.c_str()) {
        if(boost::filesystem::exists(dir))
            boost::filesystem::remove_all(dir);
        boost::filesystem::create_directory(dir);
    }
};

struct Database : Base {
    ControlBuffer c;
    DataBuffer d;
    TimingBuffer t;

    Database(const std::string& name)
    : Base(name)
    , c(dir / "c")
    , d(dir / "d")
    , t(dir / "t") {}

    inline uint64_t getIdOf(const std::string& str) { return c.getIdOf(str); }

    inline void writeStream(uint64_t id, std::string name, std::string kind) { c.writeStream(id, name, kind); }

    inline void writeGenerator(uint64_t id, std::string name, uint64_t stream) { c.writeGenerator(id, name, stream); }

    inline uint64_t writeTransaction(uint64_t id, uint64_t generator, uint64_t concurrencyLevel) {
        return d.writeTx(id, generator, concurrencyLevel);
    }

    inline void writeTxTimepoint(uint64_t id, int type, uint64_t time, uint64_t file_offset) { t.append(type, time, file_offset); }

    inline void writeAttribute(uint64_t id, EventType event, const string& name, data_type type, const string& value) {
        d.writeAttribute(id, event, c.getIdOf(name), type, c.getIdOf(value));
    }

    inline void writeAttribute(uint64_t id, EventType event, const string& name, data_type type, uint64_t value) {
        d.writeAttribute(id, event, c.getIdOf(name), type, value);
    }

    inline void writeAttribute(uint64_t id, EventType event, const string& name, data_type type, double value) {
        //        int exponent;
        //        const double mantissa = frexp(value, &exponent);
    }

    inline void writeRelation(const std::string& name, uint64_t sink_id, uint64_t src_id) {
        d.writeRelation(c.getIdOf(name), src_id, sink_id);
    }
};

vector<vector<uint64_t>*> concurrencyLevel;
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
void recordAttribute(uint64_t id, EventType event, const string& name, data_type type, const string& value) {
    try {
        db->writeAttribute(id, event, name, type, value);
    } catch(std::runtime_error& e) {
        _scv_message::message(_scv_message::TRANSACTION_RECORDING_INTERNAL, "Can't create attribute entry");
    }
}
// ----------------------------------------------------------------------------
void recordAttribute(uint64_t id, EventType event, const string& name, data_type type, long long value) {
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
        recordAttribute(id, eventType, name, scv_extensions_if::ENUMERATION, my_exts_p->get_enum_string((int)(my_exts_p->get_integer())));
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
    uint64_t streamId = t.get_scv_tr_stream().get_id();
    vector<uint64_t>::size_type concurrencyIdx;
    const scv_extensions_if* my_exts_p;
    switch(reason) {
    case scv_tr_handle::BEGIN: {
        try {
            if(concurrencyLevel.size() <= streamId)
                concurrencyLevel.resize(streamId + 1);
            vector<uint64_t>* levels = concurrencyLevel.at(streamId);
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
            auto offset = db->writeTransaction(id, t.get_scv_tr_generator_base().get_id(), concurrencyIdx);
            db->writeTxTimepoint(id, BEGIN, t.get_begin_sc_time().value(), offset);
            id2offset[id] = offset;
        } catch(std::runtime_error& e) {
            _scv_message::message(_scv_message::TRANSACTION_RECORDING_INTERNAL, e.what());
        }
        my_exts_p = t.get_begin_exts_p();
        if(my_exts_p == nullptr)
            my_exts_p = t.get_scv_tr_generator_base().get_begin_exts_p();
        if(my_exts_p) {
            string tmp_str =
                t.get_scv_tr_generator_base().get_begin_attribute_name() ? t.get_scv_tr_generator_base().get_begin_attribute_name() : "";
            recordAttributes(id, BEGIN, tmp_str, my_exts_p);
        }
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
                (*levels)[concurrencyIdx] = id;
            db->writeTxTimepoint(id, END, t.get_begin_sc_time().value(), id2offset[id]);
            id2offset.erase(id);
        } catch(std::runtime_error& e) {
            _scv_message::message(_scv_message::TRANSACTION_RECORDING_INTERNAL, "Can't create transaction end");
        }
        my_exts_p = t.get_end_exts_p();
        if(my_exts_p == nullptr)
            my_exts_p = t.get_scv_tr_generator_base().get_end_exts_p();
        if(my_exts_p) {
            string tmp_str =
                t.get_scv_tr_generator_base().get_end_attribute_name() ? t.get_scv_tr_generator_base().get_end_attribute_name() : "";
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
    string tmp_str(name == nullptr ? "" : name);
    recordAttributes(t.get_id(), RECORD, tmp_str, ext);
}
// ----------------------------------------------------------------------------
void relationCb(const scv_tr_handle& tr_1, const scv_tr_handle& tr_2, void* data, scv_tr_relation_handle_t relation_handle) {
    if(!db)
        return;
    if(tr_1.get_scv_tr_stream().get_scv_tr_db() == nullptr)
        return;
    if(tr_1.get_scv_tr_stream().get_scv_tr_db()->get_recording() == false)
        return;
    try {
        db->writeRelation(tr_1.get_scv_tr_stream().get_scv_tr_db()->get_relation_name(relation_handle), tr_1.get_id(), tr_2.get_id());
    } catch(std::runtime_error& e) {
        _scv_message::message(_scv_message::TRANSACTION_RECORDING_INTERNAL, "Can't create transaction relation");
    }
}
} // namespace
// ----------------------------------------------------------------------------
void scv_tr_binary_init() {
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
