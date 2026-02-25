#define CATCH_CONFIG_MAIN
#include <catch2/catch_all.hpp>

#include <bzlib.h>
#include <lzma.h>
#include <zlib.h>
#include <zstd.h>

#include <cstdio>
#include <fstream>
#include <sstream>
#include <vector>

#include <util/bzip2_streambuf.h>
#include <util/gzip_streambuf.h>
#include <util/xz_streambuf.h>
#include <util/zstd_streambuf.h>
// ------------------------------------------------------------
// Helpers
// ------------------------------------------------------------

using namespace util;

static std::string sampleText() {
    std::ostringstream oss;
    for(int i = 0; i < 10000; ++i)
        oss << "Line " << i << " Hello MINRES\n";
    return oss.str();
}

static std::string readAll(std::istream& in) {
    std::ostringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

// ------------------------------------------------------------
// GZIP
// ------------------------------------------------------------

static void compressGzip(const std::string& path, const std::string& data) {
    gzFile gz = gzopen(path.c_str(), "wb");
    REQUIRE(gz != nullptr);
    gzwrite(gz, data.data(), data.size());
    gzclose(gz);
}

TEST_CASE("gzip_stream decompresses", "[gzip]") {
    std::string text = sampleText();
    std::string file = "test.gz";

    compressGzip(file, text);

    gzip_stream stream(file);
    REQUIRE(stream.good());

    std::string result = readAll(stream);
    REQUIRE(result == text);

    std::remove(file.c_str());
}

// ------------------------------------------------------------
// BZIP2
// ------------------------------------------------------------

static void compressBzip2(const std::string& path, const std::string& data) {
    FILE* f = fopen(path.c_str(), "wb");
    REQUIRE(f != nullptr);

    int bzerr;
    BZFILE* bz = BZ2_bzWriteOpen(&bzerr, f, 9, 0, 0);
    REQUIRE(bzerr == BZ_OK);

    BZ2_bzWrite(&bzerr, bz, const_cast<char*>(data.data()), data.size());
    REQUIRE(bzerr == BZ_OK);

    BZ2_bzWriteClose(&bzerr, bz, 0, nullptr, nullptr);
    fclose(f);
}

TEST_CASE("bzip2_stream decompresses", "[bzip2]") {
    std::string text = sampleText();
    std::string file = "test.bz2";

    compressBzip2(file, text);

    bzip2_stream stream(file);
    REQUIRE(stream.good());

    std::string result = readAll(stream);
    REQUIRE(result == text);

    std::remove(file.c_str());
}

// ------------------------------------------------------------
// XZ
// ------------------------------------------------------------

static void compressXz(const std::string& path, const std::string& data) {
    FILE* f = fopen(path.c_str(), "wb");
    REQUIRE(f != nullptr);

    lzma_stream strm = LZMA_STREAM_INIT;
    REQUIRE(lzma_easy_encoder(&strm, 6, LZMA_CHECK_CRC64) == LZMA_OK);

    std::vector<uint8_t> outbuf(65536);

    strm.next_in = reinterpret_cast<const uint8_t*>(data.data());
    strm.avail_in = data.size();

    while(true) {
        strm.next_out = outbuf.data();
        strm.avail_out = outbuf.size();

        lzma_ret ret = lzma_code(&strm, LZMA_FINISH);

        size_t writeSize = outbuf.size() - strm.avail_out;
        fwrite(outbuf.data(), 1, writeSize, f);

        if(ret == LZMA_STREAM_END)
            break;

        REQUIRE(ret == LZMA_OK);
    }

    lzma_end(&strm);
    fclose(f);
}

TEST_CASE("xz_stream decompresses", "[xz]") {
    std::string text = sampleText();
    std::string file = "test.xz";

    compressXz(file, text);

    xz_stream stream(file);
    REQUIRE(stream.good());

    std::string result = readAll(stream);
    REQUIRE(result == text);

    std::remove(file.c_str());
}

// ------------------------------------------------------------
// ZSTD
// ------------------------------------------------------------

static void compressZstd(const std::string& path, const std::string& data) {
    size_t bound = ZSTD_compressBound(data.size());
    std::vector<char> out(bound);

    size_t compressed = ZSTD_compress(out.data(), bound, data.data(), data.size(), 3);

    REQUIRE_FALSE(ZSTD_isError(compressed));

    std::ofstream file(path, std::ios::binary);
    file.write(out.data(), compressed);
}
/*
TEST_CASE("ZstdStream decompresses correctly", "[zstd]")
{
    std::string text = sampleText();
    std::string file = "test.zst";

    compressZstd(file, text);

    zstd_stream stream(file);
    REQUIRE(stream.good());

    std::string result = readAll(stream);
    REQUIRE(result == text);

    std::remove(file.c_str());
}
*/
// ------------------------------------------------------------
// Line-by-line streaming test
// ------------------------------------------------------------

TEST_CASE("Stream supports getline()", "[stream]") {
    std::string text = sampleText();
    std::string file = "test.gz";

    compressGzip(file, text);

    gzip_stream stream(file);

    std::string line;
    int count = 0;
    while(std::getline(stream, line))
        ++count;

    REQUIRE(count == 10000);

    std::remove(file.c_str());
}
