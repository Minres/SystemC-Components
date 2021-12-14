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
 * configurer.cpp
 *
 *  Created on: 27.09.2017
 *      Author: eyck
 */

#include "configurer.h"
#include "report.h"
#include <json/json.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/ostreamwrapper.h>
#include <fmt/format.h>
#ifdef HAS_CCI
#include <cci_configuration>
#include <cci_utils/broker.h>
#endif
#include <fstream>
#include <cstring>
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

#define FDECL(TYPE, FUNC) \
		inline void writeValue(writer_type& writer, std::string const& key, TYPE value)\
		{writer.Key(key.c_str()); writer.FUNC(value);}
FDECL(int, Int)
FDECL(unsigned int, Uint)
FDECL(long, Int64)
FDECL(unsigned long, Uint64)
FDECL(long long, Int64)
FDECL(unsigned long long, Uint64)
FDECL(bool, Bool)
FDECL(float, Double)
FDECL(double, Double)
FDECL(char*, String)
inline void writeValue(writer_type& writer, std::string const& key, std::string const& value) {
    writer.Key(key.c_str());
    writer.String(value.c_str());
}

template <typename T>
auto check_n_assign(writer_type& writer, sc_core::sc_attr_base* attr_base) -> bool {
    auto* a = dynamic_cast<sc_core::sc_attribute<T>*>(attr_base);
    if(a != nullptr) {
        writeValue(writer, a->name(), a->value);
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
    std::unordered_map<std::string,std::vector<cci::cci_param_untyped_handle>> lut;

    config_dumper(configurer::broker_t const& broker):broker(broker){
        for(auto& h: broker.get_param_handles()){
            auto value = h.get_cci_value();
            std::string paramname{h.name()};
            auto sep = paramname.rfind('.');
            auto basename = paramname.substr(0, sep);
            lut[basename].push_back(h);
        }
    }

    void dump_config(sc_core::sc_object* obj,  writer_type& writer) {
        auto obj_started=false;
        auto mod = dynamic_cast<sc_core::sc_module*>(obj);
        for(sc_core::sc_attr_base* attr_base : obj->attr_cltn()) {
            obj_started=start_object(writer, obj->basename(), obj_started);
            check_n_assign<int>(writer, attr_base) ||
                    check_n_assign<unsigned>(writer, attr_base) ||
                    check_n_assign<long>(writer, attr_base) ||
                    check_n_assign<unsigned long>(writer, attr_base) ||
                    check_n_assign<long long>(writer, attr_base) ||
                    check_n_assign<unsigned long long>(writer, attr_base) ||
                    check_n_assign<bool>(writer, attr_base) ||
                    check_n_assign<float>(writer, attr_base) || check_n_assign<double>(writer, attr_base) ||
                    check_n_assign<std::string>(writer, attr_base) || check_n_assign<char*>(writer, attr_base);
        }
#ifdef HAS_CCI
        const std::string hier_name{obj->name()};
        auto log_lvl_set=false;
        auto it = lut.find(obj->name());
        if(it!=lut.end())
            for(auto& h : it->second) {
                obj_started=start_object(writer, obj->basename(), obj_started);
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
        if(!log_lvl_set && mod) {
            obj_started=start_object(writer, obj->basename(), obj_started);
            auto val = broker.get_preset_cci_value(fmt::format("{}.{}", obj->name(), SCC_LOG_LEVEL_PARAM_NAME));
            auto global_verb = static_cast<int>(get_logging_level());
            writeValue(writer, "log_level", val.is_int() ? val.get_int() : global_verb);
        }
#endif
        for(auto* o : get_sc_objects(obj)) {
            if(dynamic_cast<sc_core::sc_module*>(obj)){
                obj_started=start_object(writer, obj->basename(), obj_started);
                dump_config(o, writer);
            }
        }
        if(obj_started) writer.EndObject();
    }
};

#define CHECK_N_ASSIGN(TYPE, ATTR, VAL)                                                                            \
		{                                                                                                          \
	auto* a = dynamic_cast<sc_core::sc_attribute<TYPE>*>(ATTR);                                                    \
	if(a != nullptr) {                                                                                             \
		a->value = VAL;                                                                                            \
		return;                                                                                                    \
	}                                                                                                              \
		}

void try_set_value(sc_core::sc_attr_base* attr_base, Json::Value& hier_val) {
    CHECK_N_ASSIGN(int, attr_base, hier_val.asInt());
    CHECK_N_ASSIGN(unsigned, attr_base, hier_val.asUInt());
    CHECK_N_ASSIGN(int64_t, attr_base, hier_val.asInt64());
    CHECK_N_ASSIGN(uint64_t, attr_base, hier_val.asUInt64());
    CHECK_N_ASSIGN(bool, attr_base, hier_val.asBool());
    CHECK_N_ASSIGN(float, attr_base, hier_val.asFloat());
    CHECK_N_ASSIGN(double, attr_base, hier_val.asDouble());
    CHECK_N_ASSIGN(std::string, attr_base, hier_val.asString());
    CHECK_N_ASSIGN(char*, attr_base, strdup(hier_val.asCString()));
}

void configure_cci_hierarchical(configurer::broker_t& broker, Json::Value const& node, std::string prefix) {
    if(node.size() > 0) {
        for(Json::Value::const_iterator itr = node.begin(); itr != node.end(); ++itr) {
            if(!itr.key().isString())
                return;
            auto key_name = itr.key().asString();
            auto hier_name=prefix.size() ? prefix + "." + key_name : key_name;
            auto val = node[key_name];
            if(val.isNull() || val.isArray())
                return;
            else if(val.isObject())
                configure_cci_hierarchical(broker, *itr, hier_name);
#ifdef HAS_CCI
            else {
                auto param_handle = broker.get_param_handle(hier_name);
                if(param_handle.is_valid()) {
                    if(val.isString()) {
                        param_handle.set_cci_value(cci::cci_value(val.asString()));
                    } else if(val.isBool()) {
                        param_handle.set_cci_value(cci::cci_value(val.asBool()));
                    } else if(val.isInt()) {
                        param_handle.set_cci_value(cci::cci_value(val.asInt()));
                    } else if(val.isInt64()) {
                        param_handle.set_cci_value(cci::cci_value(val.asInt64()));
                    } else if(val.isUInt()) {
                        param_handle.set_cci_value(cci::cci_value(val.asUInt()));
                    } else if(val.isUInt64()) {
                        param_handle.set_cci_value(cci::cci_value(val.asUInt64()));
                    } else if(val.isDouble()) {
                        param_handle.set_cci_value(cci::cci_value(val.asDouble()));
                    }
                } else {
                    if(val.isString()) {
                        broker.set_preset_cci_value(hier_name, cci::cci_value(val.asString()));
                    } else if(val.isBool()) {
                        broker.set_preset_cci_value(hier_name, cci::cci_value(val.asBool()));
                    } else if(val.isInt()) {
                        broker.set_preset_cci_value(hier_name, cci::cci_value(val.asInt()));
                    } else if(val.isInt64()) {
                        broker.set_preset_cci_value(hier_name, cci::cci_value(val.asInt64()));
                    } else if(val.isUInt()) {
                        broker.set_preset_cci_value(hier_name, cci::cci_value(val.asUInt()));
                    } else if(val.isUInt64()) {
                        broker.set_preset_cci_value(hier_name, cci::cci_value(val.asUInt64()));
                    } else if(val.isDouble()) {
                        broker.set_preset_cci_value(hier_name, cci::cci_value(val.asDouble()));
                    }
                }
            }
#endif
        }
    }
}

void configure_sc_attribute_hierarchical(sc_core::sc_object* obj, Json::Value& hier_val) {
    for(auto* attr : obj->attr_cltn()) {
        auto& val = hier_val[attr->name()];
        if(!val.isNull())
            try_set_value(attr, val);
    }
    for(auto* o : obj->get_child_objects()) {
        auto& val = hier_val[o->basename()];
        if(!val.isNull())
            configure_sc_attribute_hierarchical(o, val);
    }
}

void check_config_hierarchical(configurer::broker_t const& broker, Json::Value const& node, std::string const& prefix) {
    if(node.size() > 0) {
        for(Json::Value::const_iterator itr = node.begin(); itr != node.end(); ++itr) {
            if(!itr.key().isString())
                return;
            auto key_name = itr.key().asString();
            if(key_name == "log_level")
                continue; // virtual attribute
            auto hier_name=prefix.size() ? prefix + "." + key_name : key_name;
            auto val = node[key_name];
            if(val.isNull() || val.isArray())
                continue;
            else if(val.isObject()) {
                if(!sc_core::sc_find_object(hier_name.c_str())) {
                    throw std::domain_error(hier_name);
                }
                check_config_hierarchical(broker, *itr, hier_name);
            }
#ifdef HAS_CCI
            else {
                auto* obj = sc_core::sc_find_object(prefix.c_str());
                auto* attr = obj->get_attribute(key_name);
                if(!attr) {
                    auto param_handle = broker.get_param_handle(hier_name);
                    if(!param_handle.is_valid()) {
                        throw std::invalid_argument(hier_name);
                    }
                }
            }
#endif
        }
    }
}
}

struct configurer::ConfigHolder {
    Json::Value v;
};

configurer::configurer(const std::string& filename)
: base_type(sc_core::sc_module_name("configurer"))
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
                is >> root->v;
                configure_cci_hierarchical(cci_broker, root->v, "");
                config_valid = true;
            } catch(Json::RuntimeError& e) {
                SCCERR() << "Could not parse input file " << filename << ", reason: " << e.what();
            }
        } else {
            SCCERR() << "Could not open input file " << filename;
        }
    }
