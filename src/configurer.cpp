/*******************************************************************************
 * Copyright 2017 MINRES Technologies GmbH
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

#include "scc/configurer.h"
#include "scc/report.h"

#include <fstream>


scc::configurer::configurer(const std::string &filename)
: base_type("configurer")
, cci_originator("configurer")
, cci_broker(cci::cci_get_global_broker(cci_originator))
{
	if (filename.length() > 0) {
        std::ifstream is(filename);
        if (is.is_open()) {
            try {
                is >> root;
            	traverse_json_tree(root, "");
           } catch (Json::RuntimeError &e) {
                LOG(ERROR) << "Could not parse input file " << filename << ", reason: " << e.what();
            }
        } else {
            LOG(ERROR) << "Could not open input file " << filename;
        }
    }
}

void scc::configurer::dump_hierarchy(std::ostream& os, sc_core::sc_object* obj) {
	if (obj) {
		os << obj->name() << " of type " << typeid(*obj).name() << "\n";
		for (auto *o : obj->get_child_objects()) dump_hierarchy(os);
	} else {
		for (auto *o : sc_core::sc_get_top_level_objects(sc_core::sc_curr_simcontext)) dump_hierarchy(os, o);
	}
}

inline
const std::vector<sc_core::sc_object*>& get_sc_objects(sc_core::sc_object* obj = nullptr){
	if(obj)
		return obj->get_child_objects();
	else
		return sc_core::sc_get_top_level_objects(sc_core::sc_curr_simcontext);
}

void scc::configurer::dump_configuration(std::ostream& os, sc_core::sc_object* obj) {
	Json::Value root{Json::objectValue};
	for (auto *o : get_sc_objects(obj)){
		dump_configuration(os, o, root);
	}
	// For convenience, use `writeString()` with a specialized builder.
	Json::StreamWriterBuilder wbuilder;
	wbuilder["indentation"] = "\t";
	os<< root;
}

#define CHECK_N_ASSIGN_VAL(TYPE, ATTR)                                \
    {                                                                 \
        auto *a = dynamic_cast<sc_core::sc_attribute<TYPE> *>(ATTR);  \
        if (a != nullptr) {node[a->name()]= a->value;continue;}       \
    }

void scc::configurer::dump_configuration(std::ostream& os, sc_core::sc_object* obj, Json::Value& parent) {
	auto mod = dynamic_cast<sc_core::sc_module*>(obj);
	Json::Value node{Json::objectValue};
	for(sc_core::sc_attr_base* attr_base : obj->attr_cltn()){
		CHECK_N_ASSIGN_VAL(int, attr_base);
		CHECK_N_ASSIGN_VAL(unsigned, attr_base);
		CHECK_N_ASSIGN_VAL(int64_t, attr_base);
		CHECK_N_ASSIGN_VAL(uint64_t, attr_base);
		CHECK_N_ASSIGN_VAL(bool, attr_base);
		CHECK_N_ASSIGN_VAL(float, attr_base);
		CHECK_N_ASSIGN_VAL(double, attr_base);
		CHECK_N_ASSIGN_VAL(std::string, attr_base);
		CHECK_N_ASSIGN_VAL(char *, attr_base);
	}
	for (auto *o : get_sc_objects(obj))
		dump_configuration(os, o, node);
	if(!node.empty())
		parent[obj->basename()]=node;
}

void scc::configurer::configure() {
    for (auto *o : sc_core::sc_get_top_level_objects(sc_core::sc_curr_simcontext)) {
        Json::Value &val = root[o->name()];
        if (!val.isNull()) configure_sc_object(o, val);
    }
	//traverse_json_tree(root, "");
}

void scc::configurer::configure_sc_object(sc_core::sc_object *obj, Json::Value &hier_val) {
    sc_core::sc_attr_base *ab = dynamic_cast<sc_core::sc_attr_base *>(obj);
    if (ab != nullptr) {
    	set_value(ab, hier_val);
    } else {
        sc_core::sc_module *mod = dynamic_cast<sc_core::sc_module *>(obj);
        if (mod != nullptr) {
            for (auto *o : mod->get_child_objects()) {
                Json::Value &val = hier_val[o->basename()];
                if (!val.isNull())
                    configure_sc_object(o, val);
            }
        }
    }
}

#define CHECK_N_ASSIGN(TYPE, ATTR, VAL)                                                                                \
    {                                                                                                                  \
        auto *a = dynamic_cast<sc_core::sc_attribute<TYPE> *>(ATTR);                                                   \
        if (a != nullptr) {                                                                                            \
            a->value = VAL;                                                                                            \
            return;                                                                                                    \
        }                                                                                                              \
    }

void scc::configurer::set_value(sc_core::sc_attr_base *attr_base, Json::Value &hier_val) {
    CHECK_N_ASSIGN(int, attr_base, hier_val.asInt());
    CHECK_N_ASSIGN(unsigned, attr_base, hier_val.asUInt());
    CHECK_N_ASSIGN(int64_t, attr_base, hier_val.asInt64());
    CHECK_N_ASSIGN(uint64_t, attr_base, hier_val.asUInt64());
    CHECK_N_ASSIGN(bool, attr_base, hier_val.asBool());
    CHECK_N_ASSIGN(float, attr_base, hier_val.asFloat());
    CHECK_N_ASSIGN(double, attr_base, hier_val.asDouble());
    CHECK_N_ASSIGN(std::string, attr_base, hier_val.asString());
    CHECK_N_ASSIGN(char *, attr_base, strdup(hier_val.asCString()));
}

Json::Value &scc::configurer::get_value_from_hierarchy(const std::string &hier_name) {
    size_t npos = hier_name.find_first_of('.');
    Json::Value &val = root[hier_name.substr(0, npos)];
    if (val.isNull() || npos == hier_name.size()) return val;
    return get_value_from_hierarchy(hier_name.substr(npos + 1, hier_name.size()), val);
}

Json::Value &scc::configurer::get_value_from_hierarchy(const std::string &hier_name, Json::Value &value) {
    size_t npos = hier_name.find_first_of('.');
    Json::Value &val = value[hier_name.substr(0, npos)];
    if (val.isNull() || npos == std::string::npos || !val.isObject()) return val;
    return get_value_from_hierarchy(hier_name.substr(npos + 1, hier_name.size()), val);
}

void scc::configurer::set_configuration_value(sc_core::sc_attr_base* attr_base, sc_core::sc_module* owner) {
    std::string name(owner->name());
    name += ".";
    name += attr_base->name();
    Json::Value &val = get_value_from_hierarchy(name);
    if (!val.isNull()) set_value(attr_base, val);
}

void scc::configurer::traverse_json_tree( const Json::Value &node, std::string prefix) {
    if( node.size() > 0 ) {
        for( Json::Value::const_iterator itr = node.begin() ; itr != node.end() ; ++itr) {
    	    if(!itr.key().isString()) return;
    	    std::string key_name=itr.key().asString();
			std::string hier_name{prefix.size()?prefix+"."+key_name:key_name};
  			Json::Value val = node[key_name];
  			if (val.isNull() || val.isArray())
  				return;
  			else if(val.isObject())
  				traverse_json_tree(*itr, hier_name);
  			else {
  	    	    cci::cci_param_handle param_handle = cci_broker.get_param_handle(hier_name);
  				if(param_handle.is_valid()) {
  					if( val.isString() ) {
  						param_handle.set_cci_value(cci::cci_value(val.asString()));
  					} else if( val.isBool() ) {
  						param_handle.set_cci_value(cci::cci_value(val.asBool()));
  					} else if( val.isInt() ) {
  						param_handle.set_cci_value(cci::cci_value(val.asInt()));
  					} else if( val.isInt64() ) {
  						param_handle.set_cci_value(cci::cci_value(val.asInt64()));
  					} else if( val.isUInt() ) {
  						param_handle.set_cci_value(cci::cci_value(val.asUInt()));
  					} else if( val.isUInt64() ) {
  						param_handle.set_cci_value(cci::cci_value(val.asUInt64()));
  					} else if( val.isDouble() ) {
  						param_handle.set_cci_value(cci::cci_value(val.asDouble()));
  					}
  				} else {
  					if( val.isString() ) {
  						cci_broker.set_preset_cci_value(hier_name, cci::cci_value(val.asString()));
  					} else if( val.isBool() ) {
  						cci_broker.set_preset_cci_value(hier_name, cci::cci_value(val.asBool()));
  					} else if( val.isInt() ) {
  						cci_broker.set_preset_cci_value(hier_name, cci::cci_value(val.asInt()));
  					} else if( val.isInt64() ) {
  						cci_broker.set_preset_cci_value(hier_name, cci::cci_value(val.asInt64()));
  					} else if( val.isUInt() ) {
  						cci_broker.set_preset_cci_value(hier_name, cci::cci_value(val.asUInt()));
  					} else if( val.isUInt64() ) {
  						cci_broker.set_preset_cci_value(hier_name, cci::cci_value(val.asUInt64()));
  					} else if( val.isDouble() ) {
  						cci_broker.set_preset_cci_value(hier_name, cci::cci_value(val.asDouble()));
  					}
  				}
  			}
        }
    }
}
