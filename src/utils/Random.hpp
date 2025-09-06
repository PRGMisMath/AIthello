#ifndef NL_RANDOM_HPP
#define NL_RANDOM_HPP

#define NL_RAND_MAX ULLONG_MAX



#include <random>
#include <chrono>
#include <cstdint>

/* This is xoroshiro128+ 1.0, our best and fastest small-state generator
   for floating-point numbers, but its state space is large enough only
   for mild parallelism. We suggest to use its upper bits for
   floating-point generation, as it is slightly faster than
   xoroshiro128++/xoroshiro128**. It passes all tests we are aware of
   except for the four lower bits, which might fail linearity tests (and
   just those), so if low linear complexity is not considered an issue (as
   it is usually the case) it can be used to generate 64-bit outputs, too;
   moreover, this generator has a very mild Hamming-weight dependency
   making our test (http://prng.di.unimi.it/hwd.php) fail after 5 TB of
   output; we believe this slight bias cannot affect any application. If
   you are concerned, use xoroshiro128++, xoroshiro128** or xoshiro256+.

   We suggest to use a sign test to extract a random Boolean value, and
   right shifts to extract subsets of bits.

   The state must be seeded so that it is not everywhere zero. If you have
   a 64-bit seed, we suggest to seed a splitmix64 generator and use its
   output to fill s.

   NOTE: the parameters (a=24, b=16, c=37) of this version give slightly
   better results in our test than the 2016 version (a=55, b=14, c=36).
*/

// TODO : Allow an easier thread separation by allowing manipulation on local seeds


namespace rd {



class xoroshiro128plus {
public:
    using result_type = uint64_t;

    static constexpr uint64_t min() { return 0; }
    static constexpr uint64_t max() { return ~0ULL; }

private:
    uint64_t s[2];

public:
    xoroshiro128plus(uint64_t seed);

    uint64_t operator()();

    /* This is the jump function for the generator. It is equivalent
       to 2^64 calls to next(); it can be used to generate 2^64
       non-overlapping subsequences for parallel computations. */

    void jump();


    /* This is the long-jump function for the generator. It is equivalent to
       2^96 calls to next(); it can be used to generate 2^32 starting points,
       from each of which jump() will generate 2^32 non-overlapping
       subsequences for parallel distributed computations. */
    void long_jump();
};


template <typename _Distribution, typename _Engine, typename res_type>
class Distributor {
public:
    inline res_type next() { return distr(*rng); }

private:
    friend class Random;
    Distributor(const _Distribution& d, _Engine* e) : distr(d), rng(e) {}

private:
    _Engine* rng;
    _Distribution distr;

};


class Random {
public:
    using _Engine = xoroshiro128plus;
public:
    Random()
        : rng(std::chrono::steady_clock::now().time_since_epoch().count()) {
    }

    void set_seed(uint64_t seed) { rng = _Engine(seed); }

    // Entier dans [a,b]
    inline int uniform_int(int a, int b) {
        std::uniform_int_distribution<int> dist(a, b);
        return dist(rng);
    }

    inline Distributor<std::uniform_int_distribution<int>, _Engine, int> uniform_int_distributor(int a, int b) {
        return Distributor<std::uniform_int_distribution<int>, _Engine, int>(std::uniform_int_distribution<int>(a, b), &rng);
    }

    inline double uniform_real(double a, double b) {
        std::uniform_real_distribution<double> dist(a, b);
        return dist(rng);
    }

    inline Distributor<std::uniform_real_distribution<double>, _Engine, double> uniform_real_distributor(double a, double b) {
        return Distributor<std::uniform_real_distribution<double>, _Engine, double>(std::uniform_real_distribution<double>(a, b), &rng);
    }

    inline double normal(double mean, double stddev) {
        std::normal_distribution<double> dist(mean, stddev);
        return dist(rng);
    }

    inline Distributor<std::normal_distribution<double>, _Engine, double> normal_distributor(double mean, double stddev) {
        return Distributor<std::normal_distribution<double>, _Engine, double>(std::normal_distribution<double>(mean, stddev), &rng);
    }

    inline bool bernoulli(double p) {
        std::bernoulli_distribution dist(p);
        return dist(rng);
    }

    inline Distributor<std::bernoulli_distribution, _Engine, bool> bernoulli_distributor(double p) {
        return Distributor<std::bernoulli_distribution, _Engine, bool>(std::bernoulli_distribution(p), &rng);
    }

private:
    _Engine rng;

};


inline thread_local Random src;




}


#endif // !NL_RANDOM_HPP