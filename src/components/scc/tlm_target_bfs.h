/*******************************************************************************
 * Copyright 2021, 2021 Chair of Electronic Design Automation, TU Munich
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
/**
 * @Author: Uzair Sharif, Philip Dachs
 * @Date:   2019-03-10T02:03:41+01:00
 * @Filename: tlm_target_bfs.h
 * @Last modified by:   Johannes Geier (contact: johannes.geier@tum.de)
 * @Last modified time: 2021-01-20T18:11:00+02:00
 * @Brief: Extension for scc::tlm_target to have registers with associated and callback-able bitfields
 */

#ifndef __SCC_TLM_TARGET_BFS_H__
#define __SCC_TLM_TARGET_BFS_H__

#include <memory>
#include <utility>
#include <vector>

#include "systemc"
#include "tlm_target.h"
#include "tlm_utils/simple_target_socket.h"

#include "util/ities.h"

#define ID_SCC_TLM_TARGET_BFS "scc: tlm target bitfield support"

namespace scc {

typedef struct tlm_target_bfs_params {
    uint64_t base_addr{0};
    size_t size{0};
    size_t num_irqs{0};
    size_t num_regs{0};
    tlm_target_bfs_params(void)
    : base_addr()
    , size()
    , num_irqs()
    , num_regs() {}
    tlm_target_bfs_params(uint64_t base_addr, size_t size, size_t num_irqs, size_t num_regs)
    : base_addr(base_addr)
    , size(size)
    , num_irqs(num_irqs)
    , num_regs(num_regs) {}
} tlm_target_bfs_params_t;

template <class owner_t> class tlm_target_bfs_base {
public:
    sc_core::sc_in<bool> rst_in_{"reset_in"};
    std::unique_ptr<std::vector<sc_core::sc_out<bool>>> irq_out_{nullptr};

    tlm_target_bfs_base(tlm_target_bfs_params_t&& params, owner_t* owner = nullptr)
    : params_{std::move(params)}
    , owner_{owner} {
        irq_out_ = util::make_unique<std::vector<sc_core::sc_out<bool>>>(params_.num_irqs);
    }
    virtual ~tlm_target_bfs_base() = default;

    const owner_t* getOwner() const { return owner_; }

    void bindIRQ(size_t num, sc_core::sc_signal<bool>* sig) {
        if(num >= irq_out_->size()) {
            SC_REPORT_FATAL(ID_SCC_TLM_TARGET_BFS, "not enough IRQs in Per::connectIRQ()");
        }

        (*irq_out_)[num].bind(*sig);
    }

protected:
    const tlm_target_bfs_params_t params_{};
    owner_t* const owner_{nullptr};
};

/**
 * @brief Peripheral base class using scc::tlm_target
 *
 * @tparam regs_t Subclass of \ref tlm_target_bfs_register_base that contains the
 *                register definitions.
 * @tparam owner_t Type of the owner pointer
 */
template <typename regs_t, typename owner_t>
class tlm_target_bfs : public sc_core::sc_module, public tlm_target_bfs_base<owner_t>, public scc::tlm_target<> {
    SC_HAS_PROCESS(tlm_target_bfs);

public:
    class socket_accessor {
    public:
        constexpr socket_accessor(scc::tlm_target<>& parent) noexcept
        : parent(parent) {}
        socket_accessor(const socket_accessor&) = delete;
        socket_accessor& operator=(const socket_accessor&) = delete;

        tlm::tlm_target_socket<>* get() noexcept { return &parent.socket; }

    private:
        scc::tlm_target<>& parent;
    };

    using tlm_target_bfs_base<owner_t>::rst_in_;

    tlm_target_bfs(sc_core::sc_module_name name, tlm_target_bfs_params_t&& params, owner_t* owner = nullptr)
    : sc_core::sc_module{name}
    , tlm_target_bfs_base<owner_t>{std::move(params), owner}
    , scc::tlm_target<>{clk}
    , NAMEDD(regs, regs_t) {
        regs->registerResources(*this);
        SC_METHOD(reset_cb);
        sensitive << rst_in_;
    }

    /**
     * @brief The socket to access the memory mapped registers of this target
     *
     * Use sock_t_.get() to access a pointer to tlm::tlm_target_socket<>
     */
    socket_accessor sock_t_{*this};

protected:
    std::unique_ptr<regs_t> regs;
    sc_core::sc_time clk;

    void reset_cb() {
        if(rst_in_.read())
            regs->reset_start();
        else
            regs->reset_stop();
    }
};

} // namespace scc

#endif // __SCC_TLM_TARGET_BFS_H__
