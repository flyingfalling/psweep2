#pragma once

#include <string_manip.h>

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

std::string remove_prefix( const std::string& str, const std::string& prefix )
{
  if( string_prefix_match( str, prefix ) == true ) 
    {
      size_t last = str.find_last_of( prefix );
      std::string noprefix = str.substring( last+1, std::string.size() );
      return noprefix;
    }
  else
    {
      fprintf(stderr, "ERROR in remove_prefix: requested prefix to remove [%s] is not a prefix of me [%s]!!\n", prefix.c_str(), str.c_str());
      exit(1);
    }
}

bool string_prefix_match( const std::string& orig, const std::string& prefix )
{
  if( orig.compare(0, prefix.size(), prefix)==0 )
    {
      return true;
    }
  return false;
}

//Only expands internal guys by flag, i.e. doesnt make new length of arglist.
std::vector< std::vector< std::string > > parse_internally_by_flag( const std::vector< std::vector< std::string> >& argslist, const std::string& flag )
{
  std::vector<std::vector<std::string> > retval(argslist.size() );
  for(size_t y=0; y<argslist.size(); ++y)
    {
      arg = argslist[y];
      std::vector<std::string> newvect;
      
      for(size_t s=0; s<arg.size(); ++s)
	{
	  std::vector<std::string> newlist = tokenize_string( arg[s], flag );
	  //newvect.push_back( newlist );
	  newvect.insert( newvect.end(), newlist.begin(), newlist.end() );
	}

      retval.push_back( newvect );
    }
  return retval;
}


std::vector< std::vector< std::string > > parse_by_flag( const std::vector< std::vector< std::string> >& argslist, const std::string& flag, std::vector<std::string>& extras )
{

  //REV: TODO:: REMOVE THE FLAG!!! Or -- will catch -
  
  //std::vector<std::string> extras;
  std::vector< std::vector< std::string> > retval;

  std::vector<std::string> tmp;
  
  for(size_t y=0; y<argslist.size(); ++y)
    {
      std::vector<std::string> args = argslist[y];
      for(size_t x=0; x<args.size(); ++x)
	{
	  if( string_prefix_match( args[x], flag ) == true )
	    {
	      if( tmp.size() != 0 )
		{
		  retval.push_back( remove_prefix( tmp, flag ) );
		  tmp.clear();
		}
	      tmp.push_back( args[x] );
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
      
    } //end for all args lists...
  return retval;
}
						       

struct argparser
{
  //std::vector< std::string > seps = {" ", "-", "--"}; //initially will separate and remove these...?
  std::string sep1=" ";
  std::string sep2="--"; //for "short" options? What about "long" options? Why would I separate by -- or -?
  std::string sep3="-";
  std::string sep4="=";

  std::vector<parsedoption> parsedopts;
  std::vector<std::string> extras;
  
  //REV: First parse should already have already separated out all spaces...
  //It already separated them by whitespace for me.
  //So, now I need to parse "chunked" guys, and also parse out the vector into those that begin with - or --...etc.
  std::vector<parsedoption> doparse( const std::vector<std::string>& args )
  {
    std::vector<parsedoption> retval;
    
    std::vector<std::vector<std::string> > args2d( 1, args );

    //sep1 is already done by the argc/argv parser thing of caller?
    std::vector< std::vector<std::string> > parsed = parse_by_flag( args2d, sep2, extras );
    std::vector< std::vector<std::string> > parsed = parse_by_flag( parsed, sep3, extras );
    std::vector< std::vector<std::string> > parsed = parse_internally_by_flag( parsed, sep4 );
    for( size_t x=0; x<parsed.size(); ++x )
      {
	retval.push_back( parsedoption( parsed[x] ) );
      }

    return retval;
  }

  
  
  argparser( const std::vector<std::string>& args )
  {
    parsedopts = doparse( args );
  }
  
};


//varlist to modify (options will only ever modify varlists?!?!?! Which will then be processed elsewhere, fuck that
//noise lol...just parse the stuff directly. I.e. some guys add to varlist, some don't?), vector of strings (arglist of option)
//Some guys for example turn on/off debug modes? I.e. set global variables (?) that propogate everywhere? They are args to certain
//functions/structs basically...they are added if present, otherwise default...I guess.
typedef std::function<void( varlist&, const std::vector<std::string>& )>   handlefunct;

//Now user can "apply" his "known" registered functs to each parsedopt
//Basically have him build these "known" guys to do it?
struct optionrunner
{
  std::vector<handlefunct> functs;
  
  void register_funct( const std::string& arg, handlefunct& funct )
  {
    
  }
  
  std::vector<size_t> get_matches( const std::string& argname, std::vector< parsedoptions >& optns )
  {
    std::vector<size_t> locs;
    for(size_t x=0; x<optns.size(); ++x)
      {
	if( optns[x].arg.compare( argname ) )
	  {
	    locs.push_back( x );
	  }
      }
    return locs;
  }

  //REV: Automatically "do" things with args that we require, and keep them around for other things?
  
  
  
  apply_to_arg( const std::string& argname, const std::vector< parsedoptions >& optns, varlist& vl )
  {
    
    //If there is more than 1 example...then bam. Exit? No, in some cases we "add" to a variable in varlist etc.
    std::vector<size_t> locs = get_matches( argname, optns );
    for(size_t x=0; x<locs.size(); ++x)
      {
	opth.apply_funct( optns );
      }
    
    //Match any in optns that match argname
    //For each, check if it has "right" number of args.
    //If so, apply my function to each of those?
    //Whole goal is to e.g. set certain predetermined variables etc., right? Kind of like "get" type guys. So, basically fills up a varlist? Wow...
    //And then user parses it afterwards? If it is just filling a varlist, what the fuck do we do? For example, varlist might have multi-length
    //vector of filenames for config filenames or some shit like that that need to be handled by the blah?
    //Just put everything into a varlist...? And then user can access it like that?
    //But then options will have same name as varlist name, which may not be what we want...
    //So we can set it up to um, parse it and add all args? user will get args as he goes?
    //Result is something? How do we do parses to doubles etc.?
    //Like, user programs it to set some "global" variables. Ah, I like that. And how they are used is used at blah time. For example, we use it
    //to modify a varlist if we want...
    //We want to either ADD vars or SET vars? Gets kind of annoying?
    //What kind of things do we want to do?
    //At anyrate, this gives us "parsed options" ;)
    //Fine, This is basically varlist haha... can figure shit out from there? How do I want to use it?
    //Well, basically I want to convert it to the global varlist so that user can access these variables/values freely.
    //For example to use them to set valuse for individual models, or pass to sweeps.
    //I.e. set a way to convert them. Directly convert, or process to a specific variable in a varlist, etc.
    
    //Yappa, varlist ni siyou...
  }
};


struct opthandler
{
  std::vector<std::string> registered_opts;
  
  
  
};
