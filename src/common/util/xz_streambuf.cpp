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

#include "xz_streambuf.h"
#include <cstdint>

util::xz_streambuf::xz_streambuf(const std::string& path)
: file_(fopen(path.c_str(), "rb"))
, buffer_(64 * 1024)
, inbuf_(64 * 1024) {
    if(!file_)
        throw std::runtime_error("fopen failed");

    auto res = lzma_stream_decoder(&strm_, UINT64_MAX, 0);
    if(res != LZMA_OK)
        throw std::runtime_error("initialization of lzma_stream failed");
    setg(buffer_.data(), buffer_.data(), buffer_.data());
}

util::xz_streambuf::~xz_streambuf() {
    lzma_end(&strm_);
    if(file_)
        fclose(file_);
}

auto util::xz_streambuf::underflow() -> int_type {
    strm_.next_out = reinterpret_cast<uint8_t*>(buffer_.data());
    strm_.avail_out = buffer_.size();
    while(strm_.avail_out > 0) {
        if(strm_.avail_in == 0) {
            strm_.next_in = reinterpret_cast<uint8_t*>(inbuf_.data());
            strm_.avail_in = fread(inbuf_.data(), 1, inbuf_.size(), file_);
            if(strm_.avail_in == 0)
                break;
        }
        lzma_ret ret = lzma_code(&strm_, LZMA_RUN);
        if(ret == LZMA_STREAM_END)
            break;

        if(ret != LZMA_OK)
            return traits_type::eof();
    }
    size_t produced = buffer_.size() - strm_.avail_out;
    if(produced == 0)
        return traits_type::eof();

    setg(buffer_.data(), buffer_.data(), buffer_.data() + produced);
    return traits_type::to_int_type(*gptr());
}
