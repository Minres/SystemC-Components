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
/**
 * @brief A utility class for parsing Intel Hex (IHEX) files.
 *
 * The Intel Hex (IHEX) file format is a common format for representing binary data in a text file.
 * It is widely used in embedded systems and programmable logic devices.
 *
 * The ihex_parser class provides a method for parsing IHEX files and invoking a callback function for each data record found in the file.
 *
 * @author Your Name
 * @date YYYY-MM-DD
 */
struct ihex_parser {
    /**
     * @brief The maximum size of data in a single IHEX record.
     */
    enum { IHEX_DATA_SIZE = 255 };
    /**
     * @brief Parses an IHEX file from the given input stream and invokes a callback function for each data record found.
     *
     * @param input The input stream containing the IHEX file.
     * @param callback A function object that will be invoked for each data record found in the IHEX file.
     * The callback function should return true to continue parsing, or false to stop parsing.
     *
     * @return True if the parsing was successful, false otherwise.
     */
    static bool parse(std::istream&, std::function<bool(uint64_t, uint64_t, const uint8_t*)>);
};
} // namespace util
#endif
