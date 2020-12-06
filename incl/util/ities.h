/*******************************************************************************
 * Copyright 2017, 2018 MINRES Technologies GmbH
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

#ifndef _UTIL_ITIES_H_
#define _UTIL_ITIES_H_

#include <array>
#include <assert.h>
#include <bitset>
#include <iterator>
#include <memory>
#include <sstream>
#include <sys/stat.h>
#include <type_traits>
#include <vector>

// some helper functions
template <unsigned int bit, unsigned int width, typename T> inline constexpr T bit_sub(T v) {
    return (v >> bit) & ((T(1) << width) - 1);
}

#if __cplusplus < 201402L
template <typename T, unsigned B> inline T signextend(const T x) {
#else
template <typename T, unsigned B> inline constexpr T signextend(const typename std::make_unsigned<T>::type x) {
#endif
    struct X {
        T x : B;
        X(T x_)
        : x(x_) {}
    } s(x);
    return s.x;
}

// according to http://graphics.stanford.edu/~seander/bithacks.html#FixedSignExtend
template <unsigned int bit, unsigned int width, typename T>
inline constexpr typename std::make_signed<T>::type signed_bit_sub(T v) {
#if __cplusplus < 201402L
    return ((v << (sizeof(T) * 8 - bit - width)) >> (sizeof(T) * 8 - width));
#else
    typename std::make_signed<T>::type r = v << (sizeof(T) * 8 - bit - width);
    typename std::make_signed<T>::type ret = (r >> (sizeof(T) * 8 - width));
    return ret;
#endif
}

namespace util {

template <typename T, typename... Args> std::unique_ptr<T> make_unique(Args&&... args) {
#if __cplusplus < 201402L
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
#else
    return std::make_unique<T>(std::forward<Args>(args)...);
#endif
}

// according to
// http://graphics.stanford.edu/~seander/bithacks.html#ZerosOnRightMultLookup
static std::array<const int, 32> MultiplyDeBruijnBitPosition = {{0,  1,  28, 2,  29, 14, 24, 3,  30, 22, 20,
                                                                 15, 25, 17, 4,  8,  31, 27, 13, 23, 21, 19,
                                                                 16, 7,  26, 12, 18, 6,  11, 5,  10, 9}};
template <size_t N> constexpr size_t find_first(std::bitset<N>& bits) {
    static_assert(N <= 32, "find_first only supports bitsets smaller than 33");
    return MultiplyDeBruijnBitPosition[static_cast<uint32_t>((bits.to_ulong() & -bits.to_ulong()) * 0x077CB531U) >> 27];
}

// according to
// https://stackoverflow.com/questions/8871204/count-number-of-1s-in-binary-representation
#if __cplusplus < 201402L
constexpr size_t uCount(uint32_t u) { return u - ((u >> 1) & 033333333333) - ((u >> 2) & 011111111111); }
constexpr size_t bit_count(uint32_t u) { return ((uCount(u) + (uCount(u) >> 3)) & 030707070707) % 63; }
#else
constexpr size_t bit_count(uint32_t u) {
    size_t uCount = u - ((u >> 1) & 033333333333) - ((u >> 2) & 011111111111);
    return ((uCount + (uCount >> 3)) & 030707070707) % 63;
}
#endif
/**
 * split a given string using specified separator
 *
 * @param s the string to split
 * @param separator the separator char
 * @return vector of splitted strings
 */
inline std::vector<std::string> split(const std::string& s, char separator) {
    std::vector<std::string> output;
    std::string::size_type prev_pos = 0;
    std::string::size_type pos = 0;
    while((pos = s.find(separator, pos)) != std::string::npos) {
        std::string substring(s.substr(prev_pos, pos - prev_pos));
        output.push_back(substring);
        prev_pos = ++pos;
    }
    output.push_back(s.substr(prev_pos, pos - prev_pos)); // Last word
    return output;
    /* could also be done similar to this
        // construct a stream from the string
        std::stringstream ss(str);
        // use stream iterators to copy the stream to the vector as whitespace separated strings
        std::istream_iterator<std::string> it(ss);
        std::istream_iterator<std::string> end;
        std::vector<std::string> results(it, end);
        return results;
    */
}

