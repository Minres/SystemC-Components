/*
 * ace_axi_adapt.h
 *
 *  Created on: Aug 13, 2022
 *      Author: tientr
 */
#ifndef __GENERIC_PAYLOAD_H__
#define __GENERIC_PAYLOAD_H__

#include <tlm>
#include <axi/axi_tlm.h>

template <typename TYPES> class generic_extension : public tlm::tlm_extension<generic_extension<TYPES>> {
public:
    generic_extension() = default;
    generic_extension(const generic_extension& o) = default;
    /**
     * @brief Set the bw if ptr object
     * 
     * @param bw_if 
     */
    void set_bw_if_ptr(TYPES* bw_if);
    /**
     * @brief Get the bw if ptr object
     * 
     * @return tlm::tlm_bw_transport_if<TYPES>* 
     */
    TYPES* get_bw_if_ptr();
    /**
     * @brief the clone function to create deep copies of
     * @return pointer to heap-allocated extension
     */
    tlm::tlm_extension_base* clone() const override;
    /**
     * @brief deep copy all values from ext
     * @param ext
     */
    void copy_from(tlm::tlm_extension_base const& ext) override;

private:
    TYPES* bw_if;
};

template <typename TYPES> tlm::tlm_extension_base* generic_extension<TYPES>::clone() const {
    return new generic_extension<TYPES>(*this);
}

template <typename TYPES> void generic_extension<TYPES>::set_bw_if_ptr(TYPES* bw_if){
    this->bw_if = bw_if;
}

template <typename TYPES> TYPES* generic_extension<TYPES>::get_bw_if_ptr(){
    return this->bw_if;
}

template <typename TYPES>
inline void generic_extension<TYPES>::copy_from(const tlm::tlm_extension_base& ext) {
    auto const* generic_ext = dynamic_cast<const generic_extension<TYPES>*>(&ext);
    assert(generic_ext);
    (*this) = *generic_ext;
}

#endif
