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
namespace tlm {
namespace scc {
namespace tcp {
namespace impl {

template <typename REQ, typename RESP> class Client {
public:
    /**
     * constructor
     */
    Client();
    /**
     * destructor needed as the class has virtual functions
     */
    virtual ~Client();
    /**
     * stores the connection details
     * @param io_service
     * @param host
     * @param portNr
     */
    void configure(std::string host, unsigned portNr) {
        this->host = host;
        this->port = portNr;
    }
    /**
     * connects to the server, tries to do so for retryCount*50ms (5sec by default)
     */
    void connect();
    /**
     * checks if the client is connected
     * @return
     */
    bool isClientConnected() { return is_connected; }
    /** returns the connection object to the server
     *
     * @return
     */
    connection<REQ, RESP>& clientConnection(unsigned short retry_count = 0) {
        // try first connect
        if(!is_connected)
            connect();
        // if not connected && retry count
        while(!is_connected && retry_count > 0) {
            LOG(INFO) << "retrying connection for " << retry_count-- << " times";
            //            boost::this_thread::sleep_for(boost::chrono::milliseconds(100));
            connect();
        }
        if(!is_connected)
            throw std::runtime_error("Retry count exceeded!");
        return *trace_conn;
    }

protected:
    std::string host;
    unsigned short port;
    unsigned short retryCount;
    boost::asio::io_context io_service;
    typename connection<REQ, RESP>::ptr trace_conn;
    bool is_connected;
    bool is_local;

#ifdef BOOST_ASIO_HAS_LOCAL_SOCKETS
    void setupEndpoint(boost::asio::local::stream_protocol::endpoint& ep) {
        if(port) {
            std::string s("/tmp/server");
            s += boost::lexical_cast<std::string>(port ? port : getpid());
            ep = boost::asio::local::stream_protocol::endpoint(s);
        } else {
            ep = boost::asio::local::stream_protocol::endpoint(host);
        }
    }
#endif

    void setupEndpoint(boost::asio::ip::tcp::endpoint& ep) {
        using boost::asio::ip::tcp;
        tcp::resolver resolver(io_service);
        auto results = resolver.resolve(tcp::v4(), host, boost::lexical_cast<std::string>(port));
        ep = results.begin()->endpoint();
    }

    void setupEndpoint(boost::asio::ip::udp::endpoint& ep) {
        using boost::asio::ip::udp;
        udp::resolver resolver(io_service);
        auto results = resolver.resolve(udp::v4(), host, boost::lexical_cast<std::string>(port));
        ep = results.begin()->endpoint();
    }

    bool isLocal(boost::asio::ip::tcp::socket& socket) { return socket.local_endpoint().address() == socket.remote_endpoint().address(); }

    bool isLocal(boost::asio::ip::udp::socket& socket) { return socket.local_endpoint().address() == socket.remote_endpoint().address(); }

#ifdef BOOST_ASIO_HAS_LOCAL_SOCKETS
    bool isLocal(boost::asio::local::stream_protocol::socket& socket) { return true; }
#endif
};

template <typename REQ, typename RESP>
Client<REQ, RESP>::Client()
: host("localhost")
, port(0)
, retryCount(100)
, io_service()
, trace_conn(new connection<REQ, RESP>(io_service))
, is_connected(false) {}

template <typename REQ, typename RESP> Client<REQ, RESP>::~Client() {
    if(trace_conn->socket().is_open()) {
        trace_conn->socket().close();
    }
}

template <typename REQ, typename RESP> void Client<REQ, RESP>::connect() {
    LOG(INFO) << "connect for port " << port << " on host '" << host << "'";
    typename connection<REQ, RESP>::endpoint_t ep;
    typename connection<REQ, RESP>::socket_t& socket = trace_conn->socket();
    setupEndpoint(ep);
    boost::system::error_code ec;
    for(unsigned i = 0; i < retryCount; ++i) {
        socket.connect(ep, ec);
        if(!ec)
            break;
        if(i % 10 == 0) {
            LOG(INFO) << "retrying connection for " << i << " times with code " << ec;
        }
        boost::this_thread::sleep_for(boost::chrono::milliseconds(250));
    }
    if(ec) {
        is_connected = false;
        is_local = false;
    } else {
        is_connected = true;
        boost::asio::ip::tcp::no_delay option(true);
        socket.set_option(option);
        is_local = isLocal(socket);
    }
}
} // namespace impl
} // namespace tcp
} // namespace scc
} // namespace tlm

#endif // TLM_SCC_TCP_DETAIL_CLIENT_H_
