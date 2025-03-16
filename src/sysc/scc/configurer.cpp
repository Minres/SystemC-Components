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
#include <cci_configuration>
#include <cstring>
#include <fmt/format.h>
#include <fstream>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/ostreamwrapper.h>
#include <rapidjson/prettywriter.h>
#include <unordered_map>
#ifdef HAS_YAMPCPP
#include <yaml-cpp/exceptions.h>
#include <yaml-cpp/node/parse.h>
#include <yaml-cpp/yaml.h>

namespace {
template <typename T> struct optional {
    T val{};
    bool initialized{false};
    optional& operator=(T&& val) {
        this->val = std::move(val);
        initialized = true;
        return *this;
    }
    operator bool() const { return initialized; }
    T value() { return val; }
};
} // namespace

namespace YAML {
template <typename T> struct as_if<T, optional<T>> {
    explicit as_if(const YAML::Node& node_)
    : node(node_) {}
    const YAML::Node& node;
    const optional<T> operator()() const {
        optional<T> val;
        T t;
        if(node.m_pNode && YAML::convert<T>::decode(node, t))
            val = std::move(t);
        return val;
    }
};

// There is already a std::string partial specialisation, so we need a full specialisation here
template <> struct as_if<std::string, optional<std::string>> {
    explicit as_if(const YAML::Node& node_)
    : node(node_) {}
    const YAML::Node& node;
    const optional<std::string> operator()() const {
        optional<std::string> val;
        std::string t;
        if(node.m_pNode && YAML::convert<std::string>::decode(node, t))
            val = std::move(t);
        return val;
    }
};
} // namespace YAML
#endif
namespace scc {
namespace {
inline auto get_sc_objects(sc_core::sc_object* obj = nullptr) -> const std::vector<sc_core::sc_object*>& {
    if(obj)
        return obj->get_child_objects();
    else
        return sc_core::sc_get_top_level_objects();
}

#ifdef WIN32
#define DIR_SEPARATOR '\\'
#else
#define DIR_SEPARATOR '/'
#endif

struct config_reader {
    std::vector<std::string> includes{"."};
    void add_to_includes(std::string const& path) {
        for(auto& e : includes) {
            if(e == path)
                return;
        }
        includes.push_back(path);
    }

    std::string find_in_include_path(std::string const& file_name) {
        if(file_name[0] == '/' || file_name[0] == '\\')
            return file_name;
        else
            for(auto& incl : includes) {
                auto full_name = incl + DIR_SEPARATOR + file_name;
                std::ifstream ifs(full_name);
                if(ifs.is_open())
                    return full_name;
            }
        return file_name;
    }
};
/*************************************************************************************************
 * JSON config start
 ************************************************************************************************/
using namespace rapidjson;
using writer_type = PrettyWriter<OStreamWrapper>;

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

inline bool start_object(writer_type& writer, char const* key, bool started) {
    if(!started) {
        writer.Key(key);
        writer.StartObject();
    }
    return true;
}

struct json_config_dumper {
    configurer::broker_t const& broker;
    std::vector<std::string> const& stop_list;
    std::unordered_map<std::string, std::vector<cci::cci_param_untyped_handle>> lut;
    json_config_dumper(configurer::broker_t const& broker, std::vector<std::string> const& stop_list)
    : broker(broker)
    , stop_list(stop_list) {
        for(auto& h : broker.get_param_handles()) {
            auto value = h.get_cci_value();
            std::string paramname{h.name()};
            auto sep = paramname.rfind('.');
            auto basename = paramname.substr(0, sep);
            lut[basename].push_back(h);
        }
    }

    void dump_config(sc_core::sc_object* obj, writer_type& writer) {
        auto basename = std::string(obj->basename());
        if(basename.substr(0, 3) == "$$$" || std::find(std::begin(stop_list), std::end(stop_list), obj->name()) != std::end(stop_list))
            return;
        auto obj_started = false;
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
        if(scc::is_logging_initialized() && !log_lvl_set && mod) {
            obj_started = start_object(writer, obj->basename(), obj_started);
            auto val = broker.get_preset_cci_value(fmt::format("{}.{}", obj->name(), SCC_LOG_LEVEL_PARAM_NAME));
            if(basename.substr(0, 3) != "$$$")
                writeValue(writer, SCC_LOG_LEVEL_PARAM_NAME, val.is_int() ? val.get_int() : static_cast<int>(get_logging_level()));
        }
        for(auto* o : get_sc_objects(obj)) {
            obj_started = start_object(writer, obj->basename(), obj_started);
            dump_config(o, writer);
        }
        if(obj_started)
            writer.EndObject();
    }
};

struct json_config_reader : public config_reader {
    configurer::broker_t& broker;
    Document document;
    bool valid{false};
    bool empty{false};

