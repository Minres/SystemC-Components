/*******************************************************************************
 * Copyright 2019-2023 MINRES Technologies GmbH
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

#ifndef _AHB_TLM_H_
#define _AHB_TLM_H_

#include <array>
#include <cstdint>
#include <tlm>

//! TLM2.0 components modeling AHB
namespace ahb {

/**
 * helper function to allow SFINAE
 */
template <typename Enum> struct enable_for_enum {
    static const bool value = false;
};
/**
 * helper function to convert integer into class enums
 * @param t
 * @return
 */
template <typename E> inline E into(typename std::underlying_type<E>::type t);
/**
 * helper function to convert class enums into integer
 * @param t
 * @return
 */
template <typename E, typename ULT = typename std::underlying_type<E>::type,
          typename X = typename std::enable_if<std::is_enum<E>::value && !std::is_convertible<E, ULT>::value, bool>::type>
inline constexpr ULT to_int(E t) {
    return static_cast<typename std::underlying_type<E>::type>(t);
}
/**
 * helper function to convert class enums into char string
 * @param t
 * @return
 */
template <typename E> const char* to_char(E t);
/**
 *
 * @param os
 * @param e
 * @return
 */
template <typename E, typename std::enable_if<enable_for_enum<E>::value, bool>::type>
inline std::ostream& operator<<(std::ostream& os, E e) {
    os << to_char(e);
    return os;
}

std::ostream& operator<<(std::ostream& os, tlm::tlm_generic_payload const& t);

enum class lock_e : uint8_t { NORMAL = 0x0, EXLUSIVE = 0x1, LOCKED = 0x2 };

enum class resp_e : uint8_t { OKAY = 0x0, EXOKAY = 0x1, SLVERR = 0x2, DECERR = 0x3 };

enum class trans_e : uint8_t { IDLE = 0x0, BUSY = 0x1, NONSEQ = 0x2, SEQ = 0x3 };

enum class burst_e : uint8_t { SINGLE = 0x0, INCR = 0x1, WRAP4 = 0x2, INCR4 = 0x3, WRAP8 = 0x4, INCR8 = 0x5, WRAP16 = 0x6, INCR16 = 0x7 };

struct ahb_extension : public tlm::tlm_extension<ahb_extension> {
    // HPROT[0]
    bool is_instruction() const;
    void set_instruction(bool = true);
    // HPROT[1]
    bool is_privileged() const;
    void set_privileged(bool = true);
    // HPROT[2]
    void set_bufferable(bool = true);
    bool is_bufferable() const;
    // HPROT[3]
    void set_cacheable(bool = true);
    bool is_cacheable() const;
    // HLOCK
    bool is_locked() const;
    void set_locked(bool = true);
    // HTRANS[1]
    bool is_seq() const;
    void set_seq(bool = true);

    uint8_t get_protection() const;
    void set_protection(uint8_t);

    burst_e get_burst() const;
    void set_burst(burst_e);

    resp_e get_resp() const;
    void set_resp(resp_e);

    ahb_extension() = default;

    ahb_extension(const ahb_extension& o) = default;
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
    enum { INSTR = 1, PRIV = 2, BUFFERABLE = 4, CACHABLE = 8 };
    uint8_t prot{0};
    lock_e lock{lock_e::NORMAL};
    resp_e resp{resp_e::OKAY};
    burst_e burst{burst_e::SINGLE};
    bool seq{false};
};

/*****************************************************************************
 * Implementation details
 *****************************************************************************/
template <> struct enable_for_enum<lock_e> {
    static const bool enable = true;
};
template <> struct enable_for_enum<resp_e> {
    static const bool enable = true;
};
template <> struct enable_for_enum<trans_e> {
    static const bool enable = true;
};
template <> struct enable_for_enum<burst_e> {
    static const bool enable = true;
};

inline bool ahb_extension::is_instruction() const { return prot & INSTR; }

inline void ahb_extension::set_instruction(bool instr) {
    if(instr)
        prot |= INSTR;
    else
        prot &= ~INSTR;
}

inline bool ahb_extension::is_privileged() const { return prot & PRIV; }

inline void ahb_extension::set_privileged(bool priv) {
    if(priv)
        prot |= PRIV;
    else
        prot &= ~PRIV;
}

inline bool ahb_extension::is_bufferable() const { return prot & BUFFERABLE; }

inline void ahb_extension::set_bufferable(bool bufferable) {
    if(bufferable)
        prot |= BUFFERABLE;
    else
        prot &= ~BUFFERABLE;
}

inline bool ahb_extension::is_cacheable() const { return prot & CACHABLE; }

inline void ahb_extension::set_cacheable(bool cacheable) {
    if(cacheable)
        prot |= CACHABLE;
    else
        prot &= ~CACHABLE;
}

inline uint8_t ahb_extension::get_protection() const { return prot; }

inline void ahb_extension::set_protection(uint8_t protection) { prot = protection; }

inline bool ahb_extension::is_locked() const { return lock == lock_e::LOCKED; }

inline void ahb_extension::set_locked(bool locked) { lock = locked ? lock_e::LOCKED : lock_e::NORMAL; }

inline burst_e ahb_extension::get_burst() const { return burst; }

inline void ahb_extension::set_burst(burst_e b) { burst = b; }

inline bool ahb_extension::is_seq() const { return seq; }

inline void ahb_extension::set_seq(bool s) { seq = s; }

inline tlm::tlm_extension_base* ahb_extension::clone() const { return new ahb_extension(*this); }

inline resp_e ahb_extension::get_resp() const { return resp; }

inline void ahb_extension::set_resp(resp_e r) { resp = r; }

inline void ahb_extension::copy_from(const tlm::tlm_extension_base& ext) {
    auto const* ahb_ext = dynamic_cast<const ahb_extension*>(&ext);
    assert(ahb_ext);
    (*this) = *ahb_ext;
}
} // namespace ahb

#endif /* _AHB_TLM_H_ */
