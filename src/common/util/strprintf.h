/*******************************************************************************
 * Copyright 2020-2022 MINRES Technologies GmbH
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

#ifndef SCC_INCL_UTIL_STRPRINTF_H_
#define SCC_INCL_UTIL_STRPRINTF_H_

#include <cstdarg>
#include <iostream>
#include <string>
#include <vector>
#ifdef MSVC
#define _CRT_NO_VA_START_VALIDATION
#endif

/**
 * \ingroup scc-common
 */
/**@{*/
//! @brief SCC common utilities
namespace util {
//! allocate and print to a string buffer
inline std::string strprintf(const std::string format, ...) {
    va_list args;
    va_start(args, format);
    size_t len = std::vsnprintf(NULL, 0, format.c_str(), args);
    va_end(args);
    std::vector<char> vec(len + 1);
    va_start(args, format);
    std::vsnprintf(&vec[0], len + 1, format.c_str(), args);
    va_end(args);
    return vec.data();
}
} // namespace util
/**@}*/
#endif /* SCC_INCL_UTIL_STRPRINTF_H_ */
