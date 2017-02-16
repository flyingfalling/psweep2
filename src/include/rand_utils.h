#pragma once
#include <random>
#include <vector>
#include <idxsort.h>
#include <iostream>
#include <commontypes.h>


//template <typename T>
inline std::vector< int64_t > rand_permute(const int64_t& N, std::default_random_engine& rand_gen);


template <typename T>
inline std::vector<std::vector<T> > latin_hypercube(std::vector<T> mins, std::vector<T> maxes, size_t N, std::default_random_engine& rand_gen);


template <typename T>
inline std::vector<std::vector<T> > N_uniform(std::vector<T> mins, std::vector<T> maxes, size_t N,  std::default_random_engine& rand_gen);


//REV: This adds a "1" to each category as it is sampled hahahaha. So if I only choose 1...
inline std::vector<size_t> multinomial_sample( const std::vector<float64_t>& p, const size_t& nsamples, std::default_random_engine& rand_gen );

#include <rand_utils.cpp>
