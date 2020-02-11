/*******************************************************************************
 * Copyright 2018, 2019, 2020 MINRES Technologies GmbH
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

#ifndef _SCC_H_
#define _SCC_H_

#include "util/assert.h"
#include "util/bit_field.h"
#include "util/delegate.h"
#include "util/io-redirector.h"
#include "util/ities.h"
#include "util/logging.h"
#include "util/range_lut.h"
#include "util/sparse_array.h"
#include "util/thread_syncronizer.h"

#include "json/json-forwards.h"
#include "json/json.h"
//#include "scc/core/sc_logic_7.h"
#include "scc/utilities.h"
#include "scc/report.h"
#include "scc/peq.h"
#include "scc/signal_initiator_mixin.h"
#include "scc/signal_target_mixin.h"
#include "scc/tagged_target_mixin.h"
#ifdef WITH_CCI
#include "scc/configurable_tracer.h"
#include "scc/configurer.h"
#endif
#include "scc/ext_attribute.h"
#include "scc/initiator_mixin.h"
#include "scc/memory.h"
#include "scc/perf_estimator.h"
#include "scc/register.h"
#include "scc/resetable.h"
#include "scc/resource_access_if.h"
#include "scc/router.h"
#include "scc/scv_tr_db.h"
#include "scc/tagged_initiator_mixin.h"
#include "scc/target_mixin.h"
#include "scc/time2tick.h"
#include "scc/tlm_target.h"
#include "scc/traceable.h"
#include "scc/tracer.h"
#include "scc/tracer_base.h"
#include "scc/value_registry.h"

#ifdef WITH_SCV
#include "scv4tlm/tlm2_recorder.h"
#include "scv4tlm/tlm2_recorder_module.h"
#include "scv4tlm/tlm_gp_data.h"
#include "scv4tlm/tlm_gp_data_ext.h"
#include "scv4tlm/tlm_rec_initiator_socket.h"
#include "scv4tlm/tlm_rec_target_socket.h"
#include "scv4tlm/tlm_recording_extension.h"
#endif

#include "tlm/tlm_extensions.h"
#include "tlm/tlm_id.h"
#include "tlm/tlm_signal.h"
#include "tlm/tlm_signal_conv.h"
#include "tlm/tlm_signal_gp.h"
#include "tlm/tlm_signal_sockets.h"
#include "tlm/tlm2_pv_av.h"

#endif /* _SCC_H_ */
