//REV: header file for searcher, which provides various search functions
//User will call one of these, passing the appropriate inputs,
//and it will automatically construct everything and run the sweep I guess....

#include <filesender.h>


//takes user options and runs corresponding search funct?

//"Format" is required...for each different search, what kind of info it needs to parameterize it. I guess just parameterize it with a global varlist.
void run_search( const std::string& searchtype, const std::string& scriptfname, const std::string& mydir, const varlist& params )
{

  parampoint_generator pg(scriptfname, mydir);
  
  filesender fs;
    
  //size_t nworkers = fs.world.size();

  //std::vector<bool> workingworkers( nworkers, true ); //they start out true by default...? OK go.
  
  
  
  if( searchtype.compare( "grid" ) == 0 )
    {
      //Construct required stuff from PARAMS. I.e. min and max of each param? Need N varlists? Have them named? Specific name? Have array type of
      //name PARAMS, etc.? Probably got from a file at beginning... Some special way of reading that...it should know varnames? 
      search_grid( );
    }
  else
    {
      fprintf(stderr, "REV: ERROR, search type [%s] not found\n", searchtype.c_str() );
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
  fs.comp_pp_list(pg, vls, workingworkers);

  fprintf(stdout, "Finished comp PP list, leaving search grid\n");
}
