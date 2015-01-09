//  -*- C++ -*- <this line is for emacs to recognize it as C++ code>
/*****************************************************************************

 The following code is derived, directly or indirectly, from the SystemC
 source code Copyright (c) 1996-2014 by all Contributors.
 All Rights reserved.

 The contents of this file are subject to the restrictions and limitations
 set forth in the SystemC Open Source License (the "License");
 You may not use this file except in compliance with such restrictions and
 limitations. You may obtain instructions on how to receive a copy of the
 License at http://www.accellera.org/. Software distributed by Contributors
 under the License is distributed on an "AS IS" basis, WITHOUT WARRANTY OF
 ANY KIND, either express or implied. See the License for the specific
 language governing rights and limitations under the License.

 *****************************************************************************/
#include "scv.h"

// hack to fake a true fifo_mutex
#define fifo_mutex sc_mutex

const unsigned ram_size = 256;

class rw_task_if: virtual public sc_interface {
public:
    typedef sc_uint<8> addr_t;
    typedef sc_uint<8> data_t;
    struct write_t {
        addr_t addr;
        data_t data;
    };

    virtual data_t read(const addr_t*) = 0;
    virtual void write(const write_t*) = 0;
};

SCV_EXTENSIONS(rw_task_if::write_t){
public:
scv_extensions<rw_task_if::addr_t> addr;
scv_extensions<rw_task_if::data_t> data;
SCV_EXTENSIONS_CTOR(rw_task_if::write_t) {
    SCV_FIELD(addr);
    SCV_FIELD(data);
}
};

class pipelined_bus_ports: public sc_module {
public:
    sc_in<bool> clk;
    sc_inout<bool> rw;
    sc_inout<bool> addr_req;
    sc_inout<bool> addr_ack;
    sc_inout<sc_uint<8> > bus_addr;
    sc_inout<bool> data_rdy;
    sc_inout<sc_uint<8> > bus_data;

    SC_CTOR(pipelined_bus_ports) :
            clk("clk"), rw("rw"), addr_req("addr_req"), addr_ack("addr_ack"), bus_addr("bus_addr"), data_rdy("data_rdy"), bus_data(
                    "bus_data") {
    }
    virtual void trace( sc_trace_file* tf ) const;
};

void pipelined_bus_ports::trace( sc_trace_file* tf ) const {
    sc_trace(tf, clk, clk.name());
    sc_trace(tf, rw, rw.name());
    sc_trace(tf, addr_req, addr_req.name());
    sc_trace(tf, addr_ack, addr_ack.name());
    sc_trace(tf, bus_addr, bus_addr.name());
    sc_trace(tf, data_rdy, data_rdy.name());
    sc_trace(tf, bus_data, bus_data.name());
}

class rw_pipelined_transactor: public rw_task_if, public pipelined_bus_ports {

    fifo_mutex addr_phase;fifo_mutex data_phase;

    scv_tr_stream pipelined_stream;
    scv_tr_stream addr_stream;
    scv_tr_stream data_stream;
    scv_tr_generator<sc_uint<8>, sc_uint<8> > read_gen;
    scv_tr_generator<sc_uint<8>, sc_uint<8> > write_gen;
    scv_tr_generator<sc_uint<8> > addr_gen;
    scv_tr_generator<_scv_tr_generator_default_data, sc_uint<8> > rdata_gen;
    scv_tr_generator<sc_uint<8> > wdata_gen;

public:
    rw_pipelined_transactor(sc_module_name nm)
        : pipelined_bus_ports(nm)
        , addr_phase("addr_phase")
        , data_phase("data_phase")
        , pipelined_stream((std::string(name()) +".pipelined_stream").c_str(), "transactor")
        , addr_stream( (std::string(name()) +".addr_stream").c_str(), "transactor")
        , data_stream((std::string(name()) +".data_stream").c_str(), "transactor")
        , read_gen("read", pipelined_stream, "addr", "data")
        , write_gen("write", pipelined_stream, "addr", "data")
        , addr_gen("addr", addr_stream, "addr")
        , rdata_gen("rdata", data_stream, NULL, "data")
        , wdata_gen("wdata", data_stream, "data")
    {
    }
    virtual data_t read(const addr_t* p_addr);
    virtual void write(const write_t * req);
};

