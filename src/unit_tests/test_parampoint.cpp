//REV: This a test program that accomplishes quite a bit.
// Test MPI file send etc. first?
// Test rebasing?

#include <iostream>
#include <string>

#include <boost/mpi.hpp>

#include <include/parampoint.h>
#include <include/filesender.h>
#include <include/test_parampoint.h>


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
  fs.comp_pp_list(pg, vls); //, workingworkers);

  fprintf(stdout, "Finished comp PP list, leaving search grid\n");
}


//Do "execute", and what I do is I only define what happens? It automatically farms everything? Crap...
//Pass callback? Meh, ughly.

//REV: make it so that user is only ever writing loops in ROOTRANK.
//All other (main) guys are fed off... i.e. it will loop and NEVER return control to the user. That seems much easier.

int main()
{
  //boost::mpi::environment env;
  //boost::mpi::communicator world;

  //REV: if we include loop in construcdtor, it will never actually be constructed...
  //filesender fs = filesender( std::shared_ptr<boost::mpi::environment>(&env), std::shared_ptr<boost::mpi::communicator>(&world) );

  //std::unique_ptr<filesender> fs = std::unique_ptr<filesender>(filesender::Create());
  filesender* fs = filesender::Create();
  
  std::string scriptfname = "../configs/test_parampoint.cfg";
  std::string mydir = "./testdir";
      
  parampoint_generator pg(scriptfname, mydir);
      
      

  size_t nworkers = fs->world.size();
  std::vector<bool> workingworkers( nworkers, true ); //they start out true by default...? OK go.

      
  std::vector<std::string> varnames = {"VAR1", "VAR2"};
  std::vector<double> varmins = {1.0, 100.0};
  std::vector<double> varmaxes = {2.0, 200.0};
  std::vector<double> varsteps = {0.5, 50.0};
  //Need to determine "search". In our case, just do a grid search? Nah, do a couple generations...
  search_grid( varnames, varmins, varmaxes, varsteps,
	       pg, *fs, workingworkers);
      
  //Send exit command to each worker. I.e. broadcast.
  fprintf(stderr, "ROOT FINISHED! Broadcasting EXIT\n");
  std::string contents="EXIT";
  boost::mpi::broadcast(fs->world, contents, 0);
      
  delete(fs);
  
  
}

