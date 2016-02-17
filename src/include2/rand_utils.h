#pragma once
#include <random>
#include <vector>


//template <typename T>
std::vector< int64_t > rand_permute(const int64_t& N, std::default_random_engine& rand_gen)
{
  std::vector< int64_t > permuted(N, 0);
  std::vector< float64_t > permuted_probs(N, 0);
  std::uniform_real_distribution<float64_t> dist(0.0,  1.0); //Does NOT include B, i.e. [0, 1)
  for(size_t a=0; a<N; ++a)
    {
      permuted[a] = a;
      permuted_probs[a] =  dist(rand_gen);
    }
  
  //sort it
  sorter<float64_t> bob( &permuted_probs[0] , permuted_probs.size() );
  bob.add_i( &permuted[0] ); //lol must be int, what if we make size_T? Need to mod sorter algo?
  
  bob.runsort();
  
  //and now we check ordering of index values if we want...?
  return permuted;
}

template <typename T>
std::vector<std::vector<T> > latin_hypercube(std::vector<T> mins, std::vector<T> maxes, size_t N, std::default_random_engine& rand_gen)
{
  if(mins.size() != maxes.size())
    {
      fprintf(stderr, "ERROR in latin_hypercube sampling, mins vector size (%ld) != maxes vector size (%ld)\n", mins.size(), maxes.size());
      exit(1);
    }
  
  //REV: need to do random sampling in here, make sure random guys are initialized. So, would need to pass a global RNG or something?
  //first generate the ones within that size.
  
  std::vector< std::vector <T> > final_locations( N, std::vector<T>(mins.size(), 0) );

  std::uniform_real_distribution<float64_t> dist(0.0,  1.0); //Does NOT include B, i.e. [0, 1)
  
  for(size_t dim=0; dim<mins.size(); ++dim)
    {
      std::vector<double> locs_within_slices(N, 0);
      T slicesize = (maxes[dim] - mins[dim]) / (T)N;
      for(size_t z=0; z<N; ++z)
	{
	  //generate randomly within each slice...
	  
	  locs_within_slices[z] = (slicesize*z) + mins[dim] + dist(rand_gen) * slicesize;
	}
      //then permute...
      std::vector<int> permed = rand_permute<int>(N);
      
      //ugh just add it and sort by the random guy?
      for(size_t z=0; z<N; ++z)
	{
	  final_locations[z][dim] = locs_within_slices[ permed[z] ];
	  //final_locations[z][dim] = locs_within_slices[ z ]; //for test slices etc.
	}
    } //end for all dims (distributing based on latin hypercube)
  
  return ( final_locations );
} //end latin_hypercube


template <typename T>
std::vector<std::vector<T> > N_uniform(std::vector<T> mins, std::vector<T> maxes, size_t N,  std::default_random_engine& rand_gen)
{
  if(mins.size() != maxes.size())
    {
      fprintf(stderr, "ERROR in N_uniform sampling, mins vector size (%ld) != maxes vector size (%ld)\n", mins.size(), maxes.size());
      exit(1);
    }
  size_t sizes=mins.size();
  std::vector< std::vector< T> > finals;
  
  for(size_t c=0; c<N; ++c)
    {
      std::vector< T > tmp(sizes, 0);
      for(size_t i=0; i<sizes; ++i)
	{
	  std::uniform_real_distribution<float64_t> dist(mins[i],  maxes[i]); //Does NOT include B, i.e. [0, 1)
	  tmp[i] = dist( rand_gen );
	}
      finals.push_back( tmp );
    }
  //REV: need to do random sampling in here, make sure random guys are initialized. So, would need to pass a global RNG or something?
  return finals;
}


std::vector<size_t> multinomial_sample( const std::vector<float64_t>& p, const size_t& nsamples, std::default_random_engine& rand_gen )
{
  //Call binomial sample multiple times. As in PDFLIB.
  //All probabilities are [0, 1]
  float64_t ptot = 0.0;
  for( size_t i = 0; i < nsamples.size()-1; ++i )
  {
    ptot = ptot + p[i];
  }
  if ( 0.99999 < ptot ) 
  {
    std::cerr << std::endl;
    std::cerr << "multinomial sample: fatal error." << std::endl;
    std::cerr << "  1.0 < Sum of P()." << std::endl;
    exit ( 1 );
  }

  size_t ntot = nsamples;
  ptot = 1.0;

  std::vector<size_t> ix( nsamples.size(), 0 );

  //Make binomial distribution.
  
  
  for( size_t icat = 0; icat < nsamples.size() - 1; ++icat )
    {
      float64_t prob = p[icat] / ptot;
      std::binomial_distribution<size_t> bdist( ntot, prob );
      ix[icat] = bdist(rand_gen);
      //ntot = ntot - ix[icat];
      //if ( ntot <= 0 )
      if( ix[icat] >= ntot )
	{
	  return ix;
	}
      ntot -= ix[icat]; //only do this if it would be >0 (these are unsigned ints)
      ptot = ptot - p[icat];
    }
  
  ix[ nsamples.size()-1 ] = ntot;

  return ix;
} //end multinomial.