    json_config_reader(configurer::broker_t& broker)
    : broker(broker) {}

    void parse(std::istream& is) {
        IStreamWrapper stream(is);
        document.ParseStream(stream);
        valid = !document.HasParseError();
        empty = document.IsNull();
    }
    std::string get_error_msg() {
        std::ostringstream os;
        os << " location " << (unsigned)document.GetErrorOffset() << ", reason: " << GetParseError_En(document.GetParseError());
        return os.str();
    }

    inline void configure_cci() { configure_cci_hierarchical(document, ""); }

    void configure_cci_hierarchical(Value const& value, std::string prefix) {
        if(value.IsObject()) {
            auto o = value.GetObject();
            for(auto itr = o.MemberBegin(); itr != o.MemberEnd(); ++itr) {
                if(!itr->name.IsString())
                    return;
                std::string key_name = itr->name.GetString();
                Value const& val = itr->value;
                auto hier_name = prefix.size() ? prefix + "." + key_name : key_name;
                if(val.IsNull() || val.IsArray())
                    return;
                else if(val.IsObject())
                    configure_cci_hierarchical(val, hier_name);
                else {
                    if(key_name == "!include") {
                        json_config_reader sub_reader(broker);
                        std::ifstream ifs(find_in_include_path(val.GetString()));
                        if(ifs.is_open()) {
                            sub_reader.parse(ifs);
                            if(sub_reader.valid) {
                                sub_reader.configure_cci_hierarchical(sub_reader.document, prefix);
                            } else {
                                std::ostringstream os;
                                os << "Could not parse include file " << val.GetString();
                                throw std::runtime_error(os.str());
                            }
                        } else {
                            std::ostringstream os;
                            os << "Could not open include file " << val.GetString();
                            throw std::runtime_error(os.str());
                        }
                    } else {
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
                }
            }
        }
    }
};
/*************************************************************************************************
 * JSON config end
 ************************************************************************************************/
#ifdef HAS_YAMPCPP
/*************************************************************************************************
 * YAML config start
 ************************************************************************************************/
struct yaml_config_dumper {
    configurer::broker_t const& broker;
    bool with_description{false};
    bool complete{true};
    std::vector<std::string> const& stop_list;
    std::unordered_map<std::string, std::vector<cci::cci_param_untyped_handle>> lut;
    std::vector<cci::cci_param_untyped_handle> tl_lut;
    yaml_config_dumper(configurer::broker_t const& broker, bool with_description, bool complete, std::vector<std::string> const& stop_list)
    : broker(broker)
    , with_description(with_description)
    , complete(complete)
    , stop_list(stop_list) {
        for(auto& h : broker.get_param_handles()) {
            auto value = h.get_cci_value();
            std::string paramname{h.name()};
            auto sep = paramname.rfind('.');
            if(sep == std::string::npos) {
                tl_lut.push_back(h);
            } else {
                auto basename = paramname.substr(0, sep);
                lut[basename].push_back(h);
            }
        }
    }

    void dump_config(YAML::Node& base_node) {
        copy2yaml(tl_lut, base_node);
        for(auto* o : sc_core::sc_get_top_level_objects()) {
            dump_config(o, base_node);
        }
    }

