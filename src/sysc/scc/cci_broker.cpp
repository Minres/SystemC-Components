/*******************************************************************************
 * Copyright 2022 MINRES Technologies GmbH
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

#include <string>
#ifdef HAS_CCI
#include "cci_broker.h"
#include <util/ities.h>
namespace {
bool glob_match(std::string const& s, std::string const& p) {
    const char* cp = nullptr;
    const char* mp = nullptr;
    const char* string = s.c_str();
    const char* pat = p.c_str();
    while((*string) && (*pat != '*')) {
        if((*pat != *string) && (*pat != '?'))
            return false;

        pat++;
        string++;
    }
    while(*string) {
        if(*pat == '*') {
            if(!*++pat)
                return true;

            mp = pat;
            cp = string + 1;
        } else if((*pat == *string) || (*pat == '?')) {
            pat++;
            string++;
        } else {
            pat = mp;
            string = cp++;
        }
    }
    while(*pat == '*')
        pat++;
    return *pat == 0;
}

std::string glob_to_regex(std::string val){
    util::trim(val);
    const char* expression = "(\\*)|(\\?)|([[:blank:]])|(\\.|\\+|\\^|\\$|\\[|\\]|\\(|\\)|\\{|\\}|\\\\)";
    const char* format = "(?1\\\\w+)(?2\\.)(?3\\\\s*)(?4\\\\$&)";
    std::ostringstream oss;
    oss << "^.*";
    std::ostream_iterator<char, char> oi(oss);
    std::regex re;
    re.assign(expression);
    std::regex_replace(oi, val.begin(), val.end(), re, format, std::regex_constants::match_default | std::regex_constants::format_default);
    oss << ".*" << std::ends;
    return oss.str();
}
}
namespace scc {
using namespace cci;

void cci_broker::set_preset_cci_value(
        const std::string &parname,
        const cci_value & value,
        const cci_originator& originator) {
    super::set_preset_cci_value(parname, value, originator);
}

cci_originator cci_broker::get_preset_value_origin(const std::string &parname) const { //TODO: check globs
    return super::get_preset_value_origin(parname);
}

cci_value cci_broker::get_preset_cci_value(const std::string &parname) const { //TODO: check globs
    return super::get_preset_cci_value(parname);
}

void cci_broker::lock_preset_value(const std::string &parname) {
    super::lock_preset_value(parname);
}

cci_value cci_broker::get_cci_value(const std::string &parname, const cci_originator &originator) const {
    return consuming_broker::get_cci_value(parname, originator);
}

bool cci_broker::has_preset_value(const std::string &parname) const { //TODO: check globs
    return super::has_preset_value(parname);
}

void init_cci(std::string name) {
    thread_local cci_broker broker(name);
    cci::cci_register_broker(broker);
}
}
#else
namespace scc {
void init_cci(std::string name) {
}
}
#endif
