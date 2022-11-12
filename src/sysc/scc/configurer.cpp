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
#include <cci_configuration>
#include <cci_utils/broker.h>
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

struct json_config_reader {
	configurer::broker_t& broker;
	json_config_reader(configurer::broker_t& broker)
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
			}
		}
	}
};

template<typename T>
inline
bool create_cci_param(sc_core::sc_attr_base *base_attr,
		const std::string &hier_name, configurer::cci_param_cln &params,
		configurer::broker_t &broker, cci::cci_originator& cci_originator) {
	if (auto attr = dynamic_cast<sc_core::sc_attribute<T>*>(base_attr)) {
		auto par = new cci::cci_param_typed<T>(hier_name, attr->value, "", cci::CCI_ABSOLUTE_NAME, cci_originator);
		params.emplace_back(cci::cci_param_post_write_callback_untyped([attr](const cci::cci_param_write_event<> & ev){
			attr->value = ev.new_value.get<T>();
		}), par);
		par->register_post_write_callback(params.back().first);
		attr->value = par->get_value(); // if we have a preset
		return true;
	}
	return false;
}

template<>
inline
bool create_cci_param<char*>(sc_core::sc_attr_base *base_attr,
		const std::string &hier_name, configurer::cci_param_cln &params,
		configurer::broker_t &broker, cci::cci_originator& cci_originator) {
	if (auto attr = dynamic_cast<sc_core::sc_attribute<char*>*>(base_attr)) {
		auto par = new cci::cci_param_typed<std::string>(hier_name, attr->value, "", cci::CCI_ABSOLUTE_NAME);
		params.emplace_back(cci::cci_param_post_write_callback_untyped([attr](const cci::cci_param_write_event<> & ev){
			if(attr->value) free(attr->value);
			attr->value = strdup(ev.new_value.get<std::string>().c_str());
		}), par);
		par->register_post_write_callback(params.back().first);
		attr->value = strdup(par->get_value().c_str()); // if we have a preset
		return true;
	}
	return false;
}

template<typename T>
inline
bool update_cci_param(cci::cci_param_untyped_handle& param_handle, sc_core::sc_attr_base *base_attr){
	if (auto attr = dynamic_cast<sc_core::sc_attribute<T>*>(base_attr)) {
		param_handle.set_cci_value(cci::cci_value(attr->value));
		return true;
	}
	return false;
}

template<>
inline
bool update_cci_param<char*>(cci::cci_param_untyped_handle& param_handle, sc_core::sc_attr_base *base_attr){
	if (auto attr = dynamic_cast<sc_core::sc_attribute<char*>*>(base_attr)) {
		param_handle.set_cci_value(cci::cci_value(std::string(attr->value)));
		return true;
	}
	return false;
}

inline
bool mirror_sc_attribute(configurer::broker_t& broker, configurer::cci_param_cln& params, cci::cci_originator& cci_originator,
		std::string hier_name, sc_core::sc_attr_base* base_attr, bool update = false) {
	auto param_handle = broker.get_param_handle(hier_name);
	if(!param_handle.is_valid()) {
		if(create_cci_param<int>(base_attr, hier_name, params, broker, cci_originator)) return true;
		if(create_cci_param<unsigned>(base_attr, hier_name, params, broker, cci_originator)) return true;
		if(create_cci_param<long>(base_attr, hier_name, params, broker, cci_originator)) return true;
		if(create_cci_param<unsigned long>(base_attr, hier_name, params, broker, cci_originator)) return true;
		if(create_cci_param<long long>(base_attr, hier_name, params, broker, cci_originator)) return true;
		if(create_cci_param<unsigned long long>(base_attr, hier_name, params, broker, cci_originator)) return true;
		if(create_cci_param<bool>(base_attr, hier_name, params, broker, cci_originator)) return true;
		if(create_cci_param<float>(base_attr, hier_name, params, broker, cci_originator)) return true;
		if(create_cci_param<double>(base_attr, hier_name, params, broker, cci_originator)) return true;
		if(create_cci_param<std::string>(base_attr, hier_name, params, broker, cci_originator)) return true;
		if(create_cci_param<char*>(base_attr, hier_name, params, broker, cci_originator)) return true;
	} else if(update) {
		if(update_cci_param<int>(param_handle, base_attr)) return true;
		if(update_cci_param<unsigned>(param_handle, base_attr)) return true;
		if(update_cci_param<long>(param_handle, base_attr)) return true;
		if(update_cci_param<unsigned long>(param_handle, base_attr)) return true;
		if(update_cci_param<long long>(param_handle, base_attr)) return true;
		if(update_cci_param<unsigned long long>(param_handle, base_attr)) return true;
		if(update_cci_param<bool>(param_handle, base_attr)) return true;
		if(update_cci_param<float>(param_handle, base_attr)) return true;
		if(update_cci_param<double>(param_handle, base_attr)) return true;
		if(update_cci_param<std::string>(param_handle, base_attr)) return true;
		if(update_cci_param<char*>(param_handle, base_attr)) return true;
	}
	return false;
}

void mirror_sc_attributes(configurer::broker_t& broker, configurer::cci_param_cln& params, cci::cci_originator& cci_originator, sc_core::sc_object* topobj = nullptr, bool update = false) {
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
	if (name.length() >= ending.length()) {
		return (0 == name.compare(name.length() - ending.length(), ending.length(), ending));
	} else {
		return false;
	}
}
} // namespace

struct configurer::ConfigHolder {
	Document document;
};

configurer::configurer(const std::string& filename, unsigned config_phases)
: base_type(sc_core::sc_module_name("$$$configurer$$$"))
, config_phases(config_phases)
, cci_originator("configurer")
, cci_broker(cci::cci_get_global_broker(cci_originator))
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
					json_config_reader reader(cci_broker);
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
	mirror_sc_attributes(cci_broker, cci2sc_attr, cci_originator);
}

void configurer::set_configuration_value(sc_core::sc_attr_base* attr_base, sc_core::sc_object* owner) {
	std::string hier_name = fmt::format("{}.{}", owner->name(), attr_base->name());
	mirror_sc_attribute(cci_broker, cci2sc_attr, cci_originator, hier_name, attr_base);
}

void configurer::set_value(const std::string &hier_name, cci::cci_value value) {
	cci::cci_param_handle param_handle = cci_broker.get_param_handle(hier_name);
	if(param_handle.is_valid()) {
		param_handle.set_cci_value(value);
	} else {
		cci_broker.set_preset_cci_value(hier_name, value);
	}
}

void configurer::config_check() {
	try {
		cci_broker.ignore_unconsumed_preset_values(&cci_name_ignore);
		auto res = cci_broker.get_unconsumed_preset_values();
		if(res.size()) {
			std::ostringstream oss;
			for(auto& val:res)
				oss<<"\t - "<<val.first<<"\n";
			if(res.size()==1) {
				SCCWARN("scc::configurer")<<"There is " << res.size() << " unused CCI preset value:\n"
						<< oss.str() << "Please check your setup!";
			} else {
				SCCWARN("scc::configurer")<<"There are " << res.size() << " unused CCI preset values:\n"
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
	if(config_phases & START_OF_SIMULATION) configure();
	config_check();
	if(dump_file_name.size()) {
		std::ofstream of{dump_file_name};
		if(of.is_open()) {
			mirror_sc_attributes(cci_broker, cci2sc_attr, cci_originator, nullptr, true);
			dump_configuration(of);
		}
	}
}

} // namespace scc
