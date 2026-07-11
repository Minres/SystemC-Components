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

#ifndef TLM_SCC_TCP_DETAIL_MESSAGES_H_
#define TLM_SCC_TCP_DETAIL_MESSAGES_H_

#include <boost/serialization/export.hpp>
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/version.hpp>
#include <string>

#ifdef ERROR
#undef ERROR
#endif

#define MSG_VERSION 1

#ifdef DEFINE_EXPORTS_HERE
#define EXPORT_CLASS(X)                                                                                                                    \
    BOOST_CLASS_EXPORT(X);                                                                                                                 \
    BOOST_CLASS_VERSION(X, MSG_VERSION);
#endif

#ifndef EXPORT_CLASS
#define EXPORT_CLASS(X) ;
#endif

enum response_status { ok, accepted, delayed, declined, failure };
static const char* response_names[] = {"OK", "ACCEPTED", "DELAYED", "DECLINED", "FAILURE"};
enum bus_access_type { normal_acc, debug_acc };

class request_message {
public:
    request_message()
    : id(next_id()) {}

    virtual ~request_message(){};

    const uint32_t get_id() const { return id; }

    virtual bool operator==(const request_message& o) const { return id == o.id; }

protected:
    uint32_t id;
    friend class ::boost::serialization::access;

    template <typename Archive> void serialize(Archive& ar, const uint32_t version) { ar& id; }

private:
    uint32_t next_id() {
        static uint32_t id_counter;
        return id_counter++;
    }
};
EXPORT_CLASS(request_message);

class response_message {
public:
    response_message()
    : status(ok)
    , req_id(0) {}

    response_message(const request_message* const req, response_status status = ok)
    : status(status)
    , req_id(req->get_id()) {}

    virtual ~response_message(){};

    response_status get_status() const { return status; }

    const char* get_status_str() const { return response_names[status]; }

    void set_status(response_status resp) { status = resp; }

    bool belongs_to(const request_message* req) { return req->get_id() == req_id; }

    virtual bool operator==(const response_message& o) const { return status == o.status && req_id == o.req_id; }

protected:
    response_status status;
    uint32_t req_id;
    friend class ::boost::serialization::access;

    template <typename Archive> void serialize(Archive& ar, const uint32_t version) { ar& status& req_id; }
};
EXPORT_CLASS(response_message);

class notify_endpoint_msg : public request_message {
public:
    notify_endpoint_msg()
    : request_message()
    , hostname("")
    , port(0) {}

    notify_endpoint_msg(std::string hostname_, uint16_t port)
    : request_message()
    , hostname(hostname_)
    , port(port) {}

    virtual ~notify_endpoint_msg(){};

    std::string hostname;
    uint16_t port;

    bool operator==(const notify_endpoint_msg& o) const {
        return request_message::operator==(o) && hostname == o.hostname && port == o.port;
    }

protected:
    friend class ::boost::serialization::access;

    template <typename Archive> void serialize(Archive& ar, const uint32_t version) {
        ar& BOOST_SERIALIZATION_BASE_OBJECT_NVP(request_message) & hostname& port;
    }
};
EXPORT_CLASS(notify_endpoint_msg);

class notify_shutdown_msg : public request_message {
public:
    notify_shutdown_msg()
    : request_message() {}

    virtual ~notify_shutdown_msg(){};

    bool operator==(const notify_shutdown_msg& o) const { return request_message::operator==(o); }

protected:
    friend class ::boost::serialization::access;

    template <typename Archive> void serialize(Archive& ar, const uint32_t version) {
        ar& BOOST_SERIALIZATION_BASE_OBJECT_NVP(request_message);
    }
};
EXPORT_CLASS(notify_shutdown_msg);

class sync_msg : public request_message {
public:
    sync_msg()
    : request_message()
    , time_stamp(0ULL) {}

    sync_msg(uint64_t timestamp)
    : request_message()
    , time_stamp(timestamp) {}

    virtual ~sync_msg(){};

    uint64_t time_stamp;

    bool operator==(const sync_msg& o) const { return request_message::operator==(o) && time_stamp == o.time_stamp; }

protected:
    friend class ::boost::serialization::access;

    template <typename Archive> void serialize(Archive& ar, const uint32_t version) {
        ar& ::boost::serialization::base_object<request_message>(*this) & time_stamp;
    }
};
EXPORT_CLASS(sync_msg);

class bus_op_msg : public sync_msg {
public:
    bus_op_msg()
    : sync_msg()
    , time_offset(0ULL)
    , type(normal_acc)
    , index(0)
    , address(0)
    , size(0)
    , no_response(false)
    , data(0) {}

    virtual ~bus_op_msg(){};

    uint64_t time_offset;
    bus_access_type type;
    uint32_t index;
    uint64_t address;
    uint32_t size;
    bool no_response;
    std::vector<uint8_t> data;
    std::vector<uint8_t> byte_enable;

    bool operator==(const bus_op_msg& o) const {
        return sync_msg::operator==(o) && time_offset == o.time_offset && index == o.index && address == o.address && size == o.size &&
               data.size() == o.data.size() && no_response == o.no_response;
    }

protected:
    friend class ::boost::serialization::access;

    template <typename Archive> void serialize(Archive& ar, const uint32_t version) {
        ar& ::boost::serialization::base_object<sync_msg>(*this) & time_offset& index& address& size& data& no_response;
    }
};
EXPORT_CLASS(bus_op_msg);

class bus_data_msg : public response_message {
public:
    bus_data_msg()
    : response_message()
    , data(0) {}

    bus_data_msg(const request_message* const req, response_status status = ok)
    : response_message(req, status)
    , data(0) {}

    virtual ~bus_data_msg(){};

    std::vector<uint8_t> data;

    virtual bool operator==(const bus_data_msg& o) const { return response_message::operator==(o) && data == o.data; }

protected:
    friend class ::boost::serialization::access;

    template <typename Archive> void serialize(Archive& ar, const uint32_t version) {
        ar& ::boost::serialization::base_object<response_message>(*this) & data;
    }
};
EXPORT_CLASS(bus_data_msg);

class sig_op_msg : public sync_msg {
public:
    sig_op_msg()
    : sync_msg()
    , index(0)
    , value(0) {}

    virtual ~sig_op_msg(){};

    uint32_t index;
    uint8_t value;

    bool operator==(const sig_op_msg& o) const { return sync_msg::operator==(o) && index == o.index && value == o.value; }

protected:
    friend class ::boost::serialization::access;

    template <typename Archive> void serialize(Archive& ar, const uint32_t version) {
        ar& ::boost::serialization::base_object<sync_msg>(*this) & index& value;
    }
};
EXPORT_CLASS(sig_op_msg);

class log_msg : public request_message {
public:
    log_msg()
    : request_message()
    , msg("") {}

    virtual ~log_msg(){};

    std::string msg;

    bool operator==(const log_msg& o) const { return request_message::operator==(o) && msg == o.msg; }

protected:
    friend class ::boost::serialization::access;

    template <typename Archive> void serialize(Archive& ar, const uint32_t version) {
        ar& ::boost::serialization::base_object<request_message>(*this) & msg;
    }
};
EXPORT_CLASS(log_msg);
#endif // TLM_SCC_TCP_DETAIL_MESSAGES_H_
