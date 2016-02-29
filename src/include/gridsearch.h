
#pragma once

#include <filesender.h>
#include <recursive_permute.h>

//REV: Need a way to "load" a search? use HDF etc. there?
//First, enum all perumutations, then step through them one by one as a "generation".
//We can "load" by starting search from last "finished" permuation from full permuation list.

struct grid_state
{
  std::vector<std::string> _varnames;
  std::vector<double> _mins;
  std::vector<double> _maxes;
  std::vector<double> _steps;
  std::string _gridfile="__ERROR_NOGRIDFILE";
  
  void search_grid( const optlist& opts,
		    parampoint_generator& pg,
		    filesender& fs )
  {
    parseopts( opts );
    
    search_grid( _varnames,
		 _mins,
		 _maxes,
		 _steps,
		 pg,
		 fs );
  }
  
  void parseopts( const optlist& opts)
  {
    auto a = opts.get_opt_args( "GRIDFILE" );
    if( a.size() == 0)
      {
	fprintf(stderr, "ERROR GRISEARCH: Parseopts, did *not* specify a -GRIDFILE for running, please specify and run again\n");
	exit(1);
      }
    else
      {
	if(a[0].size() == 0 || a[0].size() > 1)
	  {
	    fprintf(stderr, "ERROR GRIDSEARCH: Parseopts, -GRIDFILE option had [%ld] arguments, expects only 1 (name of gridfile)\n", a[0].size() );
	    exit(1);
	  }
	else
	  {
	     _gridfile = a[0][0];
	  }
	
	fprintf(stdout, "GRIDSEARCH: Using specified FILE [%s] as GRIDFILE (will load varnames, min, max, step)\n", _gridfile.c_str());
      }

    //Parse a "RESTART", or "LOAD" option?
    //Also, parse a GRIDCONFIG file or something that has params in it.
    auto b = opts.get_opt_args( "RESTART" );
    //How do restarts work? Do I load an old HDF5 file I guess.
    //All things are stored in .state type files
    if( b.size() > 0 )
      {
	fprintf(stdout, "GRIDSEARCH: Option RESTART is not yet supported! Coming soon...\n");
	exit(1);
      }

    
  }

  void parse_gridfile( const std::string& gridfname )
  {
    bool hascolnames = true;
    data_table dtable( gridfname, hascolnames );
    fprintf(stdout, "GRIDSEARCH: PARSE GRIDFILE: Trying to get VARNAMEs\n");
    _varnames = dtable.get_col( "NAME" );
    fprintf(stdout, "Got varnames. Getting MINS\n");
    _mins = data_table::to_float64( dtable.get_col( "MIN" ) );
    fprintf(stdout, "Got mins, getting MAXES\n");
    _maxes = data_table::to_float64( dtable.get_col( "MAX" ) );
    fprintf(stdout, "Got maxes, getting STEP\n");
    _steps = data_table::to_float64( dtable.get_col( "STEP" ) );
    fprintf(stdout, "Got STEP. Finished parse\n");
  }
  
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

    seedgen sg;
    sg.seed(0);
  
    fprintf(stdout, "EXECUTING SEARCH GRID: Now doing COMP PP LIST\n");
  
    //Now, run on vls.
    fs.comp_pp_list(pg, vls, sg, fs.todisk); //, workingworkers);

    fprintf(stdout, "Finished comp PP list, leaving search grid\n");
  }

  
};


void search_grid( const optlist& opts,
		  parampoint_generator& pg,
		  filesender& fs
		  )
{
  grid_state state;
  
  state.search_grid( opts, pg, fs );

  return;
}
