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

#include <scc/report.h>
#include <chrono>
#include <list>

#include "lwtr/lwtr.h"

using namespace sc_core;
using namespace sc_dt;

// hack to fake a true fifo_mutex
#define fifo_mutex sc_mutex

const unsigned ram_size = 256;

class rw_task_if : virtual public sc_interface {
public:
	using addr_t = sc_uint<8>;
	using data_t = sc_uint<8>;
	struct write_t {
		addr_t addr;
		data_t data;
		int blub() const { return 42;}
	};
	virtual data_t read(const addr_t *) = 0;
	virtual void write(const write_t *) = 0;
};

namespace lwtr {
template <class Archive>
void record(Archive &ar, rw_task_if::write_t const& u) {
	ar & field("addr", u.addr) & field("data", u.data) & field("blub", u.blub());
}
}

class pipelined_bus_ports : public sc_module {
public:
	sc_in<bool> clk;
	sc_inout<bool> rw;
	sc_inout<bool> addr_req;
	sc_inout<bool> addr_ack;
	sc_inout<sc_uint<8>> bus_addr;
	sc_inout<bool> data_rdy;
	sc_inout<sc_uint<8>> bus_data;

	SC_CTOR(pipelined_bus_ports)
	: clk("clk")
	, rw("rw")
	, addr_req("addr_req")
	, addr_ack("addr_ack")
	, bus_addr("bus_addr")
	, data_rdy("data_rdy")
	, bus_data("bus_data") {}
	void trace(sc_trace_file *tf) const override;
};

void pipelined_bus_ports::trace(sc_trace_file *tf) const {
	sc_trace(tf, clk, clk.name());
	sc_trace(tf, rw, rw.name());
	sc_trace(tf, addr_req, addr_req.name());
	sc_trace(tf, addr_ack, addr_ack.name());
	sc_trace(tf, bus_addr, bus_addr.name());
	sc_trace(tf, data_rdy, data_rdy.name());
	sc_trace(tf, bus_data, bus_data.name());
}

class rw_pipelined_transactor : public rw_task_if, public pipelined_bus_ports {

	fifo_mutex addr_phase;
	fifo_mutex data_phase;

	lwtr::tx_fiber pipelined_stream;
	lwtr::tx_fiber addr_stream;
	lwtr::tx_fiber data_stream;
	lwtr::tx_generator<sc_uint<8>, sc_uint<8>> read_gen;
	lwtr::tx_generator<write_t> write_gen;
	lwtr::tx_generator<sc_uint<8>> addr_gen;
	lwtr::tx_generator<lwtr::no_data, sc_uint<8>> rdata_gen;
	lwtr::tx_generator<sc_uint<8>> wdata_gen;

public:
	rw_pipelined_transactor(sc_module_name nm)
	: pipelined_bus_ports(nm)
	, addr_phase("addr_phase")
	, data_phase("data_phase")
	, pipelined_stream((std::string(name()) + ".pipelined_stream").c_str(), "transactor")
	, addr_stream((std::string(name()) + ".addr_stream").c_str(), "transactor")
	, data_stream((std::string(name()) + ".data_stream").c_str(), "transactor")
	, read_gen("read", pipelined_stream, "addr", "data")
	, write_gen("write", pipelined_stream)
	, addr_gen("addr", addr_stream, "addr")
	, rdata_gen("rdata", data_stream, nullptr, "data")
	, wdata_gen("wdata", data_stream, "data") {}
	data_t read(const addr_t *p_addr) override;
	void write(const write_t *req) override;
};

rw_task_if::data_t rw_pipelined_transactor::read(const addr_t *addr) {
	addr_phase.lock();
	auto h = read_gen.begin_tx(*addr);
	h.record_attribute("data_size", sizeof(data_t));
	auto h1 = addr_gen.begin_tx(*addr, "addr_phase", h);
	wait(clk->posedge_event());
	bus_addr = *addr;
	rw = false;
	addr_req = true;
	wait(addr_ack->posedge_event());
	wait(clk->negedge_event());
	addr_req = false;
	wait(addr_ack->negedge_event());
	addr_gen.end_tx(h1);
	addr_phase.unlock();

	data_phase.lock();
	auto h2 = rdata_gen.begin_tx("data_phase", h);
	wait(data_rdy->posedge_event());
	data_t data = bus_data.read();
	wait(data_rdy->negedge_event());
	rdata_gen.end_tx(h2, data);
	read_gen.end_tx(h, data);
	data_phase.unlock();

	return data;
}

void rw_pipelined_transactor::write(const write_t *req) {
	addr_phase.lock();
	auto h = write_gen.begin_tx(*req);
	h.record_attribute("data_size", sizeof(data_t));
	auto h1 = addr_gen.begin_tx(req->addr, "addr_phase", h);
	wait(clk->posedge_event());
	bus_addr = req->addr;
	rw = true;
	addr_req = true;
	wait(addr_ack->posedge_event());
	wait(clk->negedge_event());
	addr_req = false;
	wait(addr_ack->negedge_event());
	addr_gen.end_tx(h1);
	addr_phase.unlock();

	data_phase.lock();
	auto h2 = wdata_gen.begin_tx(req->data, "data_phase", h);
	bus_data = req->data;
	wait(data_rdy->posedge_event());
	wait(data_rdy->negedge_event());
	wdata_gen.end_tx(h2);
	write_gen.end_tx(h);
	data_phase.unlock();
}

class test : public sc_module {
public:
	sc_port<rw_task_if> transactor;
	SC_HAS_PROCESS(test);
	test(::sc_core::sc_module_name) {
		SC_THREAD(main1);
		SC_THREAD(main2);
	}
	void main1();
	void main2();
};

