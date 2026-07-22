/*
 * sc_main.cpp
 *
 *  Created on: Jul 9, 2014
 *      Author: ejentzsx
 */

#include "scc/report.h"
#include "top_client.h"
#include "top_server.h"
#include <boost/asio.hpp>
#include <boost/program_options.hpp>
#include <cstdlib>
#include <memory>
#include <sysc/kernel/sc_module.h>
#include <systemc.h>
#include <util/logging.h>

namespace po = boost::program_options;

int sc_main(int argc, char* argv[]) {
    unsigned port_nr = 1024;
    unsigned num_clients = 1;
    unsigned other_port_nr = 0;
    size_t base_addr = 0;
    std::string other_host_name;

    po::options_description desc("Options");
    // clang-format off
    desc.add_options()
        ("help,h", "Print help message")
        ("no-systemc-sync", "Disable SystemC synchronization")
        ("remote-host", po::value<std::string>(&other_host_name), "Host name to connect to")
        ("remote-port", po::value<unsigned>(&other_port_nr), "Port to connect to")
        ("local-port", po::value<unsigned>(&port_nr)->default_value(port_nr), "Local host port")
        ("num-clients", po::value<unsigned>(&num_clients)->default_value(num_clients), "Local host port")
        ("base-addr", po::value<size_t>(&base_addr)->default_value(0), "base address of client accesses");
    // clang-format on

    po::variables_map vm;
    try {
        po::store(po::command_line_parser(argc, argv).options(desc).run(), vm);
        if(vm.count("help")) {
            std::cout << desc << std::endl;
            return EXIT_SUCCESS;
        }
        po::notify(vm);
    } catch(const po::error& e) {
        std::cerr << "ERROR: " << e.what() << std::endl << std::endl;
        std::cerr << desc << std::endl;
        return EXIT_FAILURE;
    }
    scc::init_logging(scc::log::TRACEALL);
    LOGGER(DEFAULT)::set_reporting_level(logging::TRACEALL);

    std::unique_ptr<sc_core::sc_module> tb;
    if(other_host_name.empty()) {
        auto* srv = new tcp4tlm_bridge::top_server("top_srv", num_clients, port_nr);
        tb.reset(srv);
    } else {
        auto* client = new tcp4tlm_bridge::top_client("top_client");
        client->bridge.this_host_port = port_nr;
        client->bridge.other_host_name = other_host_name;
        client->intor.base_addr = base_addr;
        if(other_port_nr) {
            client->bridge.other_host_port.set_value(other_port_nr);
        }
        tb.reset(client);
    }
    try {
        sc_start(SC_ZERO_TIME);
    } catch(std::exception& e) {
        SCCERR() << e.what();
    }

    try {
        sc_start();
    } catch(std::exception& e) {
        SCCERR() << e.what();
    }
    // call sc_stop if time is exhausted
    if(sc_get_status() != sc_core::SC_STOPPED) {
        sc_stop();
        SCCINFO() << "time out occured @" << sc_time_stamp() << "sec!";
        return EXIT_FAILURE;
    } else {
        SCCINFO() << "FINISHED @" << sc_time_stamp() << "!";
        return EXIT_SUCCESS;
    }
}
