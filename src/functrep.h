
#pragma once

#include <hierarchical_varlist.h>


typedef (variable<std::string>)  myvar_t;
//typedef (std::function< myvar_t(const client::STMNT&, std::vector< hierarchical_varlist >&, const std::vector< size_t >&) >) functtype;
typedef (std::function< myvar_t( std::vector< myvar_t >, std::vector< hierarchical_varlist >&, const std::vector< size_t >&) >) functtype;


myvar_t XXX;


//What is the goal here? The goal is to build a representation so that I can actually execute each simple function representation, i.e.
//it takes the statement and converts it to a variable. Up until now it was as std::string.
//It also looks up the first tag thing as the function name and gets the function.
//For the second, it executes the function. For most functions, it is just a variable get (i.e. VARNAME()). If it has arguments, it executes it
//as a functrep to get the variable out? Note, that will only be for SETVAR things, where I am setting it of course ;)
//It will ALWAYS only be SETVAR anyway! So have two possible settings? At any rate, it builds a function...that has the argument as a variable
//OR as a statement? If they're separate...I need to choose appropriate representation for each.

//Note we start with effectively a list of STMNT. We can first extract the guys to arguments of statements. And then those can all be done
//in one go? Except SETVAR. Have a branch there to make things easier heh.
//Or just use one struct but include a possibly "unused" branch for setvar in case it is a more complex statement than a variable read.


//make recursive...at only one level? I could actually make this more recursive...oh crap...instead of unrolling the statements, I could just make this
//recursive? In which case it needs a "list" of functreps? Ugh. Has some desired # of arguments. Ugh...




//Uhoh, I'm confused. Eventually, we want just a string. But for arguments we want to leave variables?
//We execute each in turn, which may have side effects such as setting variables etc.
//Remember eventual goal is to "run" this script for each time/each pitem, but variables may be changed. And, this may do things like set
//some variables (locally), read other variables, etc.
//In the end, we set CMD? Or do we set a variable that appends many guys together.

struct functsig
{
  std::string mytag;
  size_t nargs;
  functtype funct;
  
functsig( const std::string& _tag, const size_t& argsize, const functtype& f )
:
  mytag(_tag),
    nargs( argsize ),
    funct( f )
  {
    //Do nothing.
  }
};


//User defines functs here, and "builds" functsigs...


struct functlist
{
  std::vector< functsig > sigs;
  
  functtype findfunct( client::STMNT& s )
  {
    for(size_t x=0; x<sigs.size(); ++x)
      {
	//Match tag to sigs[x]
	if( s.TAG == sigs[x].mytag )
	  {
	    if( s.ARGS.size() != sigs[x].nargs )
	      {
		fprintf(stderr, "ERROR: in findfunct, found function matching tag [%s], however #args of STMNT (=%ld) != #args expected by function signature (=%ld)\n", s.TAG.c_str(), s.ARGS.size(), sigs[x].nargs );
		exit(1);
	      }
	    else
	      {
		return sigs[x].funct;
	      }
	  }
      }
    fprintf(stderr, "REV: ERROR: findfunct could not find function with tag we are searching for [%s] (searched through [%ld] function signatures)\n", s.TAG.c_str(), sigs.size() );
    exit(1);
    
  }

  functtype findfunct( client::LEAF_STMNT& s )
  {
    //as above...
  }
  
  functlist()
  {
    //add sigs from user?
  }
  
  
};


struct functrep
{
  functtype ft;

  //This will read tag and select ft appropriately.
  std::vector< functrep > args;
  
  //REV: crap, it possibly will take a LEAF_STMNT too? Just use another one.
  functrep( const client::STMNT& s, const functlist& fl )
  {
    //Functs to not have types, arguments do not have types. Only # of arguments orz.
    //REV: this will check # of arguments etc. It will use s.TAG to get function, and all that.
    ft = fl.findfunct( s );
    for( size_t nesti=0; nesti<s.ARGS.size(); ++nesti )
      {
	args.push_back( functrep( s.ARGS[nesti] ) );
      }
  }

  //Constructor that takes leaf stmnt instead. Whatever.
  functrep( const client::LEAF_STMNT& s, const functlist& fl )
  {
    ft = fl.findfunct( s );
    for( size_t nesti=0; nesti<s.ARGS.size(); ++nesti )
      {
	args.push_back( functrep( s.ARGS[nesti] ) );
      }
  }
  
  //std::vector< hierarchical_varlist >&, const std::vector< size_t >&)
  //Executes and makes the variable output of this guy.
  //Note it needs access to the hierarchical varlists to use.
  //Wait, it takes a list of variables now, at the lowest level I guess.
  myvar_t execute( std::vector< hierarchical_varlist >& hvl, const std::vector< size_t >& hvi )
  {
    std::vector< myvar_t > mv;
    for( size_t x=0; x<args.size(); ++x ) //This is the nested STMNT. I need to convert those to VARIABLES (via user functions) to actually do things.
      //But, what about if a function is e.g. "construct variable from args" type thing, that constructs an array or composes one? Or what about one
      //that builds a large string? User function needs to e.g. handle parsing to integer, blah. We do that but then still return to myvar in the end.
      {
	//REV: WTF? Wait, I need user to specify how to individually handle each thing into a variable? No, into actual STRING at the end!
	//Each args[x] will have its own set of blah?
	//Note, none of these guys should update actual things?
	//Note some variables may return empty things (if they're empty? No, they won't be in here...)
	mv.push_back( args[x].execute( hvl, hvi ) );
      }
    //execute ft and recursively do so.
    myvar_t ret = ft( mv, hvl, hvi ); //however, this might return an "empty" variable if it just has a side-effect...

    return ret; //This may return e.g. CMD. Or final side effect may be to set CMD to it? Some special lowest-level variable named CMD?
  }
  
};
