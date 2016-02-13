
#pragma once

#include <filesender.h>
#include <test_parampoint.h>

void search_grid( const std::vector<std::string>& varnames,
		  const std::vector<double>& mins,
		  const std::vector<double>& maxes,
		  const std::vector<double>& steps,
		  parampoint_generator& pg,
		  filesender& fs )
{

  fprintf(stdout, "Inside search_grid algorithm\n");
  //Search_grid will give pg new std::vector<varlist> of vars to send.
  //PROBLEM: Varlists are going to be as varlist<string> (fuck...)
    
  std::vector< std::vector< double > > list_of_permuted_lists;
  std::vector< double > permuted_list_workspace; /* can't do this in parallel I guess... */

  std::vector< std::vector< double > > list_to_permute;
  for(int bin=0; bin<varnames.size(); ++bin)
    {
      std::vector< double > binvect;
      float val=mins[bin];
      while(val < maxes[bin])
	{
	  binvect.push_back(val);
	  val += steps[bin];
	}
      binvect.push_back(val); //assume this is max.
      
      if(val != maxes[bin])
	{
	  fprintf(stdout, "WARNING: PARAM SWEEP GRID: PARAM [%s] STEP DOES NOT EVENLY DIVIDE MIN AND MAX (MIN=%lf MAX=%lf STEP=%lf, but final val is %lf)\n", 
		  varnames[bin].c_str(), mins[bin], maxes[bin], steps[bin], val);
	}
      
      list_to_permute.push_back( binvect );
      
    }
  
  recursive_permute_params(list_to_permute, 0, permuted_list_workspace, list_of_permuted_lists);

  //We now have a list of lists...to run ;) Need to turn each one into a varlist
  std::vector< varlist<std::string> > vls;

  for(size_t x=0; x<list_of_permuted_lists.size(); ++x)
    {
      varlist<std::string> vl;
      vl.add_to_varlist<double>( varnames, list_of_permuted_lists[x] );

      vls.push_back(vl);
    }

  fprintf(stdout, "EXECUTING SEARCH GRID: Now doing COMP PP LIST\n");
  
  //Now, run on vls.
  fs.comp_pp_list(pg, vls, fs.todisk); //, workingworkers);

  fprintf(stdout, "Finished comp PP list, leaving search grid\n");
}
