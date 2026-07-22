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

#ifndef TLM_SCC_TCP4TLM_SERVER
#define TLM_SCC_TCP4TLM_SERVER

#include "i_server.h"
#include <boost/lexical_cast.hpp>
#include <boost/thread.hpp>
#include <util/logging.h>

namespace scc {
namespace tcp4tlm {

template <typename REQ, typename RESP> class server : public i_server<REQ, RESP> {
    typedef typename std::shared_ptr<connection<RESP, REQ>> con_ptr;
    typedef typename connection<RESP, REQ>::async_listener con_listener;

    struct forward_session : public con_listener {
        forward_session(i_server<REQ, RESP>* server_)
        : server_instance(server_)
        , conn_shptr(new connection<RESP, REQ>(server_instance->get_io_service())) {}

        virtual ~forward_session() {}

        con_ptr& get_connection() { return conn_shptr; }

        bool start() {
            std::shared_ptr<con_listener> ptr = std::enable_shared_from_this<con_listener>::shared_from_this();
            conn_shptr->add_listener(ptr);
            typename connection<RESP, REQ>::endpoint_t endpoint = conn_shptr->socket().remote_endpoint();
            CPPLOG(TRACE, "tcp4tlm::server") << "forward_session::start(), got connected";
            server_instance->server_send_completed(conn_shptr, true);
            return server_instance->is_shutdown_requested();
        }

        void receive_completed(const boost::system::error_code& e, const REQ* const data) {
            if(e.value() == 2) {
                CPPLOG(WARN, "tcp4tlm::server") << "Client closed connection (" << e.message() << ")";
                return;
            } else if(e) {
                CPPLOG(ERR, "tcp4tlm::server") << "Communication error (" << e.message() << ")";
                return;
            }
            server_instance->server_receive_completed(conn_shptr, data);
        }

        void send_completed(const boost::system::error_code& e) {
            if(!e) {
                server_instance->server_send_completed(conn_shptr);
            } else {
                CPPLOG(ERR, "tcp4tlm::server") << e.message() << "(" << e << ")";
            }
        }

    private:
        i_server<REQ, RESP>* server_instance;
        con_ptr conn_shptr;
    };

    friend struct forward_session;

public:
    server(std::size_t thread_pool_size = 4)
    : thread_pool_size(thread_pool_size)
    , io_service()
    , io_service_work()
    , acceptor()
    , server_running(false)
    , shutdown_requested(false) {}

    virtual ~server() {}

#ifdef BOOST_ASIO_HAS_LOCAL_SOCKETS
    long set_acceptor_endpoint(std::shared_ptr<boost::asio::local::stream_protocol::acceptor>& a, unsigned short port);
#endif
    long set_acceptor_endpoint(std::shared_ptr<boost::asio::ip::tcp::acceptor>& a, unsigned short port);

    void start_server(unsigned short port, char* name = NULL);

    void request_shutdown() {
        if(!shutdown_requested) {
            shutdown_requested = true;
            io_service.stop();
        }
    }

    bool is_shutdown_requested() { return shutdown_requested; }

    void shutdown_server();

    virtual boost::asio::io_context& get_io_service() { return io_service; }

    bool is_server_running() { return server_running; }

    void create_new_session() {
        std::shared_ptr<forward_session> forward_session_ptr(new forward_session(this));
        acceptor->async_accept(
            forward_session_ptr->get_connection()->socket(),
            [this, forward_session_ptr](const boost::system::error_code& error) { handle_accept(error, forward_session_ptr); });
    }

protected:
    const typename connection<RESP, REQ>::acceptor_t& get_acceptor() { return *acceptor; };

private:
    void handle_accept(const boost::system::error_code& e, std::shared_ptr<forward_session> session) {
        if(!e) {
            if(!session->start())
                create_new_session();
        } else {
            CPPLOG(ERR, "tcp4tlm::server") << e.message();
        }
    }

    std::size_t thread_pool_size;
    boost::asio::io_context io_service;
    std::shared_ptr<boost::asio::executor_work_guard<boost::asio::io_context::executor_type>> io_service_work;
    std::shared_ptr<typename connection<RESP, REQ>::acceptor_t> acceptor;
    boost::thread_group threads;
    bool server_running;
    bool shutdown_requested;
};

#ifdef BOOST_ASIO_HAS_LOCAL_SOCKETS
template <typename REQ, typename RESP>
long server<REQ, RESP>::set_acceptor_endpoint(std::shared_ptr<boost::asio::local::stream_protocol::acceptor>& a, unsigned short port) {
    std::string s("/tmp/server");
    s += boost::lexical_cast<std::string>(port ? port : getpid());
    boost::asio::local::stream_protocol::endpoint endpoint(s);
    std::remove(endpoint.path().c_str());
    acceptor.reset(new typename connection<RESP, REQ>::acceptor_t(io_service));
    acceptor->open(endpoint.protocol());
    acceptor->bind(endpoint);
    return port;
}
#endif

template <typename REQ, typename RESP>
long server<REQ, RESP>::set_acceptor_endpoint(std::shared_ptr<boost::asio::ip::tcp::acceptor>& a, unsigned short port) {
    unsigned short retry_count = 128;
    bool connected = false;
    do {
        try {
            boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::tcp::v4(), port);
            acceptor.reset(new typename connection<RESP, REQ>::acceptor_t(io_service));
            acceptor->open(endpoint.protocol());
            acceptor->set_option(boost::asio::ip::tcp::no_delay(true));
            acceptor->set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
            acceptor->bind(endpoint);
            connected = true;
        } catch(std::exception& ex) {
            port++;
            retry_count--;
            CPPLOG(DEBUG, "tcp4tlm::server") << "Got '" << ex.what() << "', retrying with port " << port;
        }
    } while(!connected && retry_count > 0 && port <= 0xffff);
    return connected ? port : -1;
}

template <typename REQ, typename RESP> void server<REQ, RESP>::start_server(unsigned short port, char* name) {
    if(server_running)
        return;
    CPPLOG(TRACE, "tcp4tlm::server") << "starting tcp server";
    io_service_work.reset(new boost::asio::executor_work_guard<boost::asio::io_context::executor_type>(io_service.get_executor()));
    for(std::size_t i = 0; i < thread_pool_size; ++i) {
        threads.create_thread([this]() { io_service.run(); });
    }
    long actual_port = set_acceptor_endpoint(acceptor, port);
    if(actual_port < 0)
        throw new std::runtime_error(std::string("Could not open socket!"));
    if(actual_port != (long)port) {
        CPPLOG(INFO, "tcp4tlm::server") << "started the listener on " << boost::asio::ip::host_name() << ":" << actual_port
                                        << " (instead of port " << port << ")";
    } else {
        CPPLOG(INFO, "tcp4tlm::server") << "started the listener on " << boost::asio::ip::host_name() << ":" << port;
    }
    acceptor->listen();
    create_new_session();
    server_running = true;
}

template <typename REQ, typename RESP> void server<REQ, RESP>::shutdown_server() {
    CPPLOG(TRACE, "tcp4tlm::server") << "shutting down tcp server";
    io_service.stop();
    io_service_work.reset();
    threads.interrupt_all();
    threads.join_all();
    server_running = false;
}

} // namespace tcp4tlm
} // namespace scc

#endif // TLM_SCC_TCP4TLM_SERVER
