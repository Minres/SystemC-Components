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

#ifndef _SCC_PERFORMANCETRACER_H_
#define _SCC_PERFORMANCETRACER_H_

#include <boost/date_time/posix_time/posix_time.hpp>
#include <systemc>

namespace scc {
/**
 * a performance estimator
 * it records the time stamps a various time points (start and end of simulation) and calculates
 * some performance figures
 */
class perf_estimator : public sc_core::sc_module {
    //! some internal data structure to record a time stamp
    struct time_stamp {
        boost::posix_time::ptime wall_clock_stamp;
        double proc_clock_stamp;
        time_stamp()
        : wall_clock_stamp(boost::posix_time::microsec_clock::universal_time())
        , proc_clock_stamp(get_cpu_time()) {}
        time_stamp &operator=(const time_stamp &o) {
            wall_clock_stamp = o.wall_clock_stamp;
            proc_clock_stamp = o.proc_clock_stamp;
            return *this;
        }
        void set() {
            wall_clock_stamp = boost::posix_time::microsec_clock::universal_time();
            proc_clock_stamp = get_cpu_time();
        }

    private:
        static double get_cpu_time();
    };

public:
    /**
     * default constructor
     */
    perf_estimator();
    /**
     * the destructor
     */
    virtual ~perf_estimator();

protected:
    //! SystemC callbacks
    void end_of_elaboration() override;
    void start_of_simulation() override;
    void end_of_simulation() override;
    //! the recorded time stamps
    time_stamp soc, eoe, sos, eos;
};

} /* namespace scc */

#endif /* _SCC_PERFORMANCETRACER_H_ */
