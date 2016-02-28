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


template <typename T>
void recursive_permute_params(const std::vector< std::vector<T> >& list_to_permute,
			      const size_t& binlevel, /* REV: I guess this is 1d specification of 2d space? */
			      std::vector< T >& permuted_list_in_progress,
			      std::vector< std::vector< T > >& finished_list
			      )
{
  
  if(binlevel == list_to_permute.size()) /* if we've added all the guys we need */
    {
      /* terminate this "branch", push back to final result */
      finished_list.push_back(permuted_list_in_progress);
      //if(DEBUG>0) fprintf(stdout, "\n");
      return;
    }
  else
    {
      
      for(int binitem=0; binitem < list_to_permute[binlevel].size(); ++binitem)
	{
	  
	  permuted_list_in_progress.resize(binlevel); /* REV: this will clear if 0, and resize back since this is another "pop" */
	  //if(DEBUG>0) fprintf(stdout, "Processing bin item %d of binlevel %d (list size: %d)\n", binitem, binlevel, permuted_list_in_progress.size());
	  
	  //if(DEBUG>0) fprintf(stdout, "%d ", binitem);
	  
	  /* REV: I need to make it back to binsize */
	  permuted_list_in_progress.push_back( list_to_permute[binlevel][binitem] );
	  
	  recursive_permute_params<T>(list_to_permute, binlevel+1, permuted_list_in_progress, finished_list);
	}
    }
}
