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

#ifndef TLM_SCC_TCP_DETAIL_SERIALIZED_CONNECTION_H_
#define TLM_SCC_TCP_DETAIL_SERIALIZED_CONNECTION_H_

#include <boost/asio.hpp>
#ifdef BINARY_ARCHIVE
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#else
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#endif
#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/tuple/tuple.hpp>
#include <functional>
#include <iomanip>
#include <sstream>
#include <string>
#include <util/logging.h>
#include <vector>

#ifdef BINARY_ARCHIVE
typedef boost::archive::binary_oarchive oarchive_type;
typedef boost::archive::binary_iarchive iarchive_type;
#else
typedef boost::archive::text_oarchive oarchive_type;
typedef boost::archive::text_iarchive iarchive_type;
#endif

namespace tlm {
namespace scc {
namespace tcp {
namespace impl {

/// The connection class provides serialization primitives on top of a socket.
/**
 * Each message sent using this class consists of:
 * @li An 8-byte header containing the length of the serialized data in
 * hexadecimal.
 * @li The serialized data.
 */
template <typename TSEND, typename TREC> class connection : public boost::enable_shared_from_this<connection<TSEND, TREC>> {
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
    typedef boost::shared_ptr<connection<TSEND, TREC>> ptr;

    struct async_listener : public boost::enable_shared_from_this<async_listener> {
        virtual void send_completed(const boost::system::error_code& error) = 0;
        virtual void receive_completed(const boost::system::error_code& error, const TREC* const result) = 0;
    };
    /// Constructor.
    connection(boost::asio::io_context& io_context)
    : socket_(io_context) {}
    /// Get the underlying socket. Used for making a connection or for accepting
    /// an incoming connection.
    socket_t& socket() { return socket_; }
    ///
    void add_listener(boost::shared_ptr<async_listener> l) { listener = l; }
    /// Asynchronously write a data structure to the socket.
    void async_write(TSEND& t) { async_write(&t); }
    /// Asynchronously write a data structure to the socket.
    void async_write(TSEND* t) {
        // Serialize the data first so we know how large it is.
        std::ostringstream archive_stream;
        oarchive_type archive(archive_stream);
        archive << t;
        outbound_data_ = archive_stream.str();
#ifdef TRACE_COMMUNICATION
        LOG(TRACE) << "outbound async data (" << outbound_data_.size() << "):[" << outbound_data_ << "]" << std::endl;
#endif
        // Format the header.
        std::ostringstream header_stream;
        header_stream << std::setw(header_length) << std::hex << std::setfill('0') << outbound_data_.size();
        if(!header_stream || header_stream.str().size() != header_length) {
            // Something went wrong, inform the caller.
            boost::system::error_code err(boost::asio::error::invalid_argument);
            if(listener)
                listener->send_completed(err);
            return;
        }
        outbound_header_ = header_stream.str();
        // Write the serialized data to the socket. We use "gather-write" to send
        // both the header and the data in a single write operation.
        std::vector<boost::asio::const_buffer> buffers;
        buffers.push_back(boost::asio::buffer(outbound_header_));
        buffers.push_back(boost::asio::buffer(outbound_data_));
        auto self = this->shared_from_this();
        boost::asio::async_write(socket_, buffers, [self](const boost::system::error_code& error, std::size_t bytes_transferred) {
            self->handle_async_write(error, bytes_transferred);
        });
    }

protected:
    /// notify the listener about the finish of the send process
    void handle_async_write(const boost::system::error_code& err, size_t /*bytes_transferred*/) {
        if(listener != NULL)
            listener->send_completed(err);
    }

public:
    /// Asynchronously read a data structure from the socket.
    void async_read() {
        // Issue a read operation to read exactly the number of bytes in a header.
        auto self = this->shared_from_this();
        boost::asio::async_read(socket_, boost::asio::buffer(inbound_header_),
                                [self](const boost::system::error_code& error, std::size_t) { self->async_read_header(error); });
    }

protected:
    /// Handle a completed read of a message header.
    void async_read_header(const boost::system::error_code& e) {
        if(e) {
            if(listener)
                listener->receive_completed(e, NULL);
        } else {
            // Determine the length of the serialized data.
            std::string header(inbound_header_, header_length);
            std::istringstream is(header);
            std::size_t inbound_data_size = 0;
            is >> std::hex >> inbound_data_size;
            if(inbound_data_size == 0) {
                // Header doesn't seem to be valid. Inform the caller.
                boost::system::error_code error(boost::asio::error::invalid_argument);
                if(listener)
                    listener->receive_completed(e, NULL);
                return;
            }
            // Start an asynchronous call to receive the data.
            inbound_data_.resize(inbound_data_size);
            auto self = this->shared_from_this();
            boost::asio::async_read(socket_, boost::asio::buffer(inbound_data_),
                                    [self](const boost::system::error_code& error, std::size_t) { self->async_read_data(error); });
        }
    }

    /// Handle a completed read of message data.
    void async_read_data(const boost::system::error_code& e) {
        if(e) {
            if(listener)
                listener->receive_completed(e, NULL);
        } else {
            // Extract the data structure from the data just received.
            try {
                std::string archive_data(&inbound_data_[0], inbound_data_.size());
#ifdef TRACE_COMMUNICATION
                LOG(TRACE) << "inbound async data (" << inbound_data_.size() << "):[" << archive_data << "]" << std::endl;
#endif
                std::istringstream archive_stream(archive_data);
                iarchive_type archive(archive_stream);
                TREC* t;
                archive >> t;
                if(listener)
                    listener->receive_completed(e, t);
                delete t;
            } catch(std::exception& /* unnamed */) {
                // Unable to decode data.
                boost::system::error_code err(boost::asio::error::invalid_argument);
                if(listener)
                    listener->receive_completed(err, NULL);
            }
            return;
        }
    }

public:
    ///
    void write_data(boost::shared_ptr<TSEND>& t) { write_data(t.get()); }

    void write_data(TSEND& t) { write_data(&t); }

    void write_data(TSEND* t) {
        boost::system::error_code ec;
        this->write_data(t, ec);
        boost::asio::detail::throw_error(ec);
    }

    void write_data(TSEND* t, boost::system::error_code& ec) {
        // Serialize the data first so we know how large it is.
        std::ostringstream archive_stream;
        oarchive_type archive(archive_stream);
        archive << t;
        outbound_data_ = archive_stream.str();
#ifdef TRACE_COMMUNICATION
        LOG(TRACE) << "outbound sync data (" << outbound_data_.size() << "):[" << outbound_data_ << "]" << std::endl;
#endif // Format the header.
        std::ostringstream header_stream;
        header_stream << std::setw(header_length) << std::hex << std::setfill('0') << outbound_data_.size();
        if(!header_stream || header_stream.str().size() != header_length) {
            // Something went wrong, inform the caller.
            ec.assign(boost::asio::error::invalid_argument, boost::asio::error::get_system_category());
            return;
        }
        outbound_header_ = header_stream.str();
        // Write the serialized data to the socket. We use "gather-write" to send
        // both the header and the data in a single write operation.
        std::vector<boost::asio::const_buffer> buffers;
        buffers.push_back(boost::asio::buffer(outbound_header_));
        buffers.push_back(boost::asio::buffer(outbound_data_));
#ifdef GENERATE_STATISTICS
        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &getTStamp());
#endif
        boost::asio::write(socket_, buffers);
    }

