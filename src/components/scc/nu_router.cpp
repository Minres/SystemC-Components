#include "nu_router.h"
#include <stdexcept>
#include <variant>

namespace scc {
nu_router::nu_router(const sc_core::sc_module_name& nm, size_t slave_cnt, size_t master_cnt, bool check_overlap_on_add_target)
: sc_module(nm)
, targets(master_cnt)
, initiators(slave_cnt)
, ibases(master_cnt)
, tranges(slave_cnt)
, mutexes(slave_cnt)
, addr_decoder(std::numeric_limits<unsigned>::max())
, check_overlap_on_add_target(check_overlap_on_add_target) {
    for(size_t i = 0; i < targets.size(); ++i) {
        ibases[i] = 0ULL;
    }
    for(size_t i = 0; i < initiators.size(); ++i) {
        tranges[i] = {0ULL, 0ULL, false};
    }
}

void nu_router::set_target_range(size_t idx, uint64_t base, uint64_t size, bool remap) {
    tranges[idx].base = base;
    tranges[idx].size = size;
    tranges[idx].remap = remap;
    addr_decoder.addEntry(idx, base, size);
    if(check_overlap_on_add_target)
        addr_decoder.validate();
}

void nu_router::b_transport(int i, tlm::tlm_generic_payload& trans, sc_core::sc_time& delay) {
    ::sc_dt::uint64 address = trans.get_address();
    if(ibases[i]) {
        address += ibases[i];
        trans.set_address(address);
    }
    size_t idx = addr_decoder.getEntry(address);
    if(idx == addr_decoder.null_entry) {
        if(default_idx == std::numeric_limits<size_t>::max()) {
            if(warn_on_address_error) {
                SCCWARN(SCMOD) << "target address=0x" << std::hex << address << " not found for "
                               << (trans.get_command() == tlm::TLM_READ_COMMAND ? "read" : "write") << " transaction.";
            }
            trans.set_response_status(tlm::TLM_ADDRESS_ERROR_RESPONSE);
            return;
        }
        idx = default_idx;
    } else {
        // Modify address within transaction
        trans.set_address(address - (tranges[idx].remap ? tranges[idx].base : 0));
    }
    // Forward transaction to appropriate target
    std::visit(
        [this, &trans, &delay](auto& initiator) {
            using T = std::decay_t<decltype(initiator)>;
            if constexpr(std::is_same_v<T, std::monostate>) {
                throw std::runtime_error("uninitialized initiator");
            } else
                initiator->b_transport(trans, delay);
        },
        initiators[idx].sckt);
}

bool nu_router::get_direct_mem_ptr(int i, tlm::tlm_generic_payload& trans, tlm::tlm_dmi& dmi_data) {
    ::sc_dt::uint64 address = trans.get_address();
    if(ibases[i]) {
        address += ibases[i];
        trans.set_address(address);
    }
    size_t idx = addr_decoder.getEntry(address);
    if(idx == addr_decoder.null_entry) {
        if(default_idx == std::numeric_limits<size_t>::max()) {
            if(warn_on_address_error) {
                SCCWARN(SCMOD) << "target address=0x" << std::hex << address << " not found for "
                               << (trans.get_command() == tlm::TLM_READ_COMMAND ? "read" : "write") << " transaction.";
            }
            trans.set_response_status(tlm::TLM_ADDRESS_ERROR_RESPONSE);
            return false;
        }
        idx = default_idx;
    }
    // Modify address within transaction
    auto offset = tranges[idx].remap ? tranges[idx].base : 0;
    trans.set_address(address - offset);
    bool status = false; // initiator[idx]->
    std::visit(
        [this, &trans, &dmi_data, &status](auto& sckt) {
            using T = std::decay_t<decltype(sckt)>;
            if constexpr(std::is_same_v<T, std::monostate>) {
                throw std::runtime_error("uninitialized initiator");
            } else {
                status = sckt->get_direct_mem_ptr(trans, dmi_data);
            }
        },
        initiators[idx].sckt);
    // make sure end address does not exceed size
    auto const remap_base = tranges[idx].remap ? 0 : tranges[idx].base;
    auto const remap_size = tranges[idx].size;
    auto const remap_end = remap_base + remap_size;
    auto const remap_end_overflow = remap_end < remap_base;
    if(remap_size && !remap_end_overflow && dmi_data.get_end_address() >= remap_end)
        dmi_data.set_end_address(remap_end - 1);
    // Calculate DMI address of target in system address space
    dmi_data.set_start_address(dmi_data.get_start_address() - ibases[i] + offset);
    dmi_data.set_end_address(dmi_data.get_end_address() - ibases[i] + offset);
    return status;
}

unsigned nu_router::transport_dbg(int i, tlm::tlm_generic_payload& trans) {
    if(trans.get_command() == tlm::TLM_IGNORE_COMMAND) {
        if(auto ext = trans.get_extension<tlm::scc::memory_map_extension>()) {
            ext->node.name = name();
            auto start_addr = ext->node.start;
            for(auto e : addr_decoder) {
                auto addr = ext->offset + e.first;
                auto entry = e.second;
                switch(entry.type) {
                case util::range_lut<unsigned>::BEGIN_RANGE:
                    start_addr = addr;
                    break;
                case util::range_lut<unsigned>::SINGLE_BYTE_RANGE:
                    start_addr = addr;
                case util::range_lut<unsigned>::END_RANGE: {
                    ext->node.elemets.emplace_back(start_addr, ext->offset + addr);
                    auto new_ext = tlm::scc::memory_map_extension(ext->node.elemets.back());
                    new_ext.offset = start_addr;
                    trans.set_extension(&new_ext);
                    std::visit(
                        [this, &trans](auto& sckt) {
                            using T = std::decay_t<decltype(sckt)>;
                            if constexpr(std::is_same_v<T, std::monostate>) {
                                throw std::runtime_error("uninitialized initiator");
                            } else {
                                sckt->transport_dbg(trans);
                            }
                        },
                        initiators[entry.index].sckt);
                    trans.set_extension(ext);
                    break;
                }
                }
            }
            return 0;
        }
    }
    ::sc_dt::uint64 address = trans.get_address();
    if(ibases[i]) {
        address += ibases[i];
        trans.set_address(address);
    }
    size_t idx = addr_decoder.getEntry(address);
    if(idx == addr_decoder.null_entry) {
        if(default_idx == std::numeric_limits<size_t>::max()) {
            if(warn_on_address_error) {
                SCCWARN(SCMOD) << "target address=0x" << std::hex << address << " not found for "
                               << (trans.get_command() == tlm::TLM_READ_COMMAND ? "read" : "write") << " transaction.";
            }
            trans.set_response_status(tlm::TLM_ADDRESS_ERROR_RESPONSE);
            return 0;
        }
        idx = default_idx;
    } else {
        // Modify address within transaction
        trans.set_address(address - (tranges[idx].remap ? tranges[idx].base : 0));
    }
    // Forward debug transaction to appropriate target
    unsigned res = 0;
    std::visit(
        [this, &trans, &res](auto& sckt) {
            using T = std::decay_t<decltype(sckt)>;
            if constexpr(std::is_same_v<T, std::monostate>) {
                throw std::runtime_error("uninitialized initiator");
            } else {
                res = sckt->transport_dbg(trans);
            }
        },
        initiators[idx].sckt);
    return res;
}

void nu_router::invalidate_direct_mem_ptr(int id, ::sc_dt::uint64 start_range, ::sc_dt::uint64 end_range) {
    // Reconstruct address range in system memory map
    ::sc_dt::uint64 bw_start_range = start_range;
    if(tranges[id].remap)
        bw_start_range += tranges[id].base;
    ::sc_dt::uint64 bw_end_range = end_range;
    if(tranges[id].remap)
        bw_end_range += tranges[id].base;
    for(size_t i = 0; i < targets.size(); ++i) {
        std::visit(
            [this, i, bw_start_range, bw_end_range](auto& sckt) {
                using T = std::decay_t<decltype(sckt)>;
                if constexpr(std::is_same_v<T, std::monostate>) {
                    throw std::runtime_error("uninitialized initiator");
                } else {
                    sckt->invalidate_direct_mem_ptr(bw_start_range - ibases[i], bw_end_range - ibases[i]);
                }
            },
            targets[i].sckt);
    }
}

void nu_router::before_end_of_elaboration() {
    if(creator) {
        rt = creator(64, targets.size(), initiators.size(), addr_decoder, tranges);
        rt->set_default_target(default_idx);
        for(auto i = 0u; i < targets.size(); ++i) {
            auto& igress = rt->igress[i];
            auto& tgt = targets[i];
            std::visit(
                [this, &igress](auto& sckt) {
                    using T = std::decay_t<decltype(sckt)>;
                    if constexpr(std::is_same_v<T, std::monostate>) {
                        throw std::runtime_error("uninitialized initiator");
                    } else {
                        sckt.register_nb_transport_fw(
                            [&igress](tlm::tlm_generic_payload& trans, tlm::tlm_phase& phase, sc_core::sc_time& t) -> tlm::tlm_sync_enum {
                                return igress.fw->nb_transport_fw(trans, phase, t);
                            });
                        t_port_bw_adapt.emplace_back(sckt.get_base_port());
                    }
                },
                tgt.sckt);
            igress.bw(t_port_bw_adapt.back());
            igress.clk(tgt.clk);
        }
        for(auto i = 0u; i < initiators.size(); ++i) {
            auto& egress = rt->egress[i];
            auto& intor = initiators[i];
            std::visit(
                [this, &egress](auto& sckt) {
                    using T = std::decay_t<decltype(sckt)>;
                    if constexpr(std::is_same_v<T, std::monostate>) {
                        throw std::runtime_error("uninitialized initiator");
                    } else {
                        sckt.register_nb_transport_bw(
                            [&egress](tlm::tlm_generic_payload& trans, tlm::tlm_phase& phase, sc_core::sc_time& t) -> tlm::tlm_sync_enum {
                                return egress.bw->nb_transport_bw(trans, phase, t);
                            });
                        i_port_bw_adapt.emplace_back(sckt.get_base_port());
                    }
                },
                intor.sckt);
            egress.fw(i_port_bw_adapt.back());
            egress.clk(intor.clk);
        }
    }
}

void nu_router::end_of_elaboration() {
    addr_decoder.validate();
    if(rt) {
        if(!clk_i.get_interface()) {
            SCCFATAL(SCMOD) << "When using a AT nu_router implementation, the clock input of the dyn_router needs to be connected!";
        }
        rt->set_clock_if(clk_i.get_interface(0));
    }
}

} // namespace scc