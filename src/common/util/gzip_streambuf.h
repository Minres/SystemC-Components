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

#ifndef _UTIL_GZIP_STREAMBUF_H_
#define _UTIL_GZIP_STREAMBUF_H_

#include <cctype>
#include <climits>
#include <fstream>
#include <stdexcept>
#include <string>
#include <sys/stat.h>
#include <vector>
#include <zlib.h>

namespace util {
// ============================================================
// GZIP
// ============================================================

class gzip_streambuf : public std::streambuf {
public:
    explicit gzip_streambuf(const std::string& path);

    ~gzip_streambuf() override;

protected:
    int_type underflow() override;

private:
    gzFile gz_;
    std::vector<char> buffer_;
};

class gzip_stream : public std::istream {
public:
    gzip_stream() = delete;
    explicit gzip_stream(const std::string& path)
    : buf_(path) {
        rdbuf(&buf_);
    }

private:
    gzip_streambuf buf_;
};

} // namespace util
#endif // _UTIL_GZIP_STREAMBUF_H_
