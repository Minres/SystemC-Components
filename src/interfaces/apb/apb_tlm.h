/*******************************************************************************
 * Copyright 2019-2024 MINRES Technologies GmbH
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

#ifndef _APB_TLM_H_
#define _APB_TLM_H_

#include <array>
#include <cstdint>
#include <tlm>

//! TLM2.0 components modeling APB
namespace apb {

/**
 *
 * @param os
 * @param e
 * @return
 */
std::ostream& operator<<(std::ostream& os, tlm::tlm_generic_payload const& t);

struct apb_extension : public tlm::tlm_extension<apb_extension> {
    // PPROT[0]
    bool is_privileged() const;
    void set_privileged(bool = true);
    // HPROT[2]
    void set_non_secure(bool = true);
    bool is_non_secure() const;
    // PPROT[2]
    bool is_instruction() const;
    void set_instruction(bool = true);

    uint8_t get_protection() const;
    void set_protection(uint8_t);

    // PNSE
    void set_nse(bool = true);
    bool is_nse() const;

    apb_extension() = default;

    apb_extension(const apb_extension& o) = default;
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
    enum { PRIV = 1, NON_SEC = 2, INSTR = 4, NSE = 8 };
    uint8_t ext_prot{0};
};

/*****************************************************************************
 * Implementation details
 *****************************************************************************/
inline bool apb_extension::is_privileged() const { return ext_prot & PRIV; }

inline void apb_extension::set_privileged(bool priv) {
    if(priv)
        ext_prot |= PRIV;
    else
        ext_prot &= ~PRIV;
}

inline bool apb_extension::is_non_secure() const { return ext_prot & NON_SEC; }

inline void apb_extension::set_non_secure(bool priv) {
    if(priv)
        ext_prot |= NON_SEC;
    else
        ext_prot &= ~NON_SEC;
}

inline bool apb_extension::is_instruction() const { return ext_prot & INSTR; }

inline void apb_extension::set_instruction(bool instr) {
    if(instr)
        ext_prot |= INSTR;
    else
        ext_prot &= ~INSTR;
}

inline uint8_t apb_extension::get_protection() const { return ext_prot & 0x7; }
inline void apb_extension::set_protection(uint8_t prot) { ext_prot = (ext_prot & 0x8) | (prot & 0x7); }

inline bool apb_extension::is_nse() const { return ext_prot & NSE; }

inline void apb_extension::set_nse(bool instr) {
    if(instr)
        ext_prot |= NSE;
    else
        ext_prot &= ~NSE;
}

inline tlm::tlm_extension_base* apb_extension::clone() const { return new apb_extension(*this); }

inline void apb_extension::copy_from(const tlm::tlm_extension_base& ext) {
    auto const* ahb_ext = dynamic_cast<const apb_extension*>(&ext);
    assert(ahb_ext);
    (*this) = *ahb_ext;
}
} // namespace apb

#endif /* _AHB_TLM_H_ */
