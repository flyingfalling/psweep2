

//REV: header file for searcher, which provides various search functions
//User will call one of these, passing the appropriate inputs,
//and it will automatically construct everything and run the sweep I guess....


//REV: TOdo, need to make it so that RESTART will automatically find/choose correct algo. In other words, dont need to tell it type for it to load...
//So, we need to store the algo in the state file (duh..).



//takes user options and runs corresponding search funct?

//"Format" is required...for each different search, what kind of info it needs to parameterize it. I guess just parameterize it with a global varlist.
//REV: OK how to parameterize it with some global varlist?
//Do like PARAMNAME, PARAMMIN, PARAMMAX? How to name it? In individual varlists?
//Use varlists NAMES to find it, inside what? A hierarchical? No...just use a varlist structure... I.e. named list of varlists...Ugh.
//This seems "best", but then user needs to be careful to construct his scripts/etc. as max/min/etc. Or give a better way to read varlists,
//like VAL MIN MAX STEP etc.? So, basically ways of taking 2-d values? Need to take "column" headers, etc. Need it for simulation anyway (read mii-sans
//data?). OK, I guess I can do this. So, now we have some kind of 2d var list structure, where we tell it the variable (row#?) and the column name
// (e.g. var name vs val vs etc.?)
//Effectively we have a 2-column thing now. Make it even more general? We want to eventually have more "abstract" i.e. jagged ones, as we talked about
//before...i.e. nested information.
//I.e. varlists are arbitrary? but then how do we hold arrays? In varlist, it's just an arbitrary list until some endthing? How do I know where it
//ends in read-in? That is the problem?
//Need a way to have arbitrary length array stored somewhere? Only up to 3D array? Either 1D of array type (i.e. all guys are arrays), or 2D of single
//value types...? Need a way to arbitrarily "flatten" matrices of arbitrary length. Ways of storing in memory. Of course, we can but reading/writing
//will not be compressed/efficient. If it includes strings, that will make it only more complex.
//But in the eventuality that I want to run experiments/store it on NSIM side, need a way to do it, right? I don't want to have to repeat every single
//variable every single time. E.g., better to have a way to compress it uniquely. But representation will change if I change number of guys.
//E.g.

//We can rearrange this in some way to make it "hierarchical" based on most efficient graph/tree search... i.e. might be more efficient to "list" it
//by timepoints (var4) if there are more (fewer?) of those. But then how to arbitrarily do it without actually listing out every single one, e.g.
//tell it what each "location" is and then store it that way. I.e. don't write "neuron#" or "time point" each time., just do it? HDF5 might be the
//most elegant way of doing this? But how do I go through and analyse that? E.g. electrode # etc. Doing e.g. EL1 and EL2, or some arbitrary number of
//electrodes (and their positions). How to list those out then? Like, how to specify "experiment" to do most efficiently?
//I.e. TIMING, LOCATION, STRENGTH of each stimulation or something like that. Need a way to parameterize that in an N dimensional space. Timing of
//each is a dimension? Similar to eye movement/visual stimulation analysis. Too many dimensions, and not clear how to orient them to make it most
//informative. Like, I want to basically "list" parameters in some hierarhical way, and finally have e.g. TIMEPOINT and NEURONNUMBER known without
//listing them. Need a "organization" file (how to interpret data file), and then "data file" itself. Same file is obviously best. HDF5?
//Anyway, in this case, do named varlists obviously, easiest. Wasteful. Want to have 2d table things to read too though. Use HDF5...exists, and
//efficient. PRoblem is that stored information might have arbitrary lengths (i.e. strings), or might be doubles or whatever. But to process what
//user is doing, he will make it via some CMD program? Some GUI?

// VAR1 VAR2 VAR3 VAR4 THING
//  1    1     1    1    2.5
//  1    1     1    2    3.5
//  1    1     1    3    95.9
//  1    1     2    1    2.5
//  1    1     2    2    3.5

//For now just pass MIN MAX etc.. Yea, way to handle 2d after all. It contains..? How about going from HDF5 to?
//Hm, this is kind of nasty...because when I "extracted" each column, I would need to appropriately have the return function return
//the kind, or cast it manually each time -_-

#pragma once

#include <filesender.h>

#include <optionhandler.h>

#include <iterator>     // ostream_operator
#include <boost/tokenizer.hpp>
#include <math.h>

#include <utility_functs.h>
#include <csvparser.h>

#include <gridsearch.h>
#include <dream_abc.h>
#include <dream_abc_z.h>

#include <commontypes.h>

//REV: Where to set parameters?
//We can do it "above" to handle options, but that makes it a pain. Assume all variables are already set and I just "check" for them?
//Inside the algorithm itself?
//Another option is to have a set "config" file for running the sweep. For example "load" etc.? I don't want to specify (on command line?)
//The arguments each time I start a run? Or do I? I want it to be "stored". It is stored, in the file itself of course.

//Make an error if user "doubly defines" things. E.g. allow him to set a "config" file for the search algorithm, or a "default", but then also add
//cmd line arguments. Give a warning (error/exit?) if cmnd line overrides confnig file, or config overrides config,e tc.


//REV: This is the thing that is compiled into the library! I.e. globally accessible is this!
//I need to add everything to CPP files so that I can separately compile them...and do it faster...
//Let's do it now I guess? They are all including each other though, which causes some problems.

struct searcher
{

  //Has a fake memsystem in there too hahaha...
  fake_system fakesys;

  parampoint_generator pg;

  //MY OPTIONS
  std::string _scriptfname="__ERROR_NOSCRIPTFNAME";
  std::string _searchtype="__ERROR_NOSEARCHTYPE";
  std::string _mydir="__ERROR_NOMYDIR";
  std::string _runtag="scratch";
  bool _writefiles=false;

  searcher( );
  

  void register_funct( const std::string& name, const fake_system_funct_t& funct);
  
  void run_search( optlist& opts );
  

  //REV: Faster to specify some struct to handle all options, this way it can easily know how many args it wants, and what are usage things so that
  //they can be printed...

  void preparseopts( optlist& opts );
  
  
  void parseopts( optlist& opts );

  //varlist will contain required um, data files I guess?
  void run_search( const std::string& searchtype, const std::string& scriptfname,
		   const std::string& mydir, optlist& opts,
		   filesender& fs,
		   const bool& writefiles=false );
  

  void doexit( filesender* myfs );
 



  /////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////// OLD RUN SEARCH
  
  //varlist will contain required um, data files I guess?
  void run_search( const std::string& searchtype, const std::string& scriptfname, const std::string& mydir, /*const*/ varlist<std::string>& params, const bool& writefiles=false );
  

}; //end struct searcher
