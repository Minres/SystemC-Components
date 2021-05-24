#include <scc/mt19937_rng.h>
#include <systemc>
#include <unordered_map>
namespace {
struct {
    std::mt19937_64 global;
    std::unordered_map<void*, std::mt19937_64> inst;
    uint64_t seed{std::mt19937_64::default_seed};
    bool global_seed;
} rng;
}; // namespace

auto scc::MT19937::inst() -> std::mt19937_64& {
    if(auto* obj = sc_core::sc_get_current_object()) {
        auto sz = rng.inst.size();
        auto& ret = rng.inst[obj];
        if(rng.inst.size() > sz) {
            std::string name{obj->name()};
            std::hash<std::string> h;
            if(rng.global_seed)
                ret.seed(reinterpret_cast<uintptr_t>(&rng.inst) ^ rng.seed);
            else
                ret.seed(h(name) ^ rng.seed);
        }
        return ret;
    } else {
        return rng.global;
    }
}

void scc::MT19937::seed(uint64_t new_seed) {
    rng.seed = new_seed;
    rng.global.seed(new_seed);
    for(auto& e : rng.inst)
        e.second.seed(new_seed);
}

void scc::MT19937::enable_global_seed(bool enable) {
    rng.global_seed=enable;
}
