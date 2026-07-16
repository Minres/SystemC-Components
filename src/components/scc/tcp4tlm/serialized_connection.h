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

#ifndef TLM_SCC_TCP4TLM_SERIALIZED_CONNECTION_H_
#define TLM_SCC_TCP4TLM_SERIALIZED_CONNECTION_H_

#include <boost/asio.hpp>
#include <cstdint>
#include <iomanip>
#include <sstream>
#include <string>
#include <util/logging.h>
#include <vector>

namespace scc {
namespace tcp4tlm {

/// The connection class provides FlatBuffers framing primitives on top of a socket.
/**
 * Each message sent using this class consists of:
 * @li An 8-byte header containing the length of the serialized data in
 * hexadecimal.
 * @li The serialized FlatBuffer payload.
 */
template <typename TSEND, typename TREC> class connection : public std::enable_shared_from_this<connection<TSEND, TREC>> {
public:
#if defined(BOOST_ASIO_HAS_LOCAL_SOCKETS) && defined(USE_UDS)
#warning "This build uses stream sockets and will not work across the network"
    typedef boost::asio::local::stream_protocol::socket socket_t;
    typedef boost::asio::local::stream_protocol::endpoint endpoint_t;
    typedef boost::asio::local::stream_protocol::acceptor acceptor_t;
#elif defined(USE_UDP)
    typedef boost::asio::ip::udp::socket socket_t;
    typedef boost::asio::ip::udp::endpoint endpoint_t;
#else
    typedef boost::asio::ip::tcp::socket socket_t;
    typedef boost::asio::ip::tcp::endpoint endpoint_t;
    typedef boost::asio::ip::tcp::acceptor acceptor_t;
#endif
    typedef std::shared_ptr<connection<TSEND, TREC>> ptr;

    struct async_listener : public std::enable_shared_from_this<async_listener> {
        virtual void send_completed(const boost::system::error_code& error) = 0;
        virtual void receive_completed(const boost::system::error_code& error, const TREC* const result) = 0;
    };

    connection(boost::asio::io_context& io_context)
    : socket_(io_context) {}

    socket_t& socket() { return socket_; }
    void add_listener(std::shared_ptr<async_listener> l) { listener = l; }

    void async_write(TSEND& t) { async_write(&t); }

    void async_write(TSEND* t) {
        if(!prepare_outbound_data(t)) {
            boost::system::error_code err(boost::asio::error::invalid_argument);
            if(listener)
                listener->send_completed(err);
            return;
        }

        std::vector<boost::asio::const_buffer> buffers;
        buffers.push_back(boost::asio::buffer(outbound_header_));
        buffers.push_back(boost::asio::buffer(outbound_data_));
        auto self = this->shared_from_this();
        boost::asio::async_write(socket_, buffers, [self](const boost::system::error_code& error, std::size_t bytes_transferred) {
            self->handle_async_write(error, bytes_transferred);
        });
    }

protected:
    void handle_async_write(const boost::system::error_code& err, size_t /*bytes_transferred*/) {
        if(listener != NULL)
            listener->send_completed(err);
    }

public:
    void async_read() {
        auto self = this->shared_from_this();
        boost::asio::async_read(socket_, boost::asio::buffer(inbound_header_),
                                [self](const boost::system::error_code& error, std::size_t) { self->async_read_header(error); });
    }

protected:
    void async_read_header(const boost::system::error_code& e) {
        if(e) {
            if(listener)
                listener->receive_completed(e, NULL);
        } else {
            std::string header(inbound_header_, header_length);
            std::istringstream is(header);
            std::size_t inbound_data_size = 0;
            is >> std::hex >> inbound_data_size;
            if(inbound_data_size == 0) {
                boost::system::error_code error(boost::asio::error::invalid_argument);
                if(listener)
                    listener->receive_completed(error, NULL);
                return;
            }
            inbound_data_.resize(inbound_data_size);
            auto self = this->shared_from_this();
            boost::asio::async_read(socket_, boost::asio::buffer(inbound_data_),
                                    [self](const boost::system::error_code& error, std::size_t) { self->async_read_data(error); });
        }
    }

    void async_read_data(const boost::system::error_code& e) {
        if(e) {
            if(listener)
                listener->receive_completed(e, NULL);
        } else {
            if(!TREC::verify(inbound_data_.data(), inbound_data_.size())) {
                boost::system::error_code err(boost::asio::error::invalid_argument);
                if(listener)
                    listener->receive_completed(err, NULL);
                return;
            }
#ifdef TRACE_COMMUNICATION
            std::string raw_data(reinterpret_cast<const char*>(inbound_data_.data()), inbound_data_.size());
            CPPLOG(TRACE, "tcp4tlm") << "inbound async data (" << inbound_data_.size() << "):[" << raw_data << "]" << std::endl;
#endif
            TREC* t = new TREC(std::move(inbound_data_));
            if(listener)
                listener->receive_completed(e, t);
            delete t;
        }
    }

public:
    void write_data(std::shared_ptr<TSEND>& t) { write_data(t.get()); }

    void write_data(TSEND& t) { write_data(&t); }

    void write_data(TSEND* t) {
        boost::system::error_code ec;
        this->write_data(t, ec);
        boost::asio::detail::throw_error(ec);
    }

    void write_data(TSEND* t, boost::system::error_code& ec) {
        if(!prepare_outbound_data(t)) {
            ec.assign(boost::asio::error::invalid_argument, boost::asio::error::get_system_category());
            return;
        }
#ifdef GENERATE_STATISTICS
        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &get_t_stamp());
#endif
        std::vector<boost::asio::const_buffer> buffers;
        buffers.push_back(boost::asio::buffer(outbound_header_));
        buffers.push_back(boost::asio::buffer(outbound_data_));
        boost::asio::write(socket_, buffers);
    }

