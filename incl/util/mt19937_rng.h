/*
 * mt19937_rng.h
 *
 *  Created on: Jan 9, 2020
 *      Author: eyck
 */

#ifndef _UTIL_MT19937_RNG_H_
#define _UTIL_MT19937_RNG_H_

#include <iostream>
#include <random>
#include <assert.h>

namespace util {
class MT19937 {
public:
    /**
     * Seeds the mersenne twister PRNG with the given value
     * @param new_seed
     */
    static void seed(uint64_t new_seed = std::mt19937_64::default_seed) { inst().seed(new_seed); }
    /**
     * generates the next random integer number with uniform distribution (similar to rand() )
     * @return
     */
    static uint64_t uniform() {
        std::uniform_int_distribution<uint64_t> u;
        return u(inst());
    }
    /**
     * generates the next random integer number with uniform distribution in the range of the given type
     * @return
     */
    template <typename T> static T uniform() {
        std::uniform_int_distribution<T> u;
        return u(inst());
    }
    /**
     * generates the next random integer number with uniform distribution between (and including) min and max
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
     * @return
     */
    static double normal() {
        std::normal_distribution<> u;
        return u(inst());
    }
    /**
     * generates the next random integer number with log normal distribution (similar to rand() )
     * @return
     */
    static double lognormal() {
        std::lognormal_distribution<> u;
        return u(inst());
    }

private:
    static std::mt19937_64& inst() {
        static std::mt19937_64 rng;
        return rng;
    }
};
} // namespace util
#endif /* _UTIL_MT19937_RNG_H_ */
