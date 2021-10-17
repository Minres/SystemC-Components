/*
 * tlm_ahb_extension.h
 *
 *  Created on: 12.12.2019
 *      Author: developer
 */

#ifndef _OBI_TLM_H_
#define _OBI_TLM_H_

#include <array>
#include <cstdint>
#include <tlm>

//! TLM2.0 components modeling OBI
namespace obi {

struct obi_extension : public tlm::tlm_extension<obi_extension> {

    uint32_t get_id() const;
    void set_id(uint32_t);

    uint32_t get_auser() const;
    void set_auser(uint32_t);

    uint32_t get_duser() const;
    void set_duser(uint32_t);

    obi_extension() = default;

    obi_extension(const obi_extension& o) = default;
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
    uint32_t id{0};
    uint32_t auser{0};
    uint32_t duser{0};
};

inline tlm::tlm_extension_base* obi_extension::clone() const { return new obi_extension(*this); }

inline uint32_t obi_extension::get_id() const {
    return id;
}

inline void obi_extension::set_id(uint32_t unsignedInt) {
    id=unsignedInt;
}

inline uint32_t obi_extension::get_auser() const {
    return auser;
}

inline void obi_extension::set_auser(uint32_t unsignedInt) {
    auser=unsignedInt;
}

inline uint32_t obi_extension::get_duser() const {
    return duser;
}

inline void obi_extension::set_duser(uint32_t unsignedInt) {
    duser=unsignedInt;
}

inline void obi_extension::copy_from(const tlm::tlm_extension_base& ext) {
    auto const* ahb_ext = dynamic_cast<const obi_extension*>(&ext);
    assert(ahb_ext);
    (*this) = *ahb_ext;
}

inline unsigned get_obi_id(tlm::tlm_generic_payload& trans){
    auto* ext = trans.get_extension<obi::obi_extension>();
}
} // namespace obi

#endif /* _OBI_TLM_H_ */
