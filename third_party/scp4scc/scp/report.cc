/*******************************************************************************
 * Copyright 2017-2022 MINRES Technologies GmbH
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
/*
 * report.cpp
 *
 *  Created on: 19.09.2017
 *      Author: eyck@minres.com
 */

#include "report.h"

sc_core::sc_verbosity scp::scp_logger_cache::get_log_verbosity_cached(const char* scname, const char* tname = "") {
    if(level != sc_core::SC_UNSET) {
        return level;
    }

    if(!scname && features.size())
        scname = features[0].c_str();
    if(!scname)
        scname = "";

    type = std::string(scname);

#ifdef HAS_CCI
    try {
        // we rely on there being a broker, allow this to throw if not
        auto broker = sc_core::sc_get_current_object() ? cci::cci_get_broker() : cci::cci_get_global_broker(scp_global_originator);

        std::multimap<int, std::string, std::greater<int>> allfeatures;

        /* initialize */
        for(auto scn = split(scname); scn.size(); scn.pop_back()) {
            for(int first = 0; first < scn.size(); first++) {
                auto f = scn.begin() + first;
                std::vector<std::string> p(f, scn.end());
                auto scn_str = ((first > 0) ? "*." : "") + join(p);

                for(auto ft : features) {
                    for(auto ftn = split(ft); ftn.size(); ftn.pop_back()) {
                        insert(allfeatures, scn_str + "." + join(ftn), first == 0);
                    }
                }
                insert(allfeatures, scn_str + "." + demangle(tname), first == 0);
                insert(allfeatures, scn_str, first == 0);
            }
        }
        for(auto ft : features) {
            for(auto ftn = split(ft); ftn.size(); ftn.pop_back()) {
                insert(allfeatures, join(ftn), true);
                insert(allfeatures, "*." + join(ftn), false);
            }
        }
        insert(allfeatures, demangle(tname), true);
        insert(allfeatures, "*", false);
        insert(allfeatures, "", false);

        for(std::pair<int, std::string> f : allfeatures) {
            sc_core::sc_verbosity v = cci_lookup(broker, f.second);
            if(v != sc_core::SC_UNSET) {
                level = v;
                return v;
            }
        }
    } catch(const std::exception&) {
        // If there is no global broker, revert to initialized verbosity level
    }

#endif

    return level = static_cast<sc_core::sc_verbosity>(::sc_core::sc_report_handler::get_verbosity_level());
}
