/*******************************************************************************
 * Copyright 2026 MINRES Technologies GmbH
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

#ifndef _SCC_WALL_TIME_SPEED_LIMITER_H
#define _SCC_WALL_TIME_SPEED_LIMITER_H

#include "report.h"
#include <sys/time.h>
#include <sysc/kernel/sc_module.h>

/** \ingroup scc-sysc
 *  @{
 */
/**@{*/
//! @brief SCC SystemC utilities
namespace scc {
/**
 * @class wall_time_speed_limiter
 * @brief a component traversing the SystemC object hierarchy and tracing the objects
 *
 */
struct wall_time_speed_limiter : public sc_core::sc_module {
    static wall_time_speed_limiter& get() {
        static wall_time_speed_limiter limiter("sped_limiter");
        return limiter;
    }

private:
    wall_time_speed_limiter(sc_core::sc_module_name const& nm)
    : sc_module(nm) {
        SC_HAS_PROCESS(wall_time_speed_limiter);
        SC_THREAD(main_thread);
    }

    inline long long int get_time_of_day_us() {
        timeval checkpoint;
#if defined __x86_64__
        gettimeofday(&checkpoint, 0);
        return checkpoint.tv_sec * 100000 + checkpoint.tv_usec;
#else
        return 0;
#endif
    }

    void main_thread() {
        SCCDEBUG(SCMOD) << "Limiting simulation speed to wall time mode";
        const auto interval = 1000LL;
#if defined(__x86_64__) || 1
        auto checkpoint_us = get_time_of_day_us();
        while(true) {
            wait(interval, sc_core::SC_US);
            auto consumed = get_time_of_day_us() - checkpoint_us;
            if(consumed > 0 && interval > consumed) {
                struct timespec tv;
                tv.tv_sec = static_cast<time_t>(interval - consumed) / 1000000;
                tv.tv_nsec = static_cast<decltype(tv.tv_nsec)>((interval - consumed) * 1000);
                nanosleep(&tv, &tv);
            }
            checkpoint_us = get_time_of_day_us();
        }
#else
        boost::posix_time::ptime checkpoint = std::posix_time::microsec_clock::local_time();
        boost::posix_time::time_duration duration = std::posix_time::microsec(interval);
        while(true) {
            wait(duration, sc_core::SC_US);
            boost::posix_time::time_duration consumed = std::posix_time::microsec_clock::local_time() - checkpoint;
            if(consumed > 0 && duration > consumed) {
                std::this_thread::sleep(duration - consumed);
            }
            checkpoint = boost::posix_time::microsec_clock::local_time();
        }
#endif
    }
};

} /* namespace scc */
/** @} */ // end of scc-sysc
#endif    /* _SCC_WALL_TIME_SPEED_LIMITER_H */
