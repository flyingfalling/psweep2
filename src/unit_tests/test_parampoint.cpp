
//REV: This a test program that accomplishes quite a bit.
// Test MPI file send etc. first?
// Test rebasing?

#include <iostream>
#include <string>

#include <boost/mpi.hpp>

#include <include/parampoint.h>
#include <include/filesender.h>



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
	  
	  recursive_permute_params<double>(list_to_permute, binlevel+1, permuted_list_in_progress, finished_list);
	}
    }
}

void search_grid( const std::vector<std::string>& varnames,
		  const std::vector<double>& mins,
		  const std::vector<double>& maxes,
		  const std::vector<double>& steps,
		  parampoint_generator& pg,
		  filesender& fs,
		  std::vector<bool>& workingworkers )
{
  //Search_grid will give pg new std::vector<varlist> of vars to send.
  //PROBLEM: Varlists are going to be as varlist<string> (fuck...)

  
  
  std::vector< std::vector< double > > list_of_permuted_lists;
  std::vector< double > permuted_list_workspace; /* can't do this in parallel I guess... */

  std::vector< std::vector< float > > list_to_permute;
  for(int bin=0; bin<varnames.size(); ++bin)
    {
      std::vector< double > binvect;
      float val=mins[bin];
      while(val < maxs[bin])
	{
	  binvect.push_back(val);
	  val += steps[bin];
	}
      binvect.push_back(val); //assume this is max.
      
      if(val != maxs[bin])
	{
	  fprintf(stdout, "WARNING: PARAM SWEEP GRID: PARAM [%s] STEP DOES NOT EVENLY DIVIDE MIN AND MAX (MIN=%lf MAX=%lf STEP=%lf, but final val is %lf)\n", 
		  varnames[bin].c_str(), mins[bin], maxs[bin], steps[bin], val);
	}
      
      list_to_permute.push_back( binvect );
      
    }
  
  recursive_permute_params(list_to_permute, 0, permuted_list_workspace, list_of_permuted_lists);

  //We now have a list of lists...to run ;) Need to turn each one into a varlist
  std::vector<varlist<std::string>> vls;

  for(size_t x=0; x<list_of_permuted_lists.size(); ++x)
    {
      varlist<std::string> vl;
      vl.add_to_varlist<double>( varnames, list_of_permuted_lists[x] );

      vls.push_back(vl);
    }

  
  //Now, run on vls.
  fs.comp_pp_list(pg, vls, workingworkers);
}



int main()
{
  boost::mpi::environment env;
  boost::mpi::communicator world;

  if( world.rank() == 0 )
    {
      std::string scriptfname = "../configs/test_parampoint.cfg";
      std::string mydir = "./testdir";
      
      parampoint_generator pg(scriptfname, mydir);
      
      filesender fs( env, world );

      size_t nworkers = world.size();
      std::vector<bool> workingworkers( nworkers, true ); //they start out true by default...? OK go.

      
      std::vector<std::string> varnames = {"VAR1", "VAR2"};
      std::vector<double> varmins = {1.0, 100.0};
      std::vector<double> varmaxes = {2.0, 200.0};
      std::vector<double> varsteps = {0.5, 50.0};
      //Need to determine "search". In our case, just do a grid search? Nah, do a couple generations...
      search_grid( varnames, varmins, varmaxes, varsteps,
		   pg, fs, workingworkers);

      //Send exit command to each worker. I.e. broadcast.
      fprintf(stderr, "ROOT FINISHED! Broadcasting EXIT\n");
      broadcast(world, "EXIT", 0);
    }
  else
    {
      //RUN WORKER LOOP.
      //Tell it which slave I am etc.?
      fs.execute_slave_loop();
    }

  
  
}

