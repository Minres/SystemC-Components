#include <scc/mt19937_rng.h>
#include <systemc>
#include <unordered_map>
namespace {
struct {
    std::mt19937_64 global;
    std::unordered_map<void*, std::mt19937_64> inst;
    uint64_t seed{std::mt19937_64::default_seed};
} rng;
};

std::mt19937_64& scc::MT19937::inst() {
    if(auto* obj = sc_core::sc_get_current_object()) {
        auto sz = rng.inst.size();
        auto& ret = rng.inst[obj];
        if(rng.inst.size() > sz) {
            std::string name{obj->name()};
            std::hash<std::string> h;
            ret.seed(h(name) ^ rng.seed);
        }
        return ret;
    } else {
        return rng.global;
    }
}

void scc::MT19937::seed(uint64_t new_seed){
    rng.seed=new_seed;
    rng.global.seed(new_seed);
    for(auto& e: rng.inst) e.second.seed(new_seed);
}
