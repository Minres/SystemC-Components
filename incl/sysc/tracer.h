/*******************************************************************************
 * Copyright 2016 MINRES Technologies GmbH
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
/*
 * tracer.h
 *
 *  Created on: Nov 9, 2016
 *      Author: developer
 */

#ifndef _SYSC_TRACER_H_
#define _SYSC_TRACER_H_

#include "utilities.h"
#ifdef WITH_SCV
#include <scv.h>
#endif
#include <string>
#include <vector>

namespace sc_core {
class sc_object;
class sc_trace_file;
}

namespace sysc {

struct tracer : public sc_core::sc_module {
    /**
     *
     */
    enum file_type { NONE, TEXT, COMPRESSED, SQLITE };
    /**
     *
     * @param
     * @param
     * @param enable
     */
    tracer(std::string &&, file_type, bool enable = true);
    /**
     *
     */
    virtual ~tracer() override;

protected:
    void end_of_elaboration() override;
    virtual void descend(const std::vector<sc_core::sc_object *> &);
    virtual void try_trace_signal(sc_core::sc_object *);
    virtual void try_trace_port(sc_core::sc_object *);
    bool enabled;
    sc_core::sc_trace_file *trf;
#ifdef WITH_SCV
    scv_tr_db *txdb;
#endif
};

} /* namespace sysc */

#endif /* _SYSC_TRACER_H_ */
