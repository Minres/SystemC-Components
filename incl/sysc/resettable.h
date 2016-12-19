/*
 * resettable.h
 *
 *  Created on: Nov 16, 2016
 *      Author: developer
 */

#ifndef SYSC_RESETTABLE_H_
#define SYSC_RESETTABLE_H_

#include <vector>
#include "resource_access_if.h"

namespace sysc {

struct resettable {
    virtual ~resettable(){}

    void reset_start(){
        in_reset=true;
        for(auto res:resources) res->reset();
    }

    void reset_stop(){
        for(auto res:resources) res->reset();
        in_reset=false;
    }

    void register_resource(resource_access_if* res){
        resources.push_back(res);
    }
protected:
    std::vector<resource_access_if*> resources;
    bool in_reset=false;
};

} /* namespace sysc */

#endif /* SYSC_RESETTABLE_H_ */
