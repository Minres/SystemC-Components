/*******************************************************************************
 * Copyright 2022 MINRES Technologies GmbH
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

#include <string>
#include "cci_broker.h"
#include "report.h"
#include <util/ities.h>
namespace {

}

namespace scc {
using namespace cci;

cci_originator cci_broker::get_preset_value_origin(const std::string &parname) const { //TODO: check globs
    const_cast<cci_broker*>(this)->insert_matching_preset_value(parname);
	return consuming_broker::get_preset_value_origin(parname);
}

/*
 * private function to determine if we send to the parent broker or not
 */
bool cci_broker::sendToParent(const std::string &parname) const {
	return  ((expose.find(parname) != expose.end()) && (!is_global_broker()));
}

/*
 * public interface functions
 */
cci_broker::cci_broker(const std::string& name)
: consuming_broker(name)
, m_parent(get_parent_broker()) // local convenience function
{
	sc_assert (name.length() > 0 && "Name must not be empty");
}

cci_broker::~cci_broker() {
}

cci_originator cci_broker::get_value_origin(const std::string &parname) const {
	if (sendToParent(parname)) {
		return m_parent.get_value_origin(parname);
	} else {
		return consuming_broker::get_value_origin(parname);
	}
}

void cci_broker::insert_matching_preset_value(const std::string &parname) {
	bool match=false;
	for(auto const& e: wildcard_presets){
		if(std::regex_match(parname, e.second.rr)) {
			consuming_broker::set_preset_cci_value(parname, e.second.value, e.second.originator);
			match=true;
			break;
		}
	}
	if(match) for(auto const& e: wildcard_locks){
		if(std::regex_match(parname, e.second)) {
			consuming_broker::lock_preset_value(parname);
		}
	}
}

bool cci_broker::has_preset_value(const std::string &parname) const {
	if (sendToParent(parname)) {
		return m_parent.has_preset_value(parname);
	} else {
		const_cast<cci_broker*>(this)->insert_matching_preset_value(parname);
		return consuming_broker::has_preset_value(parname);
	}
}

cci_value cci_broker::get_preset_cci_value(const std::string &parname) const {
	if (sendToParent(parname)) {
		return m_parent.get_preset_cci_value(parname);
	} else {
		const_cast<cci_broker*>(this)->insert_matching_preset_value(parname);
		return consuming_broker::get_preset_cci_value(parname);
	}
}

cci_value cci_broker::get_cci_value(const std::string &parname,
		const cci::cci_originator& originator) const {
	if (sendToParent(parname)) {
		return m_parent.get_cci_value(parname);
	} else {
		return consuming_broker::get_cci_value(parname);
	}
}

void cci_broker::add_param(cci_param_if* par) {
	if (sendToParent(par->name())) {
		return m_parent.add_param(par);
	} else {
		return consuming_broker::add_param(par);
	}
}

void cci_broker::remove_param(cci_param_if* par) {
	if (sendToParent(par->name())) {
		return m_parent.remove_param(par);
	} else {
		return consuming_broker::remove_param(par);
	}
}

cci_param_untyped_handle cci_broker::get_param_handle(
		const std::string &parname,
		const cci_originator& originator) const {
	if (sendToParent(parname)) {
		return m_parent.get_param_handle(parname, originator);
	}
	cci_param_if* orig_param = get_orig_param(parname);
	if (orig_param) {
		return cci_param_untyped_handle(*orig_param, originator);
	}
	if (has_parent) {
		return m_parent.get_param_handle(parname, originator);
	}
	return cci_param_untyped_handle(originator);
}


std::vector<cci_param_untyped_handle>
cci_broker::get_param_handles(const cci_originator& originator) const {
	if (has_parent) {
		std::vector<cci_param_untyped_handle> p_param_handles=m_parent.get_param_handles();
		std::vector<cci_param_untyped_handle> param_handles=consuming_broker::get_param_handles(originator);
		param_handles.insert(param_handles.end(),p_param_handles.begin(), p_param_handles.end());
		return param_handles;
	} else {
		return consuming_broker::get_param_handles(originator);
	}
}

bool cci_broker::is_global_broker() const {
	return  (!has_parent);
}

void cci_broker::set_preset_cci_value(
		const std::string &parname,
		const cci_value &value,
		const cci_originator& originator) {
	if (sendToParent(parname)) {
		return m_parent.set_preset_cci_value(parname,value, originator);
	} else {
		try {
			if(parname.find_first_of("*?[")!=std::string::npos) {
				wildcard_presets.insert(std::pair<std::string, wildcard_entry>(parname,
						wildcard_entry{std::regex(util::glob_to_regex(parname)), value, originator}));
			} else if(parname[0]=='^') {
				wildcard_presets.insert(std::pair<std::string, wildcard_entry>(parname,
						wildcard_entry{std::regex(parname), value, originator}));
			} else
				consuming_broker::set_preset_cci_value(parname, value, originator);
		} catch (std::regex_error& e) {
			SCCERR()<<"Invalid preset parameter name '"<<parname<<"', "<<e.what();
		}
	}
}

void cci_broker::lock_preset_value(const std::string &parname) {
	if (sendToParent(parname)) {
		m_parent.lock_preset_value(parname);
	} else {
		try {
			if(parname.find_first_of("*?[")!=std::string::npos) {
				wildcard_locks.insert(std::pair<std::string, std::regex>(parname, std::regex(util::glob_to_regex(parname))));
			} else if(parname[0]=='^') {
				wildcard_locks.insert(std::pair<std::string, std::regex>(parname, std::regex(parname)));
			} else
				consuming_broker::lock_preset_value(parname);
		} catch (std::regex_error& e) {
			SCCERR()<<"Invalid preset parameter name '"<<parname<<"', "<<e.what();
		}
	}
}

void init_cci(std::string name) {
	thread_local cci_broker broker(name);
	cci::cci_register_broker(broker);
}
}
