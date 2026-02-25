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

#include "bzip2_streambuf.h"

util::bzip2_streambuf::bzip2_streambuf(const std::string& path)
: file_(fopen(path.c_str(), "rb"))
, buffer_(64 * 1024) {
    if(!file_)
        throw std::runtime_error("fopen failed");

    bz_ = BZ2_bzReadOpen(&bzerror_, file_, 0, 0, nullptr, 0);
    if(bzerror_ != BZ_OK)
        throw std::runtime_error("BZ2_bzReadOpen failed");

    setg(buffer_.data(), buffer_.data(), buffer_.data());
}

util::bzip2_streambuf::~bzip2_streambuf() {
    if(bz_)
        BZ2_bzReadClose(&bzerror_, bz_);

    if(file_)
        fclose(file_);
}

auto util::bzip2_streambuf::underflow() -> int_type {
    int n = BZ2_bzRead(&bzerror_, bz_, buffer_.data(), buffer_.size());
    if(bzerror_ != BZ_OK && bzerror_ != BZ_STREAM_END)
        return traits_type::eof();

    if(n <= 0)
        return traits_type::eof();

    setg(buffer_.data(), buffer_.data(), buffer_.data() + n);
    return traits_type::to_int_type(*gptr());
}
