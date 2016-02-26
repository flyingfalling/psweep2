#pragma once

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

struct argparser
{
  //std::vector< std::string > seps = {" ", "-", "--"}; //initially will separate and remove these...?
  std::string sep1=" ";
  std::string sep2="--"; //for "short" options? What about "long" options? Why would I separate by -- or -?
  std::string sep3="-";
  std::string sep4="=";
  
  //In other words, I assume that all guys are "known". Problem is that not all guys necessarily need to be "known" to me,
  //some other (user) program may receive some as pass-through... (only unknown ones?)
  //They're available to "consume" I guess? Specific ones for certain models might be set in some way so they aren't stolen by others?
};


struct opthandler
{
  std::vector<std::string> registered_opts;

  
  
};
