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

#ifndef TLM_SCC_TCP4TLM_MESSAGES_H_
#define TLM_SCC_TCP4TLM_MESSAGES_H_

#include "messages_generated.h"
#include <atomic>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

namespace scc {
namespace tcp4tlm {

template <typename Root> class message_buffer {
public:
    message_buffer() = default;

    explicit message_buffer(std::vector<uint8_t> storage)
    : storage_(std::move(storage))
    , root_(nullptr) {
        refresh_root();
    }

    const Root* root() const { return root_; }
    const Root* operator->() const { return root_; }
    explicit operator bool() const { return root_ != nullptr; }

    const uint8_t* data() const { return storage_.empty() ? nullptr : storage_.data(); }
    std::size_t size() const { return storage_.size(); }

    static bool verify(const uint8_t* data, std::size_t size) {
        if(data == nullptr || size == 0) {
            return false;
        }
        flatbuffers::Verifier verifier(data, size);
        return verifier.VerifyBuffer<Root>(nullptr);
    }

private:
    void refresh_root() { root_ = verify(data(), size()) ? flatbuffers::GetRoot<Root>(data()) : nullptr; }

    std::vector<uint8_t> storage_;
    const Root* root_;
};

using request_message = message_buffer<RequestEnvelope>;
using response_message = message_buffer<ResponseEnvelope>;
using response_status = ResponseStatus;
using bus_access_type = BusAccessType;

constexpr response_status ok = ResponseStatus_OK;
constexpr response_status accepted = ResponseStatus_ACCEPTED;
constexpr response_status delayed = ResponseStatus_DELAYED;
constexpr response_status declined = ResponseStatus_DECLINED;
constexpr response_status failure = ResponseStatus_FAILURE;

constexpr bus_access_type normal_acc = BusAccessType_NORMAL_ACC;
constexpr bus_access_type debug_acc = BusAccessType_DEBUG_ACC;

inline const char* get_status_str(response_status status) { return EnumNameResponseStatus(status); }

inline uint32_t next_request_id() {
    static std::atomic<uint32_t> id_counter{0};
    return id_counter++;
}

template <typename Root> inline message_buffer<Root> make_message(flatbuffers::FlatBufferBuilder& builder) {
    auto detached = builder.Release();
    return message_buffer<Root>(std::vector<uint8_t>(detached.data(), detached.data() + detached.size()));
}

inline request_message make_request(RequestPayload payload_type, flatbuffers::Offset<void> payload,
                                    flatbuffers::FlatBufferBuilder& builder) {
    auto envelope = CreateRequestEnvelope(builder, payload_type, payload);
    builder.Finish(envelope);
    return make_message<RequestEnvelope>(builder);
}

inline response_message make_response_message(ResponsePayload payload_type, flatbuffers::Offset<void> payload,
                                              flatbuffers::FlatBufferBuilder& builder) {
    auto envelope = CreateResponseEnvelope(builder, payload_type, payload);
    builder.Finish(envelope);
    return make_message<ResponseEnvelope>(builder);
}

inline request_message make_notify_endpoint_msg(const std::string& hostname, uint16_t port, uint32_t id = next_request_id()) {
    flatbuffers::FlatBufferBuilder builder;
    auto payload = CreateNotifyEndpointMsgDirect(builder, id, hostname.c_str(), port);
    return make_request(RequestPayload_NotifyEndpointMsg, payload.Union(), builder);
}

inline request_message make_notify_shutdown_msg(uint32_t id = next_request_id()) {
    flatbuffers::FlatBufferBuilder builder;
    auto payload = CreateNotifyShutdownMsg(builder, id);
    return make_request(RequestPayload_NotifyShutdownMsg, payload.Union(), builder);
}

inline request_message make_sync_msg(uint64_t time_stamp, uint32_t id = next_request_id()) {
    flatbuffers::FlatBufferBuilder builder;
    auto payload = CreateSyncMsg(builder, id, time_stamp);
    return make_request(RequestPayload_SyncMsg, payload.Union(), builder);
}

inline request_message make_bus_op_msg(uint64_t time_stamp, uint64_t time_offset, bus_access_type type, uint32_t index, uint64_t address,
                                       uint32_t size, bool no_response, const std::vector<uint8_t>& data,
                                       const std::vector<uint8_t>& byte_enable, uint32_t id = next_request_id()) {
    flatbuffers::FlatBufferBuilder builder;
    auto payload = CreateBusOpMsgDirect(builder, id, time_stamp, time_offset, type, index, address, size, no_response, &data, &byte_enable);
    return make_request(RequestPayload_BusOpMsg, payload.Union(), builder);
}

inline request_message make_sig_op_msg(uint64_t time_stamp, uint32_t index, uint8_t value, uint32_t id = next_request_id()) {
    flatbuffers::FlatBufferBuilder builder;
    auto payload = CreateSigOpMsg(builder, id, time_stamp, index, value);
    return make_request(RequestPayload_SigOpMsg, payload.Union(), builder);
}

inline request_message make_log_msg(const std::string& msg, uint32_t id = next_request_id()) {
    flatbuffers::FlatBufferBuilder builder;
    auto payload = CreateLogMsgDirect(builder, id, msg.c_str());
    return make_request(RequestPayload_LogMsg, payload.Union(), builder);
}

inline uint32_t get_request_id(const RequestEnvelope* req) {
    if(req == nullptr) {
        return 0;
    }
    switch(req->payload_type()) {
    case RequestPayload_NotifyEndpointMsg:
        return req->payload_as_NotifyEndpointMsg()->id();
    case RequestPayload_NotifyShutdownMsg:
        return req->payload_as_NotifyShutdownMsg()->id();
    case RequestPayload_SyncMsg:
        return req->payload_as_SyncMsg()->id();
    case RequestPayload_BusOpMsg:
        return req->payload_as_BusOpMsg()->id();
    case RequestPayload_SigOpMsg:
        return req->payload_as_SigOpMsg()->id();
    case RequestPayload_LogMsg:
        return req->payload_as_LogMsg()->id();
    default:
        return 0;
    }
}

inline response_status get_status(const ResponseEnvelope* resp) {
    if(resp == nullptr) {
        return failure;
    }
    switch(resp->payload_type()) {
    case ResponsePayload_ResponseMsg:
        return resp->payload_as_ResponseMsg()->status();
    case ResponsePayload_BusDataMsg:
        return resp->payload_as_BusDataMsg()->status();
    default:
        return failure;
    }
}

inline uint32_t get_req_id(const ResponseEnvelope* resp) {
    if(resp == nullptr) {
        return 0;
    }
    switch(resp->payload_type()) {
    case ResponsePayload_ResponseMsg:
        return resp->payload_as_ResponseMsg()->req_id();
    case ResponsePayload_BusDataMsg:
        return resp->payload_as_BusDataMsg()->req_id();
    default:
        return 0;
    }
}

inline bool belongs_to(const ResponseEnvelope* resp, const RequestEnvelope* req) { return get_req_id(resp) == get_request_id(req); }

inline response_message make_response(uint32_t req_id, response_status status = ok) {
    flatbuffers::FlatBufferBuilder builder;
    auto payload = CreateResponseMsg(builder, status, req_id);
    return make_response_message(ResponsePayload_ResponseMsg, payload.Union(), builder);
}

inline response_message make_response(const RequestEnvelope* req, response_status status = ok) {
    return make_response(get_request_id(req), status);
}

inline response_message make_bus_data_msg(uint32_t req_id, const std::vector<uint8_t>& data, response_status status = ok) {
    flatbuffers::FlatBufferBuilder builder;
    auto payload = CreateBusDataMsgDirect(builder, status, req_id, &data);
    return make_response_message(ResponsePayload_BusDataMsg, payload.Union(), builder);
}

inline response_message make_bus_data_msg(const RequestEnvelope* req, const std::vector<uint8_t>& data, response_status status = ok) {
    return make_bus_data_msg(get_request_id(req), data, status);
}

} // namespace tcp4tlm
} // namespace scc

#endif // TLM_SCC_TCP_DETAIL_MESSAGES_H_
