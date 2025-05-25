/*******************************************************************************
 * Copyright 2025 MINRES Technologies GmbH
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

#ifndef _UTIL_IHEX_PARSER_H_
#define _UTIL_IHEX_PARSER_H_

#include <cstdint>
#include <functional>
#include <iostream>
/**
 * \ingroup scc-common
 */
/**@{*/
namespace util {
struct ihex_parser {

    enum { IHEX_DATA_SIZE = 255 };

    static bool parse(std::istream&, std::function<bool(uint64_t, uint64_t, const uint8_t*)>);
};
} // namespace util
#endif
