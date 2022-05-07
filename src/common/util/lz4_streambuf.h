/*******************************************************************************
 * Copyright 2022 MINRES Technologies GmbH
 * SPDX-License-Identifier: Apache-2.0
 *******************************************************************************
 *
 *  Created on: May 7, 2022
 *      Author: eyck
 */

#ifndef _UTIL_LZ4_STREAMBUF_H_
#define _UTIL_LZ4_STREAMBUF_H_

#include <lz4frame.h>
#include <istream>
#include <ostream>
#include <streambuf>
#include <vector>
#include <cassert>

namespace util {
class lz4c_steambuf: public std::streambuf {
public:
    lz4c_steambuf(std::ostream &sink, size_t buf_size);

    ~lz4c_steambuf();

    void close();

    lz4c_steambuf(const lz4c_steambuf&) = delete;
    lz4c_steambuf(lz4c_steambuf&&) = delete;
    lz4c_steambuf& operator=(const lz4c_steambuf&) = delete;
    lz4c_steambuf& operator=(lz4c_steambuf&&) = delete;
private:
    int_type overflow(int_type ch) override;

    int_type sync() override;

    void compress_and_write();

    std::ostream &sink;
    std::vector<char> src_buf;
    std::vector<char> dest_buf;
    LZ4F_compressionContext_t ctx{ nullptr };
    bool closed{ false };
};


class lz4d_streambuf: public std::streambuf {
public:
    lz4d_streambuf(std::istream &source, size_t buf_size);

    ~lz4d_streambuf();

    int_type underflow() override;

    lz4d_streambuf(const lz4d_streambuf&) = delete;
    lz4d_streambuf(lz4d_streambuf&&) = delete;
    lz4d_streambuf& operator=(const lz4d_streambuf&) = delete;
    lz4d_streambuf& operator=(lz4d_streambuf&&) = delete;
private:
    std::istream &src_str;
    std::vector<char> src_buf;
    std::vector<char> dest_buf;
    size_t offset{ 0 };
    size_t src_buf_size{ 0 };
    LZ4F_decompressionContext_t ctx{ nullptr };
};

}

#endif /* _UTIL_LZ4_STREAMBUF_H_ */
