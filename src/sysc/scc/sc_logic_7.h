/*******************************************************************************
 * Copyright 2016, 2018 MINRES Technologies GmbH
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

#ifndef INCL_SYSC_CORE_SC_LOGIC_7_H_
#define INCL_SYSC_CORE_SC_LOGIC_7_H_
#include <array>
#include <cstdio>

#include "sysc/datatypes/bit/sc_bit.h"
#include "sysc/kernel/sc_macros.h"
#include "sysc/utils/sc_mempool.h"

/** \ingroup scc-sysc
 *  @{
 */
/**@{*/
//! @brief SCC SystemC utilities
namespace scc {
namespace dt {

// classes defined in this module
class sc_logic_7;

// ----------------------------------------------------------------------------
//  ENUM : sc_logic_7_value_t
//
//  Enumeration of values for sc_logic_7.
// ----------------------------------------------------------------------------

enum sc_logic_7_value_t { Log_0 = 0, Log_1, Log_L, Log_H, Log_Z, Log_X, Log_U };

// ----------------------------------------------------------------------------
//  CLASS : sc_logic_7
//
//  Four-valued logic type.
// ----------------------------------------------------------------------------

class sc_logic_7 {
private:
    // support methods

    static void invalid_value(sc_logic_7_value_t);
    static void invalid_value(char);
    static void invalid_value(int);

    static sc_logic_7_value_t to_value(sc_logic_7_value_t v) {
        if(v < Log_0 || v > Log_X) {
            invalid_value(v);
        }
        return v;
    }

    static sc_logic_7_value_t to_value(bool b) { return (b ? Log_1 : Log_0); }

    static sc_logic_7_value_t to_value(char c) {
        sc_logic_7_value_t v;
        unsigned int index = (int)c;
        if(index > 127) {
            invalid_value(c);
            v = Log_X;
        } else {
            v = char_to_logic[index];
            if(v < Log_0 || v > Log_X) {
                invalid_value(c);
            }
        }
        return v;
    }

    static sc_logic_7_value_t to_value(int i) {
        if(i < 0 || i > 3) {
            invalid_value(i);
        }
        return sc_logic_7_value_t(i);
    }

    void invalid_01() const;

public:
    // conversion tables

    static std::array<const sc_logic_7_value_t, 128> char_to_logic;
    static std::array<const char, 7> logic_to_char;
    static std::array<std::array<const sc_logic_7_value_t, 7>, 7> and_table;
    static std::array<std::array<const sc_logic_7_value_t, 7>, 7> or_table;
    static std::array<std::array<const sc_logic_7_value_t, 7>, 7> xor_table;
    static std::array<const sc_logic_7_value_t, 7> not_table;

    // constructors

    sc_logic_7() = default;

    sc_logic_7(const sc_logic_7& a) = default;

    sc_logic_7(sc_logic_7_value_t v)
    : m_val(to_value(v)) {}

    explicit sc_logic_7(bool a)
    : m_val(to_value(a)) {}

    explicit sc_logic_7(char a)
    : m_val(to_value(a)) {}

    explicit sc_logic_7(int a)
    : m_val(to_value(a)) {}

    explicit sc_logic_7(const ::sc_dt::sc_bit& a)
    : m_val(to_value(a.to_bool())) {}

    // destructor

    ~sc_logic_7() = default;

    // (bitwise) assignment operators

#define DEFN_ASN_OP_T(op, tp)                                                                                          \
    sc_logic_7& operator op(tp v) {                                                                                    \
        *this op sc_logic_7(v);                                                                                        \
        return *this;                                                                                                  \
    }

#define DEFN_ASN_OP(op)                                                                                                \
    DEFN_ASN_OP_T(op, sc_logic_7_value_t)                                                                              \
    DEFN_ASN_OP_T(op, bool)                                                                                            \
    DEFN_ASN_OP_T(op, char)                                                                                            \
    DEFN_ASN_OP_T(op, int)                                                                                             \
    DEFN_ASN_OP_T(op, const ::sc_dt::sc_bit&)

    sc_logic_7& operator=(const sc_logic_7& a) = default;

    sc_logic_7& operator&=(const sc_logic_7& b) {
        m_val = and_table[m_val][b.m_val];
        return *this;
    }

    sc_logic_7& operator|=(const sc_logic_7& b) {
        m_val = or_table[m_val][b.m_val];
        return *this;
    }

    sc_logic_7& operator^=(const sc_logic_7& b) {
        m_val = xor_table[m_val][b.m_val];
        return *this;
    }

    DEFN_ASN_OP(=)
    DEFN_ASN_OP(&=)
    DEFN_ASN_OP(|=)
    DEFN_ASN_OP(^=)

#undef DEFN_ASN_OP_T
#undef DEFN_ASN_OP

