/*
 * cci_memory_base.h
 *
 *  Created on: Jan 1, 2023
 *      Author: eyck
 */

#ifndef _SCC_CCI_MEMORY_BASE_H_
#define _SCC_CCI_MEMORY_BASE_H_

#include <cci_mem/cci_memory_introspection.hpp>
#include <systemc>
#include <map>

namespace scc {

struct storage_base: public cci_mem::cci_mem_memory_if {

	storage_base(std::string const& hier_name, std::string const& desc, cci_mem::memory_type type);

	const std::string& get_name() const override { return hier_name;}

    std::string get_description() const override {return desc;}

	cci_mem::memory_type get_type() const override { return type; }

	bool register_write_cb(CBIF, size_t start, size_t len) override;

	bool register_read_cb(CBIF, size_t start, size_t len) override;

	bool register_access_cb(CBIF, size_t start, size_t len) override;

	bool register_modify_cb(CBIF, size_t start, size_t len) override;

	bool unregister_write_callback(size_t start, size_t len) override;

	bool unregister_read_callback(size_t start, size_t len) override;

	bool unregister_access_callback(size_t start, size_t len) override;

	bool unregister_modify_callback(size_t start, size_t len) override;

	static std::string get_hier_name(sc_core::sc_module_name const&);

protected:
	struct cb_key { size_t start; size_t len; CBIF cb;};
	void invoke_wr_cb(size_t start, size_t len, bool changed){
		invoke_cb(wr_cb, start, len);
		invoke_cb(access_cb, start, len);
		if(changed) invoke_cb(modify_cb, start, len);
	}
	void invoke_rd_cb(size_t start, size_t len){
		invoke_cb(rd_cb, start, len);
		invoke_cb(access_cb, start, len);
	}
	friend bool operator<(cb_key const& a, cb_key const& b);
	friend bool operator==(cb_key const& a, cb_key const& b);
	std::vector<cb_key> rd_cb, wr_cb, access_cb, modify_cb;
private:
	std::string const hier_name;
	std::string const desc;
	cci_mem::memory_type const type;
	void invoke_cb(std::vector<cb_key> const& cbs, size_t start, size_t len);
};
}
#endif /* _SCC_CCI_MEMORY_BASE_H_ */
