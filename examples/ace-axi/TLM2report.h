#pragma once
#include "systemc.h"
#include "tlm.h"
#include <iomanip>

namespace report {
std::string print(const tlm::tlm_sync_enum status);
std::string print(const tlm::tlm_phase phase);

std::string print(tlm::tlm_generic_payload&);
std::string pretty_print(tlm::tlm_generic_payload&, tlm::tlm_phase_enum phase = tlm::BEGIN_REQ, std::string StartString = "",
                         std::string timeString = "");
tlm::tlm_generic_payload* parse_command(std::string);
//  std::string print ( amba_pv::amba_pv_extension& );

std::string print(tlm::tlm_dmi&);
// void sc_trace_trans (sc_trace_file*, std::string, tlm_trace_payload& );
bool compare_gp(tlm::tlm_generic_payload&, tlm::tlm_generic_payload&, unsigned long long org_time = 0, unsigned long long ref_time = 0);

template <sc_core::sc_severity SEVERITY> struct ScLogger {
    ScLogger(const char* file, int line, sc_core::sc_verbosity level = sc_core::SC_MEDIUM)
    : t(nullptr)
    , file(file)
    , line(line)
    , level(level){};

    ScLogger() = delete;

    ScLogger(const ScLogger&) = delete;

    ScLogger& operator=(const ScLogger&) = delete;

    virtual ~ScLogger() { ::sc_core::sc_report_handler::report(SEVERITY, t ? t : "SystemC", os.str().c_str(), level, file, line); }

    inline ScLogger& type() { return *this; }

    inline ScLogger& type(const char* t) {
        this->t = const_cast<char*>(t);
        return *this;
    }

    inline std::ostringstream& get() { return os; };

protected:
    std::ostringstream os;
    char* t;
    const char* file;
    const int line;
    const sc_core::sc_verbosity level;
};

} // namespace report
