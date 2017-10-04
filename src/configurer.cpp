/*
 * configurer.cpp
 *
 *  Created on: 27.09.2017
 *      Author: eyck
 */


#include <sysc/configurer.h>
#include <sysc/report.h>
#include <istream>


sysc::configurer::configurer(const std::string& filename):base_type(sc_core::sc_module_name("configurer")) {
    if(filename.length()>0){
        std::ifstream is(filename);
        if(is.is_open()){
            try {
                is>>root;
            }catch(Json::RuntimeError& e){
                LOG(ERROR)<<"Could not parse input file "<<filename<<", reason: "<<e.what();
            }
        } else {
            LOG(ERROR)<<"Could not open input file "<<filename;
        }
    }
}

void sysc::configurer::configure_sc_object(sc_core::sc_object* obj, Json::Value& hier_val) {
    sc_core::sc_attr_base* ab = dynamic_cast<sc_core::sc_attr_base*>(obj);
    if(ab!=nullptr){

    }else{
        sc_core::sc_module* mod=dynamic_cast<sc_core::sc_module*>(obj);
        if(mod!=nullptr){
            for(auto* o: mod->get_child_objects()){
                Json::Value& val = root[o->basename()];
                if(!val.isNull()) configure_sc_object(o, val);
            }
        }
    }
}

#define CHECK_N_ASSIGN(TYPE, ATTR, VAL) {\
    auto* a = dynamic_cast<sc_core::sc_attribute<TYPE>*>(ATTR); \
    if(a!=nullptr) { a->value=VAL; return; } \
}

void sysc::configurer::set_value(sc_core::sc_attr_base* attr_base, Json::Value& hier_val) {
    CHECK_N_ASSIGN(int, attr_base, hier_val.asInt());
    CHECK_N_ASSIGN(unsigned, attr_base, hier_val.asUInt());
    CHECK_N_ASSIGN(int64_t, attr_base, hier_val.asInt64());
    CHECK_N_ASSIGN(uint64_t, attr_base, hier_val.asUInt64());
    CHECK_N_ASSIGN(bool, attr_base, hier_val.asBool());
    CHECK_N_ASSIGN(float, attr_base, hier_val.asFloat());
    CHECK_N_ASSIGN(double, attr_base, hier_val.asDouble());
    CHECK_N_ASSIGN(std::string, attr_base, hier_val.asString());
    CHECK_N_ASSIGN(char*, attr_base, strdup(hier_val.asCString()));
    CHECK_N_ASSIGN(bool, attr_base, hier_val.asBool());
}

Json::Value& sysc::configurer::get_value_from_hierarchy(const std::string& hier_name) {
    size_t npos= hier_name.find_first_of('.');
    Json::Value& val = root[hier_name.substr(0, npos)];
    if(val.isNull() || npos==hier_name.size()) return val;
    return get_value_from_hierarchy(hier_name.substr(npos+1, hier_name.size()), val);
}

Json::Value& sysc::configurer::get_value_from_hierarchy(const std::string& hier_name, Json::Value& value) {
    size_t npos= hier_name.find_first_of('.');
    Json::Value& val = value[hier_name.substr(0, npos)];
    if(val.isNull() || npos==hier_name.size()) return val;
    return get_value_from_hierarchy(hier_name.substr(npos+1, hier_name.size()), val);
}
