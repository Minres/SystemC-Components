/*
 * performancetracer.h
 *
 *  Created on: 08.08.2018
 *      Author: eyck
 */

#ifndef _SCC_PERFORMANCETRACER_H_
#define _SCC_PERFORMANCETRACER_H_

#include <boost/date_time/posix_time/posix_time.hpp>
#include <systemc>

namespace scc {

class perf_estimator: public sc_core::sc_module {
    struct time_stamp {
        boost::posix_time::ptime wall_clock_stamp;
        double proc_clock_stamp;
        time_stamp(): wall_clock_stamp(boost::posix_time::microsec_clock::universal_time()), proc_clock_stamp(get_cpu_time()){}
        time_stamp& operator=(const time_stamp& o){
            wall_clock_stamp=o.wall_clock_stamp;
            proc_clock_stamp=o.proc_clock_stamp;
            return *this;
        }
        void set(){
            wall_clock_stamp=boost::posix_time::microsec_clock::universal_time();
            proc_clock_stamp=get_cpu_time();
        }
    private:
        static double get_cpu_time();
    };
public:
    SC_HAS_PROCESS(perf_estimator);
    perf_estimator();
    virtual ~perf_estimator();
protected:
    void start_of_simulation() override;
    void end_of_simulation() override;

    time_stamp sos, eos;
};

} /* namespace scc */

#endif /* _SCC_PERFORMANCETRACER_H_ */
