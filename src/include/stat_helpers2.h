#pragma once

#include <random>
#include <vector>
#include <cstdio>
#include <cstdlib>
#include <algorithm>


//choose k from N (indices)
//Use "slice" to get slice from this. Not so efficient though haha.
inline std::vector<size_t> choose_k_indices_from_N_no_replace(size_t _N, size_t _k, std::default_random_engine& rand_gen)
{
  std::vector<size_t> selected;
  std::vector<size_t> selected2; //ordered
  if(_k > _N)
    {
      fprintf(stderr, "Choose_k_from_N: ERROR, k>N\n"); 
      exit(1);
    }
  //draw uniform from 0 to [N choose k]. N  choose k can be computed as :
  //N! / (k! * (N-k)!). But that is massive.
    
  //Easiest impl: just select x from N, N-1, N-2, N-3... N-k, each time, and take "xth element"
  //skipping ones that have already been selected/marked.
      
  while(selected2.size() < _k)
    {
      //This should be drawn uniformly from the remaining possible selections.
      std::uniform_int_distribution<size_t> dist(0,  _N-1-selected2.size());
      
      //generate an actual random number. Might overlap within draws.
      size_t res2 =  dist(rand_gen);
      
      for(size_t s=0; s<selected2.size(); ++s)
	{
	  if(res2 >= selected[s])
	    {
	      res2 = (res2+1);
	    }
	}
      
      selected.push_back( res2 );
      selected2.push_back( res2 ); //this keeps the random order because we have to sort selected otherwise massive biases.
      
      //REV: mustn't sort except to check! Need random choice for each chain, so keep 2 of these around even if wasteful, or re-shuffle it.
      std::sort(selected.begin(), selected.end());
      
    }
  return selected2;
}


//shuffle indices. There should be no bias. We could do faster by simply drawing N random numbers and somehow mapping those to targets...?
inline std::vector<size_t> shuffle_indices(size_t _N, std::default_random_engine& rand_gen)
{
  //just choose all N.
  std::vector<size_t> result = choose_k_indices_from_N_no_replace(_N, _N, rand_gen);
  return result;
}

