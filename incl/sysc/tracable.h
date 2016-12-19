/*
 * tracable.h
 *
 *  Created on: Nov 9, 2016
 *      Author: developer
 */

#ifndef SYSC_TRACABLE_H_
#define SYSC_TRACABLE_H_

namespace sc_core {
    class sc_trace_file;
}

namespace sysc {

class tracable {
public:
    virtual ~tracable(){};
    virtual void trace(sc_core::sc_trace_file* trf) = 0;
};

} /* namespace sysc */

#endif /* SYSC_TRACABLE_H_ */
