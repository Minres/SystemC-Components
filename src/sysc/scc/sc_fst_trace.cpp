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

#include <cstdlib>
#include <cstring>
#include <limits>
#include <vector>

#include "sc_fst_trace.h"
#include <sysc/kernel/sc_simcontext.h>
#include <sysc/kernel/sc_ver.h>
#include <sysc/kernel/sc_event.h>
#include <sysc/datatypes/bit/sc_bit.h>
#include <sysc/datatypes/bit/sc_logic.h>
#include <sysc/datatypes/bit/sc_lv_base.h>
#include <sysc/datatypes/int/sc_signed.h>
#include <sysc/datatypes/int/sc_unsigned.h>
#include <sysc/datatypes/int/sc_int_base.h>
#include <sysc/datatypes/int/sc_uint_base.h>
#include <sysc/datatypes/fx/fx.h>
#include <sysc/utils/sc_report.h> // sc_assert
#include <sysc/utils/sc_string_view.h>

#include <iomanip>
#include <map>
#include <sstream>

#if defined(_MSC_VER)
# pragma warning(disable:4309) // truncation of constant value
#endif

scc::fst_trace_file::fst_trace_file(const char *name, std::function<bool()> &enable):
sc_core::sc_trace_file_base(name, "fst")
{
}

scc::fst_trace_file::~fst_trace_file() {
    if (m_fst) fstWriterClose(m_fst);
    delete[] m_symbolp; m_symbolp = nullptr;
    delete[] m_strbuf; m_strbuf = nullptr;
}

void scc::fst_trace_file::trace(const sc_core::sc_event& object, const std::string& name){
}

void scc::fst_trace_file::trace(const sc_core::sc_time& object, const std::string& name){
}

void scc::fst_trace_file::trace(const bool& object, const std::string& name){
}

void scc::fst_trace_file::trace(const sc_dt::sc_bit& object, const std::string& name){
}

void scc::fst_trace_file::trace(const sc_dt::sc_logic& object, const std::string& name){
}

void scc::fst_trace_file::trace(const unsigned char& object, const std::string& name, int width){
}

void scc::fst_trace_file::trace(const unsigned short& object, const std::string& name, int width){
}

void scc::fst_trace_file::trace(const unsigned int& object, const std::string& name, int width){
}

void scc::fst_trace_file::trace(const unsigned long& object, const std::string& name, int width){
}

void scc::fst_trace_file::trace(const char& object, const std::string& name, int width){
}

void scc::fst_trace_file::trace(const short& object, const std::string& name, int width){
}

void scc::fst_trace_file::trace(const int& object, const std::string& name, int width){
}

void scc::fst_trace_file::trace(const long& object, const std::string& name, int width){
}

void scc::fst_trace_file::trace(const sc_dt::int64& object, const std::string& name, int width){
}

void scc::fst_trace_file::trace(const sc_dt::uint64& object, const std::string& name, int width){
}

void scc::fst_trace_file::trace(const float& object, const std::string& name){
}

void scc::fst_trace_file::trace(const double& object, const std::string& name){
}

void scc::fst_trace_file::trace(const sc_dt::sc_int_base& object, const std::string& name){
}

void scc::fst_trace_file::trace(const sc_dt::sc_uint_base& object, const std::string& name){
}

void scc::fst_trace_file::trace(const sc_dt::sc_signed& object, const std::string& name){
}

void scc::fst_trace_file::trace(const sc_dt::sc_unsigned& object, const std::string& name) {
}

void scc::fst_trace_file::trace(const sc_dt::sc_fxval& object, const std::string& name){
}

void scc::fst_trace_file::trace(const sc_dt::sc_fxval_fast& object, const std::string& name){
}

void scc::fst_trace_file::trace(const sc_dt::sc_fxnum& object, const std::string& name){
}

void scc::fst_trace_file::trace(const sc_dt::sc_fxnum_fast& object, const std::string& name){
}

void scc::fst_trace_file::trace(const sc_dt::sc_bv_base& object, const std::string& name){
}

void scc::fst_trace_file::trace(const sc_dt::sc_lv_base& object, const std::string& name){
}

void scc::fst_trace_file::write_comment(const std::string &comment) {
}

void scc::fst_trace_file::cycle(bool delta_cycle) {
}

void scc::fst_trace_file::do_initialize() {
}

void scc::fst_trace_file::print_time_stamp(sc_trace_file_base::unit_type now_units_high,
        sc_trace_file_base::unit_type now_units_low) const {
}

bool scc::fst_trace_file::get_time_stamp(sc_trace_file_base::unit_type &now_units_high,
        sc_trace_file_base::unit_type &now_units_low) const {
}

sc_core::sc_trace_file* scc::scc_create_fst_trace_file(const char *name, std::function<bool()> enable) {
}

void scc::scc_close_fst_trace_file(sc_core::sc_trace_file *tf) {
}
