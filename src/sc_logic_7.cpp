/*
 * sc_logic_7.cpp
 *
 *  Created on: Nov 21, 2016
 *      Author: developer
 */

#include <sysc/core/sc_logic_7.h>

#include "sysc/datatypes/bit/sc_bit_ids.h"

namespace sysc {
namespace sc_dt {

void sc_logic_7::invalid_value(sc_logic_7_value_t v) {
    char msg[BUFSIZ];
    std::sprintf(msg, "sc_logic_7( %d )", v);
    SC_REPORT_ERROR(sc_core::SC_ID_VALUE_NOT_VALID_, msg);
}

void sc_logic_7::invalid_value(char c) {
    char msg[BUFSIZ];
    std::sprintf(msg, "sc_logic_7( '%c' )", c);
    SC_REPORT_ERROR(sc_core::SC_ID_VALUE_NOT_VALID_, msg);
}

void sc_logic_7::invalid_value(int i) {
    char msg[BUFSIZ];
    std::sprintf(msg, "sc_logic_7( %d )", i);
    SC_REPORT_ERROR(sc_core::SC_ID_VALUE_NOT_VALID_, msg);
}

void sc_logic_7::invalid_01() const {
    if ((int)m_val == Log_Z) {
        SC_REPORT_WARNING(sc_core::SC_ID_LOGIC_Z_TO_BOOL_, "");
    } else {
        SC_REPORT_WARNING(sc_core::SC_ID_LOGIC_X_TO_BOOL_, "");
    }
}

// conversion tables

const sc_logic_7_value_t sc_logic_7::char_to_logic[128] = {
    Log_0, Log_1, Log_L, Log_H, Log_Z, Log_X, Log_U, Log_X,                                                         // 0
    Log_X, Log_X, Log_X, Log_X, Log_X, Log_X, Log_X, Log_X, Log_X, Log_X, Log_X, Log_X, Log_X, Log_X, Log_X, Log_X, // 1
    Log_X, Log_X, Log_X, Log_X, Log_X, Log_X, Log_X, Log_X, Log_X, Log_X, Log_X, Log_X, Log_X, Log_X, Log_X, Log_X, // 2
    Log_X, Log_X, Log_X, Log_X, Log_X, Log_X, Log_X, Log_X, Log_0, Log_1, Log_X, Log_X, Log_X, Log_X, Log_X, Log_X, // 3
    Log_X, Log_X, Log_X, Log_X, Log_X, Log_X, Log_X, Log_X, Log_X, Log_X, Log_X, Log_X, Log_X, Log_X, Log_X, Log_X, // 4
    Log_H, Log_X, Log_X, Log_X, Log_L, Log_X, Log_X, Log_X, Log_X, Log_X, Log_X, Log_X, Log_X, Log_U, Log_X, Log_X, // 5
    Log_X, Log_X, Log_Z, Log_X, Log_X, Log_X, Log_X, Log_X, Log_X, Log_X, Log_X, Log_X, Log_X, Log_X, Log_X, Log_X, // 6
    Log_X, Log_X, Log_X, Log_X, Log_X, Log_X, Log_X, Log_X, Log_X, Log_X, Log_X, Log_X, Log_X, Log_X, Log_X, Log_X, // 9
    Log_X, Log_X, Log_Z, Log_X, Log_X, Log_X, Log_X, Log_X};

const char sc_logic_7::logic_to_char[7] = {'0', '1', 'L', 'H', 'Z', 'X', 'U'};

const sc_logic_7_value_t sc_logic_7::and_table[7][7] = { // 0      1      L      H      Z      X      U
    /*0*/ {Log_0, Log_0, Log_0, Log_0, Log_0, Log_0, Log_0},
    /*1*/ {Log_0, Log_1, Log_L, Log_1, Log_X, Log_X, Log_X},
    /*L*/ {Log_0, Log_L, Log_L, Log_L, Log_L, Log_L, Log_L},
    /*H*/ {Log_0, Log_1, Log_L, Log_H, Log_X, Log_X, Log_X},
    /*Z*/ {Log_0, Log_X, Log_L, Log_X, Log_X, Log_X, Log_X},
    /*X*/ {Log_0, Log_X, Log_L, Log_X, Log_X, Log_X, Log_X},
    /*U*/ {Log_0, Log_X, Log_L, Log_X, Log_X, Log_X, Log_X}};

const sc_logic_7_value_t sc_logic_7::or_table[7][7] = { // 0      1      L      H      Z      X      U
    /*0*/ {Log_0, Log_1, Log_L, Log_H, Log_Z, Log_X, Log_U},
    /*1*/ {Log_1, Log_1, Log_1, Log_1, Log_1, Log_1, Log_1},
    /*L*/ {Log_L, Log_1, Log_L, Log_H, Log_Z, Log_X, Log_U},
    /*H*/ {Log_H, Log_1, Log_H, Log_H, Log_H, Log_H, Log_H},
    /*Z*/ {Log_Z, Log_1, Log_Z, Log_H, Log_X, Log_X, Log_X},
    /*X*/ {Log_X, Log_1, Log_X, Log_H, Log_X, Log_X, Log_X},
    /*U*/ {Log_U, Log_1, Log_U, Log_H, Log_X, Log_X, Log_X}};

const sc_logic_7_value_t sc_logic_7::xor_table[7][7] = { // 0      1      L      H      Z      X      U
    /*0*/ {Log_0, Log_1, Log_0, Log_1, Log_X, Log_X, Log_X},
    /*1*/ {Log_1, Log_0, Log_1, Log_0, Log_X, Log_X, Log_X},
    /*L*/ {Log_0, Log_1, Log_H, Log_L, Log_X, Log_X, Log_X},
    /*H*/ {Log_1, Log_0, Log_L, Log_H, Log_X, Log_X, Log_X},
    /*Z*/ {Log_X, Log_X, Log_X, Log_X, Log_X, Log_X, Log_X},
    /*X*/ {Log_X, Log_X, Log_X, Log_X, Log_X, Log_X, Log_X},
    /*U*/ {Log_X, Log_X, Log_X, Log_X, Log_X, Log_X, Log_X}};

const sc_logic_7_value_t sc_logic_7::not_table[7] = {Log_1, Log_0, Log_H, Log_L, Log_X, Log_X, Log_X};

// other methods

void sc_logic_7::scan(::std::istream &is) {
    char c;
    is >> c;
    *this = c;
}

const sc_logic_7 SC_LOGIC7_0(Log_0);
const sc_logic_7 SC_LOGIC7_1(Log_1);
const sc_logic_7 SC_LOGIC7_Z(Log_Z);
const sc_logic_7 SC_LOGIC7_X(Log_X);

} // namespace sc_dt
} // namespace sysc