rw_task_if::data_t rw_pipelined_transactor::read(const addr_t* addr) {
    addr_phase.lock();
    scv_tr_handle h = read_gen.begin_transaction(*addr);
    h.record_attribute("data_size", sizeof(data_t));
    scv_tr_handle h1 = addr_gen.begin_transaction(*addr, "addr_phase", h);
    wait(clk->posedge_event());
    bus_addr = *addr;
    rw=false;
    addr_req = 1;
    wait(addr_ack->posedge_event());
    wait(clk->negedge_event());
    addr_req = 0;
    wait(addr_ack->negedge_event());
    addr_gen.end_transaction(h1);
    addr_phase.unlock();

    data_phase.lock();
    scv_tr_handle h2 = rdata_gen.begin_transaction("data_phase", h);
    wait(data_rdy->posedge_event());
    data_t data = bus_data.read();
    wait(data_rdy->negedge_event());
    rdata_gen.end_transaction(h2, data);
    read_gen.end_transaction(h, data);
    data_phase.unlock();

    return data;
}

void rw_pipelined_transactor::write(const write_t * req) {
    addr_phase.lock();
    scv_tr_handle h = write_gen.begin_transaction(req->addr);
    h.record_attribute("data_size", sizeof(data_t));
    scv_tr_handle h1 = addr_gen.begin_transaction(req->addr, "addr_phase", h);
    wait(clk->posedge_event());
    bus_addr = req->addr;
    rw=true;
    addr_req = 1;
    wait(addr_ack->posedge_event());
    wait(clk->negedge_event());
    addr_req = 0;
    wait(addr_ack->negedge_event());
    addr_gen.end_transaction(h1);
    addr_phase.unlock();

    data_phase.lock();
    scv_tr_handle h2 = wdata_gen.begin_transaction(req->data, "data_phase", h);
    bus_data=req->data;
    wait(data_rdy->posedge_event());
    wait(data_rdy->negedge_event());
    wdata_gen.end_transaction(h2);
    write_gen.end_transaction(h, req->data);
    data_phase.unlock();
}

class test: public sc_module {
public:
    sc_port<rw_task_if> transactor;
    SC_HAS_PROCESS(test);
    test( ::sc_core::sc_module_name ){
        SC_THREAD(main1);
        SC_THREAD(main2);
    }
    void main1();
    void main2();
};

class write_constraint: virtual public scv_constraint_base {
public:
    scv_smart_ptr<rw_task_if::write_t> write;SCV_CONSTRAINT_CTOR(write_constraint) {
        SCV_CONSTRAINT(write->addr() <= ram_size);
        SCV_CONSTRAINT(write->addr() != write->data());
    }
};

inline void process(scv_smart_ptr<int> data) {
}

inline void test::main1() {
    // simple sequential tests
    for (int i = 0; i < 3; i++) {
        rw_task_if::addr_t addr = i;
        rw_task_if::data_t data = transactor->read(&addr);
        cout << "at time " << sc_time_stamp() << ": ";
        cout << "received data : " << data << endl;
    }

    scv_smart_ptr<rw_task_if::addr_t> addr;
    for (int i = 0; i < 3; i++) {

        addr->next();
        rw_task_if::data_t data = transactor->read(addr->get_instance());
        cout << "data for address " << *addr << " is " << data << endl;
    }

    scv_smart_ptr<rw_task_if::write_t> write;
    for (int i = 0; i < 3; i++) {
        write->next();
        transactor->write(write->get_instance());
        cout << "send data : " << write->data << endl;
    }

    scv_smart_ptr<int> data;
    scv_bag<int> distribution;
    distribution.push(1, 40);
    distribution.push(2, 60);
    data->set_mode(distribution);
    for (int i = 0; i < 3; i++) {
        data->next();
        process(data);
    }
}

