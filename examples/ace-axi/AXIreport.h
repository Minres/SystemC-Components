#pragma once
#include "axi/axi_tlm.h"
#include "systemc.h"
#include "tlm.h"
#include <iomanip>

namespace report {
std::string printAXI(const axi::axi_protocol_types::tlm_phase_type&);
std::string print(axi::axi4_extension&);
std::string pretty_print(axi::axi4_extension&, tlm::tlm_phase_enum phase = tlm::BEGIN_REQ, std::string StartString = "",
                         std::string timeString = "");
bool compare_gpaxi(tlm::tlm_generic_payload&, tlm::tlm_generic_payload&, unsigned long long org_time = 0, unsigned long long ref_time = 0);
} // namespace report
