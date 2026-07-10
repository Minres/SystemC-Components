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

#ifndef TLM_SCC_TCP_DETAIL_SERVER_H
#define TLM_SCC_TCP_DETAIL_SERVER_H

#include "IServer.h"
#include <boost/enable_shared_from_this.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/thread.hpp>
#include <functional>
#include <util/logging.h>
namespace tlm {
namespace scc {
namespace tcp {
namespace impl {

template <typename REQ, typename RESP> class Server : public IServer<REQ, RESP> {
    /**
     * a shortcut for the type of the shared pointer to the connection
     */
    typedef typename boost::shared_ptr<connection<RESP, REQ>> con_ptr;
    /**
     * a shortcut for the listener of the connection. Needed for asyncronous implementation
     */
    typedef typename connection<RESP, REQ>::async_listener con_listener;
    /**
     * the session being used for each connection. Just forwards the calls to the server.
     */
    struct ForwardSession : public con_listener {
        /**
         * Constructor
         *
         * @param server_
         */
        ForwardSession(IServer<REQ, RESP>* server_)
        : server(server_)
        , conn_shptr(new connection<RESP, REQ>(server->getIoService())) {}
        /**
         * destructor, needed as the class has virtual functions
         */
        virtual ~ForwardSession() {}
        /**
         * returns the shared pointer to the connection
         * @return
         */
        con_ptr& getConnection() { return conn_shptr; }
        /**
         * starts the session and will be called upon connect
         *
         * @return
         */
        bool start() {
            boost::shared_ptr<con_listener> ptr = boost::enable_shared_from_this<con_listener>::shared_from_this();
            conn_shptr->add_listener(ptr);
            typename connection<RESP, REQ>::endpoint_t endpoint = conn_shptr->socket().remote_endpoint();
            LOG(DEBUG)
                << "ForwardSession::start(), got connected"; // to remote port "<<endpoint.port()<<" on "<<endpoint.address().to_string();
            // Successfully accepted a new connection. Notify the server so that it waits for a request
            server->serverSendCompleted(conn_shptr, true);
            return server->isShutdownRequested();
        }
        /**
         * Asynchronous callback called if a receive is finished. Checks for error and calls
         * Server::serverReceiveCompleted if no error
         *
         * @param e
         * @param data
         */
        void receive_completed(const boost::system::error_code& e, const REQ* const data) {
            if(e.value() == 2) {
                LOG(WARN) << "Client closed connection (" << e.message() << ")";
                return;
            } else if(e) {
                LOG(ERR) << "Communication error (" << e.message() << ")";
                return;
            }
            server->serverReceiveCompleted(conn_shptr, data);
        }
        /**
         * Asynchronous callback called if a send is finished. Checks for error and calls
         * Server::serverSendCompleted if no error
         *
         * @param e
         */
        void send_completed(const boost::system::error_code& e) {
            if(!e) {
                server->serverSendCompleted(conn_shptr);
            } else {
                LOG(ERR) << e.message() << "(" << e << ")";
            }
        }

    private:
        IServer<REQ, RESP>* server;
        con_ptr conn_shptr;
    };

    friend struct ForwardSession;

public:
    /**
     * constructor
     * @param thread_pool_size
     */
    Server(std::size_t thread_pool_size = 4)
    : thread_pool_size(thread_pool_size)
    , io_service()
    , acceptor()
    , server_running(false)
    , shutdownRequested(false){};
    /**
     * destructor
     */
    virtual ~Server() {}
#ifdef BOOST_ASIO_HAS_LOCAL_SOCKETS
    long setAcceptorEndpoint(boost::shared_ptr<boost::asio::local::stream_protocol::acceptor>& a, unsigned short port);
#endif
    long setAcceptorEndpoint(boost::shared_ptr<boost::asio::ip::tcp::acceptor>& a, unsigned short port);

    /**
     * starts the server listening on port port. The created server runs within its own
     * thread
     * @param port
     */
    void startServer(unsigned short port, char* name = NULL);
    /**
     * requests the server to stop but does not wait for it
     */
    void requestShutdown() {
        shutdownRequested = true;
        io_service.stop();
    }
    /**
     * checks if a shutdown is requested
     * @return
     */
    bool isShutdownRequested() { return shutdownRequested; }
    /**
     * shut down the server and wait for the thread to finish
     */
    void shutdownServer();
    /**
     * returns the io_service managing the socket interactions
     * @return
     */
    virtual boost::asio::io_context& getIoService() { return io_service; }
    /**
     * returns true if the server is active and running
     * @return
     */
    bool isServerRunning() { return server_running; }
    /**
     * creates a new session using the ForwardSession class
     */
    void createNewSession() {
        boost::shared_ptr<ForwardSession> fwSession(new ForwardSession(this));
        acceptor->async_accept(fwSession->getConnection()->socket(),
                               [this, fwSession](const boost::system::error_code& error) { handle_accept(error, fwSession); });
    }

protected:
    const typename connection<RESP, REQ>::acceptor_t& getAcceptor() { return *acceptor; };

private:
    /// Handle completion of a accept operation by starting a session.
    void handle_accept(const boost::system::error_code& e, boost::shared_ptr<ForwardSession> session) {
        if(!e) {
            // Start an accept operation for a new connection. If it finishes create a new session
            if(!session->start())
                createNewSession();
        } else {
            // An error occurred. Log it and return. Since we are not starting a new
            // accept operation the io_service will run out of work to do and the
            // thread will exit.
            LOG(ERR) << e.message();
        }
    }
    // server related members
    std::size_t thread_pool_size;
    boost::asio::io_context io_service;
    boost::shared_ptr<boost::asio::executor_work_guard<boost::asio::io_context::executor_type>> io_service_work;
    boost::shared_ptr<typename connection<RESP, REQ>::acceptor_t> acceptor;
    boost::thread_group threads;
    bool server_running;
    bool shutdownRequested;
};

#ifdef BOOST_ASIO_HAS_LOCAL_SOCKETS
template <typename REQ, typename RESP>
long Server<REQ, RESP>::setAcceptorEndpoint(boost::shared_ptr<boost::asio::local::stream_protocol::acceptor>& a, unsigned short port) {
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
long Server<REQ, RESP>::setAcceptorEndpoint(boost::shared_ptr<boost::asio::ip::tcp::acceptor>& a, unsigned short port) {
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
            LOG(DEBUG) << "Got '" << ex.what() << "', retrying with port " << port;
        }
    } while(!connected && retry_count > 0 && port <= 0xffff);
    return connected ? port : -1;
}

template <typename REQ, typename RESP> void Server<REQ, RESP>::startServer(unsigned short port, char* name) {
    LOG(INFO) << "starting tcp server listening on port " << port;
    io_service_work.reset(new boost::asio::executor_work_guard<boost::asio::io_context::executor_type>(io_service.get_executor()));
    // Create a pool of threads to run all of the io_services.
    for(std::size_t i = 0; i < thread_pool_size; ++i) {
        threads.create_thread([this]() { io_service.run(); });
    }
    long actual_port = setAcceptorEndpoint(acceptor, port);
    if(actual_port < 0)
        throw new std::runtime_error(std::string("Could not open socket!"));
    if(actual_port != (long)port) {
        LOG(WARN) << "starting the listening acceptor on " << boost::asio::ip::host_name() << ":" << actual_port << " (instead of port "
                  << port << ")";
    } else {
        LOG(INFO) << "starting the listening acceptor on " << boost::asio::ip::host_name() << ":" << port;
    }
    acceptor->listen();
    LOG(INFO) << "createNewSession";
    createNewSession();
    server_running = true;
}

template <typename REQ, typename RESP> void Server<REQ, RESP>::shutdownServer() {
    LOG(INFO) << "shutting down tcp server";
    io_service.stop();
    // Wait for all threads in the pool to exit.
    io_service_work.reset();
    threads.interrupt_all();
    threads.join_all();
    server_running = false;
}
} // namespace impl
} // namespace tcp
} // namespace scc
} // namespace tlm
#endif // TLM_SCC_TCP_DETAIL_SERVER
