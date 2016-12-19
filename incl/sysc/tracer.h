/*
 * tracer.h
 *
 *  Created on: Nov 9, 2016
 *      Author: developer
 */

#ifndef SYSC_TRACER_H_
#define SYSC_TRACER_H_

#ifdef WITH_SCV
#include <scv.h>
#endif

#include <vector>
#include <string>

namespace sc_core{
class sc_object;
class sc_trace_file;
}

namespace sysc {

struct tracer: public sc_core::sc_module {
    enum file_type { NONE, TEXT, COMPRESSED, SQLITE};
    tracer(std::string&&, file_type, bool enable=true);
    virtual ~tracer();
protected:
    void end_of_elaboration();
    virtual void descend(const std::vector<sc_core::sc_object*>&);
    virtual void try_trace_signal(sc_core::sc_object*);
    virtual void try_trace_port(sc_core::sc_object*);
    bool enabled;
    sc_core::sc_trace_file* trf;
#ifdef WITH_SCV
    scv_tr_db* txdb;
#endif
};

} /* namespace sysc */

#endif /* SYSC_TRACER_H_ */
