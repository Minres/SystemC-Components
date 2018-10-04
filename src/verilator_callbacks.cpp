/*
 * verilator_callbacks.cpp
 *
 *  Created on: 04.10.2018
 *      Author: eyck
 */

#include <verilated_sc.h>
#include <verilated_heavy.h>

void vl_stop (const char* filename, int linenum, const char* hier) {
    Verilated::gotFinish(true);
    Verilated::flushCall();
    sc_stop();
}

void vl_finish (const char* filename, int linenum, const char* hier) {
    Verilated::gotFinish(true);
    Verilated::flushCall();
    sc_stop();
}