/*! note: delimiter cannot contain NUL characters
 */
template <typename Range, typename Value = typename Range::value_type>
std::string join(Range const& elements, char const* const delimiter) {
    std::ostringstream os;
    auto b = std::begin(elements);
    auto e = std::end(elements);
    if(b != e) {
        std::copy(b, std::prev(e), std::ostream_iterator<Value>(os, delimiter));
        b = std::prev(e);
    }
    if(b != e)
        os << *b;
    return os.str();
}

/*! note: imput is assumed to not contain NUL characters
 */
template <typename Input, typename Output, typename Value = typename Output::value_type>
void split(char delimiter, Output& output, Input const& input) {
    for(auto cur = std::begin(input), beg = cur;; ++cur) {
        if(cur == std::end(input) || *cur == delimiter || !*cur) {
            output.insert(std::end(output), Value(beg, cur));
            if(cur == std::end(input) || !*cur)
                break;
            beg = std::next(cur);
        }
    }
}
/**
 * @fn bool iequals(const std::string&, const std::string&)
 * @brief compare two string ignoring case
 *
 * @param a string a to compare
 * @param b string b to compare
 * @result true if the are equal otherwise false
 */
inline bool iequals(const std::string& a, const std::string& b) {
#if __cplusplus < 201402L
    auto sz = a.size();
    if(b.size() != sz)
        return false;
    for(auto i = 0U; i < sz; ++i)
        if(tolower(a[i]) != tolower(b[i]))
            return false;
    return true;
#else
    return std::equal(a.begin(), a.end(), b.begin(), b.end(), [](char a, char b) { return tolower(a) == tolower(b); });
#endif
}
/**
 * @fn std::string padded(std::string, size_t, bool=true)
 * @brief pad a string to a given length by either cutting of the overflow or inserting an ellipsis
 *
 * @param str string to adjust
 * @param width of the targeted field
 * @param show_ellipsis use ellipsis (...) when shortening
 * @return string with the given length
 */
inline std::string padded(std::string str, size_t width, bool show_ellipsis = true) {
    if(width < 7)
        return str;
    if(str.length() > width) {
        if(show_ellipsis) {
            auto pos = str.size() - (width - 6);
            return str.substr(0, 3) + "..." + str.substr(pos, str.size() - pos);
        } else
            return str.substr(0, width);
    } else {
        return str + std::string(width - str.size(), ' ');
    }
}
/**
 * checks if a file exists
 * @param name the file name
 * @return true if file exists and can be opened
 */
inline bool file_exists(const std::string& name) {
    struct stat buffer;
    return (stat(name.c_str(), &buffer) == 0);
}
/**
 * return directory name portion of a given path (as string)
 * @param path
 * @param delims the path delimiters to use
 * @return
 */
template <class T> inline T dir_name(T const& path, T const& delims = "/\\") {
    auto pos = path.find_last_of(delims);
    return pos > path.length() ? "." : path.substr(0, pos);
}
/**
 * return file name portion of a given path (as string)
 * @param path
 * @param delims the path delimiters to use
 * @return
 */
template <class T> inline T base_name(T const& path, T const& delims = "/\\") {
    auto pos = path.find_last_of(delims);
    return pos > path.length() ? path : path.substr(pos + 1);
}
/**
 * return the base name (without extension) of a file name (as string)
 * @param filename
 * @return
 */
template <class T> inline T remove_ext(T const& filename) {
    typename T::size_type const p(filename.find_last_of('.'));
    return p > 0 && p != T::npos ? filename.substr(0, p) : filename;
}
} // namespace util
#endif /* _UTIL_ITIES_H_ */