    void dump_config(sc_core::sc_object* obj, YAML::Node& base_node) {
        auto basename = std::string(obj->basename());
        if(basename.substr(0, 3) == "$$$" || std::find(std::begin(stop_list), std::end(stop_list), obj->name()) != std::end(stop_list))
            return;
        auto obj_started = false;
        auto log_lvl_set = false;
        auto it = lut.find(obj->name());
        YAML::Node this_node;
        if(it != lut.end())
            log_lvl_set |= copy2yaml(it->second, this_node);
        auto mod = dynamic_cast<sc_core::sc_module*>(obj);
        if(!log_lvl_set && mod && complete) {
            auto val = broker.get_preset_cci_value(fmt::format("{}.{}", obj->name(), SCC_LOG_LEVEL_PARAM_NAME));
            auto global_verb = static_cast<int>(get_logging_level());
            if(basename.substr(0, 11) != "scc_tracer")
                this_node["log_level"] = val.is_int() ? val.get_int() : global_verb;
        }
        for(auto* o : get_sc_objects(obj)) {
            dump_config(o, this_node);
        }
        if(this_node.size())
            base_node[obj->basename()] = this_node;
    }

private:
    bool copy2yaml(const std::vector<cci::cci_param_untyped_handle>& params, YAML::Node& this_node) {
        bool log_lvl_set = false;
        for(auto& h : params) {
            auto value = h.get_cci_value();
            std::string paramname{h.name()};
            auto basename = paramname.substr(paramname.rfind('.') + 1);
            if(basename == SCC_LOG_LEVEL_PARAM_NAME)
                log_lvl_set = true;

            auto descr = h.get_description();
            if(with_description && descr.size()) {
                auto descr_name = fmt::format("{}::descr", basename);
                this_node[descr_name] = descr;
                this_node[descr_name].SetTag("desc");
            }
            sc_core::sc_time t;
            if(value.is_bool())
                this_node[basename] = (bool)(value.get_bool());
            else if(value.is_int())
                this_node[basename] = (int)(value.get_int());
            else if(value.is_int64())
                this_node[basename] = static_cast<int64_t>(value.get_int64());
            else if(value.is_uint())
                this_node[basename] = value.get_uint();
            else if(value.is_uint64())
                this_node[basename] = static_cast<uint64_t>(value.get_uint64());
            else if(value.is_double())
                this_node[basename] = value.get_double();
            else if(value.is_string())
                this_node[basename] = value.get_string().c_str();
            else if(value.try_get(t))
                this_node[basename] = t.to_string();
        }
        return log_lvl_set;
    }
};

struct yaml_config_reader : public config_reader {
    configurer::broker_t& broker;
    YAML::Node document;
    bool valid{false};
    bool empty{true};

    yaml_config_reader(configurer::broker_t& broker)
    : broker(broker) {}

    void parse(std::istream& is) {
        std::string buf((std::istreambuf_iterator<char>(is)), std::istreambuf_iterator<char>());
        document = YAML::Load(buf);
        valid = document.IsDefined() && (document.IsMap() || document.IsNull());
        empty = document.IsNull();
    }

    std::string get_error_msg() { return "YAML file does not start with a map"; }

    inline void configure_cci() {
        try {
            configure_cci_hierarchical(document, "");
        } catch(YAML::ParserException& e) {
            throw std::runtime_error(e.what());
        } catch(YAML::BadFile& e) {
            throw std::runtime_error(e.what());
        } catch(YAML::Exception& e) {
            throw std::runtime_error(e.what());
        }
    }