inline void test::main2() {
    // simple sequential tests
    for (int i = 0; i < 3; i++) {
        rw_task_if::addr_t addr = i;
        rw_task_if::data_t data = transactor->read(&addr);
        cout << "at time " << sc_time_stamp() << ": ";
        cout << "received data : " << data << endl;
    }

    scv_smart_ptr<rw_task_if::addr_t> addr;
    for (int i = 0; i < 3; i++) {

        addr->next();
        rw_task_if::data_t data = transactor->read(addr->get_instance());
        cout << "data for address " << *addr << " is " << data << endl;
    }

    scv_smart_ptr<rw_task_if::write_t> write;
    for (int i = 0; i < 3; i++) {
        write->next();
        transactor->write(write->get_instance());
        cout << "send data : " << write->data << endl;
    }

    scv_smart_ptr<int> data;
    scv_bag<int> distribution;
    distribution.push(1, 140);
    distribution.push(2, 160);
    data->set_mode(distribution);
    for (int i = 0; i < 3; i++) {
        data->next();
        process(data);
    }
}
class design: public pipelined_bus_ports {
    std::list<sc_uint<8> > outstandingAddresses;
    std::list<bool> outstandingType;
    sc_uint<8> memory[ram_size];

public:
    SC_HAS_PROCESS(design);
    design(sc_module_name nm) :
            pipelined_bus_ports(nm) {
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
    while (1) {
        while (addr_req.read() != 1) {
            wait(addr_req->value_changed_event());
        }
        sc_uint<8> _addr = bus_addr.read();
        bool _rw = rw.read();

        int cycle = rand() % 10 + 1;
        while (cycle-- > 0) {
            wait(clk->posedge_event());
        }

        addr_ack = 1;
        wait(clk->posedge_event());
        addr_ack = 0;

        outstandingAddresses.push_back(_addr);
        outstandingType.push_back(_rw);
        cout << "at time " << sc_time_stamp() << ": ";
        cout << "received request for memory address " << _addr << endl;
    }
}

inline void design::data_phase() {
    while (1) {
        while (outstandingAddresses.empty()) {
            wait(clk->posedge_event());
        }
        int cycle = rand() % 10 + 1;
        while (cycle-- > 0) {
            wait(clk->posedge_event());
        }
        if (outstandingType.front() == false) {
            cout << "reading memory address " << outstandingAddresses.front() << " with value "
                    << memory[outstandingAddresses.front().to_ulong()] << endl;
            bus_data = memory[outstandingAddresses.front().to_ulong()];
            data_rdy = 1;
            wait(clk->posedge_event());
            data_rdy = 0;

        } else {
            cout << "writing memory address " << outstandingAddresses.front() << " with value "
                    << bus_data << endl;
            memory[outstandingAddresses.front().to_ulong()]=bus_data;
            data_rdy = 1;
            wait(clk->posedge_event());
            data_rdy = 0;
        }
        outstandingAddresses.pop_front();
        outstandingType.pop_front();
    }
}
extern void scv_tr_sqlite_init();

int sc_main(int argc, char *argv[]) {
    scv_startup();

#if 0
    scv_tr_text_init();
    const char* fileName = "my_db.txlog";
#else
    scv_tr_sqlite_init();
    const char* fileName = "my_db";
#endif
    scv_tr_db db(fileName);
    scv_tr_db::set_default_db(&db);
    sc_trace_file* tf = sc_create_vcd_trace_file("my_db");
    // create signals
    sc_clock clk("clk", 20.0, SC_NS, 0.5, 0.0, SC_NS, true);
    sc_signal<bool> rw;
    sc_signal<bool> addr_req;
    sc_signal<bool> addr_ack;
    sc_signal<sc_uint<8> > bus_addr;
    sc_signal<bool> data_rdy;
    sc_signal<sc_uint<8> > bus_data;

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
    sc_report_handler::set_actions(SC_ID_MORE_THAN_ONE_SIGNAL_DRIVER_, SC_DO_NOTHING);
    // run the simulation
    sc_start(1.0, SC_MS);
    sc_close_vcd_trace_file(tf);
    return 0;
}

