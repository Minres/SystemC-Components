/*******************************************************************************
 * Copyright 2020-2022 MINRES Technologies GmbH
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

#ifndef _SCC_MT19937_RNG_H_
#define _SCC_MT19937_RNG_H_

#include <assert.h>
#include <iostream>
#include <random>

/** \ingroup scc-sysc
 *  @{
 */
/**@{*/
//! @brief SCC SystemC utilities
namespace scc {
/**
 * @class MT19937
 * @brief a mersenne-twister based random number generator
 *
 * This random number generator provides various distribution of random numbers being specific to the SystemC process
 * invoking the generator function. This makes the generator independent of the order of invocation in a delta cycle and
 * allows to replay with the same seed
 */
class MT19937 {
public:
    /**
     * Seeds the mersenne twister PRNG with the given value
     *
     * @param new_seed
     */
    static void seed(uint64_t new_seed = std::mt19937_64::default_seed);
    /**
     * By default each SystemC process has its own MT rng with a modified seed based on the
     * process name. If set to true each MT rng gets exactly the same seed thus producing the
     * same sequence.
     *
     * @param enable use the same seed for all MT rng instances
     */
    static void enable_global_seed(bool enable);

    /**
     * generates the next random integer number with uniform distribution (similar to rand() )
     *
     * @return
     */
    static uint64_t uniform() {
        std::uniform_int_distribution<uint64_t> u;
        return u(inst());
    }
    /**
     * generates the next random integer number with uniform distribution in the range of the given type
     *
     * @return
     */
    template <typename T> static T uniform() {
        std::uniform_int_distribution<T> u;
        return u(inst());
    }
    /**
     * generates the next random integer number with uniform distribution between (and including) min and max
     *
     * @param min the lower limit of the interval
     * @param max the upper limit of the interval
     * @return
     */
    static uint64_t uniform(uint64_t min, uint64_t max) {
        assert(min < max);
        std::uniform_int_distribution<uint64_t> u(min, max);
        return u(inst());
    }
    /**
     * generates the next random double precision float number with normal distribution (similar to rand() )
     *
     * @return
     */
    static double normal() {
        std::normal_distribution<> u;
        return u(inst());
    }
    /**
     * generates the next random integer number with log normal distribution (similar to rand() )
     *
     * @return
     */
    static double lognormal() {
        std::lognormal_distribution<> u;
        return u(inst());
    }

private:
    static std::mt19937_64& inst();
};

} // namespace scc
/** @} */ // end of scc-sysc
#endif /* _SCC_MT19937_RNG_H_ */