    bool read_data_available() {
        boost::asio::socket_base::bytes_readable command(true);
        socket_.io_control(command);
        std::size_t bytes_readable = command.get();
        return bytes_readable > 0;
    }

    void read_data(std::shared_ptr<TREC>& msg) {
        TREC* m = nullptr;
        read_data(m);
        msg.reset(m);
    }

    void read_data(TREC*& t) {
        boost::system::error_code ec;
        this->read_data(t, ec);
        boost::asio::detail::throw_error(ec);
    }

    void read_data(TREC*& t, boost::system::error_code& ec) {
        boost::asio::read(socket_, boost::asio::buffer(inbound_header_, header_length), boost::asio::transfer_exactly(header_length));
        std::string header(inbound_header_, header_length);
        std::istringstream is(header);
        std::size_t inbound_data_size = 0;
        is >> std::hex >> inbound_data_size;
        if(inbound_data_size == 0) {
            ec.assign(boost::asio::error::invalid_argument, boost::asio::error::get_system_category());
            return;
        }

        inbound_data_.resize(inbound_data_size);
        boost::asio::read(socket_, boost::asio::buffer(inbound_data_, inbound_data_size), boost::asio::transfer_exactly(inbound_data_size));
        if(!TREC::verify(inbound_data_.data(), inbound_data_.size())) {
            ec.assign(boost::asio::error::invalid_argument, boost::asio::error::get_system_category());
            return;
        }
#ifdef TRACE_COMMUNICATION
        std::string raw_data(reinterpret_cast<const char*>(inbound_data_.data()), inbound_data_.size());
        CPPLOG(TRACE, "tcp4tlm") << "inbound sync data (" << inbound_data_.size() << "):[" << raw_data << "]" << std::endl;
#endif
        t = new TREC(std::move(inbound_data_));
    }

#ifdef GENERATE_STATISTICS
    static timespec& get_t_stamp() {
        static timespec tstamp;
        return tstamp;
    }
#endif

private:
    bool prepare_outbound_data(TSEND* t) {
        if(t == nullptr || !(*t) || t->size() == 0) {
            return false;
        }

        outbound_data_.assign(reinterpret_cast<const char*>(t->data()), reinterpret_cast<const char*>(t->data()) + t->size());
#ifdef TRACE_COMMUNICATION
        CPPLOG(TRACE, "tcp4tlm") << "outbound data (" << outbound_data_.size() << "):[" << outbound_data_ << "]" << std::endl;
#endif
        std::ostringstream header_stream;
        header_stream << std::setw(header_length) << std::hex << std::setfill('0') << outbound_data_.size();
        if(!header_stream || header_stream.str().size() != header_length) {
            return false;
        }
        outbound_header_ = header_stream.str();
        return true;
    }

    socket_t socket_;
    enum { header_length = 8 };
    std::string outbound_header_;
    std::string outbound_data_;
    char inbound_header_[header_length];
    std::vector<uint8_t> inbound_data_;
    std::shared_ptr<async_listener> listener;
};

} // namespace tcp4tlm
} // namespace scc

#endif // TLM_SCC_TCP4TLM_SERIALIZED_CONNECTION_H_
