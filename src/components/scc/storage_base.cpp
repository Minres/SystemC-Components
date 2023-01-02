/*******************************************************************************
 * Copyright 2023 MINRES Technologies GmbH
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

#include "storage_base.h"
#include <algorithm>
#include <sstream>

namespace scc {

storage_base::storage_base(std::string const&hier_name, std::string const& desc, cci_mem::memory_type type, bool en_callbacks)
: hier_name(hier_name)
, desc(desc)
, type(type)
, en_callbacks(en_callbacks)
{
	cci_mem::get_cci_mem_portal().register_memory(*this);
}

bool operator<(storage_base::cb_key const& a,  storage_base::cb_key const& b){
	if(a.start==b.start) return a.len<b.len;
	else return a.start<b.start;
}

template< typename T >
typename std::vector<T>::iterator insert_sorted( std::vector<T> & vec, T const& item ) {
	return vec.insert(std::upper_bound(std::begin(vec), std::end(vec), item), item);
}

template< typename T, typename Pred >
typename std::vector<T>::iterator insert_sorted( std::vector<T> & vec, T const& item, Pred pred ) {
	return vec.insert(std::upper_bound(std::begin(vec), std::end(vec), item, pred ), item);
}


bool storage_base::register_write_cb(CBIF cb, size_t start, size_t len) {
	if(en_callbacks) insert_sorted(wr_cb, {start, len, cb});
	return en_callbacks;
}

bool storage_base::register_read_cb(CBIF cb, size_t start,	size_t len) {
	if(en_callbacks) insert_sorted(rd_cb, {start, len, cb});
	return en_callbacks;
}

bool storage_base::register_access_cb(CBIF cb, size_t start, size_t len) {
	if(en_callbacks) insert_sorted(access_cb, {start, len, cb});
	return en_callbacks;
}

bool storage_base::register_modify_cb(CBIF cb, size_t start, size_t len) {
	if(en_callbacks) insert_sorted(modify_cb, {start, len, cb});
	return en_callbacks;
}

bool storage_base::unregister_write_callback(size_t start,	size_t len) {
	auto it = std::find_if(std::begin(wr_cb), std::end(wr_cb), [start, len](cb_key const& e)->bool { return e.start==start && e.len==len;});
	if(it!=std::end(wr_cb)) wr_cb.erase(it);
	return it!=std::end(wr_cb);
}

bool storage_base::unregister_read_callback(size_t start, size_t len) {
	auto it = std::find_if(std::begin(rd_cb), std::end(rd_cb), [start, len](cb_key const& e)->bool { return e.start==start && e.len==len;});
	if(it!=std::end(rd_cb)) rd_cb.erase(it);
	return it!=std::end(rd_cb);
}

bool storage_base::unregister_access_callback(size_t start, size_t len) {
	auto it = std::find_if(std::begin(access_cb), std::end(access_cb), [start, len](cb_key const& e)->bool { return e.start==start && e.len==len;});
	if(it!=std::end(access_cb)) access_cb.erase(it);
	return it!=std::end(access_cb);
}

bool storage_base::unregister_modify_callback(size_t start, size_t len) {
	auto it = std::find_if(std::begin(modify_cb), std::end(modify_cb), [start, len](cb_key const& e)->bool { return e.start==start && e.len==len;});
	if(it!=std::end(modify_cb)) modify_cb.erase(it);
	return it!=std::end(modify_cb);
}

std::string storage_base::get_hier_name(const sc_core::sc_module_name& basename_) {
	auto* simc = sc_core::sc_get_curr_simcontext();
	auto* curr_module = simc->hierarchy_curr();
	auto name = curr_module != 0? curr_module->gen_unique_name( basename_, true ):
		simc->gen_unique_name( basename_, true );
    auto parent = simc->active_object();
	std::ostringstream oss;
    oss << parent->name() << sc_core::SC_HIERARCHY_CHAR<<name;
    return oss.str();
}

void storage_base::invoke_cb(std::vector<cb_key> const& cbs, size_t start, size_t len) const {
	auto begin = std::find_if(std::begin(cbs), std::end(cbs), [start, len](const cb_key &e) -> bool {
		return e.start<=start && (start+len)<=(e.start+e.len);
	});
	for(; begin!=std::end(cbs) && begin->start<=start && (start+len)<<(begin->start+begin->len); ++begin){
		begin->cb(*const_cast<storage_base*>(this), start, len);
	}
}
}
