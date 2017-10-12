/*******************************************************************************
 * Copyright 2017 MINRES Technologies GmbH
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

#include <bitset>
#include <vector>
#include <type_traits>

// some helper functions
template <unsigned int bit, unsigned int width, typename T> inline constexpr T bit_sub(T v) {
    return (v >> bit) & ((T(1) << width) - 1);
}

template <unsigned int bit, unsigned int width, typename T>
inline constexpr typename std::make_signed<T>::type signed_bit_sub(T v) {
    typename std::make_signed<T>::type r = v << (sizeof(T) * 8 - bit - width);
    typename std::make_signed<T>::type ret = (r >> (sizeof(T) * 8 - width));
    return ret;
}

namespace util {
// according to
// http://graphics.stanford.edu/~seander/bithacks.html#ZerosOnRightMultLookup
static const int MultiplyDeBruijnBitPosition[32] = {0,  1,  28, 2,  29, 14, 24, 3, 30, 22, 20, 15, 25, 17, 4,  8,
                                                    31, 27, 13, 23, 21, 19, 16, 7, 26, 12, 18, 6,  11, 5,  10, 9};
template <size_t N> constexpr size_t find_first(std::bitset<N> &bits) {
    static_assert(N <= 32, "find_first only supports bitsets smaller than 33");
    return MultiplyDeBruijnBitPosition[((uint32_t)((bits.to_ulong() & -bits.to_ulong()) * 0x077CB531U)) >> 27];
}

// according to
// https://stackoverflow.com/questions/8871204/count-number-of-1s-in-binary-representation
constexpr size_t bit_count(uint32_t u) {
    size_t uCount = u - ((u >> 1) & 033333333333) - ((u >> 2) & 011111111111);
    return ((uCount + (uCount >> 3)) & 030707070707) % 63;
}

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

//std::vector<std::string> split(std::string& str, char split_char){
//    std::vector<std::string> res;
//    auto split_pos=str.find_first_of(split_char);
//    decltype(split_pos) start{0};
//    while(start!=str.length()){
//        res.push_back(str.substr(start, start-split_pos));
//        start=std::min(split_pos+1, str.length());
//    }
//    return res;
//}
}
#endif /* _UTIL_ITIES_H_ */
