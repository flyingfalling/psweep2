//REV: problem is variables themselves have a "TAG" (name) and a "Value". Tag is always a string! Ah.


#pragma once

#include <hierarchical_varlist.h>
#include <commontypes.h>


#include <psweep_cmd_script_parser.h>

typedef variable<std::string>  myvar_t;
//typedef (std::function< myvar_t(const client::STMNT&, std::vector< hierarchical_varlist >&, const std::vector< size_t >&) >) functtype;
typedef std::function< myvar_t( std::vector< myvar_t >&, std::vector< hierarchical_varlist<std::string> >&, const std::vector< size_t >&) > functtype;




struct functsig
{
  std::string mytag;
  size_t nargs;
  functtype funct;

  functsig();
  
  functsig( const std::string& _tag, const size_t& argsize, const functtype& f );
};

#define ADDFUNCT(numarg, fname)						\
  {									\
    functtype fa = fname;						\
    functsig varname(std::string( #fname ), (size_t)numarg, fa );	\
    sigs.push_back( varname );						\
  }



//REV: Will this work? The HVL in there are not going to be carried over...need to use ptrs orz.
#define FUNCTDEF( fname ) myvar_t fname( std::vector< myvar_t >& args, std::vector< hierarchical_varlist<std::string> >& hvl, const std::vector< size_t >& hvi )

//typedef  myvar_t( std::vector< myvar_t > a, std::vector< hierarchical_varlist<std::string> >& b, const std::vector< size_t >& c) functdefn;

//typedef myvar_t 

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
//Crap, LEAF_STMNT can have zero arguments. In which case, it's either a variable read (function), OR it's a literal. We could differentiate based
//on a starter special character (e.g. $).



//OK this might include its directory. Yea, we need to add DIR as specific var ;)
FUNCTDEF( GET_PREV_VAR );

FUNCTDEF( GET_PREV_DIR );

FUNCTDEF( ITER_ARRAY_WITH_FLAG );


FUNCTDEF( GET_OUTPUT_FILES );

FUNCTDEF( SET_INPUT_FILE );

FUNCTDEF( GET_INPUT_FILE );


FUNCTDEF( VARLIST_TO_FILE );

FUNCTDEF( ADD_SUCCESS_FILE );

FUNCTDEF( ADD_REQUIRED_FILE );

FUNCTDEF( ADD_CMD_ITEM );

FUNCTDEF( ADD_CMD_ITEMS );

FUNCTDEF( CAT_ARRAY_TO_STR );

FUNCTDEF( ADD_OUTPUT_FILE );

FUNCTDEF( SETVAR );



FUNCTDEF( IDENTITY );

FUNCTDEF( GET_MY_DIR );

FUNCTDEF( READVAR );

FUNCTDEF( CAT );

struct functlist
{
  std::vector< functsig > sigs;
  
  functsig findfunct( /*const*/ client::STMNT& s );
    

  
  
  functsig findfunct( /*const*/ client::LEAF_STMNT& s );
  
  functlist();
 
};

struct functrep
{
  //functtype ft;

  functsig fs; //keep the whole functsig around to make it easier to know which it is.
  
  //This will read tag and select ft appropriately.
  std::vector< functrep > args;

  
  
  //A functrep could be JUST a simple VARIABLE. Need to always parse down to a literal.
  //Problem is for function IDENTITY only, we just return here.
  
  //REV: crap, it possibly will take a LEAF_STMNT too? Just use another one.
  functrep( client::STMNT& s, functlist& fl );

  //Constructor that takes leaf stmnt instead. Whatever.
  functrep( client::LEAF_STMNT& s, functlist& fl );
 
  functrep( functsig f );
  
  void enumerate();
  
  //std::vector< hierarchical_varlist >&, const std::vector< size_t >&)
  //Executes and makes the variable output of this guy.
  //Note it needs access to the hierarchical varlists to use.
  //Wait, it takes a list of variables now, at the lowest level I guess.

  //REV: we do not actually resolve to VARIABLE types until now (execution time)
  myvar_t execute( std::vector< hierarchical_varlist<std::string> >& hvl,  const std::vector< size_t >& hvi );
  
};
