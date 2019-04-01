/*
 * tlm_id.h
 *
 *  Created on: Mar 9, 2019
 *      Author: eyck
 */

#pragma once

#include <tlm>
#include <cstdint>

struct tlm_id_extension : public tlm::tlm_extension<tlm_id_extension> {
    virtual tlm_extension_base *clone() const { tlm_id_extension *t = new tlm_id_extension(this->id);return t; }
    virtual void copy_from(tlm_extension_base const &from) { id = static_cast<tlm_id_extension const &>(from).id; }
    tlm_id_extension(unsigned i) : id(i) {}
    unsigned id;
};

inline intptr_t getId(tlm::tlm_generic_payload& gp){
//    auto ext = gp.get_extension<tlm_id_extension>();
//    if(ext)
//        return ext->id;
//    else
        return (intptr_t)&gp;
}

inline intptr_t getId(tlm::tlm_generic_payload* gp){
//    auto ext = gp->get_extension<tlm_id_extension>();
//    if(ext)
//        return ext->id;
//    else
        return (intptr_t)gp;
}

inline void setId(tlm::tlm_generic_payload& gp, unsigned id){
    auto ext = gp.get_extension<tlm_id_extension>();
    if(ext)
        ext->id=id;
    else
        if(gp.has_mm())
            gp.set_auto_extension(new tlm_id_extension(id));
        else
            gp.set_extension(new tlm_id_extension(id));
}
