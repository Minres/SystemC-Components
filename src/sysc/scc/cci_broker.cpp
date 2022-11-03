/*
 * cci_broker.cpp
 *
 *  Created on: Nov 2, 2022
 *      Author: eyck
 */

#include <string>
#ifdef HAS_CCI
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <regex>
#include <util/ities.h>
#include <cci_cfg/cci_broker_handle.h>
#include <cci_cfg/cci_broker_if.h>
#include <cci_cfg/cci_broker_manager.h>
#include <cci_cfg/cci_config_macros.h>
#include <cci_cfg/cci_param_if.h>
#include <cci_cfg/cci_report_handler.h>
#include <cci_utils/consuming_broker.h>

namespace {
using namespace cci;
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

struct scc_broker: public cci_utils::consuming_broker {
    using super = cci_utils::consuming_broker;

    explicit scc_broker(const std::string& name): cci_utils::consuming_broker(name) { }

    ~scc_broker() = default;

    void set_preset_cci_value(
            const std::string &parname,
            const cci_value & value,
            const cci_originator& originator) override {
        super::set_preset_cci_value(parname, value, originator);
    }

    std::vector<cci_name_value_pair> get_unconsumed_preset_values() const override {
        return super::get_unconsumed_preset_values();
    }

    cci_preset_value_range get_unconsumed_preset_values(const cci_preset_value_predicate &pred) const override {
        return super::get_unconsumed_preset_values(pred);
    }

    void ignore_unconsumed_preset_values(const cci_preset_value_predicate &pred) override {
        super::ignore_unconsumed_preset_values(pred);
    }

    cci_originator get_preset_value_origin(const std::string &parname) const override { //TODO: check globs
        return super::get_preset_value_origin(parname);
    }

    cci_value get_preset_cci_value(const std::string &parname) const override { //TODO: check globs
        return super::get_preset_cci_value(parname);
    }

    void lock_preset_value(const std::string &parname) override {
        super::lock_preset_value(parname);
    }

    cci_value get_cci_value(const std::string &parname, const cci_originator &originator) const override {
        return super::get_cci_value(parname, originator);
    }

    bool has_preset_value(const std::string &parname) const override { //TODO: check globs
        return super::has_preset_value(parname);
     }

    void add_param(cci_param_if* par) override {
        super::add_param(par);
    }

    bool is_global_broker() const override {
        return true;
    }

};
}
namespace scc {
void init_cci(std::string name) {
    thread_local scc_broker broker(name);
    cci::cci_register_broker(broker);
}
}
#else
namespace scc {
void init_cci(std::string name) {
}
}
#endif
