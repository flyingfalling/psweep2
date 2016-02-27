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


struct parsedoption
{
  std::string arg;
  std::vector< std::string > parsedargs;

  void enumerate()
  {
    fprintf(stdout, "NAME: [%s]\n", arg.c_str() );
    for(size_t x=0; x<parsedargs.size(); ++x)
      {
	fprintf(stdout, "\t[%s]\n", parsedargs[x].c_str() );
      }
    return;
  }
  
  parsedoption( const std::vector<std::string>& toparse )
  {
    if(toparse.size() < 1)
      {
	fprintf(stderr, "ERROR, trying to make a parsedoption with an empty vector of strings!\n");
	exit(1);
      }
    else
      {
	arg = toparse[0];
	parsedargs = std::vector<std::string>( toparse.begin()+1, toparse.end() ); //copy second to last arg...
	//This will work if size == 1 right (i.e. begin -> end will iterate zero times, making an empty vect)
      }
  }

  parsedoption()
  {
    arg="__SUPERERRORARG";
  }

  
  size_t get_nargs()
  {
    return parsedargs.size();
  }

  std::string get_arg( const size_t& argn )
  {
    if(argn >= parsedargs.size() )
      {
	fprintf(stderr, "ERROR in get_arg: requested [%ld]th arg > length of parsed args [%ld]\n", argn, parsedargs.size() );
	exit(1);
      }
    else
      {
	return parsedargs[argn];
      }
  }

  float64_t argn_as_float64( const size_t& argn )
  {
    std::string argval = get_arg( argn );

    //Try parsing it, give an error if not?
    //std::stod() etc...works too...meh.
    //sscanf, etc...
    std::stringstream ss( argval );

    float64_t retval;
    
    ss >> retval;
    
    if( !ss )
      {
	fprintf(stderr, "ERROR in argn_as_float64: parsed arg was not a float64, or there was no argument, etc.\n");
	exit(1);
      }
    else
      {
	return retval;
      }
  }

  float64_t argn_as_int64( const size_t& argn )
  {
    std::string argval = get_arg( argn );

    //Try parsing it, give an error if not?
    std::stringstream ss( argval );

    int64_t retval;
    
    ss >> retval;
    
    if( !ss )
      {
	fprintf(stderr, "ERROR in argn_as_int64: parsed arg was not a int64, or there was no argument, etc.\n");
	exit(1);
      }
    else
      {
	return retval;
      }
  }

  //I.e. runs on it and gets any unused guys, and gives them back as "orphan arguments"
  //Need a way to "undo" any extra argument added to the end of a guy by accident?
  
  //Allow me to "name" certain values, which is how I access them.
  //After that, it is just getting any of them. E.g. to iterate through
  //setting config files or something...
};

bool string_prefix_match( const std::string& orig, const std::string& prefix )
{
  if( orig.compare(0, prefix.size(), prefix)==0 )
    {
      return true;
    }
  return false;
}

bool any_string_prefix_match( const std::string& orig, const std::vector<std::string>& prefixes, size_t& matchedidx )
{
  bool match=false;
  for(size_t x=0; x<prefixes.size(); ++x)
    {
      if( string_prefix_match ( orig, prefixes[x] ) == true )
	{
	  match = true;
	  matchedidx = x;
	  return match; //hopefully it doesn't match more than one? Need to make sure to "sort" them...
	}
    }
  return match;
}

std::string remove_prefix( const std::string& str, const std::string& prefix )
{
  if( string_prefix_match( str, prefix ) == true ) 
    {
      size_t last = str.find_last_of( prefix );
      std::string noprefix = str.substr( last+1, str.size() );
      return noprefix;
    }
  else
    {
      fprintf(stderr, "ERROR in remove_prefix: requested prefix to remove [%s] is not a prefix of me [%s]!!\n", prefix.c_str(), str.c_str());
      exit(1);
    }
}


//Only expands internal guys by flag, i.e. doesnt make new length of arglist.
std::vector< std::vector< std::string > > parse_internally_by_flag( const std::vector< std::vector< std::string> >& argslist, const std::string& flag )
{
  std::vector<std::vector<std::string> > retval( argslist.size() );
  for(size_t y=0; y<argslist.size(); ++y)
    {
      std::vector<std::string> arg = argslist[y];
      std::vector<std::string> newvect;
      
      for(size_t s=0; s<arg.size(); ++s)
	{
	  std::vector<std::string> newlist = tokenize_string( arg[s], flag );
	  //fprintf(stdout, "Successfully tokenized [%s]...to newlist of [%ld]\n", arg[s].c_str(), newlist.size());
	  /*for(size_t x=0; x<newlist.size(); ++x)
	    {
	      fprintf(stdout, "Tokenized: [%s]\n", newlist[x].c_str());
	      }*/
	  //newvect.push_back( newlist );
	  
	  newvect.insert( newvect.end(), newlist.begin(), newlist.end() );
	}
      retval[y] = newvect;
    }
  assert( retval.size() == argslist.size() );
  return retval;
}