    bool read_data_avail() {
        boost::asio::socket_base::bytes_readable command(true);
        socket_.io_control(command);
        std::size_t bytes_readable = command.get();
        return bytes_readable > 0;
    }

    void read_data(boost::shared_ptr<TREC>& msg) {
        TREC* m;
        read_data(m);
        msg.reset(m);
    }

    void read_data(TREC*& t) {
        boost::system::error_code ec;
        this->read_data(t, ec);
        boost::asio::detail::throw_error(ec);
    }

    void read_data(TREC*& t, boost::system::error_code& ec) {
        boost::asio::transfer_at_least(23);
        boost::asio::read(socket_, boost::asio::buffer(inbound_header_, header_length), boost::asio::transfer_exactly(header_length));
        // Determine the length of the serialized data.
        std::string header(inbound_header_, header_length);
        std::istringstream is(header);
        std::size_t inbound_data_size = 0;
        is >> std::hex >> inbound_data_size;
        if(inbound_data_size == 0) {
            // Header doesn't seem to be valid. Inform the caller.
            ec.assign(boost::asio::error::invalid_argument, boost::asio::error::get_system_category());
            return;
        }
        // Start an synchronous call to receive the data.
        inbound_data_.resize(inbound_data_size);
        boost::asio::read(socket_, boost::asio::buffer(inbound_data_, inbound_data_size), boost::asio::transfer_exactly(inbound_data_size));
        std::string archive_data(&inbound_data_[0], inbound_data_.size());
#ifdef TRACE_COMMUNICATION
        LOG(TRACE) << "inbound sync data (" << inbound_data_.size() << "):[" << archive_data << "]" << std::endl;
#endif
        std::istringstream archive_stream(archive_data);
        iarchive_type archive(archive_stream);
        archive >> t;
        return;
    }
#ifdef GENERATE_STATISTICS
    static timespec& getTStamp() {
        static timespec tstamp;
        return tstamp;
    }
#endif

private:
    /// The underlying socket.
    socket_t socket_;
    /// The size of a fixed length header.
    enum { header_length = 8 };
    /// Holds an outbound header.
    std::string outbound_header_;
    /// Holds the outbound data.
    std::string outbound_data_;
    /// Holds an inbound header.
    char inbound_header_[header_length];
    /// Holds the inbound data.
    std::vector<char> inbound_data_;

    boost::shared_ptr<async_listener> listener;
};
} // namespace impl
} // namespace tcp
} // namespace scc
} // namespace tlm

#endif // TLM_SCC_TCP_DETAIL_SERIALIZATION_CONNECTION_HPP
