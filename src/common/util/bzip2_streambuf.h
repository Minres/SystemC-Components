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

#ifndef _UTIL_BZIP2_STREAMBUF_H_
#define _UTIL_BZIP2_STREAMBUF_H_

#include <bzlib.h>
#include <cctype>
#include <climits>
#include <fstream>
#include <stdexcept>
#include <string>
#include <sys/stat.h>
#include <vector>

namespace util {
// ============================================================
// BZIP2
// ============================================================
class bzip2_streambuf : public std::streambuf {
public:
    explicit bzip2_streambuf(const std::string& path);

    ~bzip2_streambuf() override;

protected:
    int_type underflow() override;

private:
    FILE* file_;
    BZFILE* bz_;
    int bzerror_;
    std::vector<char> buffer_;
};

class bzip2_stream : public std::istream {
public:
    bzip2_stream() = delete;
    explicit bzip2_stream(const std::string& path)
    : buf_(path) {
        rdbuf(&buf_);
    }

private:
    bzip2_streambuf buf_;
};

} // namespace util
#endif // _UTIL_BZIP2_STREAMBUF_H_
