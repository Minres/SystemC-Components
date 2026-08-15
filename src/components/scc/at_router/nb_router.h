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

#ifndef _SYSC_NB_ROUTER_NB_ROUTER_H_
#define _SYSC_NB_ROUTER_NB_ROUTER_H_

#include "nb_arbiter.h"
#include "nb_decoder.h"
#include "types.h"
#include <memory>
#include <sysc/communication/sc_export.h>
#include <tlm>

namespace scc {
namespace at_router {

template <typename TYPES = tlm::tlm_base_protocol_types> struct nb_router {

    sc_core::sc_time const& clk_period;
    sc_core::sc_vector<t_port<TYPES>> igress{"igress"};
    sc_core::sc_vector<i_port<TYPES>> egress{"egress"};

    sc_core::sc_vector<nb_decoder<TYPES>> decoder{"decoder"};
    sc_core::sc_vector<nb_arbiter<TYPES>> arbiter{"arbiter"};
    nb_router(sc_core::sc_time const& clk_period)
    : clk_period(clk_period) {}

    void set_clock_if(sc_core::sc_signal_in_if<sc_core::sc_time>* clk_if) {
        for(auto& a : arbiter)
            a.clk_if = clk_if;
    }
};

template <typename TYPES = tlm::tlm_base_protocol_types>
using creator_fct = std::function<std::unique_ptr<nb_router<TYPES>>(unsigned, unsigned, unsigned, util::range_lut<unsigned> const&,
                                                                    sc_core::sc_time const&)>;

namespace hub {
template <typename TYPES = tlm::tlm_base_protocol_types>
std::unique_ptr<nb_router<TYPES>> create(unsigned bus_width, unsigned igress_cnt, unsigned egress_cnt, util::range_lut<unsigned> const&,
                                         sc_core::sc_time const&);
}
namespace crossbar {
template <typename TYPES = tlm::tlm_base_protocol_types>
std::unique_ptr<nb_router<TYPES>> create(unsigned bus_width, unsigned igress_cnt, unsigned egress_cnt, util::range_lut<unsigned> const&,
                                         sc_core::sc_time const&);
}
} // namespace at_router
} // namespace scc
#endif