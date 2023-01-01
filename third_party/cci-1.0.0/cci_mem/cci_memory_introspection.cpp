#include "cci_memory_introspection.hpp"
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <iostream>

namespace cci_mem {

struct cci_mem_portal : public cci_mem_portal_if {
	cci_mem_portal();
	virtual ~cci_mem_portal() = default;
	const std::string& get_name() const override;
	std::string get_description() const override;
	cci_mem_memory_if*	get_memory(std::string const & name) const override;
	std::vector<cci_mem_memory_if*> get_memories( memory_type type = memory_type::ALL ) const override;
	std::vector<cci_mem_memory_if*> get_memories( cci_mem_memory_if const& scope, memory_type type = memory_type::ALL ) const override;
	bool register_memory(cci_mem_memory_if& m, bool add_hierarchy = false ) override;
	bool unregister_memory(cci_mem_memory_if& m, bool unregister_hierarchy = false ) override;
	std::vector<cci_mem_memory_if*> memories;
	std::vector<std::string> memory_names;
	std::unordered_map<std::string, cci_mem_memory_if*> mem_by_name;
	const std::string name{"cci_mem::portal"};
};

cci_mem::cci_mem_portal::cci_mem_portal() {
}

const std::string& cci_mem::cci_mem_portal::get_name() const {
	return name;
}

std::string cci_mem::cci_mem_portal::get_description() const {
	return "A generic implementation of the cci_mem::cci_mem_portal_if interface";
}

cci_mem_memory_if* cci_mem::cci_mem_portal::get_memory(const std::string &name) const {
	auto it  = std::find(std::begin(memory_names), std::end(memory_names), name);
	if(it==memory_names.end()) return nullptr;
	return memories[std::distance(std::begin(memory_names), it)];
}

std::vector<cci_mem_memory_if*> cci_mem::cci_mem_portal::get_memories(memory_type type) const {
	if(type==memory_type::ALL) return memories;
	std::vector<cci_mem_memory_if*> result;
	std::copy_if(std::begin(memories), std::end(memories), std::back_inserter(result), [type](cci_mem_memory_if* m)->bool {
		return m->get_type()==type;
	});
	return result;
}

std::vector<cci_mem_memory_if*> cci_mem::cci_mem_portal::get_memories(const cci_mem_memory_if &scope, memory_type type) const {
	std::vector<cci_mem_memory_if*> result;
	auto scope_name = scope.get_name();
	std::copy_if(std::begin(memories), std::end(memories), std::back_inserter(result), [scope_name, type](cci_mem_memory_if* m)->bool {
		return m->get_type()==type && m->get_name().rfind(scope_name, 0)==0;
	});
	return result;
}

bool cci_mem::cci_mem_portal::register_memory(cci_mem_memory_if &m,	bool add_hierarchy) {
	bool result=true;
	memories.push_back(&m);
	memory_names.emplace_back(m.get_name());
	if(add_hierarchy) {
		for(auto& mm:m.get_children()){
			result &= register_memory(*mm, add_hierarchy);
		}
	}
	return result;
}

bool cci_mem::cci_mem_portal::unregister_memory(cci_mem_memory_if &m, bool unregister_hierarchy) {
	bool result=true;
	auto it  = std::find(std::begin(memories), std::end(memories), &m);
	memory_names.erase(std::begin(memory_names)+std::distance(std::begin(memories), it));
	memories.erase(it);
	if(unregister_hierarchy) {
		for(auto& mm:m.get_children()){
			result &= unregister_memory(*mm, unregister_hierarchy);
		}
	}
	return result;
}

std::ostream& operator <<(std::ostream &os, memory_type memory_type) {
	switch(memory_type){
	case memory_type::MEMORY: os<<"MEMORY"; break;
	case memory_type::REGISTER_BLOCK: os<<"REGISTER_BLOCK"; break;
	case memory_type::REGISTER: os<<"REGISTER"; break;
	case memory_type::BITFIELD: os<<"BITFIELD"; break;
	case memory_type::ALL: os<<"ALL"; break;
	default:
		os<<"(ILLEGAL)";
	}
	return os;
}

cci_mem_portal_if& get_cci_mem_portal() {
	static cci_mem_portal portal;
	return portal;
}

}

