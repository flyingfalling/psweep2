#pragma once

#include <vector>
#include <string>
#include <cstdlib>

template <typename T>
void recursive_permute_params(const std::vector< std::vector<T> >& list_to_permute,
			      const size_t& binlevel, /* REV: I guess this is 1d specification of 2d space? */
			      std::vector< T >& permuted_list_in_progress,
			      std::vector< std::vector< T > >& finished_list
			      );



#include <recursive_permute.cpp>