    // bitwise operators and functions

    friend const sc_logic_7 operator&(const sc_logic_7&, const sc_logic_7&);
    friend const sc_logic_7 operator|(const sc_logic_7&, const sc_logic_7&);
    friend const sc_logic_7 operator^(const sc_logic_7&, const sc_logic_7&);

    // relational operators

    friend bool operator==(const sc_logic_7&, const sc_logic_7&);
    friend bool operator!=(const sc_logic_7&, const sc_logic_7&);

    // bitwise complement

    const sc_logic_7 operator~() const { return sc_logic_7(not_table[m_val]); }

    sc_logic_7& b_not() {
        m_val = not_table[m_val];
        return *this;
    }

    // explicit conversions

    sc_logic_7_value_t value() const { return m_val; }

    bool is_01() const { return ((int)m_val == Log_0 || (int)m_val == Log_1); }

    bool to_bool() const {
        if(!is_01()) {
            invalid_01();
        }
        return ((int)m_val != Log_0);
    }

    char to_char() const { return logic_to_char[m_val]; }

    // other methods

    void print(::std::ostream& os = ::std::cout) const { os << to_char(); }

    void scan(::std::istream& is = ::std::cin);

    // memory (de)allocation

    static void* operator new(std::size_t, void* p) // placement new
    {
        return p;
    }

    static void* operator new(std::size_t sz) { return sc_core::sc_mempool::allocate(sz); }

    static void operator delete(void* p, std::size_t sz) { sc_core::sc_mempool::release(p, sz); }

    static void* operator new[](std::size_t sz) { return sc_core::sc_mempool::allocate(sz); }

    static void operator delete[](void* p, std::size_t sz) { sc_core::sc_mempool::release(p, sz); }

private:
    sc_logic_7_value_t m_val = Log_U;

private:
    // disabled
    explicit sc_logic_7(const char*);
    sc_logic_7& operator=(const char*);
};

// ----------------------------------------------------------------------------

// bitwise operators

inline const sc_logic_7 operator&(const sc_logic_7& a, const sc_logic_7& b) {
    return sc_logic_7(sc_logic_7::and_table[a.m_val][b.m_val]);
}

inline const sc_logic_7 operator|(const sc_logic_7& a, const sc_logic_7& b) {
    return sc_logic_7(sc_logic_7::or_table[a.m_val][b.m_val]);
}

inline const sc_logic_7 operator^(const sc_logic_7& a, const sc_logic_7& b) {
    return sc_logic_7(sc_logic_7::xor_table[a.m_val][b.m_val]);
}

#define DEFN_BIN_OP_T(ret, op, tp)                                                                                     \
    inline ret operator op(const sc_logic_7& a, tp b) { return (a op sc_logic_7(b)); }                                 \
    inline ret operator op(tp a, const sc_logic_7& b) { return (sc_logic_7(a) op b); }

#define DEFN_BIN_OP(ret, op)                                                                                           \
    DEFN_BIN_OP_T(ret, op, sc_logic_7_value_t)                                                                         \
    DEFN_BIN_OP_T(ret, op, bool)                                                                                       \
    DEFN_BIN_OP_T(ret, op, char)                                                                                       \
    DEFN_BIN_OP_T(ret, op, int)

DEFN_BIN_OP(const sc_logic_7, &)
DEFN_BIN_OP(const sc_logic_7, |)
DEFN_BIN_OP(const sc_logic_7, ^)

// relational operators and functions

inline bool operator==(const sc_logic_7& a, const sc_logic_7& b) { return ((int)a.m_val == b.m_val); }

inline bool operator!=(const sc_logic_7& a, const sc_logic_7& b) { return ((int)a.m_val != b.m_val); }

DEFN_BIN_OP(bool, ==)
DEFN_BIN_OP(bool, !=)

#undef DEFN_BIN_OP_T
#undef DEFN_BIN_OP

// ----------------------------------------------------------------------------

inline ::std::ostream& operator<<(::std::ostream& os, const sc_logic_7& a) {
    a.print(os);
    return os;
}

inline ::std::istream& operator>>(::std::istream& is, sc_logic_7& a) {
    a.scan(is);
    return is;
}

extern const sc_logic_7 SC_LOGIC7_0;
extern const sc_logic_7 SC_LOGIC7_1;
extern const sc_logic_7 SC_LOGIC7_Z;
extern const sc_logic_7 SC_LOGIC7_X;

} // namespace dt
} // namespace scc
/** @} */ // end of scc-sysc
#endif /* INCL_SYSC_CORE_SC_LOGIC_7_H_ */
