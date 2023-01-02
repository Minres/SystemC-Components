////////////////////////////////////////////////////////////////////////////////
// Copyright 2017 eyck@minres.com
//
// Licensed under the Apache License, Version 2.0 (the "License"); you may not
// use this file except in compliance with the License.  You may obtain a copy
// of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
// WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
// License for the specific language governing permissions and limitations under
// the License.
////////////////////////////////////////////////////////////////////////////////
/*
 * sc_main.cpp
 *
 *  Created on: 17.09.2017
 *      Author: eyck@minres.com
 */

#include "simple_system.h"
#include <scc/report.h>
#include <scc/tracer.h>
#include <boost/program_options.hpp>

using namespace sysc;
using namespace scc;
namespace po = boost::program_options;

namespace {
const size_t ERROR_IN_COMMAND_LINE = 1;
const size_t SUCCESS = 0;
const size_t ERROR_UNHANDLED_EXCEPTION = 2;
} // namespace


void read_cb(cci_mem::cci_mem_memory_if& m, size_t, size_t){SCCINFO(m.get_name())<<"read callback triggered";}
void write_cb(cci_mem::cci_mem_memory_if& m, size_t, size_t){SCCINFO(m.get_name())<<"write callback triggered";}
void access_cb(cci_mem::cci_mem_memory_if& m, size_t, size_t){SCCINFO(m.get_name())<<"access callback triggered";}
void modify_cb(cci_mem::cci_mem_memory_if& m, size_t, size_t){SCCINFO(m.get_name())<<"modify callback triggered";}
int sc_main(int argc, char *argv[]) {
	sc_core::sc_report_handler::set_actions( "/IEEE_Std_1666/deprecated", sc_core::SC_DO_NOTHING );
	sc_core::sc_report_handler::set_actions(sc_core::SC_ID_MORE_THAN_ONE_SIGNAL_DRIVER_, sc_core::SC_DO_NOTHING);
	///////////////////////////////////////////////////////////////////////////
	// CLI argument parsing
	///////////////////////////////////////////////////////////////////////////
	po::options_description desc("Options");
	// clang-format off
	desc.add_options()
    								("help,h",  "Print help message")
									("debug,d", "set debug level")
									("trace,t", "trace SystemC signals");
	// clang-format on
	po::variables_map vm;
	try {
		po::store(po::parse_command_line(argc, argv, desc), vm); // can throw
		// --help option
		if (vm.count("help")) {
			std::cout << "JIT-ISS simulator for AVR" << std::endl << desc << std::endl;
			return SUCCESS;
		}
		po::notify(vm); // throws on error, so do after help in case
		// there are any problems
	} catch (po::error &e) {
		std::cerr << "ERROR: " << e.what() << std::endl << std::endl;
		std::cerr << desc << std::endl;
		return ERROR_IN_COMMAND_LINE;
	}
	///////////////////////////////////////////////////////////////////////////
	// configure logging
	///////////////////////////////////////////////////////////////////////////
	scc::init_logging(vm.count("debug")?scc::log::DEBUG:scc::log::INFO);
	///////////////////////////////////////////////////////////////////////////
	// set up tracing & transaction recording
	///////////////////////////////////////////////////////////////////////////
	tracer trace("simple_system", vm.count("trace")?tracer::COMPRESSED:tracer::NONE, vm.count("trace"));
	///////////////////////////////////////////////////////////////////////////
	// instantiate top level
	///////////////////////////////////////////////////////////////////////////
	simple_system i_simple_system("i_simple_system");
	///////////////////////////////////////////////////////////////////////////
	// run simulation
	///////////////////////////////////////////////////////////////////////////
	auto& portal = cci_mem::get_cci_mem_portal();
	{
		auto memories = portal.get_memories(cci_mem::memory_type::MEMORY);
		std::ostringstream oss;
		for(auto m:memories) oss<<"\t"<<m->get_name()<<"|"<<m->get_type()<<"|"<<m->get_description()<<"|"<<m->get_size()<<"|"<<m->get_width()<<"\n";
		SCCINFO()<<"Found memories:\n"<<oss.str();
	}
	{
		auto memories = portal.get_memories(cci_mem::memory_type::REGISTER_BLOCK);
		std::ostringstream oss;
		for(auto m:memories) oss<<"\t"<<m->get_name()<<"|"<<m->get_type()<<"|"<<m->get_description()<<"|"<<m->get_size()<<"|"<<m->get_width()<<"\n";
		SCCINFO()<<"Found register files:\n"<<oss.str();
	}
	{
		auto memories = portal.get_memories(cci_mem::memory_type::REGISTER);
		std::ostringstream oss;
		for(auto m:memories) oss<<"\t"<<m->get_name()<<"|"<<m->get_type()<<"|"<<m->get_description()<<"|"<<m->get_size()<<"|"<<m->get_width()<<"\n";
		SCCINFO()<<"Found registers:\n"<<oss.str();
	}

	auto * txdata = portal.get_memory("i_simple_system.uart.regs.txdata");
	if(txdata) {
		txdata->register_read_cb(&read_cb, 0, 4);
		txdata->register_write_cb(&write_cb, 0, 4);
		txdata->register_access_cb(access_cb, 0, 4);
		txdata->register_modify_cb(modify_cb, 0, 4);
	}
	auto * rxdata = portal.get_memory("i_simple_system.uart.regs.rxdata");
	if(rxdata) {
		rxdata->register_write_cb([txdata](cci_mem::cci_mem_memory_if& m, size_t, size_t){
			std::array<uint8_t, 4> data;
			m.cci_mem_peek(data.data(), 0, 4);
			txdata->cci_mem_poke(data.data(), 0, 4);
		}, 0, 4);
	}
	auto * mem = portal.get_memory("i_simple_system.mem");
	if(mem) {
		mem->register_read_cb([](cci_mem::cci_mem_memory_if& m, size_t start, size_t len){
			SCCINFO(m.get_name())<<"read callback triggered for address 0x"<<start<<" with length "<<len;
		}, 0, 1_kB);
		mem->register_write_cb([](cci_mem::cci_mem_memory_if& m, size_t start, size_t len){
			SCCINFO(m.get_name())<<"write callback triggered for address 0x"<<start<<" with length "<<len;
		}, 0, 1_kB);
		mem->register_access_cb([](cci_mem::cci_mem_memory_if& m, size_t start, size_t len){
			SCCINFO(m.get_name())<<"access callback triggered for address 0x"<<start<<" with length "<<len;
		}, 0, 1_kB);
		mem->register_modify_cb([](cci_mem::cci_mem_memory_if& m, size_t start, size_t len){
			SCCINFO(m.get_name())<<"modify callback triggered for address 0x"<<start<<" with length "<<len;
		}, 0, 1_kB);
	}
	auto * rd = portal.get_memory("i_simple_system.uart.regs.priority");
	if(rd) {
		if(!rd->register_read_cb(&read_cb, 0, rd->get_size()))
			for(auto* r:portal.get_memories(*rd))
				r->register_read_cb(&read_cb, 0, 4);
		if(!rd->register_write_cb(&read_cb, 0, rd->get_size()))
			for(auto* r:portal.get_memories(*rd))
				r->register_write_cb(&write_cb, 0, 4);
		if(!rd->register_access_cb(&read_cb, 0, rd->get_size()) && rd->has_child())
			for(auto* r:rd->get_children())
				r->register_access_cb(&access_cb, 0, 4);
		if(!rd->register_modify_cb(&read_cb, 0, rd->get_size()) && rd->has_child())
			for(auto* r:rd->get_children())
				r->register_modify_cb(&modify_cb, 0, 4);
	}


	sc_start(sc_core::sc_time(1, sc_core::SC_MS));

	if (!sc_core::sc_end_of_simulation_invoked()) {
		SCCERR() << "simulation timed out";
		sc_core::sc_stop();
	}
	return SUCCESS;
}
