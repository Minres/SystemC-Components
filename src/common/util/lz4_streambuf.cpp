/*******************************************************************************
 * Copyright 2022 MINRES Technologies GmbH
 * SPDX-License-Identifier: Apache-2.0
 ******************************************************************************/

#include "lz4_streambuf.h"
#include <stdexcept>

namespace util {

lz4c_steambuf::lz4c_steambuf(std::ostream &sink, size_t buf_size)
: sink(sink)
, src_buf(buf_size)
, dest_buf(LZ4F_compressBound(buf_size, nullptr))
{
    auto errCode = LZ4F_createCompressionContext(&ctx, LZ4F_VERSION);
    if (LZ4F_isError(errCode) != 0)
        throw std::runtime_error(std::string("Failed to create LZ4 context: ") + LZ4F_getErrorName(errCode));
    size_t sz = LZ4F_compressBegin(ctx, dest_buf.data(), dest_buf.capacity(), nullptr);
    if (LZ4F_isError(sz) != 0)
        throw std::runtime_error(std::string("Failed to start LZ4 compression: ") + LZ4F_getErrorName(sz));
    setp(src_buf.data(), src_buf.data() + src_buf.size() - 1);
    sink.write(dest_buf.data(), sz);
}

lz4c_steambuf::~lz4c_steambuf() {
    close();
}

void lz4c_steambuf::close() {
    if (closed)
        return;
    lz4c_steambuf::sync();
    auto sz = LZ4F_compressEnd(ctx, dest_buf.data(), dest_buf.capacity(), nullptr);
    if (LZ4F_isError(sz) != 0)
        throw std::runtime_error(std::string("Failed to finish LZ4 compression: ") + LZ4F_getErrorName(sz));
    if(sz){
        sink.write(dest_buf.data(), sz);
        sink.flush();
    }
    LZ4F_freeCompressionContext(ctx);
    closed = true;
}

std::streambuf::int_type lz4c_steambuf::sync() {
    compress_and_write();
    sink.flush();
    return 0;
}

std::streambuf::int_type lz4c_steambuf::overflow(int_type ch) {
    compress_and_write();
    *pptr() = static_cast<char_type>(ch);
    pbump(1);
    return ch;
}

void lz4c_steambuf::compress_and_write() {
    if (closed)
        throw std::runtime_error("Cannot write to closed stream");
    if (auto orig_size = pptr() - pbase()) {
        auto ret = LZ4F_compressUpdate(ctx, dest_buf.data(), dest_buf.capacity(), pbase(), orig_size, nullptr);
        if (LZ4F_isError(ret) != 0)
            throw std::runtime_error(std::string("LZ4 compression failed: ") + LZ4F_getErrorName(ret));
        if (ret)
            sink.write(dest_buf.data(), ret);
        pbump(-orig_size);
    }
}

lz4d_streambuf::lz4d_streambuf(std::istream &source, size_t buf_size)
: src_str(source)
, src_buf(buf_size)
, dest_buf(4 * buf_size)
{
    auto ret = LZ4F_createDecompressionContext(&ctx, LZ4F_VERSION);
    if (LZ4F_isError(ret) != 0)
        throw std::runtime_error(std::string("Failed to create LZ4 context: ") + LZ4F_getErrorName(ret));
    setg(src_buf.data(), src_buf.data(), src_buf.data());
}

lz4d_streambuf::~lz4d_streambuf() {
    LZ4F_freeDecompressionContext(ctx);
}

std::streambuf::int_type lz4d_streambuf::underflow() {
    auto written_size = 0UL;
    do {
        if (offset == src_buf_size) {
            src_str.read(src_buf.data(), src_buf.size());
            src_buf_size = static_cast<size_t>(src_str.gcount());
            offset = 0;
        }
        if (src_buf_size == 0)
            return traits_type::eof();
        auto src_size = src_buf_size - offset;
        auto dest_size = dest_buf.size();
        auto ret = LZ4F_decompress(ctx, dest_buf.data(), &dest_size, src_buf.data() + offset, &src_size, nullptr);
        if (LZ4F_isError(ret) != 0)
            throw std::runtime_error(std::string("LZ4 decompression failed: ") + LZ4F_getErrorName(ret));
        written_size = dest_size;
        offset += src_size;
    } while (written_size == 0);
    setg(dest_buf.data(), dest_buf.data(), dest_buf.data() + written_size);
    return traits_type::to_int_type(*gptr());
}

}
