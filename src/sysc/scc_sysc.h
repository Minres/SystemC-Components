/*******************************************************************************
 * Copyright 2021 MINRES Technologies GmbH
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

#pragma once

#ifdef WITH_CCI
#include "scc/configurable_tracer.h"
#include "scc/configurer.h"
#endif
#include "scc/ext_attribute.h"
#include "scc/fifo_w_cb.h"
#include "scc/mt19937_rng.h"
#include "scc/ordered_semaphore.h"
#include "scc/peq.h"
#include "scc/perf_estimator.h"
#include "scc/report.h"
#include "scc/sc_logic_7.h"
#include "scc/sc_owning_signal.h"
#include "scc/sc_variable.h"
#include "scc/scv/scv_tr_db.h"
#include "scc/scv/sqlite3.h"
#include "scc/tick2time.h"
#include "scc/time2tick.h"
#include "scc/traceable.h"
#include "scc/tracer.h"
#include "scc/tracer_base.h"
#include "scc/utilities.h"
#include "scc/value_registry.h"
#include "tlm/scc/scv/tlm_extension_recording_registry.h"
#include "tlm/scc/scv/tlm_gp_data.h"
#include "tlm/scc/scv/tlm_gp_data_ext.h"
#include "tlm/scc/scv/tlm_rec_initiator_socket.h"
#include "tlm/scc/scv/tlm_rec_target_socket.h"
#include "tlm/scc/scv/tlm_recorder.h"
#include "tlm/scc/scv/tlm_recorder_module.h"
#include "tlm/scc/scv/tlm_recording_extension.h"

#include "tlm/scc/initiator_mixin.h"
#include "tlm/scc/tlm2_pv_av.h"
#include "tlm/scc/tlm_extensions.h"
#include "tlm/scc/tlm_id.h"
#include "tlm/scc/tlm_mm.h"
#if(SYSTEMC_VERSION >= 20171012)
#include "tlm/scc/signal_initiator_mixin.h"
#include "tlm/scc/signal_target_mixin.h"
#include "tlm/scc/tagged_initiator_mixin.h"
#include "tlm/scc/tagged_target_mixin.h"
#include "tlm/scc/tlm_signal.h"
#include "tlm/scc/tlm_signal_conv.h"
#include "tlm/scc/tlm_signal_gp.h"
#include "tlm/scc/tlm_signal_sockets.h"
#endif

#include "tlm/scc/pe/intor_if.h"
#include "tlm/scc/pe/parallel_pe.h"
