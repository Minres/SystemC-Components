/*******************************************************************************
 * Copyright 2026 MINRES Technologies GmbH
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

#ifndef TLM_SCC_TCP_DETAIL_CLIENT_H_
#define TLM_SCC_TCP_DETAIL_CLIENT_H_

#include "serialized_connection.h"
#include <boost/asio.hpp>
#include <boost/lexical_cast.hpp>
#include <thread>

namespace scc {
namespace tcp4tlm {

template <typename REQ, typename RESP> class client {
public:
    client();
    virtual ~client();

    void configure(std::string host, unsigned port_nr) {
        this->host = host;
        this->port = port_nr;
    }

    void connect();

    bool is_remote_connected() { return is_connected; }

    connection<REQ, RESP>& client_connection(unsigned short retry_count = 0) {
        if(!is_connected)
            connect();
        while(!is_connected && retry_count > 0) {
            CPPLOG(INFO, "tcp4tlm::client") << "retrying connection for " << retry_count-- << " times";
            connect();
        }
        if(!is_connected)
            throw std::runtime_error("Retry count exceeded!");
        return *trace_conn;
    }

protected:
    std::string host;
    unsigned short port;
    unsigned short retry_count;
    boost::asio::io_context io_service;
    typename connection<REQ, RESP>::ptr trace_conn;
    std::atomic<bool> is_connected;
    bool local_connection;

#ifdef BOOST_ASIO_HAS_LOCAL_SOCKETS
    void setup_endpoint(boost::asio::local::stream_protocol::endpoint& ep) {
        if(port) {
            std::string s("/tmp/server");
            s += boost::lexical_cast<std::string>(port ? port : getpid());
            ep = boost::asio::local::stream_protocol::endpoint(s);
        } else {
            ep = boost::asio::local::stream_protocol::endpoint(host);
        }
    }
#endif

    void setup_endpoint(boost::asio::ip::tcp::endpoint& ep) {
        using boost::asio::ip::tcp;
        tcp::resolver resolver(io_service);
        auto results = resolver.resolve(tcp::v4(), host, boost::lexical_cast<std::string>(port));
        ep = results.begin()->endpoint();
    }

    void setup_endpoint(boost::asio::ip::udp::endpoint& ep) {
        using boost::asio::ip::udp;
        udp::resolver resolver(io_service);
        auto results = resolver.resolve(udp::v4(), host, boost::lexical_cast<std::string>(port));
        ep = results.begin()->endpoint();
    }

    bool is_local_socket(boost::asio::ip::tcp::socket& socket) {
        return socket.local_endpoint().address() == socket.remote_endpoint().address();
    }

    bool is_local_socket(boost::asio::ip::udp::socket& socket) {
        return socket.local_endpoint().address() == socket.remote_endpoint().address();
    }

#ifdef BOOST_ASIO_HAS_LOCAL_SOCKETS
    bool is_local_socket(boost::asio::local::stream_protocol::socket& socket) { return true; }
#endif
};

template <typename REQ, typename RESP>
client<REQ, RESP>::client()
: host("localhost")
, port(0)
, retry_count(100)
, io_service()
, trace_conn(new connection<REQ, RESP>(io_service))
, is_connected(false)
, local_connection(false) {}

template <typename REQ, typename RESP> client<REQ, RESP>::~client() {
    if(trace_conn->socket().is_open()) {
        trace_conn->socket().close();
    }
}

template <typename REQ, typename RESP> void client<REQ, RESP>::connect() {
    CPPLOG(INFO, "tcp4tlm::client") << "connect for port " << port << " on host '" << host << "'";
    typename connection<REQ, RESP>::endpoint_t ep;
    typename connection<REQ, RESP>::socket_t& socket = trace_conn->socket();
    setup_endpoint(ep);
    boost::system::error_code ec;
    for(unsigned i = 0; i < retry_count; ++i) {
        socket.connect(ep, ec);
        if(!ec)
            break;
        if(i % 10 == 0) {
            CPPLOG(INFO, "tcp4tlm::client") << "retrying connection for " << i << " times with code " << ec;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(250));
    }
    if(ec) {
        is_connected = false;
        local_connection = false;
    } else {
        is_connected = true;
        boost::asio::ip::tcp::no_delay option(true);
        socket.set_option(option);
        local_connection = is_local_socket(socket);
    }
}

} // namespace tcp4tlm
} // namespace scc

#endif // TLM_SCC_TCP_DETAIL_CLIENT_H_