std::vector< std::vector< std::string > > parse_by_flag( const std::vector< std::vector< std::string> >& argslist,
							 const std::vector<std::string>& flags,
							 std::vector<std::string>& extras )
{
  
  //fprintf(stdout, "Parsing by flag [%s]\n", flag.c_str() );
  //REV: TODO:: REMOVE THE FLAG!!! Or -- will catch -
  
  //std::vector<std::string> extras;
  std::vector< std::vector< std::string> > retval;

  std::vector<std::string> tmp;
  
  for(size_t y=0; y<argslist.size(); ++y)
    {
      std::vector<std::string> args = argslist[y];
      for(size_t x=0; x<args.size(); ++x)
	{
	  //fprintf(stdout, "Checking [%s]\n", args[x].c_str() );
	  //for(size_t f=0; f<flags.size(); ++f)
	  size_t matchedprefixidx=0;
	  if( any_string_prefix_match( args[x], flags, matchedprefixidx ) == true )
	    {
	      //fprintf(stdout, "Matched a prefix flag! ([%s])\n", args[x].c_str());
	      if( tmp.size() != 0 ) //We've "found" a new one...
		{
		  //fprintf(stdout, "Pushed back vector, i.e. new prefix flag...\n");
		  retval.push_back( tmp );
		  tmp.clear();
		}
	      tmp.push_back( remove_prefix( args[x], flags[ matchedprefixidx ] ) );
	    }
	  else
	    {
	      if( tmp.size() == 0 ) //if still no seps, i.e. extras
		{
		  extras.push_back( args[x] );
		}
	      else
		{
		  tmp.push_back( args[x] );
		}
	    }
	
	} //end for each args in this list
      //REV: Add the last one ;)
      if( tmp.size() != 0 )
	{
	  retval.push_back(tmp);
	}
    } //end for all args lists...
  //fprintf(stdout, "Returning new matrix with [%ld] rows\n", retval.size() );
  return retval;
}
						       

struct optlist
{
  //std::vector< std::string > seps = {" ", "-", "--"}; //initially will separate and remove these...?
  std::string sep1=" ";
  std::string sep2="--"; //for "short" options? What about "long" options? Why would I separate by -- or -?
  std::string sep3="-";
  std::string sep4="=";

  void enumerateparsed()
  {
    fprintf(stdout, "PARSED OPTIONS LIST:\n");
    for(size_t x=0; x<parsedopts.size(); ++x)
      {
	fprintf(stdout, "Parsed option #[%ld] of [%ld]\n", x, parsedopts.size() );
	parsedopts[x].enumerate();
	fprintf(stdout, "\n");
      }
  }

  void enumerateextras()
  {
    fprintf(stdout, "EXTRA OPTIONS LIST:\n");
    for(size_t x=0; x<extras.size(); ++x)
      {
	fprintf(stdout, "[%s]\n", extras[x].c_str());
      }
  }
  
  std::vector<parsedoption> parsedopts;
  std::vector<std::string> extras;
  
  //REV: First parse should already have already separated out all spaces...
  //It already separated them by whitespace for me.
  //So, now I need to parse "chunked" guys, and also parse out the vector into those that begin with - or --...etc.
  std::vector<parsedoption> doparse( const std::vector<std::string>& args )
  {
    fprintf(stdout, "Parsing options! Arglist has [%ld] elements:\n[ ", args.size());
    for(size_t x=0; x<args.size(); ++x)
      {
	fprintf( stdout, "[%s] ", args[x].c_str() );
      }
    fprintf(stdout, "]\n\n");
    
    std::vector<parsedoption> retval;
    
    std::vector<std::vector<std::string> > args2d( 1, args );
    
    //sep1 is already done by the argc/argv parser thing of caller?
    std::vector< std::vector<std::string> > parsed;

    std::vector<std::string> flags = { sep2, sep3 };

    parsed = parse_by_flag( args2d, flags, extras );
    //parsed = parse_by_flag( parsed, sep3, extras );
    //for(size_t a=0; a<parsed.size(); ++a)
    //  {
    //fprintf(stdout, "Vect sizes: [%ld]\n", parsed[a].size());
    //  }
    parsed = parse_internally_by_flag( parsed, sep4 );
    //for(size_t a=0; a<parsed.size(); ++a)
    //  {
    //	fprintf(stdout, "Vect sizes: [%ld]\n", parsed[a].size());
    //  }
    //fprintf(stdout, "After parse, we have [%ld] rows in our vector list!\n", parsed.size() );
    for( size_t x=0; x<parsed.size(); ++x )
      {
	//fprintf(stdout, "Trying to pushback to parsedoption thing...\n");
	retval.push_back( parsedoption( parsed[x] ) );
      }

    return retval;
  }

  std::vector< std::string > strvect_from_charptr_array( const int& argc,  char* argv[] )
  {
    std::vector<std::string> ret;
    if( argc > 1 )
      {
	for(size_t x=1; x<argc; ++x)
	  {
	    ret.push_back( std::string( argv[x] ) );
	  }
      }
    return ret;
  }
  
  //optlist( const std::vector<std::string>& args )
  optlist( const int& argc, char* argv[] )
  {
    std::vector<std::string> args = strvect_from_charptr_array( argc, argv );
    parsedopts = doparse( args );
    enumerateparsed();
    enumerateextras();
  }

  //Returns vect of vect of the args of each one?
  std::vector< std::vector< std::string> > get_opt_args( const std::string& optname )
  {
    std::vector<size_t> locs = get_matches( optname, parsedopts );
    std::vector< std::vector< std::string> > ret (locs.size() );
    for(size_t x=0; x<locs.size(); ++x)
      {
	ret[x] = parsedopts[ locs[x] ].parsedargs;
      }

    return ret;
  }

  std::vector<size_t> get_matches( const std::string& argname, std::vector< parsedoption >& optns )
  {
    std::vector<size_t> locs;
    for(size_t x=0; x<optns.size(); ++x)
      {
	if( optns[x].arg.compare( argname ) == 0 )
	  {
	    locs.push_back( x );
	  }
      }
    return locs;
  }
};


//REV: I need a "consume" option type thing. e.g. if it only takes a certain # of opts and I want to end with a thing, it will feed back the last
//guy to "extras"
