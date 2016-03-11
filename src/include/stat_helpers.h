#pragma once

#include <random>
#include <vector>
#include <cstdio>
#include <cstdlib>
#include <algorithm>
//#include "stat_helpers2.h"

template <typename T>
std::vector< T > indices_to_vector_slices(const std::vector< T >& source, const std::vector<size_t>& indices);


//This is same as indices_to_vector_slices...
//REV: reorganize inline? Nah, return, memory
/*template <typename T>
std::vector<T> reorganize_by_index_array( std::vector<T>& targ, const std::vector<size_t>& ind )
{
  std::vector<T> newthing( targ.size() );
  for(size_t x=0; x<targ.size(); ++x)
    {
      //So, 1 in the new one will be where it is? Hm, index array tells where original guys are. E.g. if it says 37, I should take 37 from original guy
      //E.g. if 37 is in index[1], then newthing[1] should be targ[ 37 ]. I.e. newthing[x] = targ[ index[ x ]]
      newthing[x] = targ[ ind[x] ];
    }

  return newthing;
  }*/




//actually return the (k) values
template <typename T>
std::vector<T> choose_k_values_from_N_no_replace(std::vector<T>& input, size_t _k, std::default_random_engine& rand_gen);


//actually return the (N) values in random order
template <typename T>
std::vector<T> shuffle_values(std::vector<T>& input, std::default_random_engine& rand_gen);



//REV: TODO some static sanity checks for these functions (and to check randomness?) (check sweep_mpi.h with DEBUG > 10 for test of the N choose k)



#include <stat_helpers.cpp>
