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

#include "gzip_streambuf.h"

util::gzip_streambuf::gzip_streambuf(const std::string& path)
: gz_(gzopen(path.c_str(), "rb"))
, buffer_(64 * 1024) {
    if(!gz_)
        throw std::runtime_error("gzopen failed");

    setg(buffer_.data(), buffer_.data(), buffer_.data());
}

util::gzip_streambuf::~gzip_streambuf() {
    if(gz_)
        gzclose(gz_);
}

auto util::gzip_streambuf::underflow() -> int_type {
    int n = gzread(gz_, buffer_.data(), buffer_.size());
    if(n <= 0)
        return traits_type::eof();

    setg(buffer_.data(), buffer_.data(), buffer_.data() + n);
    return traits_type::to_int_type(*gptr());
}