    void configure_cci_hierarchical(YAML::Node const& value, std::string const& prefix) {
        if(value.IsMap()) {
            for(auto it = value.begin(); it != value.end(); ++it) {
                auto key_name = it->first.as<std::string>();
                YAML::Node const& val = it->second;
                auto hier_name = prefix.size() ? prefix + "." + key_name : key_name;
                if(!val.IsDefined() || val.IsSequence())
                    return;
                else if(val.IsMap())
                    configure_cci_hierarchical(val, hier_name);
                else if(val.IsScalar()) {
                    auto& tag = val.Tag();
                    if(tag == "!include") {
                        yaml_config_reader sub_reader(broker);
                        std::ifstream ifs(find_in_include_path(val.as<std::string>()));
                        if(ifs.is_open()) {
                            sub_reader.parse(ifs);
                            if(sub_reader.valid) {
                                sub_reader.configure_cci_hierarchical(sub_reader.document, hier_name);
                            } else {
                                std::ostringstream os;
                                os << "Could not parse include file " << val.as<std::string>();
                                throw std::runtime_error(os.str());
                            }
                        } else {
                            std::ostringstream os;
                            os << "Could not open include file " << val.as<std::string>();
                            throw std::runtime_error(os.str());
                        }
                    } else if(tag.size() && tag[0] == '?') {
                        auto param_handle = broker.get_param_handle(hier_name);
                        if(param_handle.is_valid()) {
                            auto param = param_handle.get_cci_value();
                            if(param.is_bool()) {
                                param.set_bool(val.as<bool>());
                            } else if(param.is_int()) {
                                param.set_int(val.as<int>());
                            } else if(param.is_uint()) {
                                param.set_uint(val.as<unsigned>());
                            } else if(param.is_int64()) {
                                param.set_int64(val.as<int64_t>());
                            } else if(param.is_uint64()) {
                                param.set_uint64(val.as<uint64_t>());
                            } else if(param.is_double()) {
                                param.set_double(val.as<double>());
                            } else if(param.is_string()) {
                                param.set_string(val.as<std::string>());
                            }
                        } else {
                            if(auto res = YAML::as_if<bool, optional<bool>>(val)()) {
                                broker.set_preset_cci_value(hier_name, cci::cci_value(res.value()));
                            } else if(auto res = YAML::as_if<int, optional<int>>(val)()) {
                                broker.set_preset_cci_value(hier_name, cci::cci_value(res.value()));
                            } else if(auto res = YAML::as_if<int64_t, optional<int64_t>>(val)()) {
                                broker.set_preset_cci_value(hier_name, cci::cci_value(res.value()));
                            } else if(auto res = YAML::as_if<unsigned, optional<unsigned>>(val)()) {
                                broker.set_preset_cci_value(hier_name, cci::cci_value(res.value()));
                            } else if(auto res = YAML::as_if<uint64_t, optional<uint64_t>>(val)()) {
                                broker.set_preset_cci_value(hier_name, cci::cci_value(res.value()));
                            } else if(auto res = YAML::as_if<double, optional<double>>(val)()) {
                                broker.set_preset_cci_value(hier_name, cci::cci_value(res.value()));
                            } else if(auto res = YAML::as_if<std::string, optional<std::string>>(val)()) {
                                broker.set_preset_cci_value(hier_name, cci::cci_value(res.value()));
                            }
                        }
                    }
                }
            }
        }
    }
};
/*************************************************************************************************
 * YAML config end
 ************************************************************************************************/
#endif
template <typename T>
inline bool create_cci_param(sc_core::sc_attr_base* base_attr, const std::string& hier_name, configurer::cci_param_cln& params,
                             configurer::broker_t& broker, cci::cci_originator& cci_originator) {
    if(auto attr = dynamic_cast<sc_core::sc_attribute<T>*>(base_attr)) {
        auto par = new cci::cci_param_typed<T>(hier_name, attr->value, broker, "", cci::CCI_ABSOLUTE_NAME, cci_originator);
        params.emplace_back(cci::cci_param_post_write_callback_untyped([attr](const cci::cci_param_write_event<>& ev) {
                                T result;
                                if(ev.new_value.try_get(result))
                                    attr->value = result;
                            }),
                            par);
        par->register_post_write_callback(params.back().first);
        attr->value = par->get_value(); // if we have a preset
        return true;
    }
    return false;
}

template <>
inline bool create_cci_param<char*>(sc_core::sc_attr_base* base_attr, const std::string& hier_name, configurer::cci_param_cln& params,
                                    configurer::broker_t& broker, cci::cci_originator& cci_originator) {
    if(auto attr = dynamic_cast<sc_core::sc_attribute<char*>*>(base_attr)) {
        auto par = new cci::cci_param_typed<std::string>(hier_name, attr->value, broker, "", cci::CCI_ABSOLUTE_NAME);
        params.emplace_back(cci::cci_param_post_write_callback_untyped([attr](const cci::cci_param_write_event<>& ev) {
                                if(attr->value)
                                    free(attr->value);
                                attr->value = strdup(ev.new_value.get<std::string>().c_str());
                            }),
                            par);
        par->register_post_write_callback(params.back().first);
        attr->value = strdup(par->get_value().c_str()); // if we have a preset
        return true;
    }
    return false;
}

template <typename T> inline bool update_cci_param(cci::cci_param_untyped_handle& param_handle, sc_core::sc_attr_base* base_attr) {
    if(auto attr = dynamic_cast<sc_core::sc_attribute<T>*>(base_attr)) {
        param_handle.set_cci_value(cci::cci_value(attr->value));
        return true;
    }
    return false;
}

template <> inline bool update_cci_param<char*>(cci::cci_param_untyped_handle& param_handle, sc_core::sc_attr_base* base_attr) {
    if(auto attr = dynamic_cast<sc_core::sc_attribute<char*>*>(base_attr)) {
        param_handle.set_cci_value(cci::cci_value(std::string(attr->value)));
        return true;
    }
    return false;
}

inline bool mirror_sc_attribute(configurer::broker_t& broker, configurer::cci_param_cln& params, cci::cci_originator& cci_originator,
                                std::string hier_name, sc_core::sc_attr_base* base_attr, bool update = false) {
    auto param_handle = broker.get_param_handle(hier_name);
    if(!param_handle.is_valid()) {
        if(create_cci_param<int>(base_attr, hier_name, params, broker, cci_originator))
            return true;
        if(create_cci_param<unsigned>(base_attr, hier_name, params, broker, cci_originator))
            return true;
        if(create_cci_param<long>(base_attr, hier_name, params, broker, cci_originator))
            return true;
        if(create_cci_param<unsigned long>(base_attr, hier_name, params, broker, cci_originator))
            return true;
        if(create_cci_param<long long>(base_attr, hier_name, params, broker, cci_originator))
            return true;
        if(create_cci_param<unsigned long long>(base_attr, hier_name, params, broker, cci_originator))
            return true;
        if(create_cci_param<bool>(base_attr, hier_name, params, broker, cci_originator))
            return true;
        if(create_cci_param<float>(base_attr, hier_name, params, broker, cci_originator))
            return true;
        if(create_cci_param<double>(base_attr, hier_name, params, broker, cci_originator))
            return true;
        if(create_cci_param<std::string>(base_attr, hier_name, params, broker, cci_originator))
            return true;
        if(create_cci_param<char*>(base_attr, hier_name, params, broker, cci_originator))
            return true;
    } else if(update) {
        if(update_cci_param<int>(param_handle, base_attr))
            return true;
        if(update_cci_param<unsigned>(param_handle, base_attr))
            return true;
        if(update_cci_param<long>(param_handle, base_attr))
            return true;
        if(update_cci_param<unsigned long>(param_handle, base_attr))
            return true;
        if(update_cci_param<long long>(param_handle, base_attr))
            return true;
        if(update_cci_param<unsigned long long>(param_handle, base_attr))
            return true;
        if(update_cci_param<bool>(param_handle, base_attr))
            return true;
        if(update_cci_param<float>(param_handle, base_attr))
            return true;
        if(update_cci_param<double>(param_handle, base_attr))
            return true;
        if(update_cci_param<std::string>(param_handle, base_attr))
            return true;
        if(update_cci_param<char*>(param_handle, base_attr))
            return true;
    }
    return false;
}

void mirror_sc_attributes(configurer::broker_t& broker, configurer::cci_param_cln& params, cci::cci_originator& cci_originator,
                          sc_core::sc_object* topobj = nullptr, bool update = false) {
    for(auto obj : get_sc_objects(topobj)) {
        if(auto mod = dynamic_cast<sc_core::sc_module*>(obj)) {
            for(auto base_attr : mod->attr_cltn()) {
                std::string hier_name = fmt::format("{}.{}", mod->name(), base_attr->name());
                mirror_sc_attribute(broker, params, cci_originator, hier_name, base_attr, update);
            }
            mirror_sc_attributes(broker, params, cci_originator, mod, update);
        }
    }
}

bool cci_name_ignore(std::pair<std::string, cci::cci_value> const& preset_value) {
    std::string ending(SCC_LOG_LEVEL_PARAM_NAME);
    auto& name = preset_value.first;
    if(name.length() >= ending.length()) {
        return (0 == name.compare(name.length() - ending.length(), ending.length(), ending));
    } else {
        return false;
    }
}
} // namespace
#ifdef HAS_YAMPCPP
struct configurer::ConfigHolder : public yaml_config_reader {
    ConfigHolder(configurer::broker_t& broker)
    : yaml_config_reader(broker) {}
};
#else
struct configurer::ConfigHolder : public json_config_reader {
    ConfigHolder(configurer::broker_t& broker)
    : json_config_reader(broker) {}
};
#endif

configurer::configurer(const std::string& filename, unsigned config_phases)
: configurer(filename, config_phases, "$$$configurer$$$") {}

configurer::configurer(const std::string& filename, unsigned config_phases, sc_core::sc_module_name nm)
: base_type(nm)
, config_phases(config_phases)
, cci_broker(cci::cci_get_broker())
, cci_originator(cci_broker.get_originator())
, root(new ConfigHolder(cci_broker)) {
    if(filename.length() > 0)
        read_input_file(filename);
}

configurer::~configurer() {}

void configurer::read_input_file(const std::string& filename) {
    root->add_to_includes(util::dir_name(filename));
    std::ifstream is(filename);
    if(is.is_open()) {
        try {
            root->parse(is);
            if(!root->valid) {
                SCCERR() << "Could not parse input file " << filename << ", " << root->get_error_msg();
            } else if(!root->empty) {
                root->configure_cci();
            }
        } catch(std::runtime_error& e) {
            SCCERR() << "Could not parse input file " << filename << ", reason: " << e.what();
        }
    } else {
        SCCWARN() << "Could not open input file " << filename;
    }
}

void configurer::dump_configuration(std::ostream& os, bool as_yaml, bool with_description, bool complete, sc_core::sc_object* obj) {
#ifdef HAS_YAMPCPP
    if(as_yaml) {
        YAML::Node root; // starts out as null
        yaml_config_dumper dumper(cci_broker, with_description, complete, stop_list);
        if(obj)
            for(auto* o : obj->get_child_objects()) {
                dumper.dump_config(o, root);
            }
        else
            dumper.dump_config(root);
        os << root;
        return;
    }
#endif
    OStreamWrapper stream(os);
    writer_type writer(stream);
    writer.StartObject();
    json_config_dumper dumper(cci_broker, stop_list);
    for(auto* o : get_sc_objects(obj)) {
        dumper.dump_config(o, writer);
    }
    writer.EndObject();
}

void configurer::configure() { mirror_sc_attributes(cci_broker, cci2sc_attr, cci_originator); }

void configurer::set_configuration_value(sc_core::sc_attr_base* attr_base, sc_core::sc_object* owner) {
    std::string hier_name = fmt::format("{}.{}", owner->name(), attr_base->name());
    mirror_sc_attribute(cci_broker, cci2sc_attr, cci_originator, hier_name, attr_base);
}

inline std::string hier_name_as_regex(std::string const& parname) {
    if(parname.find_first_of("*?[") != std::string::npos) {
        return util::glob_to_regex(parname);
    } else if(parname[0] == '^') {
        return parname;
    } else
        return "";
}

void configurer::set_value(const std::string& hier_name, cci::cci_value value) {
    auto regex_str = hier_name_as_regex(hier_name);
    if(regex_str.length()) {
        auto rr = std::regex(regex_str);
        cci::cci_param_predicate pred = [rr](cci::cci_param_untyped_handle const& hndl) { return regex_match(hndl.name(), rr); };
        for(auto& hndl : cci_broker.get_param_handles(pred)) {
            hndl.set_cci_value(value);
        }
    } else {
        cci::cci_param_handle param_handle = cci_broker.get_param_handle(hier_name);
        if(param_handle.is_valid()) {
            param_handle.set_cci_value(value);
            return;
        }
    }
    cci_broker.set_preset_cci_value(hier_name, value);
}

void configurer::set_value_from_str(const std::string &hier_name, const std::string &value) {
    try {
        auto i = std::stoi(value);
        set_value(hier_name, i);
        return;
    } catch (...) {}
    try {
        auto l = std::stol(value);
        set_value(hier_name, l);
        return;
    } catch (...) {}
    try {
        auto ll = std::stoll(value);
        set_value(hier_name, ll);
        return;
    } catch (...) {}
    try {
        auto f = std::stof(value);
        set_value(hier_name, f);
        return;
    } catch (...) {}
    try {
        auto d = std::stod(value);
        set_value(hier_name, d);
        return;
    } catch (...) { }
    auto lower_value = util::str_tolower(value);
    if(lower_value == "true"){
        set_value(hier_name, true);
        return;
    }
    if(lower_value == "false"){
        set_value(hier_name, false);
        return;
    }
    set_value(hier_name, value);
}

void configurer::config_check() {
    try {
        cci_broker.ignore_unconsumed_preset_values(&cci_name_ignore);
        auto res = cci_broker.get_unconsumed_preset_values();
        if(res.size()) {
            std::ostringstream oss;
            for(auto& val : res)
                oss << "\t - " << val.first << "\n";
            if(res.size() == 1) {
                SCCWARN("scc::configurer") << "There is " << res.size() << " unused CCI preset value:\n"
                                           << oss.str() << "Please check your setup!";
            } else {
                SCCWARN("scc::configurer") << "There are " << res.size() << " unused CCI preset values:\n"
                                           << oss.str() << "Please check your setup!";
            }
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
        auto as_json = util::ends_with(dump_file_name, "json");
        std::ofstream of{dump_file_name};
        if(of.is_open()) {
            mirror_sc_attributes(cci_broker, cci2sc_attr, cci_originator, nullptr, true);
            dump_configuration(of, !as_json, with_description, complete);
        }
    }
}

} // namespace scc
