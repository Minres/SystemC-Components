/*
 * tracer_base.h
 *
 *  Created on: 30.12.2018
 *      Author: eyck
 */

#ifndef _SCC_TRACER_BASE_H_
#define _SCC_TRACER_BASE_H_

#include "utilities.h"

namespace scc {

class tracer_base  : public sc_core::sc_module {
public:

    tracer_base(const sc_core::sc_module_name& nm ) :sc_core::sc_module(nm), trf(nullptr){}

protected:

    virtual void try_trace_signal(const sc_core::sc_object *);

    virtual void try_trace_port(const sc_core::sc_object *);

    virtual void descend(const sc_core::sc_object *);

    sc_core::sc_trace_file *trf;
};

}
#endif /* _SCC_TRACER_BASE_H_ */