#ifdef WITH_SIM_PHASE_CALLBACKS
    register_simulation_phase_callback(sc_core::sc_status::SC_END_OF_ELABORATION);
#endif
}

configurer::~configurer(){}

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
    if(config_valid && root)
        for(auto* o : sc_core::sc_get_top_level_objects()) {
            Json::Value& val = root->v[o->name()];
            if(!val.isNull())
                configure_sc_attribute_hierarchical(o, val);
        }
}

auto get_value_from_hierarchy(const std::string& hier_name, Json::Value& value) -> Json::Value& {
    size_t npos = hier_name.find_first_of('.');
    Json::Value& val = value[hier_name.substr(0, npos)];
    if(val.isNull() || npos == std::string::npos || !val.isObject())
        return val;
    return get_value_from_hierarchy(hier_name.substr(npos + 1, hier_name.size()), val);
}

void configurer::set_configuration_value(sc_core::sc_attr_base* attr_base, sc_core::sc_object* owner) {
    if(root){
        std::string name(owner->name());
        name += ".";
        name += attr_base->name();
        size_t npos = name.find_first_of('.');
        Json::Value& val = get_value_from_hierarchy(name, root->v[name.substr(0, npos)]);
        if(!val.isNull())
            try_set_value(attr_base, val);
    }
}

void configurer::end_of_elaboration_check()  {
    try {
        if(root) check_config_hierarchical(cci_broker, root->v, "");
    } catch(std::domain_error& e) {
        SCCFATAL(this->name()) << "Illegal hierarchy name: '" << e.what() << "'";
    } catch(std::invalid_argument& e){
        SCCFATAL(this->name()) << "Illegal parameter name: '" << e.what() << "'";
    }
}

void init_cci(std::string name) {
#ifdef HAS_CCI
    thread_local static cci_utils::broker broker(name);
    cci::cci_register_broker(&broker);
#endif
}
}
