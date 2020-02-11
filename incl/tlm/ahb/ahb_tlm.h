/*
 * tlm_ahb_extension.h
 *
 *  Created on: 12.12.2019
 *      Author: developer
 */

#ifndef _AHB_TLM_H_
#define _AHB_TLM_H_

#include <array>
#include <cstdint>
#include <tlm>

namespace tlm {
namespace ahb {

enum class lock_e : uint8_t { NORMAL = 0x0, EXLUSIVE = 0x1, LOCKED = 0x2 };

enum class resp_e : uint8_t { OKAY = 0x0, EXOKAY = 0x1, SLVERR = 0x2, DECERR = 0x3 };

enum class trans_e : uint8_t { IDLE = 0x0, BUSY = 0x1, NONSEQ = 0x2, SEQ = 0x3 };

enum class burst_e : uint8_t {
    SINGLE = 0x0,
    INCR = 0x1,
    WRAP4 = 0x2,
    INCR4 = 0x3,
    WRAP8 = 0x4,
    INCR8 = 0x5,
    WRAP16 = 0x6,
    INCR16 = 0x7
};
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

    uint8_t get_protection() const;
    void set_protection(uint8_t);

    uint8_t get_burst() const;
    burst_e get_burst_e() const;
    void set_burst(uint8_t);
    void set_burst(burst_e);

    /**
     * @brief the default constructor
     */
    ahb_extension() = default;
    /**
     * @brief the copy constructor
     * @param the extension to copy from
     */
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

inline uint8_t ahb_extension::get_burst() const { return static_cast<uint8_t>(burst); }

inline burst_e ahb_extension::get_burst_e() const { return burst; }

inline void ahb_extension::set_burst(uint8_t b) {
    assert(b <= static_cast<uint8_t>(burst_e::INCR16));
    burst = static_cast<burst_e>(b);
}

inline void ahb_extension::set_burst(burst_e b) { burst = b; }

inline tlm::tlm_extension_base* ahb_extension::clone() const { return new ahb_extension(*this); }

inline void ahb_extension::copy_from(const tlm::tlm_extension_base& ext) {
    auto const* ahb_ext = dynamic_cast<const ahb_extension*>(&ext);
    assert(ahb_ext);
    (*this) = *ahb_ext;
}

} // namespace ahb
} // namespace tlm

#endif /* SYSTEMC_COMPONENTS_AHB_TLM_H_ */
