/*
 * tlm_extensions.h
 *
 *  Created on: 12.07.2018
 *      Author: eyck
 */

#ifndef SC_COMPONENTS_INCL_TLM_TLM_EXTENSIONS_H_
#define SC_COMPONENTS_INCL_TLM_TLM_EXTENSIONS_H_

#include "tlm_core/tlm_2/tlm_generic_payload/tlm_gp.h"

namespace tlm {

template<typename T>
struct tlm_unmanaged_extension : public tlm_extension<T> {
    using type = T;

    tlm_extension_base* clone() const override {
        return new type(static_cast<const T&>(*this));
    }

    void copy_from(tlm_extension_base const & other) override {
        this->operator=(static_cast<const type&>(other));
    }

protected:
    tlm_unmanaged_extension(){};
};

template<typename T>
struct tlm_managed_extension {

    using type = T;

    template<typename... Args>
    static type* alloacte(Args&&... args){
        auto* ret = new(pool::allocate()) type(std::forward<Args>(args)...);
        ret->is_pooled=true;
        return ret;
    }

    static type* allocate(){
        auto* ret = new(pool::allocate()) type();
        ret->is_pooled=true;
        return ret;
    }

    tlm_extension_base* clone() const override {
        return allocate(); // Maybe static_cast<const T&>(*this)
    }

    void copy_from(tlm_extension_base const & other) override {
        this->operator=(static_cast<const type&>(other));
    }

    void free() override {
        if(is_pooled){
            this->~type();
            pool::dealllocate(this);
        } else {
            delete this;
        }
    }
    struct pool {
        static void* allocate(){
            if(free_list.size()>0){
                auto ret = free_list.back();
                free_list.pop_back();
                return ret;
            } else
                return calloc(1, sizeof(type));
        }

        static void dealllocate(void* p){
            free_list.push_back(p);
        }

    private:
        static std::vector<void*> free_list;
    };

protected:
    tlm_managed_extension():is_pooled(false) {}
    tlm_managed_extension(const tlm_managed_extension&):is_pooled(false) {}
    tlm_managed_extension& operator=(const tlm_managed_extension& other){return *this;}

private:
    bool is_pooled;
};

}
#endif /* SC_COMPONENTS_INCL_TLM_TLM_EXTENSIONS_H_ */
