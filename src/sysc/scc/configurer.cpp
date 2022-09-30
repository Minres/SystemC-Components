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

#include "configurer.h"
#include "rapidjson/document.h"
#include "rapidjson/error/en.h"
#include "report.h"
#include <fmt/format.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/ostreamwrapper.h>
#include <rapidjson/prettywriter.h>
#ifdef HAS_CCI
#include <cci_configuration>
#include <cci_utils/broker.h>
#endif
#include <cstring>
#include <fstream>
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

#define FDECL(TYPE, FUNC)                                                                                              \
		inline void writeValue(writer_type& writer, std::string const& key, TYPE value) {                                  \
	writer.Key(key.c_str());                                                                                       \
	writer.FUNC(value);                                                                                            \
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
    configurer::broker_t const& broker;
#ifdef HAS_CCI
    std::unordered_map<std::string, std::vector<cci::cci_param_untyped_handle>> lut;
    config_dumper(configurer::broker_t const& broker)
    : broker(broker) {
        for(auto& h : broker.get_param_handles()) {
            auto value = h.get_cci_value();
            std::string paramname{h.name()};
            auto sep = paramname.rfind('.');
            auto basename = paramname.substr(0, sep);
            lut[basename].push_back(h);
        }
    }
#else
    config_dumper(configurer::broker_t const& broker)
    : broker(broker) {}
#endif

    void dump_config(sc_core::sc_object* obj, writer_type& writer) {
        auto start = std::string(obj->basename()).substr(0, 3);
        if(start == "$$$") return;
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
#ifdef HAS_CCI
        const std::string hier_name{obj->name()};
        auto log_lvl_set = false;
        auto it = lut.find(obj->name());
        if(it != lut.end())
            for(auto& h : it->second) {
                obj_started = start_object(writer, obj->basename(), obj_started);
                auto value = h.get_cci_value();
                std::string paramname{h.name()};
                auto basename = paramname.substr(paramname.rfind('.') + 1);
                if(basename == SCC_LOG_LEVEL_PARAM_NAME)
                    log_lvl_set = true;
                if(value.is_bool())
                    writeValue(writer, basename, (bool)value.get_bool());
                else if(value.is_int())
                    writeValue(writer, basename, (int)value.get_int());
                else if(value.is_int64())
                    writeValue(writer, basename, static_cast<int64_t>(value.get_int64()));
                else if(value.is_uint())
                    writeValue(writer, basename, value.get_uint());
                else if(value.is_uint64())
                    writeValue(writer, basename, static_cast<uint64_t>(value.get_uint64()));
                else if(value.is_double())
                    writeValue(writer, basename, value.get_double());
                else if(value.is_string())
                    writeValue(writer, basename, value.get_string().c_str());
            }
        auto mod = dynamic_cast<sc_core::sc_module*>(obj);
        if(!log_lvl_set && mod) {
            obj_started = start_object(writer, obj->basename(), obj_started);
            auto val = broker.get_preset_cci_value(fmt::format("{}.{}", obj->name(), SCC_LOG_LEVEL_PARAM_NAME));
            auto global_verb = static_cast<int>(get_logging_level());
            writeValue(writer, "log_level", val.is_int() ? val.get_int() : global_verb);
        }
#endif
        for(auto* o : get_sc_objects(obj)) {
            obj_started = start_object(writer, obj->basename(), obj_started);
            dump_config(o, writer);
        }
        if(obj_started)
            writer.EndObject();
    }
};

