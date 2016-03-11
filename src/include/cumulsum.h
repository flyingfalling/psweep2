#pragma once

#include <vector>
#include <cstdlib>

#include <cstdio>
#include <iostream>

//#include "nsim3fast.h"

/*
template <typename T>
void test_binary_search_cumsum();

template <typename T>
size_t binary_search_cumsum(const std::vector<T>& array, size_t start, size_t end, double targ);

template <typename T>
void recompute_cumsum_based_on_choice(const std::vector<T>& array, std::vector<T>& cumsum, size_t choice);

template <typename T>
std::vector<T> compute_cumsum( const std::vector<T>& array );

//template <typename T>
//std::vector<size_t> choose_k_multinomial_no_replace(std::vector<T>& array, const std::vector<T>& cumsum, size_t k);

template <typename T>
std::vector<size_t> choose_k_multinomial_no_replace_wrand(std::vector<T>& array, const std::vector<T>& cumsum, const std::vector<T>& randns);
*/
template <typename T>
inline void test_binary_search_cumsum();

template <typename T>
inline void printvect_cerr(std::vector<T> v);

template <typename T>
inline size_t binary_search_cumsum(const std::vector<T>& array, size_t start, size_t end, T targ);
  
template <typename T>
inline void recompute_cumsum_based_on_choice(const std::vector<T>& array, std::vector<T>& cumsum, size_t choice);

template <typename T>
inline std::vector<T> compute_cumsum( const std::vector<T>& array );

template <typename T>
inline std::vector<size_t> choose_k_multinomial_no_replace_wrand(std::vector<T>& array, const std::vector<T>& cumsum, const std::vector<T>& randns);



#include <cumulsum.cpp>
