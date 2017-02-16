#pragma once

#include <string_manip.h>
#include <commontypes.h>
#include <sstream>
#include <cstring>
#include <string>
#include <assert.h>

//REV: header for optionhandler, a wrapper or my code written to handle
//user-passed CMD line options, and push them appropriately into a
//varlist etc.

//How do I want to specify it? Obviously calling MPIRUN will cause some *other* options, but I can't reorganize those programmatically.
//The -n etc. is consumed by the mpirun... so I need to add them to my list. I get that with get_n_ranks etc. Assuming they are homogenous,
//I can also estimate #threads for openMPI based on master MPI rank?

//I want to somehow (arbitrarily) specify model variables. For example, scmodel:varval=X

//Need to assume that the thing doesn't have another meaning. At any rate, do it yay ;)
//And then for when I access specific variables, I can get it and have it do it manually?
//Namedlist : X : access.
//It will try to get from child, but also from parent. So, I can specify a specific (single) named list (startpoint) for me?

//And of course I want to pass some arguments like dreamabcz:nchains=1, etc. What if there is a space in between? Allow it?

//-dreamabcz : nchains = 1 or something? If we allow that it's difficult... Or can we do like
//-dreamabcz: nchains=1 blahs



//Basically takes all args, parses them (by spaces). Then parses them by acceptable breaks (e.g. =).
//Then finds "acceptable" argument names (e.g. )
//User accesses "args" of args by number...(?). Allow "nested" model guys...? What's the point haha. E.g. all args from here
//refer to model X, etc.

//Anyway, first give user program the arg parser thing, that gives you the arg and list. Then user can tell which functs to apply to it haha.
//I like that better.

struct parsedoption;

struct parsedoption
{
  std::string arg;
  std::vector< std::string > parsedargs;

  void enumerate();
  
  
  parsedoption( const std::vector<std::string>& toparse );
  

  parsedoption();
  

  
  size_t get_nargs();
  
  std::string get_arg( const size_t& argn );
 
  float64_t argn_as_float64( const size_t& argn );
  
  float64_t argn_as_int64( const size_t& argn );
  

  //I.e. runs on it and gets any unused guys, and gives them back as "orphan arguments"
  //Need a way to "undo" any extra argument added to the end of a guy by accident?
  
  //Allow me to "name" certain values, which is how I access them.
  //After that, it is just getting any of them. E.g. to iterate through
  //setting config files or something...
};

bool string_prefix_match( const std::string& orig, const std::string& prefix );

bool any_string_prefix_match( const std::string& orig, const std::vector<std::string>& prefixes, size_t& matchedidx );

std::string remove_prefix( const std::string& str, const std::string& prefix );


//Only expands internal guys by flag, i.e. doesnt make new length of arglist.
std::vector< std::vector< std::string > > parse_internally_by_flag( const std::vector< std::vector< std::string> >& argslist, const std::string& flag );


std::vector< std::vector< std::string > > parse_by_flag( const std::vector< std::vector< std::string> >& argslist,
							 const std::vector<std::string>& flags,
							 std::vector<std::string>& extras );
						       

struct optlist
{

  std::vector<parsedoption> parsedopts;
  std::vector<std::string> extras;

  
  //std::vector< std::string > seps = {" ", "-", "--"}; //initially will separate and remove these...?
  std::string sep1=" ";
  std::string sep2="--"; //for "short" options? What about "long" options? Why would I separate by -- or -?
  std::string sep3="-";
  std::string sep4="=";

  parsedoption get_opt( const std::string& optname );
  
  void enumerateparsed();
 

  void enumerateextras();
  
  
  
  //REV: First parse should already have already separated out all spaces...
  //It already separated them by whitespace for me.
  //So, now I need to parse "chunked" guys, and also parse out the vector into those that begin with - or --...etc.
  std::vector<parsedoption> doparse( const std::vector<std::string>& args );
  
  std::vector< std::string > strvect_from_charptr_array( const int& argc,  char* argv[] );
 
  
  //optlist( const std::vector<std::string>& args )
  optlist( const int& argc, char* argv[] );
  

  //Returns vect of vect of the args of each one?
  std::vector< std::vector< std::string> > get_opt_args( const std::string& optname );

  std::vector<size_t> get_matches( const std::string& argname, std::vector< parsedoption >& optns );
};


//REV: I need a "consume" option type thing. e.g. if it only takes a certain # of opts and I want to end with a thing, it will feed back the last
//guy to "extras"
