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

#ifndef TLM_SCC_TCP_DETAIL_ISERVER_H_
#define TLM_SCC_TCP_DETAIL_ISERVER_H_

#include "serialized_connection.h"
#include <boost/asio.hpp>
namespace tlm {
namespace scc {
namespace tcp {
namespace impl {

template <typename REQ, typename RESP> struct IServer {

    virtual ~IServer() {}
    virtual boost::asio::io_context& getIoService() = 0;
    virtual void serverSendCompleted(boost::shared_ptr<connection<RESP, REQ>>& con, bool established = false) = 0;
    virtual void serverReceiveCompleted(boost::shared_ptr<connection<RESP, REQ>>& con, const REQ* const result) = 0;
    virtual bool isShutdownRequested() { return false; }
};
} // namespace impl
} // namespace tcp
} // namespace scc
} // namespace tlm

#endif // TLM_SCC_TCP_DETAIL_ISERVER_H_
