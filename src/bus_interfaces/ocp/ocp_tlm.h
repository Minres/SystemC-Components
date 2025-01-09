/*******************************************************************************
 * Copyright 2021 MINRES Technologies GmbH
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

#ifndef _BUS_OCP_TLM_H_
#define _BUS_OCP_TLM_H_

#include <array>
#include <cstdint>
#include <tlm>

//! TLM2.0 components modeling OCP
namespace ocp {

std::ostream& operator<<(std::ostream& os, tlm::tlm_generic_payload const& t);

enum class cmd_e : uint8_t { IDLE, WRITE, READ, READEX, READ_LINKED, WRITE_NON_POSTED, WRITE_CONDITIONAL, BROADCAST };

enum class resp_e : uint8_t { NULL_, DVA, FAIL, ERROR };

struct ocp_extension : public tlm::tlm_extension<ocp_extension> {

    ocp_extension() = default;

    ocp_extension(const ocp_extension& o) = default;
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

    cmd_e get_mcmd() const { return mcmd; }

    void set_mcmd(cmd_e mCmd) { mcmd = mCmd; }

    resp_e get_sresp() const { return sresp; }

    void set_sresp(resp_e sres) { this->sresp = sresp; }

private:
    cmd_e mcmd{cmd_e::IDLE};
    resp_e sresp{resp_e::NULL_};
};

inline tlm::tlm_extension_base* ocp_extension::clone() const { return new ocp_extension(*this); }

inline void ocp_extension::copy_from(const tlm::tlm_extension_base& ext) {
    auto const* ocp_ext = dynamic_cast<const ocp_extension*>(&ext);
    assert(ocp_ext);
    (*this) = *ocp_ext;
}

// inline unsigned get_ocp_id(tlm::tlm_generic_payload& trans) {
//     auto* ext = trans.get_extension<ocp::ocp_extension>();
//     assert(ext && "No ocp extension found");
//     return ext->get_id();
// }
} // namespace ocp
#endif // _BUS_OCP_TLM_H_
