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
#include <bitset>
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
// according to
// http://graphics.stanford.edu/~seander/bithacks.html#ZerosOnRightMultLookup
static std::array<const int, 32> MultiplyDeBruijnBitPosition = {{0,  1,  28, 2,  29, 14, 24, 3,  30, 22, 20,
                                                                 15, 25, 17, 4,  8,  31, 27, 13, 23, 21, 19,
                                                                 16, 7,  26, 12, 18, 6,  11, 5,  10, 9}};
template <size_t N> constexpr size_t find_first(std::bitset<N> &bits) {
    static_assert(N <= 32, "find_first only supports bitsets smaller than 33");
    return MultiplyDeBruijnBitPosition[((uint32_t)((bits.to_ulong() & -bits.to_ulong()) * 0x077CB531U)) >> 27];
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
inline std::vector<std::string> split(const std::string &s, char seperator) {
    std::vector<std::string> output;
    std::string::size_type prev_pos = 0, pos = 0;
    while ((pos = s.find(seperator, pos)) != std::string::npos) {
        std::string substring(s.substr(prev_pos, pos - prev_pos));
        output.push_back(substring);
        prev_pos = ++pos;
    }
    output.push_back(s.substr(prev_pos, pos - prev_pos)); // Last word
    return output;
}
}
#endif /* _UTIL_ITIES_H_ */
