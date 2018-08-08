/*
 * performancetracer.cpp
 *
 *  Created on: 08.08.2018
 *      Author: eyck
 */

#include <scc/perf_estimator.h>
#include <scc/report.h>

#if defined (_WIN32)
#include <Windows.h>
#elif defined(__unix__) || defined(__unix) || defined(unix) || (defined(__MACH__) && defined(__APPLE__))
#include <unistd.h>
#include <sys/resource.h>
#include <sys/times.h>
#include <time.h>
#else
#error "Cannot compile file because of an unknown method to retrieve OS time."
#endif

namespace scc {
using namespace sc_core;

perf_estimator::perf_estimator()
: sc_module(sc_module_name(sc_gen_unique_name("perf_estimator", true)))
{
}

perf_estimator::~perf_estimator() {
}

void perf_estimator::start_of_simulation() {
    sos.set();
}

void perf_estimator::end_of_simulation() {
    eos.set();
    sc_time now = sc_time_stamp();
    long elapsed_wall = (eos.wall_clock_stamp-sos.wall_clock_stamp).total_microseconds();
    long elapsed_proc = (long)((eos.proc_clock_stamp-sos.proc_clock_stamp)*1000000);
    long elapsed_sim  = (long)now.to_seconds()*1000000;
    double wall_perf = elapsed_wall/elapsed_sim;
    double proc_perf = elapsed_proc/elapsed_sim;
    LOG(INFO)<<"Wall clock (process clock) based simulation real time factor is "<<wall_perf<<"("<<proc_perf<<")";
}

} /* namespace scc */

double scc::perf_estimator::time_stamp::get_cpu_time() {
#if defined (_WIN32)
    FILETIME create_time;
    FILETIME exit_time;
    FILETIME kernel_time;
    FILETIME user_time;
    if(GetProcessTimes(GetCurrentProcess(), &create_tiem, & exit_time, &kernel_time, & user_time) != -1){
        SYSTEMTIME system_time;
        if(FileTimeToSystemTime(&user_time, system_time)!= -1)
            return (double)system_time.wHour*3600.0 +
                    (double)system_time.wMinute*60.0 +
                    (double)system_time.wSecond +
                    (double)system_time.wMilliseconds/1000.;
    }
#elif defined(__unix__) || defined(__unix) || defined(unix) || (defined(__MACH__) && defined(__APPLE__))
#if _POSIX_TIMERS > 0
    {
        clockid_t id;
        struct timespec stamp;
#if _POSIX_CPUTIME >0
        if(clock_getcpuclockid(0, &id) == -1)
#endif
#if defined(CLOCK_PROCESS_CPUTIME_ID)
            id=CLOCK_PROCESS_CPUTIME_ID;
#elif defined(CLOCK_VIRTUAL)
            id=CLOCK_VIRTUAL;
#else
            id=(clockid_t)-1;
#endif
        if(id != (clockid_t)-1 && clock_gettime(id, &stamp)!=-1)
            return (double)stamp.tv_sec+(double)stamp.tv_nsec/1000000000.0;
    }
#endif
#if defined(RUSAGE_SELF)
    {
        struct rusage usage;
        if(getrusage(RUSAGE_SELF, &usage)!=-1)
                return (double)usage.ru_utime.tv_sec + (double)usage.ru_utime.tv_usec/1000000.0;
    }
#endif
#if defined(_SC_CLK_TICK)
    {
        const double ticks = (double)sysconf(_SC_CLK_TCK);
        struct tms s;
        if(times(&s) != (clock_t)-1)
            return (double)s.tms_utime/ticks;
    }
#endif
#if defined(CLOCKS_PER_SEC)
    {
        clock_t c = clock();
        if(c != (clock_t)-1)
            return (double)c/(double)CLOCKS_PER_SEC;
    }
#endif
#endif
    return 1.0;
}