inline void test::main1() {
	// simple sequential tests
	for (int i = 0; i < 3; i++) {
		rw_task_if::addr_t addr = i;
		rw_task_if::data_t data = transactor->read(&addr);
		SCCINFO(sc_get_current_object()->name())  << "received data : " << data;
	}
	for (int i = 0; i < 3; i++) {
		rw_task_if::addr_t addr = rand()%256;
		rw_task_if::data_t data = transactor->read(&addr);
		SCCINFO(sc_get_current_object()->name()) << "data for address " << addr << " is " << data;
	}

	for (int i = 0; i < 3; i++) {
		rw_task_if::write_t write;
		write.addr = rand()%256;
		write.data = rand()%256;
		transactor->write(&write);
		SCCINFO(sc_get_current_object()->name()) << "send data : " << write.data;
	}
}

inline void test::main2() {
	// simple sequential tests
	for (int i = 0; i < 3; i++) {
		rw_task_if::addr_t addr = i;
		rw_task_if::data_t data = transactor->read(&addr);
		SCCINFO(sc_get_current_object()->name())  << "received data : " << data;
	}

	for (int i = 0; i < 3; i++) {
		rw_task_if::addr_t addr = rand()%256;
		rw_task_if::data_t data = transactor->read(&addr);
		SCCINFO(sc_get_current_object()->name()) << "data for address " << addr << " is " << data;
	}

	for (int i = 0; i < 3; i++) {
		rw_task_if::write_t write;
		write.addr = rand()%256;
		write.data = rand()%256;
		transactor->write(&write);
		SCCINFO(sc_get_current_object()->name()) << "send data : " << write.data;
	}
}
class design : public pipelined_bus_ports {
	std::list<sc_uint<8>> outstandingAddresses;
	std::list<bool> outstandingType;
	sc_uint<8> memory[ram_size];

public:
	SC_HAS_PROCESS(design);
	design(sc_module_name nm)
	: pipelined_bus_ports(nm) {
		for (unsigned i = 0; i < ram_size; ++i) {
			memory[i] = i;
		}
		SC_THREAD(addr_phase);
		SC_THREAD(data_phase);
	}
	void addr_phase();
	void data_phase();
};

inline void design::addr_phase() {
	while (true) {
		while (addr_req.read() != 1) {
			wait(addr_req->value_changed_event());
		}
		sc_uint<8> _addr = bus_addr.read();
		bool _rw = rw.read();

		int cycle = rand() % 10 + 1;
		while (cycle-- > 0) {
			wait(clk->posedge_event());
		}

		addr_ack = true;
		wait(clk->posedge_event());
		addr_ack = false;

		outstandingAddresses.push_back(_addr);
		outstandingType.push_back(_rw);
		SCCINFO(sc_get_current_object()->name())  << "received request for memory address " << _addr;
	}
}

inline void design::data_phase() {
	while (true) {
		while (outstandingAddresses.empty()) {
			wait(clk->posedge_event());
		}
		int cycle = rand() % 10 + 1;
		while (cycle-- > 0) {
			wait(clk->posedge_event());
		}
		if (outstandingType.front() == false) {
			SCCINFO(sc_get_current_object()->name()) << "reading memory address " << outstandingAddresses.front() << " with value "
					<< memory[outstandingAddresses.front().to_ulong()];
			bus_data = memory[outstandingAddresses.front().to_ulong()];
			data_rdy = true;
			wait(clk->posedge_event());
			data_rdy = false;

		} else {
			SCCINFO(sc_get_current_object()->name()) << "writing memory address " << outstandingAddresses.front() << " with value " << bus_data;
			memory[outstandingAddresses.front().to_ulong()] = bus_data;
			data_rdy = true;
			wait(clk->posedge_event());
			data_rdy = false;
		}
		outstandingAddresses.pop_front();
		outstandingType.pop_front();
	}
}


int sc_main(int argc, char *argv[]) {
	auto start = std::chrono::system_clock::now();

	scc::init_logging(scc::log::DEBUG);

	lwtr::tx_text_init();
	lwtr::tx_db db("my_db.txlog");
	lwtr::tx_db::set_default_db(&db);

	sc_trace_file *tf = sc_create_vcd_trace_file("my_db");
	// create signals
	sc_clock clk("clk", 20.0, SC_NS, 0.5, 0.0, SC_NS, true);
	sc_signal<bool> rw;
	sc_signal<bool> addr_req;
	sc_signal<bool> addr_ack;
	sc_signal<sc_uint<8>> bus_addr;
	sc_signal<bool> data_rdy;
	sc_signal<sc_uint<8>> bus_data;

	// create modules/channels
	test t("t");
	rw_pipelined_transactor tr("tr");
	design duv("duv");

	// connect them up
	t.transactor(tr);

	tr.clk(clk);
	tr.rw(rw);
	tr.addr_req(addr_req);
	tr.addr_ack(addr_ack);
	tr.bus_addr(bus_addr);
	tr.data_rdy(data_rdy);
	tr.bus_data(bus_data);
	tr.trace(tf);

	duv.clk(clk);
	duv.rw(rw);
	duv.addr_req(addr_req);
	duv.addr_ack(addr_ack);
	duv.bus_addr(bus_addr);
	duv.data_rdy(data_rdy);
	duv.bus_data(bus_data);
	duv.trace(tf);

	// Accellera SystemC >=2.2 got picky about multiple drivers.
	// Disable check for bus simulation.
	sc_report_handler::set_actions(sc_core::SC_ID_MORE_THAN_ONE_SIGNAL_DRIVER_, sc_core::SC_DO_NOTHING);
	// run the simulation
	sc_start(10.0, SC_US);
	sc_close_vcd_trace_file(tf);
	auto int_us = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now()-start);
	SCCINFO("sc_main") << "simulation duration "<<int_us.count()<<"µs";
	return 0;
}
