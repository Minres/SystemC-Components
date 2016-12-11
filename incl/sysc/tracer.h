/*
 * tracer.h
 *
 *  Created on: Nov 9, 2016
 *      Author: developer
 */

#ifndef SYSC_TRACER_H_
#define SYSC_TRACER_H_

#include <vector>
#include <string>

namespace sc_core{
class sc_object;
class sc_trace_file;
}

namespace sysc {

class tracer {
public:
    tracer(std::string&& name, bool enable=true);
    virtual ~tracer();
protected:
    virtual void descend(const std::vector<sc_core::sc_object*>&);
    virtual void try_trace_signal(sc_core::sc_object*);
    virtual void try_trace_port(sc_core::sc_object*);
    sc_core::sc_trace_file* trf;
};

} /* namespace sysc */

#endif /* SYSC_TRACER_H_ */