#define CHECK_N_ASSIGN(TYPE, ATTR, VAL)                                                                                \
		{                                                                                                                  \
	auto* a = dynamic_cast<sc_core::sc_attribute<TYPE>*>(ATTR);                                                    \
	if(a != nullptr) {                                                                                             \
		a->value = VAL;                                                                                            \
		return;                                                                                                    \
	}                                                                                                              \
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

struct config_reader {
    configurer::broker_t& broker;
    config_reader(configurer::broker_t& broker)
    : broker(broker) {}
    void configure_cci_hierarchical(Value const& value, std::string prefix) {
        if(value.IsObject()) {
            auto o = value.GetObject();
            for(auto itr = o.MemberBegin(); itr != o.MemberEnd(); ++itr) {
                if(!itr->name.IsString())
                    return;
                auto key_name = itr->name.GetString();
                Value const& val = itr->value;
                auto hier_name = prefix.size() ? prefix + "." + key_name : key_name;
                if(val.IsNull() || val.IsArray())
                    return;
                else if(val.IsObject())
                    configure_cci_hierarchical(val, hier_name);
#ifdef HAS_CCI
                else {
                    auto param_handle = broker.get_param_handle(hier_name);
                    if(param_handle.is_valid()) {
                        if(val.IsString()) {
                            param_handle.set_cci_value(cci::cci_value(std::string(val.GetString())));
                        } else if(val.IsBool()) {
                            param_handle.set_cci_value(cci::cci_value(val.Get<bool>()));
                        } else if(val.IsInt()) {
                            param_handle.set_cci_value(cci::cci_value(val.Get<int>()));
                        } else if(val.IsInt64()) {
                            param_handle.set_cci_value(cci::cci_value(val.Get<int64_t>()));
                        } else if(val.IsUint()) {
                            param_handle.set_cci_value(cci::cci_value(val.Get<unsigned>()));
                        } else if(val.IsUint64()) {
                            param_handle.set_cci_value(cci::cci_value(val.Get<uint64_t>()));
                        } else if(val.IsDouble()) {
                            param_handle.set_cci_value(cci::cci_value(val.Get<double>()));
                        }
                    } else {
                        if(val.IsString()) {
                            broker.set_preset_cci_value(hier_name, cci::cci_value(std::string(val.GetString())));
                        } else if(val.IsBool()) {
                            broker.set_preset_cci_value(hier_name, cci::cci_value(val.Get<bool>()));
                        } else if(val.IsInt()) {
                            broker.set_preset_cci_value(hier_name, cci::cci_value(val.Get<int>()));
                        } else if(val.IsInt64()) {
                            broker.set_preset_cci_value(hier_name, cci::cci_value(val.Get<int64_t>()));
                        } else if(val.IsUint()) {
                            broker.set_preset_cci_value(hier_name, cci::cci_value(val.Get<unsigned>()));
                        } else if(val.IsUint64()) {
                            broker.set_preset_cci_value(hier_name, cci::cci_value(val.Get<uint64_t>()));
                        } else if(val.IsDouble()) {
                            broker.set_preset_cci_value(hier_name, cci::cci_value(val.Get<double>()));
                        }
                    }
                }
#endif
            }
        }
    }
};

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

void check_config_hierarchical(configurer::broker_t const& broker, Value const& node, std::string const& prefix) {
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
                check_config_hierarchical(broker, val, hier_name);
        } else {
            auto pos = hier_name.rfind('.');
            if(pos != std::string::npos) {
                auto objname = hier_name.substr(0, pos);
                auto attrname = hier_name.substr(pos + 1);
                auto* obj = sc_core::sc_find_object(objname.c_str());
                if(!obj || !obj->get_attribute(attrname.c_str())) {
#ifdef HAS_CCI
                    auto param_handle = broker.get_param_handle(hier_name);
                    if(!param_handle.is_valid() && attrname != SCC_LOG_LEVEL_PARAM_NAME)
#endif
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
: base_type(sc_core::sc_module_name("$$$configurer$$$"))
, config_phases(config_phases)
#ifdef HAS_CCI
, cci_originator("configurer")
, cci_broker(cci::cci_get_global_broker(cci_originator))
#endif
{
    if(filename.length() > 0) {
        std::ifstream is(filename);
        if(is.is_open()) {
            root.reset(new ConfigHolder);
            try {
                IStreamWrapper stream(is);
                root->document.ParseStream(stream);
                if(root->document.HasParseError()) {
                    SCCERR() << "Could not parse input file " << filename << ", location "
                            << (unsigned)root->document.GetErrorOffset()
                            << ", reason: " << GetParseError_En(root->document.GetParseError());
                } else {
                    config_reader reader(cci_broker);
                    reader.configure_cci_hierarchical(root->document, "");
                    config_valid = true;
                }
            } catch(std::runtime_error& e) {
                SCCERR() << "Could not parse input file " << filename << ", reason: " << e.what();
            }
        } else {
            SCCERR() << "Could not open input file " << filename;
        }
    }
}

configurer::~configurer() {}

void configurer::dump_hierarchy(std::ostream& os, sc_core::sc_object* obj) {
    if(obj) {
        os << obj->name() << " of type " << typeid(*obj).name() << "\n";
        for(auto* o : obj->get_child_objects())
            dump_hierarchy(os, o);
    } else {
        for(auto* o : sc_core::sc_get_top_level_objects())
            dump_hierarchy(os, o);
    }
}

void configurer::dump_configuration(std::ostream& os, sc_core::sc_object* obj) {
    OStreamWrapper stream(os);
    writer_type writer(stream);
    writer.StartObject();
    config_dumper dumper(cci_broker);
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
        if(root)
            check_config_hierarchical(cci_broker, root->document, "");
    } catch(std::domain_error& e) {
        SCCFATAL(this->name()) << "Illegal hierarchy name: '" << e.what() << "'";
    } catch(std::invalid_argument& e) {
        SCCFATAL(this->name()) << "Illegal parameter name: '" << e.what() << "'";
    }
}

void init_cci(std::string name) {
#ifdef HAS_CCI
    thread_local cci_utils::broker broker(name);
    cci::cci_register_broker(&broker);
#endif
}

void configurer::start_of_simulation() {
    if(config_phases & START_OF_SIMULATION) configure();
    config_check();
    if(dump_file_name.size()) {
        std::ofstream of{dump_file_name};
        if(of.is_open())
            dump_configuration(of);
    }
}

} // namespace scc
