/*******************************************************************************
 * Copyright 2018 MINRES Technologies GmbH
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

#include "perf_estimator.h"
#include "report.h"

#if defined(_WIN32)
#include <Windows.h>
#elif defined(__unix__) || defined(__unix) || defined(unix) || (defined(__MACH__) && defined(__APPLE__))
#include <ctime>
#include <sys/resource.h>
#include <sys/times.h>
#include <unistd.h>
#else
#error "Cannot compile file because of an unknown method to retrieve OS time."
#endif

namespace scc {
using namespace sc_core;

SC_HAS_PROCESS(perf_estimator);

perf_estimator::perf_estimator(const sc_module_name& nm, sc_time beat_delay_)
: sc_module(nm)
, beat_delay(beat_delay_) {
    soc.set();
    if(beat_delay.value()) {
        SC_METHOD(beat);
    }
}

perf_estimator::~perf_estimator() {
    time_stamp eod;
    eod.set();
    SCCINFO("perf_estimator") << "constr & elab time:  " << (eoe.proc_clock_stamp - soc.proc_clock_stamp) << "s";
    SCCINFO("perf_estimator") << "simulation time:     " << (eos.proc_clock_stamp - sos.proc_clock_stamp) << "s";
    if(cycle_period.value()) {
        uint64_t cycles = sc_time_stamp().value() / cycle_period.value();
        SCCINFO("perf_estimator") << "simulation speed:   "
                       << (sc_time_stamp().value() ? cycles / (eos.proc_clock_stamp - soc.proc_clock_stamp) : 0.0)
                       << " cycles/s";
    }
    SCCINFO("perf_estimator") << "max resident memory: " << max_memory << "kB";
}

void perf_estimator::end_of_elaboration() { eoe.set(); }

void perf_estimator::start_of_simulation() {
    sos.set();
    get_memory();
}

void perf_estimator::end_of_simulation() {
    eos.set();
    sc_time now = sc_time_stamp();
    unsigned long long elapsed_wall = (eos.wall_clock_stamp - sos.wall_clock_stamp).total_microseconds();
    auto elapsed_proc = (unsigned long long)((eos.proc_clock_stamp - sos.proc_clock_stamp) * 1000000);
    auto elapsed_sim = (unsigned long long)(now.to_seconds() * 1000000.);
    if(elapsed_sim > 0) {
        double wall_perf = elapsed_wall / elapsed_sim;
        double proc_perf = elapsed_proc / elapsed_sim;
        SCCINFO("perf_estimator") << "Wall clock (process clock) based simulation real time factor is " << wall_perf << "("
                       << proc_perf << ")";
    }
    get_memory();
}

void perf_estimator::beat() {
    if(sc_time_stamp().value())
        SCCINFO("perf_estimator") << "Heart beat, rss mem: " << get_memory() << " bytes";
    next_trigger(beat_delay);
}
} /* namespace scc */

auto scc::perf_estimator::time_stamp::get_cpu_time() -> double {
#if defined(_WIN32)
    FILETIME create_time;
    FILETIME exit_time;
    FILETIME kernel_time;
    FILETIME user_time;
    if(GetProcessTimes(GetCurrentProcess(), &create_time, &exit_time, &kernel_time, &user_time) != -1) {
        SYSTEMTIME system_time;
        if(FileTimeToSystemTime(&user_time, &system_time) != -1)
            return (double)system_time.wHour * 3600.0 + (double)system_time.wMinute * 60.0 +
                   (double)system_time.wSecond + (double)system_time.wMilliseconds / 1000.;
    }
#elif defined(__unix__) || defined(__unix) || defined(unix) || (defined(__MACH__) && defined(__APPLE__))
#if _POSIX_TIMERS > 0
    {
        clockid_t id;
        struct timespec stamp {};
#if _POSIX_CPUTIME > 0
        if(clock_getcpuclockid(0, &id) == -1)
#endif
#if defined(CLOCK_PROCESS_CPUTIME_ID)
            id = CLOCK_PROCESS_CPUTIME_ID;
#elif defined(CLOCK_VIRTUAL)
        id = CLOCK_VIRTUAL;
#else
        id = (clockid_t)-1;
#endif
        if(id != (clockid_t)-1 && clock_gettime(id, &stamp) != -1)
            return (double)stamp.tv_sec + (double)stamp.tv_nsec / 1000000000.0;
    }
#endif
#if defined(RUSAGE_SELF)
    {
        struct rusage usage {};
        if(getrusage(RUSAGE_SELF, &usage) != -1)
            return (double)usage.ru_utime.tv_sec + (double)usage.ru_utime.tv_usec / 1000000.0;
    }
#endif
#if defined(_SC_CLK_TICK)
    {
        const double ticks = (double)sysconf(_SC_CLK_TCK);
        struct tms s;
        if(times(&s) != (clock_t)-1)
            return (double)s.tms_utime / ticks;
    }
#endif
#if defined(CLOCKS_PER_SEC)
    {
        clock_t c = clock();
        if(c != (clock_t)-1)
            return (double)c / (double)CLOCKS_PER_SEC;
    }
#endif
#endif
    return 1.0;
}

long scc::perf_estimator::get_memory() {
#if defined(RUSAGE_SELF)
    {
        struct rusage usage {};
        if(getrusage(RUSAGE_SELF, &usage) != -1) {
            max_memory = std::max(max_memory, usage.ru_maxrss);
            return usage.ru_maxrss;
        }
    }
#endif
    return 0L;
}
