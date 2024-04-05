/*******************************************************************************
 * Copyright 2017-2022 MINRES Technologies GmbH
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

#include "configurer.h"
#include "rapidjson/document.h"
#include "rapidjson/error/en.h"
#include "report.h"
#include <cstring>
#include <fmt/format.h>
#include <fstream>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/ostreamwrapper.h>
#include <rapidjson/prettywriter.h>
#include <unordered_map>

namespace scc {
using namespace rapidjson;
using writer_type = PrettyWriter<OStreamWrapper>;
namespace {
inline auto get_sc_objects(sc_core::sc_object* obj = nullptr) -> const std::vector<sc_core::sc_object*>& {
    if(obj)
        return obj->get_child_objects();
    else
        return sc_core::sc_get_top_level_objects();
}

#define FDECL(TYPE, FUNC)                                                                                                                  \
    inline void writeValue(writer_type& writer, std::string const& key, TYPE value) {                                                      \
        writer.Key(key.c_str());                                                                                                           \
        writer.FUNC(value);                                                                                                                \
    }
FDECL(int, Int)
FDECL(unsigned int, Uint)
FDECL(long, Int64)
FDECL(unsigned long, Uint64)
FDECL(long long, Int64)
FDECL(unsigned long long, Uint64)
FDECL(bool, Bool)
FDECL(float, Double)
FDECL(double, Double)
FDECL(char const*, String)
inline void writeValue(writer_type& writer, std::string const& key, std::string const& value) {
    writer.Key(key.c_str());
    writer.String(value.c_str());
}

template <typename T> auto check_n_assign(writer_type& writer, sc_core::sc_attr_base* attr_base) -> bool {
    auto* a = dynamic_cast<sc_core::sc_attribute<T>*>(attr_base);
    if(a != nullptr) {
        writeValue(writer, a->name(), a->value);
        return true;
    }
    return false;
}

template <> auto check_n_assign<sc_core::sc_time>(writer_type& writer, sc_core::sc_attr_base* attr_base) -> bool {
    auto* a = dynamic_cast<sc_core::sc_attribute<sc_core::sc_time>*>(attr_base);
    if(a != nullptr) {
        writeValue(writer, a->name(), a->value.to_double());
        return true;
    }
    return false;
}

inline bool start_object(writer_type& writer, char const* key, bool started) {
    if(!started) {
        writer.Key(key);
        writer.StartObject();
    }
    return true;
}

struct config_dumper {
    config_dumper() {}

    void dump_config(sc_core::sc_object* obj, writer_type& writer) {
        auto start = std::string(obj->basename()).substr(0, 3);
        if(start == "$$$")
            return;
        auto obj_started = false;
        for(sc_core::sc_attr_base* attr_base : obj->attr_cltn()) {
            obj_started = start_object(writer, obj->basename(), obj_started);
            check_n_assign<int>(writer, attr_base) || check_n_assign<unsigned>(writer, attr_base) ||
                check_n_assign<long>(writer, attr_base) || check_n_assign<unsigned long>(writer, attr_base) ||
                check_n_assign<long long>(writer, attr_base) || check_n_assign<unsigned long long>(writer, attr_base) ||
                check_n_assign<bool>(writer, attr_base) || check_n_assign<float>(writer, attr_base) ||
                check_n_assign<double>(writer, attr_base) || check_n_assign<std::string>(writer, attr_base) ||
                check_n_assign<char*>(writer, attr_base) || check_n_assign<sc_core::sc_time>(writer, attr_base);
        }
        for(auto* o : get_sc_objects(obj)) {
            obj_started = start_object(writer, obj->basename(), obj_started);
            dump_config(o, writer);
        }
        if(obj_started)
            writer.EndObject();
    }
};

#define CHECK_N_ASSIGN(TYPE, ATTR, VAL)                                                                                                    \
    {                                                                                                                                      \
        auto* a = dynamic_cast<sc_core::sc_attribute<TYPE>*>(ATTR);                                                                        \
        if(a != nullptr) {                                                                                                                 \
            a->value = VAL;                                                                                                                \
            return;                                                                                                                        \
        }                                                                                                                                  \
    }

void try_set_value(sc_core::sc_attr_base* attr_base, Value const& hier_val) {
    CHECK_N_ASSIGN(int, attr_base, hier_val.Get<int>());
    CHECK_N_ASSIGN(unsigned, attr_base, hier_val.Get<unsigned>());
    CHECK_N_ASSIGN(int64_t, attr_base, hier_val.Get<int64_t>());
    CHECK_N_ASSIGN(uint64_t, attr_base, hier_val.Get<uint64_t>());
    CHECK_N_ASSIGN(bool, attr_base, hier_val.Get<bool>());
    CHECK_N_ASSIGN(float, attr_base, hier_val.Get<float>());
    CHECK_N_ASSIGN(double, attr_base, hier_val.Get<double>());
    CHECK_N_ASSIGN(std::string, attr_base, std::string(hier_val.GetString()));
    CHECK_N_ASSIGN(char*, attr_base, strdup(hier_val.GetString()));
    {
        auto* a = dynamic_cast<sc_core::sc_attribute<sc_core::sc_time>*>(attr_base);
        if(a != nullptr) {
            a->value = sc_core::sc_time(hier_val.Get<double>(), sc_core::SC_SEC);
            return;
        }
    }
}

void configure_sc_attribute_hierarchical(Value const& node, std::string const& prefix) {
    for(auto itr = node.MemberBegin(); itr != node.MemberEnd(); ++itr) {
        if(!itr->name.IsString())
            return;
        auto key_name = itr->name.GetString();
        if(strncmp(key_name, "log_level", 9) == 0)
            continue; // virtual attribute
        auto hier_name = prefix.size() ? prefix + "." + key_name : key_name;
        Value const& val = itr->value;
        if(val.IsNull() || val.IsArray())
            continue;
        else if(val.IsObject()) {
            if(sc_core::sc_find_object(hier_name.c_str())) {
                configure_sc_attribute_hierarchical(val, hier_name);
            }
        } else {
            auto pos = hier_name.rfind('.');
            if(pos != std::string::npos) {
                auto objname = hier_name.substr(0, pos);
                auto attrname = hier_name.substr(pos + 1);
                if(auto* obj = sc_core::sc_find_object(objname.c_str()))
                    if(auto attr = obj->get_attribute(attrname.c_str()))
                        try_set_value(attr, val);
            }
        }
    }
}

void check_config_hierarchical(Value const& node, std::string const& prefix) {
    for(auto itr = node.MemberBegin(); itr != node.MemberEnd(); ++itr) {
        if(!itr->name.IsString())
            return;
        auto key_name = itr->name.GetString();
        if(strncmp(key_name, SCC_LOG_LEVEL_PARAM_NAME, 9) == 0)
            continue; // virtual attribute
        auto hier_name = prefix.size() ? prefix + "." + key_name : key_name;
        Value const& val = itr->value;
        if(val.IsNull() || val.IsArray())
            continue;
        else if(val.IsObject()) {
            if(!sc_core::sc_find_object(hier_name.c_str())) {
                if(prefix.size())
                    throw std::domain_error(hier_name);
            } else
                check_config_hierarchical(val, hier_name);
        } else {
            auto pos = hier_name.rfind('.');
            if(pos != std::string::npos) {
                auto objname = hier_name.substr(0, pos);
                auto attrname = hier_name.substr(pos + 1);
                auto* obj = sc_core::sc_find_object(objname.c_str());
                if(!obj || !obj->get_attribute(attrname.c_str())) {
                    throw std::invalid_argument(hier_name);
                }
            }
        }
    }
}
} // namespace

struct configurer::ConfigHolder {
    Document document;
};

configurer::configurer(const std::string& filename, unsigned config_phases)
: configurer(filename, config_phases, "$$$configurer$$$") {}
configurer::configurer(const std::string& filename, unsigned config_phases, sc_core::sc_module_name nm)
: base_type(nm)
, config_phases(config_phases)
, cci_broker(nullptr)
, root(new ConfigHolder) {
    if(filename.length() > 0)
        read_input_file(filename);
}

configurer::~configurer() {}

void configurer::read_input_file(const std::string& filename) {
    std::ifstream is(filename);
    if(is.is_open()) {
        try {
            IStreamWrapper stream(is);
            root->document.ParseStream(stream);
            if(root->document.HasParseError()) {
                SCCERR() << "Could not parse input file " << filename << ", location " << (unsigned)root->document.GetErrorOffset()
                         << ", reason: " << GetParseError_En(root->document.GetParseError());
            } else {
                config_valid = true;
            }
        } catch(std::runtime_error& e) {
            SCCERR() << "Could not parse input file " << filename << ", reason: " << e.what();
        }
    } else {
        SCCERR() << "Could not open input file " << filename;
    }
}

void configurer::dump_configuration(std::ostream& os, bool as_yaml, bool with_description, sc_core::sc_object* obj) {
    OStreamWrapper stream(os);
    writer_type writer(stream);
    writer.StartObject();
    config_dumper dumper;
    for(auto* o : get_sc_objects(obj)) {
        dumper.dump_config(o, writer);
    }
    writer.EndObject();
}

void configurer::configure() {
    if(config_valid && root) {
        configure_sc_attribute_hierarchical(root->document, "");
    }
}

auto get_value_from_hierarchy(const std::string& hier_name, Value const& value) -> Value const& {
    size_t npos = hier_name.find_first_of('.');
    auto member = value.FindMember(hier_name.substr(0, npos).c_str());
    auto& val = member->value;
    if(val.IsNull() || npos == std::string::npos || !val.IsObject())
        return val;
    return get_value_from_hierarchy(hier_name.substr(npos + 1, hier_name.size()), val);
}

void configurer::set_configuration_value(sc_core::sc_attr_base* attr_base, sc_core::sc_object* owner) {
    if(root) {
        std::string name(owner->name());
        name += ".";
        name += attr_base->name();
        size_t npos = name.find_first_of('.');
        auto member = root->document.FindMember(name.substr(0, npos).c_str());
        auto& val = get_value_from_hierarchy(name, member->value);
        if(!val.IsNull())
            try_set_value(attr_base, val);
    }
}
void configurer::config_check() {
    try {
        if(root) {
            check_config_hierarchical(root->document, "");
        }
    } catch(std::domain_error& e) {
        SCCFATAL("scc::configurer") << "Illegal hierarchy name: '" << e.what() << "'";
    } catch(std::invalid_argument& e) {
        SCCFATAL("scc::configurer") << "Illegal parameter name: '" << e.what() << "'";
    }
}

void configurer::start_of_simulation() {
    if(config_phases & START_OF_SIMULATION)
        configure();
    config_check();
    if(dump_file_name.size()) {
        std::ofstream of{dump_file_name};
        if(of.is_open())
            dump_configuration(of, with_description);
    }
}

} // namespace scc
