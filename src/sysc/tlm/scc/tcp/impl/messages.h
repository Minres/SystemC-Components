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

enum ResponseStatus { OK, ACCEPTED, DELAYED, DECLINED, FAILURE };
static const char* responses[] = {"OK", "ACCEPTED", "DELAYED", "DECLINED", "FAILURE"};
enum BusAccessType { NORMAL_ACC, DEBUG_ACC };

class RequestMessage {
public:
    RequestMessage()
    : id(nextId()) {}
    virtual ~RequestMessage(){};
    const uint32_t getId() const { return id; }
    virtual bool operator==(const RequestMessage& o) const { return id == o.id; }

protected:
    uint32_t id;
    friend class ::boost::serialization::access;
    template <typename Archive> void serialize(Archive& ar, const uint32_t version) { ar& id; }

private:
    uint32_t nextId() {
        static uint32_t idCounter;
        return idCounter++;
    }
};
EXPORT_CLASS(RequestMessage);

class ResponseMessage {
public:
    ResponseMessage()
    : responseStatus(OK)
    , reqId(0) {}
    ResponseMessage(const RequestMessage* const req, ResponseStatus status = OK)
    : responseStatus(status)
    , reqId(req->getId()) {}
    virtual ~ResponseMessage(){};
    ResponseStatus getStatus() const { return responseStatus; }
    const char* getStatusStr() const { return responses[responseStatus]; }
    void setStatus(ResponseStatus resp) { responseStatus = resp; }
    bool belongsTo(RequestMessage* req) { return req->getId() == reqId; }
    virtual bool operator==(const ResponseMessage& o) const { return responseStatus == o.responseStatus && reqId == o.reqId; }

protected:
    ResponseStatus responseStatus;
    uint32_t reqId;
    friend class ::boost::serialization::access;
    template <typename Archive> void serialize(Archive& ar, const uint32_t version) { ar& responseStatus& reqId; }
};
EXPORT_CLASS(ResponseMessage);

class NotifyEndpointMsg : public RequestMessage {
public:
    NotifyEndpointMsg()
    : RequestMessage()
    , hostname("")
    , port(0) {}
    NotifyEndpointMsg(std::string hostname_, uint16_t port)
    : RequestMessage()
    , hostname(hostname_)
    , port(port) {}
    virtual ~NotifyEndpointMsg(){};
    std::string hostname;
    uint16_t port;
    bool operator==(const NotifyEndpointMsg& o) const { return RequestMessage::operator==(o) && hostname == o.hostname && port == o.port; }

protected:
    friend class ::boost::serialization::access;
    template <typename Archive> void serialize(Archive& ar, const uint32_t version) {
        ar& BOOST_SERIALIZATION_BASE_OBJECT_NVP(RequestMessage) & hostname& port;
    }
};
EXPORT_CLASS(NotifyEndpointMsg);

class NotifyShutdownMsg : public RequestMessage {
public:
    NotifyShutdownMsg()
    : RequestMessage() {}
    virtual ~NotifyShutdownMsg(){};
    bool operator==(const NotifyShutdownMsg& o) const { return RequestMessage::operator==(o); }

protected:
    friend class ::boost::serialization::access;
    template <typename Archive> void serialize(Archive& ar, const uint32_t version) {
        ar& BOOST_SERIALIZATION_BASE_OBJECT_NVP(RequestMessage);
    }
};
EXPORT_CLASS(NotifyShutdownMsg);

class SyncMsg : public RequestMessage {
public:
    SyncMsg()
    : RequestMessage()
    , time_stamp(0ULL) {}
    SyncMsg(uint64_t timestamp)
    : RequestMessage()
    , time_stamp(timestamp) {}
    virtual ~SyncMsg(){};
    uint64_t time_stamp;
    bool operator==(const SyncMsg& o) const { return RequestMessage::operator==(o) && time_stamp == o.time_stamp; }

protected:
    friend class ::boost::serialization::access;
    template <typename Archive> void serialize(Archive& ar, const uint32_t version) {
        ar& ::boost::serialization::base_object<RequestMessage>(*this) & time_stamp;
    }
};
EXPORT_CLASS(SyncMsg);

class BusOpMsg : public SyncMsg {
public:
    BusOpMsg()
    : SyncMsg()
    , time_offset(0ULL)
    , type(NORMAL_ACC)
    , index(0)
    , address(0)
    , size(0)
    , noResponse(false)
    , data(0) {}
    virtual ~BusOpMsg(){};
    uint64_t time_offset;
    BusAccessType type;
    uint32_t index;
    uint64_t address;
    uint32_t size;
    bool noResponse;
    std::vector<uint8_t> data;
    std::vector<uint8_t> byte_enable;
    bool operator==(const BusOpMsg& o) const {
        return SyncMsg::operator==(o) && time_offset == o.time_offset && index == o.index && address == o.address && size == o.size &&
               data.size() == o.data.size() && noResponse == o.noResponse;
    }

protected:
    friend class ::boost::serialization::access;
    template <typename Archive> void serialize(Archive& ar, const uint32_t version) {
        ar& ::boost::serialization::base_object<SyncMsg>(*this) & time_offset& index& address& size& data& noResponse;
    }
};
EXPORT_CLASS(BusOpMsg);

class BusDataMsg : public ResponseMessage {
public:
    BusDataMsg()
    : ResponseMessage()
    , data(0) {}
    BusDataMsg(const RequestMessage* const req, ResponseStatus status = OK)
    : ResponseMessage(req, status)
    , data(0) {}
    virtual ~BusDataMsg(){};
    std::vector<uint8_t> data;
    virtual bool operator==(const BusDataMsg& o) const { return ResponseMessage::operator==(o) && data == o.data; }

protected:
    friend class ::boost::serialization::access;
    template <typename Archive> void serialize(Archive& ar, const uint32_t version) {
        ar& ::boost::serialization::base_object<ResponseMessage>(*this) & data;
    }
};
EXPORT_CLASS(BusDataMsg);

class SigOpMsg : public SyncMsg {
public:
    SigOpMsg()
    : SyncMsg()
    , index(0)
    , value(0) {}
    virtual ~SigOpMsg(){};
    uint32_t index;
    uint8_t value;
    bool operator==(const SigOpMsg& o) const { return SyncMsg::operator==(o) && index == o.index && value == o.value; }

protected:
    friend class ::boost::serialization::access;
    template <typename Archive> void serialize(Archive& ar, const uint32_t version) {
        ar& ::boost::serialization::base_object<SyncMsg>(*this) & index& value;
    }
};
EXPORT_CLASS(SigOpMsg);

class LogMsg : public RequestMessage {
public:
    LogMsg()
    : RequestMessage()
    , msg("") {}
    virtual ~LogMsg(){};
    std::string msg;
    bool operator==(const LogMsg& o) const { return RequestMessage::operator==(o) && msg == o.msg; }

protected:
    friend class ::boost::serialization::access;
    template <typename Archive> void serialize(Archive& ar, const uint32_t version) {
        ar& ::boost::serialization::base_object<RequestMessage>(*this) & msg;
    }
};
EXPORT_CLASS(LogMsg);

#endif // TLM_SCC_TCP_DETAIL_MESSAGES_H_
