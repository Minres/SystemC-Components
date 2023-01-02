/*****************************************************************************

Licensed to Accellera Systems Initiative Inc. (Accellera) under one or
more contributor license agreements. See the NOTICE file distributed
with this work for additional information regarding copyright ownership.
Accellera licenses this file to you under the Apache License, Version 2.0
(the "License"); you may not use this file except in compliance with the
License. You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
implied. See the License for the specific language governing
permissions and limitations under the License.

 ****************************************************************************/
#ifndef CCI_MEM_CCI_MEMORY_INTROSPECTION_HPP
#define CCI_MEM_CCI_MEMORY_INTROSPECTION_HPP

#include <vector>
#include <string>
#include <functional>

namespace cci_mem {

enum class memory_type{
	MEMORY = 0x1,
	REGISTER_BLOCK = 0x2,
	REGISTER = 0x4,
	BITFIELD = 0x8,
	ALL = 0xF
};

std::ostream& operator<<(std::ostream& os, memory_type);

struct cci_mem_memory_if {

	virtual ~cci_mem_memory_if() = default;

	virtual std::string const& get_name() const = 0;

	virtual std::string get_description() const = 0;

	virtual memory_type get_type() const = 0;

	virtual size_t get_size() const = 0;

	virtual size_t get_width() const = 0;

	virtual bool has_child() const = 0;

	virtual std::vector<cci_mem_memory_if*> get_children() const = 0;

	virtual bool has_parent() const = 0;

	virtual cci_mem_memory_if* get_parent() const = 0;

	virtual bool cci_mem_peek_da(unsigned char const * & data, size_t start, size_t len)  = 0;

	virtual size_t cci_mem_peek( unsigned char * const data, size_t start, size_t len, unsigned char const * const mask = nullptr ) const = 0;

	virtual size_t cci_mem_poke( unsigned char const * const data, size_t start, size_t len, unsigned char const * const mask = nullptr ) = 0;

	using  CBIF = std::function<void(cci_mem_memory_if&, size_t, size_t)>;

	virtual bool register_write_cb(CBIF, size_t start, size_t len) = 0;

	virtual bool register_read_cb(CBIF, size_t start, size_t len) = 0;

	virtual bool register_access_cb(CBIF, size_t start, size_t len) = 0;

	virtual bool register_modify_cb(CBIF, size_t start, size_t len) = 0;

	virtual bool unregister_write_callback(size_t start, size_t len) = 0;

	virtual bool unregister_read_callback(size_t start, size_t len) = 0;

	virtual bool unregister_access_callback(size_t start, size_t len) = 0;

	virtual bool unregister_modify_callback(size_t start, size_t len) = 0;
};


struct cci_mem_portal_if {

	virtual ~cci_mem_portal_if() = default;

	virtual std::string const & get_name() const = 0;

	virtual std::string get_description() const = 0;

	virtual cci_mem_memory_if*	get_memory(std::string const & name) const = 0;

	virtual std::vector<cci_mem_memory_if*> get_memories( memory_type type = memory_type::ALL ) const = 0;

	virtual std::vector<cci_mem_memory_if*> get_memories( cci_mem_memory_if const& scope, memory_type type = memory_type::ALL ) const = 0;

	virtual bool register_memory(cci_mem_memory_if & m, bool add_hierarchy = false ) = 0;

	virtual bool unregister_memory(cci_mem_memory_if & m, bool unregister_hierarchy = false ) = 0;
};

cci_mem_portal_if& get_cci_mem_portal();

} // namespace cci_mem
#endif // CCI_MEM_CCI_MEMORY_INTROSPECTION_HPP
