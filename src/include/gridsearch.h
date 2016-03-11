
#pragma once

#include <filesender.h>
#include <recursive_permute.h>
#include <optionhandler.h>
#include <csvparser.h>

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
  
  void search_grid( optlist& opts,
		    parampoint_generator& pg,
		    filesender& fs );

  void search_grid( const std::vector<std::string>& varnames,
		    const std::vector<double>& mins,
		    const std::vector<double>& maxes,
		    const std::vector<double>& steps,
		    parampoint_generator& pg,
		    filesender& fs );

  void parseopts( optlist& opts);

  void parse_gridfile( const std::string& gridfname );
  
};




void search_grid( optlist& opts,
		  parampoint_generator& pg,
		  filesender& fs
		  );
